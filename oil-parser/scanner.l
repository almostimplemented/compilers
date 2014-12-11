%{
// $Id: scanner.l,v 1.14 2014-11-25 17:14:38-08 - - $

#include <ctype.h>

#include "parser.h"
#include "semantics.h"

#define YY_USER_ACTION { lex_offset += yyleng; }

int lex_linenr = 1;
int lex_offset = 0;

void lex_newline() {
   lex_offset = 0; ++lex_linenr;
}

void lex_error() {
   char message[64];
   sprintf (message, isprint (*yytext)
            ? "stray '%c' in program"
            : "stray '\x02X' in program", *yytext);
   yyerror (message);
}

%}

%option 8bit
%option debug
%option nodefault
%option nounput
%option noyywrap
%option verbose
%option warn
%option yylineno

CHARCON    ('([^\\'\n]|\\[\\'"0nt])')
STRINGCON  (\"([^\\"\n]|\\[\\'"0nt])*\")
INTCON     (0|[1-9][0-9]*)
IDENT      ([[:alpha:]_][[:alnum:]_]*)

/**/
%%

"!"           { return '!'; }
"%"           { return '%'; }
"("           { return '('; }
")"           { return ')'; }
"*"           { return '*'; }
"+"           { return '+'; }
","           { return ','; }
"-"           { return '-'; }
"/"           { return '/'; }
"&"           { return '&'; }
";"           { return ';'; }
":"           { return ':'; }
"<"           { return '<'; }
"="           { return '='; }
">"           { return '>'; }
"{"           { return '{'; }
"}"           { return '}'; }
"["           { return '['; }
"]"           { return ']'; }
"\n"          { lex_newline(); return '\n'; }

"->"          { return PTR; }
"!="          { return NE; }
"<="          { return LT; }
"=="          { return EQ; }
">="          { return GE; }

char          { return CHAR; }
goto          { return GOTO; }
if            { return IF; }
int           { return INT; }
return        { return RETURN; }
sizeof        { return SIZEOF; }
struct        { return STRUCT; }
void          { return VOID; }

{CHARCON}     { return CHARCON; }
{STRINGCON}   { return STRINGCON; }
{INTCON}      { return INTCON; }
{IDENT}       { return IDENT; }

[ \t\f]+      { }
.             { lex_error(); }

%%

