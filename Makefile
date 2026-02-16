CC = gcc
CFLAGS=-I. -Wall

OBJECTS = client.o requests.o helpers.o buffer.o parson.o

all: client

client: $(OBJECTS)
	$(CC) -o client $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: client
	./client

clean:
	rm -f *.o client
 