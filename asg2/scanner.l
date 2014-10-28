%{
// $Id: scanner.l,v 1.3 2013-10-15 16:37:56-07 - - $

#include "auxlib.h"
#include "lyutils.h"

#define YY_USER_ACTION  { scanner_useraction (); }
#define IGNORE(THING)   { }

%}

%option 8bit
%option debug
%option ecs
%option nodefault
%option nounput
%option noyywrap
%option perf-report
%option verbose
%option warn

LETTER          [A-Za-z_]
DIGIT           [0-9]
INTEGER         ({DIGIT}+)
IDENT           ({LETTER}({LETTER}|{DIGIT})*)
BAD_IDENT       ({INTEGER}{IDENT})
CHAR_BODY       ([^\\'\n]|\\[\\'"0nt])
CHAR_CONST      (\'{CHAR_BODY}\')
BAD_CHAR        (\'{CHAR_BODY}{CHAR_BODY}+\'|\'{CHAR_BODY}*\n)
STRING_BODY     ([^\\"\n]|\\[\\'"0nt])
STRING_CONST    (\"{STRING_BODY}*\")
BAD_STRING      (\"{STRING_BODY}*\n)

%%

"void"          { return yylval_token (TOK_VOID); }
"bool"          { return yylval_token (TOK_BOOL); }
"char"          { return yylval_token (TOK_CHAR); }
"int"           { return yylval_token (TOK_INT); }
"string"        { return yylval_token (TOK_STRING); }
"struct"        { return yylval_token (TOK_STRUCT); }
"if"            { return yylval_token (TOK_IF); }
"else"          { return yylval_token (TOK_ELSE); }
"while"         { return yylval_token (TOK_WHILE); }
"return"        { return yylval_token (TOK_RETURN); }
"false"         { return yylval_token (TOK_FALSE); }
"true"          { return yylval_token (TOK_TRUE); }
"null"          { return yylval_token (TOK_NULL); }
"ord"           { return yylval_token (TOK_ORD); }
"chr"           { return yylval_token (TOK_CHR); }
"new"           { return yylval_token (TOK_NEW); }

"#".*           { scanner_include(); }
[ \t]+          { IGNORE (white space) }
\n              { scanner_newline(); }

{INTEGER}       { return yylval_token (TOK_INTCON); }
{IDENT}         { return yylval_token (TOK_IDENT); }
{CHAR_CONST}    { return yylval_token (TOK_CHARCON); }
{STRING_CONST}  { return yylval_token (TOK_STRINGCON); }

"[]"            { return yylval_token (TOK_ARRAY); }
"("             { return yylval_token ('('); }
")"             { return yylval_token (')'); }
"["             { return yylval_token ('['); }
"]"             { return yylval_token (']'); }
"{"             { return yylval_token ('{'); }
"}"             { return yylval_token ('}'); }
";"             { return yylval_token (';'); }
","             { return yylval_token (','); }
"."             { return yylval_token ('.'); }
"="             { return yylval_token ('='); }
"=="            { return yylval_token (TOK_EQ); }
"!="            { return yylval_token (TOK_NE); }
"<"             { return yylval_token (TOK_LT); }
"<="            { return yylval_token (TOK_LE); }
">"             { return yylval_token (TOK_GT); }
">="            { return yylval_token (TOK_GE); }
"+"             { return yylval_token ('+'); }
"-"             { return yylval_token ('-'); }
"*"             { return yylval_token ('*'); }
"/"             { return yylval_token ('/'); }
"%"             { return yylval_token ('%'); }
"!"             { return yylval_token ('!'); }

{BAD_IDENT}     { scanner_badtoken (yytext);
                  return yylval_token(TOK_IDENT); }

{BAD_CHAR}      { scanner_badtoken (yytext);
                  return yylval_token(TOK_CHARCON); }

{BAD_STRING}    { scanner_badtoken (yytext);
                  return yylval_token(TOK_STRINGCON); }

.               { scanner_badchar (*yytext); }
%%
