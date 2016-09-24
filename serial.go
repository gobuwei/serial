package main

import (
	"bufio"
	"flag"
	"fmt"
	"os"

	"github.com/tarm/serial"
)

func writeRoutine(s *serial.Port) {
	r := bufio.NewReader(os.Stdin)

	for {
		text, err := r.ReadString('\n')
		if err != nil {
			panic(err)
		}

		if len(text) <= 1 {
			fmt.Printf(">> ")
			continue
		}

		s.Write([]byte(text))
	}
}

func readRoutine(s *serial.Port, hex bool) {
	buf := make([]byte, 256)

	for {
		n, err := s.Read(buf)
		if err != nil {
			panic(err)
		}

		if hex {
			for i := 0; i < n; i++ {
				fmt.Printf(" %02X", buf[i])
			}
		} else {
			fmt.Printf("%s", buf[:n])
		}
	}
}

var hex *bool = flag.Bool("x", false, "output in hex format")
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
