// $Id: semantics.h,v 1.1 2014-11-25 17:15:08-08 - - $

#ifndef __SEMANTICS_H__
#define __SEMANTICS_H__

#include <cstdio>
using namespace std;

extern FILE* yyin;
extern char* yytext;
extern int yy_flex_debug;
extern int yydebug;
extern int yyleng;

extern int lex_linenr;
extern int lex_offset;

int yylex (void);
int yyparse (void);
void yyerror (const char* message);

#endif

