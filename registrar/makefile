LIB=-L../lib -lrutil -lrepro -lresip
RPATH=-Wl,-rpath=../lib

all: regrunner regconfig reg

regrunner: RegRunner.cxx
	g++ -c -g -I.. $(RPATH)  RegRunner.cxx -o regrunner.o $(LIB)
regconfig: RegConfig.cxx
	g++ -c -g -I.. $(RPATH)  RegConfig.cxx -o regconfig.o $(LIB)
reg: reg.cxx
	g++ -g -I.. $(RPATH)  reg.cxx regrunner.o regconfig.o -o reg $(LIB)
