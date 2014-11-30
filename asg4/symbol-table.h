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
       ATTR_variable, ATTR_field, ATTR_typeid, ATTR_param,
       ATTR_lval, ATTR_const, ATTR_vreg, ATTR_vaddr,
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
void enter_block();
void exit_block();
void traverse_astree(astree* root);
symbol* create_struct_sym(astree* struct_node);
symbol_entry create_field_entry(astree* field_node);
symbol* create_var_sym(astree* vardecl_node);
void visit(astree* root);
#endif
