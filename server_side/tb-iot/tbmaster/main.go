package main

import (
	"log"
	"net"
	"time"

	pb "github.com/KitFung/tb-iot/proto"
	"google.golang.org/grpc"
)

const (
	port = ":30776"
)

func main() {
	lis, err := net.Listen("tcp", port)
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}
	tbnet := NewTBNetwork()
	defer tbnet.Close()
	go func() {
		for {
			tbnet.Update()
			time.Sleep(1 * time.Second)
		}
	}()

	s := grpc.NewServer()
	pb.RegisterTestBedMasterServer(s, &TestBedCliServer{
		tbnet: tbnet,
	})
	if err := s.Serve(lis); err != nil {
		log.Fatalf("failed to serve: %v", err)
	}

}
