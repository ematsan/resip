LIB=-L../lib -lrutil -lrepro -lresip -lmysqlclient
RPATH=-Wl,-rpath=../lib

all: testmysql

lib: testmysql.cxx
	g++ -c -g -I.. $(RPATH)  testmysql.cxx -o testmysql.o
testmysql: testmysql.cxx
	g++ -g -I.. $(RPATH)  testmysql.cxx -o mysql $(LIB)
