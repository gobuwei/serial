package main

import (
	"flag"
	"fmt"

	"github.com/tarm/serial"
)

func writeRoutine(s *serial.Port) {
	/* TODO */
}

func readRoutine(s *serial.Port, hex bool) {
	buf := make([]byte, 256)
	for {
		n, err := s.Read(buf)
		if err != nil {
			log.Fatal(err)
			continue
		}
		if n <= 0 {
			continue
		}
		if hex {
			/* TODO */
		} else {
			fmt.Printf("%s", buf[:n])
		}
	}
}

var hex *bool = flag.Bool("x", false, "display in hexadecimal format")
var baud *int = flag.Int("b", 115200, "baudrate")

func main() {
	flag.Parse()

	name := "/dev/ttyUSB0"
	if flag.NArg() > 0 {
		name = flag.Arg(0)
	}

	c := &serial.Config{Name: name, Baud: *baud}
	s, err := serial.OpenPort(c)
	if err != nil {
		panic(err)
	}

	go writeRoutine(s)
	readRoutine(s, *hex)
}
