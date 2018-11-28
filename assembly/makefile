LIB=-L../lib -lrutil -lresip -lmysqlclient
RPATH=-Wl,-rpath=../lib
OPTION=-g -I..

all: thread runner server

runner: RegRunner.cxx
	g++ -c -g -I.. $(RPATH)  RegRunner.cxx -o regrunner.o $(LIB)
thread: RegThread.cxx
		g++ -c -g -I.. $(RPATH)  RegThread.cxx regrunner.o -o regthread.o $(LIB)
server: registrar.cxx
	g++ $(OPTION) $(RPATH) registrar.cxx regrunner.o regthread.o -o reg $(LIB)
