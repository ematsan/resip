# compil lib path
LIB=-L../lib -lrutil -lresip -lmysqlclient
# run path
RPATH=-Wl,-rpath=../lib
# -w - without warning
# -g - debugger
# -I.. - main dir
OPTION=-w -g -I..

all: mybd thread runner server

mybd: RegMySQL.cxx
	g++ -c $(OPTION) $(RPATH)  RegMySQL.cxx -o regmysql.o $(LIB)
runner: RegRunner.cxx
	g++ -c $(OPTION) $(RPATH)  RegRunner.cxx -o regrunner.o $(LIB)
thread: RegThread.cxx
	g++ -c $(OPTION) $(RPATH)  RegThread.cxx -o regthread.o $(LIB)
server: registrar.cxx
	g++ $(OPTION) $(RPATH) registrar.cxx regrunner.o regthread.o regmysql.o -o reg $(LIB)
