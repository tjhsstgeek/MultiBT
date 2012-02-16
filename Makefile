CFLAGS:=-g -Wall -O3
LDFLAGS:=-lchipmunk -lm -lSDL -lSDL_net
ALL_OBJECTS:=linkedlist.o player.o bubble.o wall.o network.o game.o build.o client.o server.o
SHARED_OBJECTS:=linkedlist.o player.o bubble.o wall.o network.o game.o
all:$(ALL_OBJECTS)
	gcc -o server $(SHARED_OBJECTS) server.o $(LDFLAGS)
	gcc -o client $(SHARED_OBJECTS) client.o $(LDFLAGS)
	gcc -o build $(SHARED_OBJECTS) build.o $(LDFLAGS)
clean:
	rm $(ALL_OBJECTS) 
