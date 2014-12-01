#include "symbol-table.h"
#include "astree.h"
#include "lyutils.h"

vector<symbol_table*> symbol_stack;
symbol_table* struct_table = new symbol_table();
int next_block = 1;
vector<int> blocknr_stack(1,0);

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
        printf("New table\n");
        table = new symbol_table();
    }
    return table->insert(entry).second;
}

/* Enter new nested block */
void traverse_block(astree* block_node) {
    enter_block(block_node);
    for (size_t child = 0; child < block_node->children.size(); ++child) {
        traverse_astree(block_node->children[child]);
    }
    exit_block();
}

void enter_block(astree* block_node) {
    block_node->blocknr = blocknr_stack.back();
    symbol_stack.push_back(new symbol_table());
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
        case TOK_ROOT:
            symbol_stack.push_back(new symbol_table());
            break;
        case TOK_STRUCT:
            create_struct_entry(root);
            return;
        case TOK_BLOCK:
            traverse_block(root);
            return;
        case TOK_VARDECL:
            create_var_entry(root);
            return;
        case TOK_PROTOTYPE:
            create_proto_entry(root);
            return;
        case TOK_FUNCTION:
            create_func_entry(root);
            return;
        default:
            root->blocknr = blocknr_stack.back();
    }
    for (size_t child = 0; child < root->children.size(); ++child) {
        traverse_astree(root->children[child]);
    }
}

astree* id_type_attr(astree* type_node) {
    astree* id_node;
    int type;
    if (type_node->symbol == TOK_ARRAY) {
        type    = type_node->children.at(0)->symbol;
        id_node = type_node->children.at(1);
        id_node->attributes.set(ATTR_array);
    } else {
        type    = type_node->symbol;
        id_node = type_node->children.at(0);
    }
    switch (type) {
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
    return id_node;
}

symbol_entry create_struct_entry(astree* struct_node) {
    symbol *sym;
    symbol_entry entry;
    struct_node->attributes.set(ATTR_struct);
    astree* typeid_node  = struct_node->children.at(0);
    struct_node->blocknr = 0;
    typeid_node->blocknr = 0;
    sym = new_symbol(typeid_node);
    if (struct_node->children.size() > 0) 
        sym->fields = new symbol_table();
    for (size_t child = 1; 
                child < struct_node->children.size();
                child++) {
        astree* field = struct_node->children.at(child);
        add_symbol(sym->fields, create_field_entry(field));
    }
    entry = make_pair(typeid_node->lexinfo, sym);
    add_symbol(struct_table, entry);
    return entry;
}

symbol_entry create_field_entry(astree* field_node) {
    symbol *sym;
    symbol_entry entry;
    astree* id_node;
    id_node = id_type_attr(field_node);
    id_node->attributes.set(ATTR_field);
    field_node->blocknr = 0;
    id_node->blocknr    = 0;
    sym   = new_symbol(id_node);
    entry = make_pair(id_node->lexinfo, sym);
    return entry;
}


symbol_entry create_var_entry(astree* vardecl_node) {
    symbol* sym;
    symbol_entry entry;
    astree* type_node = vardecl_node->children.at(0);
    astree* id_node   = id_type_attr(type_node);
    id_node->attributes.set(ATTR_variable);
    id_node->attributes.set(ATTR_lval);
    vardecl_node->blocknr = blocknr_stack.back();
    type_node->blocknr    = blocknr_stack.back();
    id_node->blocknr      = blocknr_stack.back();
    sym   = new_symbol(id_node);
    entry = make_pair(id_node->lexinfo, sym);
    add_symbol(symbol_stack.back(), entry);
    return entry;
}

symbol_entry create_proto_entry(astree* proto_node) {
    symbol* sym;
    symbol* psym;
    symbol_entry entry;
    astree* pid_node;
    astree* ptype_node;
    astree* type_node   = proto_node->children.at(0);
    astree* param_node  = proto_node->children.at(1);
    astree* id_node     = id_type_attr(type_node);
    proto_node->blocknr = blocknr_stack.back();
    type_node->blocknr  = blocknr_stack.back();
    id_node->blocknr    = blocknr_stack.back();
    id_node->attributes.set(ATTR_prototype);
    sym = new_symbol(id_node);
    enter_block(param_node);
    sym->parameters = new vector<symbol*>();
    for (size_t child = 0; child < param_node->children.size(); ++child) {
        ptype_node = param_node->children.at(child);
        pid_node   = id_type_attr(ptype_node);
        pid_node->attributes.set(ATTR_variable);
        pid_node->attributes.set(ATTR_lval);
        pid_node->attributes.set(ATTR_param);
        ptype_node->blocknr = blocknr_stack.back();
        pid_node->blocknr   = blocknr_stack.back();
        psym = new_symbol(pid_node);
        sym->parameters->push_back(psym);
    }
    exit_block();
    proto_check(id_node->lexinfo, sym);
    entry = make_pair(id_node->lexinfo, sym);
    add_symbol(symbol_stack.back(), entry);
    return entry;
}

symbol_entry create_func_entry(astree* func_node) {
    symbol* sym;
    symbol* psym;
    symbol_entry entry;
    astree* pid_node;
    astree* ptype_node;
    astree* type_node   = func_node->children.at(0);
    astree* param_node  = func_node->children.at(1);
    astree* block_node  = func_node->children.at(2);
    astree* id_node     = id_type_attr(type_node);
    func_node->blocknr  = blocknr_stack.back();
    type_node->blocknr  = blocknr_stack.back();
    id_node->blocknr    = blocknr_stack.back();
    block_node->blocknr = blocknr_stack.back();
    id_node->attributes.set(ATTR_function);
    sym = new_symbol(id_node);
    enter_block(param_node);
    sym->parameters = new vector<symbol*>();
    for (size_t child = 0; child < param_node->children.size(); ++child) {
        ptype_node = param_node->children.at(child);
        pid_node   = id_type_attr(ptype_node);
        pid_node->attributes.set(ATTR_variable);
        pid_node->attributes.set(ATTR_lval);
        pid_node->attributes.set(ATTR_param);
        ptype_node->blocknr = blocknr_stack.back();
        pid_node->blocknr   = blocknr_stack.back();
        psym = new_symbol(pid_node);
        sym->parameters->push_back(psym);
    }
    for (size_t child = 0; child < block_node->children.size(); ++child) {
        traverse_astree(block_node->children[child]);
    }
    exit_block();
    function_check(id_node->lexinfo, sym);
    entry = make_pair(id_node->lexinfo, sym);
    add_symbol(symbol_stack.back(), entry);
    return entry;
}

void proto_check(const string* lexinfo, symbol* p_sym) {
    symbol* sym;
    symbol_table::const_iterator got = symbol_stack.back()->find(lexinfo);
    if (got == symbol_stack.front()->end()) {
        printf("Not found.\n");
        return;
    }
    sym = got->second;
    attr_bitset attributes(sym->attributes);
    if (attributes.test(ATTR_prototype)) {
        if (p_sym->parameters->size() != sym->parameters->size()) {
            /* PARAMETER ERROR */
            printf("conflicting types for '%s'\n", lexinfo->c_str());
            exit(3);
        } else if (attributes != p_sym->attributes) {
            /* TYPE ERROR */
            printf("conflicting types for '%s'\n", lexinfo->c_str());
            exit(2);
        }
        for (unsigned i = 0; i < p_sym->parameters->size(); i++) {
            if (p_sym->parameters->at(i)->attributes 
                    != sym->parameters->at(i)->attributes) {
                /* PARAMTER ERROR */
                printf("conflicting types for '%s'\n", lexinfo->c_str());
                exit(1);
            }
        }
    } else if (attributes.test(ATTR_function)) {
        if (p_sym->parameters->size() != sym->parameters->size()) {
            /* PARAMETER ERROR */
            printf("conflicting types for '%s'\n", lexinfo->c_str());
            exit(3);
        }
        attributes.set(ATTR_function, 0);
        attributes.set(ATTR_prototype);
        if (attributes != p_sym->attributes) {
            /* TYPE ERROR */
            printf("conflicting types for '%s'\n", lexinfo->c_str());
            exit(2);
        }
        for (unsigned i = 0; i < p_sym->parameters->size(); i++) {
            if (p_sym->parameters->at(i)->attributes 
                    != sym->parameters->at(i)->attributes) {
                /* PARAMTER ERROR */
                printf("conflicting types for '%s'\n", lexinfo->c_str());
                exit(1);
            }
        }
    } else {
        /* REDEFINITION ERROR */
        printf("redefinition of '%s'", lexinfo->c_str());
    }
}

void function_check(const string* lexinfo, symbol* f_sym) {
    symbol* sym;
    symbol_table::const_iterator got = symbol_stack.back()->find(lexinfo);
    if (got == symbol_stack.front()->end()) {
        printf("Not found.\n");
        return;
    }
    sym = got->second;
    attr_bitset attributes(sym->attributes);
    if (attributes.test(ATTR_prototype)) {
        if (f_sym->parameters->size() != sym->parameters->size()) {
            /* PARAMETER ERROR */
            printf("conflicting types for '%s'\n", lexinfo->c_str());
            exit(3);
        }
        attributes.set(ATTR_prototype, 0);
        attributes.set(ATTR_function);
        if (attributes != f_sym->attributes) {
            /* TYPE ERROR */
            printf("conflicting types for '%s'\n", lexinfo->c_str());
            exit(2);
        }
        for (unsigned i = 0; i < f_sym->parameters->size(); i++) {
            if (f_sym->parameters->at(i)->attributes 
                    != sym->parameters->at(i)->attributes) {
                /* PARAMTER ERROR */
                printf("conflicting types for '%s'\n", lexinfo->c_str());
                exit(1);
            }
        }
    } else {
        /* REDEFINITION ERROR */
        printf("Redefinition of %s", lexinfo->c_str());
    }
}
