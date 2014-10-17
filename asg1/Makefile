# $Id: Makefile,v 1.8 2014-10-07 18:13:45-07 - - $

GCC        = g++ -g -O0 -Wall -Wextra -std=gnu++11
MKDEP      = g++ -MM -std=gnu++11
VALGRIND   = valgrind --leak-check=full --show-reachable=yes

MKFILE     = Makefile
DEPFILE    = Makefile.dep
SOURCES    = auxlib.cpp oc.cpp stringset.cpp
HEADERS    = auxlib.h
OBJECTS    = ${SOURCES:.cpp=.o}
STR		   = *.str
EXECBIN    = oc
SRCFILES   = ${HEADERS} ${SOURCES} ${MKFILE}

all : ${EXECBIN}

${EXECBIN} : ${OBJECTS}
	${GCC} -o${EXECBIN} ${OBJECTS}

%.o : %.cpp
	${GCC} -c $<

clean :
	- rm ${OBJECTS} ${STR}

spotless : clean
	- rm ${EXECBIN} ${LISTING} ${LISTING:.ps=.pdf} ${DEPFILE} \
	     test.out test.err misc.lis

${DEPFILE} :
	${MKDEP} ${SOURCES} >${DEPFILE}

dep :
	- rm ${DEPFILE}
	${MAKE} --no-print-directory ${DEPFILE}

include Makefile.dep

test : ${EXECBIN}
