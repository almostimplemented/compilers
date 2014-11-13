%{

#include "lyutils.h"
#include "astree.h"

%}

%debug
%defines
%error-verbose
%token-table
%verbose

%token TOK_VOID TOK_BOOL TOK_CHAR TOK_INT TOK_STRING
%token TOK_IF TOK_ELSE TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_FALSE TOK_TRUE TOK_NULL TOK_NEW TOK_ARRAY
%token TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON

%token TOK_BLOCK TOK_CALL TOK_IFELSE TOK_INITDECL
%token TOK_POS TOK_NEG TOK_NEWARRAY TOK_TYPEID TOK_FIELD
%token TOK_ORD TOK_CHR TOK_ROOT

%start program

%destructor { error_destructor($$); } <>

%right "then" TOK_ELSE
%right =
%left TOK_EQ TOK_NE TOK_LE TOK_LT TOK_GE TOK_GT
%left '+' '-'
%left '*' '/'
%right POS NEG '!' TOK_ORD TOK_CHR
%left '(' ')'
%left '[' ']' '.'
%nonassoc TOK_NEW

%%

start       : program { yyparse_astree = $1; }
            ;
program     : program structdef     { $$ = adopt1($1, $2); }
            | program function      { $$ = adopt1($1, $2); }
            | program statement     { $$ = adopt1($1, $2); }
            | program error '}'     { $$ = $1; }
            | program error ']'     { $$ = $1; }
            |                       { $$ = new_parseroot(); }
            ;
structdef   : structhead '}'
            ;
structhead  : structhead fielddecl ';'
            | TOK_STRUCT TOK_IDENT '{'
            ;
fielddecl   : basetype TOK_ARRAY TOK_IDENT
            | basetype TOK_IDENT
            ;
basetype    : TOK_VOID
            | TOK_BOOL
            | TOK_CHAR
            | TOK_INT
            | TOK_STRING
            | TOK_IDENT
            ;
function    : identdecl '(' ')' ';'
            | identdecl '(' ')' block
            | identdecl params ')' ';'
            | identdecl params ')' block
            ;
params      : params ',' identdecl
            | '(' identdecl
            ;
identdecl   : basetype TOK_ARRAY TOK_IDENT
            | basetype TOK_IDENT
            ;
block       : blockhead '}'
            ;
blockhead   : blockhead statement
            | '{'
            ;
statement   : block                 { /*  */ }
            | vardecl               {}
            | while                 {}
            | ifelse                {}
            | return                {}
            | expr ';'              {}
            | ';'
            ;
vardecl     : identdecl '=' expr ';'
            ;
while       : TOK_WHILE '(' expr ')' statement
            ;
ifelse      : TOK_IF '(' expr ')' statement TOK_ELSE statement
            | TOK_IF '(' expr ')' statement
            ;
return      : TOK_RETURN ';'
            | TOK_RETURN expr ';'
            ;
expr        : expr binop expr
            | unop expr
            | allocator
            | call
            | '(' expr ')'
            | variable
            | constant
            ;
allocator   : TOK_NEW TOK_IDENT '(' ')'
            | TOK_NEW TOK_STRING  '(' expr ')'
            | TOK_NEW basetype '[' expr ']'
            ;
call        : TOK_IDENT '(' ')'
            | args ')'
            ;
args        : TOK_IDENT '(' expr
            | args ',' expr
            ;
variable    : TOK_IDENT
            | expr '[' expr ']'
            | expr '.' TOK_IDENT
            ;
constant    : TOK_INTCON
            | TOK_CHARCON
            | TOK_STRINGCON
            | TOK_FALSE
            | TOK_TRUE
            | TOK_NULL
            ;

token   : '(' | ')' | '[' | ']' | '{' | '}' | ';' | ',' | '.'
        | '=' | '+' | '-' | '*' | '/' | '%' | '!'
        | TOK_VOID | TOK_BOOL | TOK_CHAR | TOK_INT | TOK_STRING
        | TOK_IF | TOK_ELSE | TOK_WHILE | TOK_RETURN | TOK_STRUCT
        | TOK_FALSE | TOK_TRUE | TOK_NULL | TOK_NEW | TOK_ARRAY
        | TOK_EQ | TOK_NE | TOK_LT | TOK_LE | TOK_GT | TOK_GE
        | TOK_IDENT | TOK_INTCON | TOK_CHARCON | TOK_STRINGCON
        | TOK_ORD | TOK_CHR | TOK_ROOT
        ;

%%

const char *get_yytname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}


bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}

