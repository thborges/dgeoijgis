package main

import "net/http"
import _ "net/http/pprof"

import (
	"flag"
	"fmt"
	"log"
	"net"
	"os"
	"os/signal"
	"syscall"
	"dgeodistr/peer"
	"runtime"
	"runtime/pprof"
	"strings"
)

// The flag package provides a default help printer via -h switch
var versionFlag *bool = flag.Bool("v", false, "Print the version number.")
var listenFlag *string = flag.String("port", ":8765", "Address to listen on.")
var netintfFlag *string = flag.String("intf", "lo", "Network Interface to measure net usage.")
var cpuprofFlag *string = flag.String("cpuprofile", "", "Filename to profile cpu usage.")
var enablehttpprofFlag *bool = flag.Bool("httpprofile", false, "Enable HTTP profile on port 6060.")

const (
	udp_group = "224.0.0.1:4567"
	data_dir = "./tmp"
	lock_file = "./tmp/lock"
)

func get_my_addr(port string) string {
	name, err := os.Hostname()
	if err != nil {
		peer.Flog.Fatal(err)
	}
	addrs, err := net.LookupHost(name)
	if err != nil {
		peer.Flog.Fatal(err)
	}
	if len(addrs) == 1 {
		return addrs[0] + port
	} else if len(addrs) > 0 {
		for _, s := range addrs {
			if (len(strings.Split(s, ":")) > 1) {
				// ignore IPv6 addrs for now
				continue
			} else if (s == "127.0.0.1") {
				// ignore loopback
				continue
			} else {
				return s + port
			}
		}
	} else {
		peer.Flog.Fatal("Host addr/name not found.")
	}
	return "localhost" + port
}

func createAndLockDataDir() *os.File {

	err := os.Mkdir(data_dir, 0700)
	if err != nil && !os.IsExist(err) {
		peer.Flog.Fatal("Temporary datadir ./tmp could not be created.")
		return nil
	}

	f, err := os.OpenFile(lock_file, os.O_RDWR|os.O_CREATE, os.ModeExclusive)
	if err != nil {
		peer.Flog.Fatal("Lock file could not be created. Check if the peer closed abruptally and delete the existing lock. Lock: " + lock_file)
		return nil
	}

	return f
}

func releaseLock(flock *os.File) {
	log.Println("Releasing data lock file")
	flock.Close()
	e := os.Remove(lock_file)
	if e != nil {
		log.Fatal(e)
	}
}

func main() {
	flock := createAndLockDataDir()
	defer releaseLock(flock)

	c := make(chan os.Signal)
    signal.Notify(c, syscall.SIGINT, syscall.SIGTERM)
    go func() {
        <-c
		log.Println("Signal detected, finishing...")
		releaseLock(flock)
        os.Exit(1)
    }()

	flag.Parse() // Scan the arguments list

	if *versionFlag {
		fmt.Println("Version:", peer.APP_VERSION)
		os.Exit(0)
	}

	if *enablehttpprofFlag {
		go func() {
			runtime.SetBlockProfileRate(1)
			log.Println(http.ListenAndServe("localhost:6060", nil))
		}()
	}

	if *cpuprofFlag != "" {
		f, err := os.Create(*cpuprofFlag)
		if err != nil {
			peer.Flog.Fatal(err)
		}
		pprof.StartCPUProfile(f)
		defer func() {
			log.Println("Stopping CPU Profile")
			pprof.StopCPUProfile()
		}()
	}

	log.SetFlags(log.Ltime| log.Lshortfile)
	log.Println("DGEO Build: ", peer.APP_VERSION)
	log.Println("GOMAXPROCS: ", runtime.GOMAXPROCS(0))

	addr := get_my_addr(*listenFlag)
	log.Println("Starting server listening on:", addr)

	log.Println("Activating peers discovering")
	ap := peer.ActivePeers{}
	ap.Start(udp_group, false, strings.Split(*listenFlag, ":")[1])

	log.Println("Activating services")
	gs := peer.GeoService{}
	gs.Start(*listenFlag, addr, flag.CommandLine, &ap)
}
