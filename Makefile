
#
#	Makefile for hello project
#

DEBUG = -g
CC = qcc
LD = qcc


TARGET = -Vgcc_ntox86_64
#TARGET = -Vgcc_ntoaarch64le


CFLAGS += $(DEBUG) $(TARGET) -Wall
LDFLAGS+= $(DEBUG) $(TARGET)

all:	Client_Server affinity_test

Client_Server:	
	$(CC) $(CFLAGS) -o Client_Server Client_Server.c $(LDFLAGS)

affinity_test:	
	$(CC) $(CFLAGS) -o affinity_test Affinity_test.c $(LDFLAGS)


clean_soln:
	rm -f *.o Client_Server affinity_test
