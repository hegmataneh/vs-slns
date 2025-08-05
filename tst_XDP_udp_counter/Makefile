BPF_CLANG = clang
BPF_CFLAGS = -O2 -g -Wall -target bpf

all: xdp_udp_counter.o read_counter

xdp_udp_counter.o: xdp_udp_counter.c
	$( BPF_CLANG ) $( BPF_CFLAGS ) -c xdp_udp_counter.c -o xdp_udp_counter.o

read_counter: read_counter.c
	gcc -o read_counter read_counter.c -lbpf

clean:
	 rm -f xdp_udp_counter.o read_counter