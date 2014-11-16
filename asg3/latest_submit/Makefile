MKFILE    = Makefile
DEPSFILE  = ${MKFILE}.deps
NOINCLUDE = ci clean spotless
NEEDINCL  = ${filter ${NOINCLUDE}, ${MAKECMDGOALS}}
VALGRIND  = valgrind --leak-check=full --show-reachable=yes

#
# Definitions of list of files:
#
HSOURCES  = astree.h  lyutils.h  auxlib.h  stringset.h
CSOURCES  = astree.cc lyutils.cc auxlib.cc stringset.cc main.cc
LSOURCES  = scanner.l
YSOURCES  = parser.y
ETCSRC    = README.md ${MKFILE} ${DEPSFILE}
CLGEN     = yylex.cc
HYGEN     = yyparse.h
CYGEN     = yyparse.cc
CGENS     = ${CLGEN} ${CYGEN}
ALLGENS   = ${HYGEN} ${CGENS}
EXECBIN   = oc
ALLCSRC   = ${CSOURCES} ${CGENS}
OBJECTS   = ${ALLCSRC:.cc=.o}
LREPORT   = yylex.output
YREPORT   = yyparse.output
IREPORT   = ident.output
REPORTS   = ${LREPORT} ${YREPORT} ${IREPORT}
ALLSRC    = ${ETCSRC} ${YSOURCES} ${LSOURCES} ${HSOURCES} ${CSOURCES}

#
# Definitions of the compiler and compilation options:
#
GCC       = g++ -g -O0 -Wall -Wextra -std=gnu++11
MKDEPS    = g++ -MM -std=gnu++11

#
# The first target is always ``all'', and hence the default,
# and builds the executable images
#
all : ${EXECBIN}

#
# Build the executable image from the object files.
#
${EXECBIN} : ${OBJECTS}
	${GCC} -o${EXECBIN} ${OBJECTS}
	ident ${OBJECTS} ${EXECBIN} >${IREPORT}

#
# Build an object file form a C source file.
#
%.o : %.cc
	${GCC} -c $<

#
# Build the scanner.
#
${CLGEN} : ${LSOURCES}
	flex --outfile=${CLGEN} ${LSOURCES} 2>${LREPORT}
	- grep -v '^  ' ${LREPORT}

#
# Build the parser.
#
${CYGEN} ${HYGEN} : ${YSOURCES}
	bison --defines=${HYGEN} --output=${CYGEN} ${YSOURCES}

#
# Clean and spotless remove generated files.
#
clean :
	- rm ${OBJECTS} ${ALLGENS} ${REPORTS} ${DEPSFILE}

spotless : clean
	- rm ${EXECBIN}

#
# Build the dependencies file using the C preprocessor
#
deps : ${ALLCSRC}
	@ echo "# ${DEPSFILE} created `date` by ${MAKE}" >${DEPSFILE}
	${MKDEPS} ${ALLCSRC} >>${DEPSFILE}

${DEPSFILE} :
	@ touch ${DEPSFILE}
	${MAKE} --no-print-directory deps

#
# Test
#

#
# Subimt
#

submit :
	submit cmps104a-wm.f14 asg3 ${ALLSRC}
	mkdir -p latest_submit
	cp ${ALLSRC} latest_submit

#
# Everything
#
again :
	gmake --no-print-directory spotless deps ci all lis
	
ifeq "${NEEDINCL}" ""
include ${DEPSFILE}
endif
