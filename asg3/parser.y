%{

#include "lyutils.h"
#include "astree.h"
#include "stringset.h"

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

%token TOK_BLOCK TOK_CALL TOK_IFELSE TOK_DECLID
%token TOK_POS TOK_NEG TOK_NEWARRAY TOK_TYPEID TOK_FIELD
%token TOK_ORD TOK_CHR TOK_ROOT

%token TOK_INDEX TOK_NEWSTRING TOK_RETURNVOID TOK_FUNCTION
%token TOK_PARAMLIST TOK_VARDECL TOK_PROTOTYPE

%destructor { error_destructor ($$); } <>

%start program

%right "then" TOK_ELSE
%right '='
%left TOK_EQ TOK_NE TOK_LE TOK_LT TOK_GE TOK_GT
%left '+' '-'
%left '*' '/' '%'
%right POS NEG '!' TOK_ORD TOK_CHR
%left '(' ')'
%left '[' ']' '.'
%nonassoc TOK_NEW

%%

program     : program structdef       { $$ = adopt1($1, $2); }
            | program function        { $$ = adopt1($1, $2); }
            | program statement       { $$ = adopt1($1, $2); }
            | program error '}'       { free_ast($3); $$ = $1; }
            | program error ']'       { free_ast($3); $$ = $1; }
            |                         { $$ = new_parseroot(); }
            ;
structdef   : structhead '}'          { free_ast($2); $$ = $1; }
            ;
structhead  : structhead fielddecl ';'
                                      { free_ast($3); 
                                        $$ = adopt1($1, $2); }
            | TOK_STRUCT TOK_IDENT '{'
                                      { free_ast($3);
                                        $2->symbol = TOK_TYPEID;
                                        $$ = adopt1($1, $2); }
            ;
fielddecl   : basetype TOK_ARRAY TOK_IDENT  
                                      { $3->symbol = TOK_FIELD;
                                        $$ = adopt2($2, $1, $3); }
            | basetype TOK_IDENT      { $2->symbol = TOK_FIELD; 
                                        $$ = adopt1($1, $2); }
            ;
basetype    : TOK_VOID                { $$ = $1; }
            | TOK_BOOL                { $$ = $1; } 
            | TOK_CHAR                { $$ = $1; }
            | TOK_INT                 { $$ = $1; }
            | TOK_STRING              { $$ = $1; }
            | TOK_IDENT               { $1->symbol = TOK_TYPEID;
                                        $$ = $1; }
            ;
function    : identdecl '(' ')' ';'   { $2->symbol = TOK_PARAMLIST;
                                        free_ast($3, $4);
                                        $$ = adopt2(new_astree(
                                                     TOK_PROTOTYPE,
                                                     $1->filenr,
                                                     $1->linenr,
                                                     $1->offset,
                                                     ""), $1, $2); 
                                        intern_stringset(""); }
            | identdecl '(' ')' block { $2->symbol = TOK_PARAMLIST;
                                        free_ast($3);
                                        $$ = adopt3(new_astree(
                                                     TOK_FUNCTION,
                                                     $1->filenr,
                                                     $1->linenr,
                                                     $1->offset,
                                                     ""), $1, $2, $4);  
                                        intern_stringset(""); }
            | identdecl params ')' ';'
                                      { free_ast($3, $4);
                                        $$ = adopt2(new_astree(
                                                     TOK_PROTOTYPE,
                                                     $1->filenr,
                                                     $1->linenr,
                                                     $1->offset,
                                                     ""), $1, $2); 
                                        intern_stringset(""); }
            | identdecl params ')' block 
                                      { free_ast($3);
                                        $$ = adopt3(new_astree(
                                                     TOK_FUNCTION,
                                                     $1->filenr,
                                                     $1->linenr,
                                                     $1->offset,
                                                     ""), $1, $2, $4); 
                                        intern_stringset(""); }
            ;
params      : params ',' identdecl    { free_ast($2);
                                        $$ = adopt1($1, $3); }
            | '(' identdecl           { $$ = adopt1sym($1, $2,
                                                     TOK_PARAMLIST); }
            ;
identdecl   : basetype TOK_ARRAY TOK_IDENT  { $3->symbol = TOK_DECLID;
                                              $$ = adopt2($2, $1, $3); }
            | basetype TOK_IDENT      { $2->symbol = TOK_DECLID;
                                        $$ = adopt1($1, $2); }
            ;
block       : blockhead '}'           { free_ast($2);
                                        $$ = $1; }
            ;
blockhead   : blockhead statement     { $$ = adopt1($1, $2); }
            | '{'                     { $1->symbol = TOK_BLOCK; 
                                        $$ = $1; }
            ;
statement   : block                   { $$ = $1; }
            | vardecl                 { $$ = $1; }
            | while                   { $$ = $1; }
            | ifelse                  { $$ = $1; }
            | return                  { $$ = $1; }
            | expr ';'                { free_ast($2); $$ = $1; }
            | ';'                     { $$ = $1; }
            ;
vardecl     : identdecl '=' expr ';'  { free_ast($4);
                                        $$ = adopt2sym($2, $1, $3, 
                                                       TOK_VARDECL); }
            ;
while       : TOK_WHILE '(' expr ')' statement                  
                                      { free_ast($2, $4);
                                        $$ = adopt2($1, $3, $5); }
            ;
ifelse      : TOK_IF '(' expr ')' statement TOK_ELSE statement
                                      { free_ast($2, $4, $6);
                                        $$ = adopt3sym($1, $3, $5,
                                                    $7, TOK_IFELSE); }
            | TOK_IF '(' expr ')' statement %prec "then"
                                      { free_ast($2, $4);
                                        $$ = adopt2($1, $3, $5); }
            ;
return      : TOK_RETURN ';'          { free_ast($2);
                                        $1->symbol = TOK_RETURNVOID; 
                                        $$ = $1; }
            | TOK_RETURN expr ';'     { free_ast($3);
                                        $$ = adopt1($1, $2); }
            ;
expr        : binop                   { $$ = $1; }
            | unop                    { $$ = $1; }
            | allocator               { $$ = $1; }
            | call                    { $$ = $1; }
            | '(' expr ')'            { free_ast($1, $3);
                                        $$ = $2; }
            | variable                { $$ = $1; }
            | constant                { $$ = $1; }
            ;
binop       : expr '+' expr           { $$ = adopt2($2, $1, $3); }
            | expr '-' expr           { $$ = adopt2($2, $1, $3); }
            | expr '*' expr           { $$ = adopt2($2, $1, $3); }
            | expr '/' expr           { $$ = adopt2($2, $1, $3); }
            | expr '%' expr           { $$ = adopt2($2, $1, $3); }
            | expr '=' expr           { $$ = adopt2($2, $1, $3); }
            | expr TOK_EQ expr        { $$ = adopt2($2, $1, $3); }
            | expr TOK_NE expr        { $$ = adopt2($2, $1, $3); }
            | expr TOK_LT expr        { $$ = adopt2($2, $1, $3); }
            | expr TOK_LE expr        { $$ = adopt2($2, $1, $3); }
            | expr TOK_GT expr        { $$ = adopt2($2, $1, $3); }
            | expr TOK_GE expr        { $$ = adopt2($2, $1, $3); }
            ;
unop        : '+' expr                { $$ = adopt1sym($1, $2,
                                                       TOK_POS); }
            | '-' expr                { $$ = adopt1sym($1, $2,
                                                       TOK_NEG); }
            | '!' expr                { $$ = adopt1($1, $2); }
            | TOK_ORD expr            { $$ = adopt1($1, $2); }
            | TOK_CHR expr            { $$ = adopt1($1, $2); }
            ;
allocator   : TOK_NEW TOK_IDENT '(' ')'
                                      { free_ast($3, $4);
                                        $2->symbol = TOK_TYPEID; 
                                        $$ = adopt1($1, $2); }
            | TOK_NEW TOK_STRING  '(' expr ')'  
                                      { free_ast($2, $3, $5); 
                                        $$ = adopt1sym($1, $4, 
                                                TOK_NEWSTRING); }
            | TOK_NEW basetype '[' expr ']'
                                      { free_ast($3, $5); 
                                        $$ = adopt2sym($1, $2, $4, 
                                                TOK_NEWARRAY); }
            ;
call        : TOK_IDENT '(' ')'       { free_ast($3);
                                        $$ = adopt1sym($2, $1,
                                                  TOK_CALL); }
            | args ')'                { free_ast($2);
                                        $$ = $1; }
            ;
args        : TOK_IDENT '(' expr      { $$ = adopt2sym($2, $1, $3, 
                                                       TOK_CALL); }
            | args ',' expr           { free_ast($2);
                                        $$ = adopt1($1, $3); }
            ;
variable    : TOK_IDENT               { $$ = $1; }
            | expr '[' expr ']'       { free_ast($4);
                                        $$ = adopt2sym($2, $1, $3,
                                                       TOK_INDEX); }
            | expr '.' TOK_IDENT      { $3->symbol = TOK_FIELD; 
                                        $$ = adopt2($2, $1, $3); }
            ;
constant    : TOK_INTCON              { $$ = $1; }
            | TOK_CHARCON             { $$ = $1; }
            | TOK_STRINGCON           { $$ = $1; } 
            | TOK_FALSE               { $$ = $1; }
            | TOK_TRUE                { $$ = $1; }
            | TOK_NULL                { $$ = $1; } 
            ;

%%

const char *get_yytname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}


bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}

