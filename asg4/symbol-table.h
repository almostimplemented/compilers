// Author, Andrew Edwards, ancedwar@ucsc.edu
//

#ifndef __SYMBOLTABLE__
#define __SYMBOLTABLE__

#include <string>
#include <unordered_map>
#include <bitset>
#include <vector>
using namespace std;

enum { ATTR_void, ATTR_bool, ATTR_char, ATTR_int, ATTR_null,
       ATTR_string, ATTR_struct, ATTR_array, ATTR_function,
       ATTR_prototype, ATTR_variable, ATTR_field, ATTR_typeid,
       ATTR_param, ATTR_lval, ATTR_const, ATTR_vreg, ATTR_vaddr,
       ATTR_bitset_size,
};
using attr_bitset = bitset<ATTR_bitset_size>;

struct symbol;
using symbol_table = unordered_map<const string*,symbol*>;
using symbol_entry = pair<const string*,symbol*>;

struct symbol {
   attr_bitset attributes;
   symbol_table *fields;
   size_t filenr, linenr, offset;
   size_t blocknr;
   vector<symbol*>* parameters;
};

struct astree;
symbol* new_symbol(astree* node);
bool add_symbol(symbol_table table, symbol_entry entry);
void traverse_block();
void enter_block(astree* root);
void exit_block();
void traverse_astree(astree* root);
astree* id_type_attr(astree* node);
symbol_entry create_struct_entry(astree* struct_node);
symbol_entry create_field_entry(astree* field_node);
symbol_entry create_var_entry(astree* vardecl_node);
symbol_entry create_proto_entry(astree* proto_node);
symbol_entry create_func_entry(astree* func_node);
void function_check(const string* lexinfo, symbol* sym);
void proto_check(const string* lexinfo, symbol* sym);
#endif
