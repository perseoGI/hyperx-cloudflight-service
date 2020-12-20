CFLAGS			:= -Wall -Wextra -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wstrict-prototypes -Wstrict-overflow=5 -Wwrite-strings -Waggregate-return -Wcast-qual -Wswitch-enum -Wswitch-default -Wconversion -Wunreachable-code
CC := gcc

all: hyperx-service hyperx-client

hyperx-service: hyperx-service.c
	$(CC) $(CFLAGS) $^ -o $@ -ludev -lpthread

hyperx-client: hyperx-client.c
	$(CC) $(CFLAGS) $^ -o $@


#unlink /tmp/hyperx-service; gcc hyperx-service.c -o hyperx-service -ludev -lpthread
