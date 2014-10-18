# $Id: Makefile,v 1.8 2014-10-07 18:13:45-07 - - $

GCC        = g++ -g -O0 -Wall -Wextra -std=gnu++11
MKDEP      = g++ -MM -std=gnu++11
VALGRIND   = valgrind --leak-check=full --show-reachable=yes

MKFILE     = Makefile
DEPFILE    = Makefile.dep
SOURCES    = auxlib.cpp stringset.cpp preprocess.cpp oc.cpp
HEADERS    = auxlib.h stringset.h preprocess.h
OBJECTS    = ${SOURCES:.cpp=.o}
EXECBIN    = oc
SRCFILES   = ${HEADERS} ${SOURCES} ${MKFILE}
SUBMITLOC  = cmps104a-wm.f14
ASGNO      = asg1

all : ${EXECBIN}

${EXECBIN} : ${OBJECTS}
	${GCC} -o${EXECBIN} ${OBJECTS}

%.o : %.cpp
	${GCC} -c $<

ci :
	git commit -m "$m"

clean :
	- rm ${OBJECTS}
	- rm -f *str

spotless : clean
	- rm ${EXECBIN} ${DEPFILE}

submit :
	- submit ${SUBMITLOC} ${ASGNO} ${SRCFILES}
	- cp ${SRCFILES} latest_submit

${DEPFILE} :
	${MKDEP} ${SOURCES} >${DEPFILE}

dep :
	- rm ${DEPFILE}
	${MAKE} --no-print-directory ${DEPFILE}

include Makefile.dep

test : ${EXECBIN}
