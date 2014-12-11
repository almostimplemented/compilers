%{
// $Id: parser.y,v 1.9 2014-11-25 17:14:38-08 - - $

#include "semantics.h"

%}

%debug
%defines
%error-verbose
%token-table
%verbose

%token GOTO IF INT RETURN SIZEOF STRUCT CHAR VOID
%token CHARCON STRINGCON INTCON IDENT PTR NE LT EQ GE

%%

program    : program structdef
           | program globaldef
           | program function
           | program '\n'
           |
           ;

structdef  : STRUCT IDENT '{' '\n' fields '}' ';' '\n'
           ;

fields     : fields type IDENT ';' '\n'
           |
           ;

globaldef  : type IDENT '=' STRINGCON ';' '\n'
           | type IDENT ';' '\n'
           ;

function   : type IDENT parameters ';' '\n'
           | type IDENT parameters '\n' block
           | VOID IDENT parameters ';' '\n'
           | VOID IDENT parameters '\n' block
           ;

parameters : '(' '\n' params ')'
           | '(' VOID ')'
           | '(' ')'
           ;

params     : params ',' '\n' type IDENT
           | type IDENT
           ;

block      : '{' '\n' statements '}' '\n'
           ;

statements : statements statement
           |
           ;


statement  : IDENT ':' ';' '\n'
           | IF '(' condition ')' GOTO IDENT ';' '\n'
           | GOTO IDENT ';' '\n'
           | RETURN operand ';' '\n'
           | RETURN ';' '\n'
           | type IDENT '=' expression ';' '\n'
           | IDENT '=' expression ';' '\n'
           | '*' IDENT '=' expression ';' '\n'
           | type IDENT '=' call ';' '\n'
           | call ';' '\n'
           | block
           ;

call       : IDENT '(' args ')' | IDENT '(' ')'
           ;

args       : args ',' operand | operand
           ;

expression : operand binop operand
           | unop operand
           | selection
           | operand
           ;

binop      : '+' | '-' | '*' | '/' | '%' | '<' | '>'
           | NE | LT | EQ | GE
           ;

unop       : '+' | '-' | '!' | '*' | '(' INT ')' | '(' CHAR ')'
           ;

selection  : '&' IDENT '[' operand ']'
           | '&' IDENT PTR IDENT
           ;

condition  : '!' operand | operand
           ;

operand    : IDENT | INTCON | CHARCON | SIZEOF '(' type ')'
           | '*' IDENT
           ;

type       : VOID '*' | CHAR | CHAR '*' | CHAR '*' '*'
           | CHAR '*' '*' '*' | INT | INT '*' | INT '*' '*'
           | STRUCT IDENT | STRUCT IDENT '*' | STRUCT IDENT '*' '*'
           | STRUCT IDENT '*' '*' '*'
           ;

%%

