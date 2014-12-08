// Author, Andrew Edwards, ancedwar@ucsc.edu
//

#ifndef __EMIT__
#define __EMIT__

#include <string>
#include <unordered_map>
#include <bitset>
#include <vector>
#include <queue>

#include "symboltable.h"
#include "astree.h"
#include "yyparse.h"
using namespace std;

typedef symbol_table::iterator symtab_it;

extern FILE *oilfile;
extern symbol_table* struct_table;
extern queue<astree*> string_queue;
extern symbol_table* struct_table;
extern vector<symbol_table*> symbol_stack;
extern symbol_table* global_table;

void emit_code(astree* root);
void emit_structs();
void emit_stringcons();
void emit_vardecls();
void emit_functions();
void emit_expression(astree* node);
void emit_statement(astree* node);
void emit_while(astree* root);
void emit_ifelse(astree* root);
void emit_if(astree* root);
void emit_return(astree* root);
void emit_vardecl(astree* root);
void emit_assignment(astree* root);
void emit_compare(astree* root);
void emit_ord(astree* root);
void emit_chr(astree* root);
void emit_sign(astree* root);
void emit_new(astree* root);
void emit_newarray(astree* root);
void emit_newstring(astree* root);
void emit_index(astree* root);
void emit_select(astree* root);
void emit_call(astree* root);
void emit_operand(astree* op);
void emit_int_operand(astree* op);
void emit_bin_arithmetic(astree* root);
symbol* lookup_symbol(const string* lexinfo);
astree* get_id(astree* vardecl_node);
#endif
