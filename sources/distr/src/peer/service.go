package peer

import (
	"encoding/gob"
	"flag"
	"io"
	"log"
	"net"
)

type GeoServiceHandlerParams struct {
	dec   *gob.Decoder
	enc   *gob.Encoder
	addr  string
	flags *flag.FlagSet
	ap    *ActivePeers
	conn  net.Conn
}

type GeoService struct {
	handlers map[RequestType]func(*GeoServiceHandlerParams)
}

func (g *GeoService) handle_connection(conn net.Conn, addr string, flags *flag.FlagSet, ap *ActivePeers) bool {

	dec := gob.NewDecoder(conn)
	r := RequestMsg{}
	for {
		err := dec.Decode(&r)
		if err != nil {
			if err == io.EOF {
				break
			} else {
				log.Println(err)
			}
		} else {
			enc := gob.NewEncoder(conn)

			var hp GeoServiceHandlerParams
			hp.dec = dec
			hp.enc = enc
			hp.ap = ap
			hp.flags = flags
			hp.addr = addr
			hp.conn = conn
			g.handlers[r.RType](&hp)
		}
	}

	return false
}

func (g *GeoService) Start(listen_on string, addr string, flags *flag.FlagSet, ap *ActivePeers) {

	g.handlers = GetDispatcher()

	ln, err := net.Listen("tcp", listen_on)
	if err != nil {
		log.Println(err)
		return
	}

	exit := false
	for !exit {
		conn, err := ln.Accept()
		if err != nil {
			log.Println(err)
			return
		}

		go func() {
			exit = g.handle_connection(conn, addr, flags, ap)
		}()
	}

}
