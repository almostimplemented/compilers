#include "symbol-table.h"
#include "astree.h"
#include "lyutils.h"

vector<symbol_table*> symbol_stack;
symbol_table* struct_table;
int next_block = 1;
vector<int> blocknr_stack;

symbol* new_symbol(astree* node) {
    symbol* sym = new symbol();
    sym->attributes = node->attributes;
    sym->filenr     = node->filenr;
    sym->linenr     = node->linenr;
    sym->offset     = node->offset;
    sym->blocknr    = node->blocknr;
    sym->parameters = nullptr;
    sym->fields     = nullptr;
    return sym;
}

/* Inserts entry into specified symbol table */
bool add_symbol (symbol_table *table, symbol_entry entry) {
    if (table == nullptr) {
        table = new symbol_table();
    }
    return table->insert(entry).second;
}

/* Enter new nested block */
void enter_block(astree* block_node) {
    block_node->blocknr = blocknr_stack.back();
    symbol_stack.push_back(nullptr);
    blocknr_stack.push_back(next_block);
    next_block++;
}

/* Exit current nested block */ 
void exit_block() {
    /* Print out stuff */
    symbol_stack.pop_back();
    blocknr_stack.pop_back();
}

/*  Traverse astree and build symbol tables */ 
void traverse_astree(astree* root) {
    switch (root->symbol) {
        case TOK_STRUCT:
            create_struct_sym(root);
            break;
        case TOK_BLOCK:
            enter_block(root);
            break;
        case TOK_VARDECL:
            create_var_sym(root);
            break;
        case TOK_PROTOTYPE:
            break;
        case TOK_FUNCTION:
            break;
    }
    for (size_t child = 0; child < root->children.size(); ++child) {
        traverse_astree(root->children[child]);
    }
    visit(root);
}

void set_type_attr(astree* type_node) {
    astree* id_node = type_node->children.at(0);
    switch (type_node->symbol) {
        case TOK_BOOL:
            id_node->attributes.set(ATTR_bool);
            break;
        case TOK_CHAR:
            id_node->attributes.set(ATTR_char);
            break;
        case TOK_INT:
            id_node->attributes.set(ATTR_int);
            break;
        case TOK_STRING:
            id_node->attributes.set(ATTR_string);
            break;
        case TOK_TYPEID:
            id_node->attributes.set(ATTR_typeid);
            break;
    }
}

symbol* create_struct_sym(astree* struct_node) {
    symbol *sym;
    symbol_entry entry;
    struct_node->attributes.set(ATTR_struct);
    struct_node->blocknr = 0;
    sym = new_symbol(struct_node);
    for (size_t child = 1; 
                child < struct_node->children.size();
                child++) {
        astree* field = struct_node->children.at(child);
        add_symbol(sym->fields, create_field_entry(field));
    }
    entry = make_pair(struct_node->children.at(0)->lexinfo, sym);
    add_symbol(struct_table, entry);
    return sym;
}

symbol_entry create_field_entry(astree* field_node) {
    symbol *sym;
    symbol_entry entry;
    field_node->attributes.set(ATTR_field);
    field_node->blocknr = 0;
    set_type_attr(field_node);
    if (field_node->children.at(1)->symbol == TOK_ARRAY) {
        field_node->attributes.set(ATTR_array);
        sym   = new_symbol(field_node);
        entry = make_pair(field_node->children.at(2)->lexinfo, sym);
    } else {
        sym   = new_symbol(field_node);
        entry = make_pair(field_node->children.at(1)->lexinfo, sym);
    }
    return entry;
}


symbol* create_var_sym(astree* vardecl_node) {
    symbol* sym;
    symbol_entry entry;
    vardecl_node->blocknr = blocknr_stack.back();
    astree* var_node = vardecl_node->children.at(0);
    var_node->attributes.set(ATTR_variable);
    var_node->attributes.set(ATTR_lval);
    set_type_attr(var_node);
    sym = new_symbol(var_node);
}
/* Visit a node. Basically a large switch statement that assigns
 * attributes to the astree nodes, and builds symbol tables as
 * declarations are encountered. */
void visit(astree *root) {
    return;
}
