package main

import (
	"io"
	"log"
	"net"
)

func main() {
	listener, err := net.Listen("tcp", ":5000")
	if err != nil {
		panic(err)
	}

	log.Println("Monitoring 5000 port...")
	for {
		conn, err := listener.Accept()
		if err != nil {
			panic(err)
		}
		go func(conn net.Conn) {
			defer conn.Close()
			echo(conn)
		}(conn)
	}
}

func echo(conn net.Conn) {
	buf := make([]byte, 256)
	for {
		n, err := conn.Read(buf)
		if err != nil {
			if err == io.EOF {
				break
			}
			panic(err)
		}
		if n == 0 {
			break
		}
		wn, err := conn.Write(buf[0:n])
		if err != nil {
			panic(err)
		}
		if wn != n {
			panic("could not send")
		}

		log.Print(string(buf[0:n]))
	}
}
