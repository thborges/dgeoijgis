package peer

import (
	"bufio"
	"fmt"
	"log"
	"net"
	"os"
	"strings"
	"sync"
	"time"
)

var (
	peers_count = byte(0)
)

type ActivePeers struct {
	group        string
	peers        map[string]byte
	rwlock_peers sync.RWMutex
	client       bool
	serverport   string
	heart_beat_f func(stop bool)
}

func (ap *ActivePeers) heart_beat_nomulticast(stop bool) {
	name, err := os.Hostname()
	if err != nil {
		log.Fatal(err)
	}

	cache := map[string]*net.UDPConn{}

	for {
		addrs, err := net.LookupHost(name)
		if err != nil {
			log.Fatal(err)
		}
		for _, a := range addrs {
			parts := strings.Split(a, ".")
			if len(parts) < 3 {
				continue
			}
			//fmt.Printf("Parts %s %s %s %s", parts[0], parts[1], parts[2], parts[3])
			for i := 1; i <= 254; i++ {
				i_ip := fmt.Sprintf("%s.%s.%s.%d:4567", parts[0], parts[1], parts[2], i)
				//fmt.Printf("UDP to %s\n", i_ip)
				c, ok := cache[i_ip]
				if !ok {
					addr, _ := net.ResolveUDPAddr("udp", i_ip)
					c, err = net.DialUDP("udp", nil, addr)
					defer c.Close()
					if err != nil {
						log.Fatal(err)
					}
					cache[i_ip] = c
				}
				c.Write([]byte(ap.serverport))
			}
		}
		if stop {
			break
		}
		time.Sleep(time.Minute)
	}
}

func (ap *ActivePeers) heart_beat(stop bool) {

	addr, err := net.ResolveUDPAddr("udp", ap.group)
	if err != nil {
		log.Fatal(err)
	}

	c, err := net.DialUDP("udp", nil, addr)
	if err != nil {
		log.Fatal(err)
	}
	for {
		c.Write([]byte(ap.serverport))
		if stop {
			break
		}
		time.Sleep(time.Minute)
	}
}

func (ap *ActivePeers) listen_to_heart_beats() {
	addr, err := net.ResolveUDPAddr("udp", ap.group)
	if err != nil {
		log.Fatal(err)
	}
	//l, err := net.ListenUDP("udp4", addr)
    l, err := net.ListenMulticastUDP("udp", nil, addr)
	for {
		b := make([]byte, 10)
		n, src, err := l.ReadFromUDP(b)
		if err != nil {
			log.Fatal("ReadFromUDP failed:", err)
		}

		parts := strings.Split(src.String(), ":")
		receivedport := string(b[:n])
		serveruri := parts[0] + ":" + receivedport

		ap.rwlock_peers.RLock()
		_, ok := ap.peers[serveruri]
		ap.rwlock_peers.RUnlock()

		if !ok {
			//I exist!
			if !ap.client /*&& receivedport != ap.serverport*/ {
				log.Print("Telling I exist to ", serveruri)
				ap.heart_beat_f(true)
			}

			if receivedport != "0" { //a server
				ap.rwlock_peers.Lock()
				peers_count++
				ap.peers[serveruri] = peers_count
				ap.rwlock_peers.Unlock()
			}
		}

		//log.Println("Beat from", serveruri, ":", len(ap.peers))
	}
}

func (ap *ActivePeers) Start(group string, client bool, port string) {
	ap.group = group
	ap.peers = make(map[string]byte)
	ap.client = client
	ap.serverport = port
	ap.heart_beat_f = ap.heart_beat //_nomulticast
	go ap.listen_to_heart_beats()
	go ap.heart_beat_f(ap.client)

	inFile, _ := os.Open("hosts")
	defer inFile.Close()
	scanner := bufio.NewScanner(inFile)
	scanner.Split(bufio.ScanLines)
	for scanner.Scan() {
		server := scanner.Text()
		log.Println("Adding ", server, " from hosts file.")

		ap.rwlock_peers.Lock()
		peers_count++
		ap.peers[server] = peers_count
		ap.rwlock_peers.Unlock()
	}
}

func (ap *ActivePeers) ActivePeers() map[string]byte {
	return ap.peers
}

func (ap *ActivePeers) ActivePeersCount() int {
	ap.rwlock_peers.Lock()
	r := len(ap.peers)
	ap.rwlock_peers.Unlock()
	return r
}
