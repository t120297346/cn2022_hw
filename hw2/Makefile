CC = gcc
CXX = g++
INCLUDE_OPENCV = `pkg-config --cflags --libs opencv4`
LINK_PTHREAD = -lpthread

CLIENT = client.c
SERVER = server.c
OPEN_CV = openCV.cpp
PTHREAD = pthread.c
CLI = client
SER = server
CV = openCV
PTH = pthread

all: server client opencv pthread
  
server: $(SERVER)
	$(CC) $(SERVER) util.c -o $(SER) -lstdc++
client: $(CLIENT)
	$(CC) $(CLIENT) util.c -o $(CLI) -lstdc++
opencv: $(OPEN_CV)
	$(CXX) $(OPEN_CV) -o $(CV) $(INCLUDE_OPENCV)
pthread: $(PTHREAD)
	$(CC) $(PTHREAD) -o $(PTH) $(LINK_PTHREAD)

.PHONY: clean

clean:
	rm $(CLI) $(SER) $(CV) $(PTH)
