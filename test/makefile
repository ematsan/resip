LIB=-L../lib -lrutil -lrepro -lresip
RPATH=-Wl,-rpath=../lib

all: client server

lib: testServer.cxx
	g++ -c -g -I.. $(RPATH)  RegRunner.cxx -o regrunner.o $(LIB)
server: testServer.cxx
	g++ -g -I.. $(RPATH)  testServer.cxx -o server $(LIB)
client: testClient.cxx
	g++ -g -I.. $(RPATH)  testClient.cxx -o client $(LIB)
