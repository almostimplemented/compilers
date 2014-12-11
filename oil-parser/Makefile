# $Id: Makefile,v 1.8 2014-11-25 17:16:10-08 - - $

GPP =  g++ -g -O0 -Wall -Wextra -std=gnu++11

all : oil

oil : parser.o scanner.o semantics.o
	${GPP} parser.o scanner.o semantics.o -o oil

parser.h parser.o : parser.y
	bison --defines=parser.h --output=parser.c parser.y
	${GPP} -c parser.c

scanner.o : scanner.l
	flex --outfile=scanner.c scanner.l 2>scanner.log
	${GPP} -c scanner.c

semantics.o : semantics.cpp
	${GPP} -c semantics.cpp

ci :
	cid + Makefile parser.y scanner.l semantics.h semantics.cpp

lis : all
	mkpspdf Listing.ps parser.y scanner.l semantics.h \
	        semantics.cpp Makefile oclib.oh oclib.c

clean :
	- rm parser.o scanner.o semantics.o parser.c parser.h \
	     scanner.c parser.output scanner.log \
	     Listing.ps Listing.pdf

