// Author, Andrew Edwards, ancedwar@ucsc.edu
//

#ifndef __SYMBOLTABLE__
#define __SYMBOLTABLE__

#include <string>
#include <unordered_map>
#include <bitset>
#include <vector>
#include <queue>
using namespace std;
extern FILE *symfile;

enum { ATTR_void, ATTR_bool, ATTR_char, ATTR_int, ATTR_null,
       ATTR_string, ATTR_struct, ATTR_array, ATTR_function,
       ATTR_prototype, ATTR_variable, ATTR_field, ATTR_typeid,
       ATTR_param, ATTR_lval, ATTR_const, ATTR_vreg, ATTR_vaddr,
       ATTR_bitset_size,
};
using attr_bitset = bitset<ATTR_bitset_size>;

struct symbol;
using symbol_ptr   = symbol*;
using symbol_table = unordered_map<const string*,symbol*>;
using symbol_entry = pair<const string*,symbol*>;

struct astree;

struct symbol {
   attr_bitset attributes;
   symbol_table *fields;
   const string* struct_name;
   size_t filenr, linenr, offset;
   size_t blocknr;
   vector<symbol*>* parameters;
   astree* node;
   astree* block;
};

symbol* new_symbol(astree* node);
bool add_symbol(symbol_table table, symbol_entry entry);
void traverse_block();
void enter_block(astree* root);
void exit_block();
int traverse_astree(astree* root);
astree* id_type_attr(astree* node);
symbol_entry create_struct_entry(astree* struct_node);
symbol_entry create_field_entry(astree* field_node);
symbol_entry create_var_entry(astree* vardecl_node);
symbol_entry create_proto_entry(astree* proto_node);
symbol_entry create_func_entry(astree* func_node);
void lookup_typeid(astree* node);
void vardecl_check(astree* root);
void function_check(const string* lexinfo, symbol* sym);
void proto_check(const string* lexinfo, symbol* sym);
void typecheck(astree* root);
void typecheck_vardecl(astree* root);
void typecheck_return(astree* root);
void typecheck_returnvoid(astree* root);
void typecheck_asg(astree* root);
void typecheck_eq(astree* root);
void typecheck_comp(astree* root);
void typecheck_bin_arithmetic(astree* root);
void typecheck_un_arithmetic(astree* root);
void typecheck_negate(astree* root);
void typecheck_ord(astree* root);
void typecheck_chr(astree* root);
void typecheck_select(astree* root);
void typecheck_new(astree* root);
void typecheck_array(astree* root);
void typecheck_call(astree* root);
void typecheck_var(astree* root);
void typecheck_index(astree* root);
void typecheck_ifwhile(astree* root);
bool types_compatible(astree* n1, astree* n2);
bool types_compatible(astree* n, symbol* s);
bool types_compatible(attr_bitset attr1, attr_bitset attr2);
void destroy_symbols();
#endif
