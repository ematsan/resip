# compil lib path
LIB=-L../lib -lrutil -lresip
# run path
RPATH=-Wl,-rpath=../lib
# -w - without warning
# -g - debugger
# -I.. - main dir
OPTION=-w -g -I..

all: db mysql thread runner server

command: Command.cxx
		g++ -c $(OPTION) Command.cxx -o command.o
db: RegDB.cxx
	g++ -c $(OPTION) $(RPATH)  RegDB.cxx -o regdb.o $(LIB)
mysql: RegMySQL.cxx
	g++ -c $(OPTION) $(RPATH)  RegMySQL.cxx -o regmysql.o $(LIB) -lmysqlclient
runner: RegRunner.cxx
	g++ -c $(OPTION) $(RPATH)  RegRunner.cxx -o regrunner.o $(LIB)
thread: Registrar.cxx
	g++ -c $(OPTION) $(RPATH)  Registrar.cxx -o registrar.o $(LIB) -lpthread
server: reg.cxx
	g++ $(OPTION) $(RPATH) reg.cxx regrunner.o registrar.o regmysql.o regdb.o command.o -o reg $(LIB) -lpthread -lmysqlclient
