LIB=-L../lib -lrutil -lresip
RPATH=-Wl,-rpath=../lib

all: md5.cxx
	g++ -w -g -I.. $(RPATH)  md5.cxx -o md5 $(LIB)
