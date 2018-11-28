LIB=-L../lib -lrutil -lresip -lmysqlclient
RPATH=-Wl,-rpath=../lib
OPTION=-g -I..

all: runner server

runner: RegRunner.cxx
	g++ -c -g -I.. $(RPATH)  RegRunner.cxx -o regrunner.o $(LIB)
server: registrar.cxx
	g++ $(OPTION) $(RPATH) registrar.cxx regrunner.o -o reg $(LIB)
