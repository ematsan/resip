LIB=-L../lib -lrutil -lrepro -lresip
RPATH=-Wl,-rpath=../lib

all: client server

lib: testServerReg.cxx
	g++ -c -g -I.. $(RPATH)  testServerReg.cxx -o regrunner.o $(LIB)
server: testServerReg.cxx
	g++ -g -I.. $(RPATH)  testServerReg.cxx -o server $(LIB)
client: testClientReg.cxx
	g++ -g -I.. $(RPATH)  testClientReg.cxx -o client $(LIB)
