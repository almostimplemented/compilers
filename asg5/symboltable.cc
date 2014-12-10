#include "symboltable.h"
#include "astree.h"
#include "lyutils.h"

vector<symbol_table*> symbol_stack;
symbol_table* struct_table = new symbol_table();
int next_block = 1;
vector<int> blocknr_stack(1,0);
astree* current_function = nullptr;
queue<astree*> string_queue;
queue<symbol*> function_queue;
symbol_table* global_table;

symbol* new_symbol(astree* node) {
    symbol* sym = new symbol();
    sym->attributes = node->attributes;
    sym->filenr     = node->filenr;
    sym->linenr     = node->linenr;
    sym->offset     = node->offset;
    sym->blocknr    = node->blocknr;
    sym->parameters = nullptr;
    sym->block      = nullptr;
    sym->struct_name = node->struct_name;
    sym->fields     = node->fields;
    sym->node       = node;
    node->symptr    = sym;
    return sym;
}

/* Inserts entry into specified symbol table */
bool add_symbol (symbol_table *table, symbol_entry entry) {
    if (table == nullptr) {
        printf("New table\n");
        table = new symbol_table();
    }
    symbol* sym = entry.second;
    if (table == symbol_stack.back()) {
        for (unsigned i = 0; i < blocknr_stack.size() - 1; i++)
            fprintf(symfile, "   ");
    }
    fprintf(symfile, "%s (%lu.%lu.%lu) {%lu} ",
        entry.first->c_str(), sym->filenr,
        sym->linenr, sym->offset, sym->blocknr);
    if (sym->attributes.test(ATTR_void))
        fprintf(symfile, "void ");
    if (sym->attributes.test(ATTR_int))
        fprintf(symfile, "int ");
    if (sym->attributes.test(ATTR_char))
        fprintf(symfile, "char ");
    if (sym->attributes.test(ATTR_bool))
        fprintf(symfile, "bool ");
    if (sym->attributes.test(ATTR_string))
        fprintf(symfile, "string ");
    if (sym->attributes.test(ATTR_null))
        fprintf(symfile, "null ");
    if (sym->attributes.test(ATTR_array))
        fprintf(symfile, "array ");
    if (sym->attributes.test(ATTR_struct)) {
        fprintf(symfile, "struct ");
        if (sym->struct_name != nullptr)
            fprintf(symfile, "\"%s\" ", sym->struct_name->c_str());
    }
    if (sym->attributes.test(ATTR_function))
        fprintf(symfile, "function ");
    if (sym->attributes.test(ATTR_prototype))
        fprintf(symfile, "prototype ");
    if (sym->attributes.test(ATTR_variable))
        fprintf(symfile, "variable ");
    if (sym->attributes.test(ATTR_field))
        fprintf(symfile, "field ");
    if (sym->attributes.test(ATTR_typeid))
        fprintf(symfile, "typeid ");
    if (sym->attributes.test(ATTR_param))
        fprintf(symfile, "param ");
    if (sym->attributes.test(ATTR_lval))
        fprintf(symfile, "lval ");

    fprintf(symfile, "\n");
    return table->insert(entry).second;
}

/* Enter new nested block */
void traverse_block(astree* block_node) {
    enter_block(block_node);
    for (size_t child = 0; 
                child < block_node->children.size(); 
                child++) {
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
    if (root->symbol == TOK_FUNCTION) {
        astree* type_node = root->children.at(0);
        if (type_node->symbol == TOK_ARRAY) {
            current_function = type_node->children.at(1);
        } else {
            current_function = type_node->children.at(0);
        }
    }
    switch (root->symbol) {
        case TOK_ROOT:
            symbol_stack.push_back(new symbol_table());
            global_table = symbol_stack.back();
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
    }
    for (size_t child = 0; child < root->children.size(); child++) {
        traverse_astree(root->children[child]);
    }
    root->blocknr = blocknr_stack.back();
    switch (root->symbol) {
        case '=':
            typecheck_asg(root);
            break;
        case TOK_RETURN:
            typecheck_return(root);
            break;
        case TOK_RETURNVOID:
            typecheck_returnvoid(root);
            break;
        case TOK_EQ:
        case TOK_NE:
            typecheck_eq(root);
            break;
        case TOK_LT:
        case TOK_LE:
        case TOK_GT:
        case TOK_GE:
            typecheck_comp(root);
            break;
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
            typecheck_bin_arithmetic(root);
            break;
        case TOK_POS:
        case TOK_NEG:
            typecheck_un_arithmetic(root);
            break;
        case '!':
            typecheck_negate(root);
            break;
        case TOK_ORD:
            typecheck_ord(root);
            break;
        case TOK_CHR:
            typecheck_chr(root);
            break;
        case '.':
            typecheck_select(root);
            break;
        case TOK_NEW:
            typecheck_new(root);
            break;
        case TOK_NEWSTRING:
            root->attributes.set(ATTR_string);
            root->attributes.set(ATTR_vreg);
            break;
        case TOK_NEWARRAY:
            typecheck_array(root);
            break;
        case TOK_CALL:
            typecheck_call(root);
            break;
        case TOK_IDENT:
            typecheck_var(root);
            break;
        case TOK_INDEX:
            typecheck_index(root);
            break;
        case TOK_INTCON:
            root->attributes.set(ATTR_const);
            root->attributes.set(ATTR_int);
            break;
        case TOK_CHARCON:
            root->attributes.set(ATTR_const);
            root->attributes.set(ATTR_char);
            break;
        case TOK_STRINGCON:
            root->attributes.set(ATTR_const);
            root->attributes.set(ATTR_string);
            string_queue.push(root);
            break;
        case TOK_FALSE:
        case TOK_TRUE:
            root->attributes.set(ATTR_const);
            root->attributes.set(ATTR_bool);
        case TOK_NULL:
            root->attributes.set(ATTR_const);
            root->attributes.set(ATTR_null);
            break;
        case TOK_WHILE:
        case TOK_IF:
            typecheck_ifwhile(root);
            break;
        case TOK_FIELD:
            root->attributes.set(ATTR_field);
    }
}

astree* id_type_attr(astree* type_node) {
    astree* id_node;
    int type;
    if (type_node->symbol == TOK_ARRAY) {
        id_node   = type_node->children.at(1);
        type_node = type_node->children.at(0);
        type_node->blocknr = blocknr_stack.back();
        id_node->attributes.set(ATTR_array);
    } else {
        id_node = type_node->children.at(0);
    }
    type = type_node->symbol;
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
        case TOK_VOID:
            id_node->attributes.set(ATTR_void);
            break;
        case TOK_TYPEID:
            lookup_typeid(type_node);
            symbol_table::const_iterator got;
            got = struct_table->find(type_node->lexinfo);
            id_node->struct_name = type_node->lexinfo;
            id_node->fields = got->second->fields;
            id_node->attributes.set(ATTR_struct);
            break;
    }
    return id_node;
}

symbol_entry create_struct_entry(astree* struct_node) {
    symbol *sym;
    symbol_entry entry;
    astree* typeid_node  = struct_node->children.at(0);
    struct_node->blocknr = 0;
    typeid_node->blocknr = 0;
    typeid_node->struct_name = typeid_node->lexinfo;
    typeid_node->attributes.set(ATTR_typeid);
    sym = new_symbol(typeid_node);
    sym->fields = new symbol_table();
    entry = make_pair(typeid_node->lexinfo, sym);
    add_symbol(struct_table, entry);
    for (size_t child = 1; 
                child < struct_node->children.size();
                child++) {
        astree* field = struct_node->children.at(child);
        fprintf(symfile, "   ");
        add_symbol(sym->fields, create_field_entry(field));
    }
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
    vardecl_check(id_node);
    entry = make_pair(id_node->lexinfo, sym);
    add_symbol(symbol_stack.back(), entry);
    traverse_astree(vardecl_node->children.at(1));
    if (!types_compatible(vardecl_node->children.at(1),
                id_node)) {
        fprintf(stderr, "Incompatible types\n");
        fprintf(stderr, "location (%lu.%lu.%lu)\n",
                id_node->filenr, id_node->linenr, id_node->offset);
    }
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
    sym->parameters = new vector<symbol*>();
    if (param_node->children.size() > 0) {
        enter_block(param_node);
        for (size_t child = 0; 
                    child < param_node->children.size(); 
                    child++) {
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
    }
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
    sym->parameters = new vector<symbol*>();
    sym->block = block_node;
    function_queue.push(sym);
    entry = make_pair(id_node->lexinfo, sym);
    enter_block(param_node);
    for (size_t child = 0; 
                child < param_node->children.size(); 
                child++) {
        ptype_node = param_node->children.at(child);
        pid_node   = id_type_attr(ptype_node);
        pid_node->attributes.set(ATTR_variable);
        pid_node->attributes.set(ATTR_lval);
        pid_node->attributes.set(ATTR_param);
        ptype_node->blocknr = blocknr_stack.back();
        pid_node->blocknr   = blocknr_stack.back();
        psym = new_symbol(pid_node);
        sym->parameters->push_back(psym);
        add_symbol(symbol_stack.back(), make_pair(pid_node->lexinfo,
                                                  psym));
    }
    function_check(id_node->lexinfo, sym);
    add_symbol(symbol_stack.front(), entry);
    for (size_t child = 0;
                child < block_node->children.size();
                child++) {
        traverse_astree(block_node->children[child]);
    }
    exit_block();
    current_function = nullptr;
    return entry;
}

void lookup_typeid(astree* node) {
    const string* lexinfo = node->lexinfo;
    symbol_table::const_iterator got = struct_table->find(lexinfo);
    symbol *sym;
    if (got == struct_table->end()) {
        sym = new_symbol(node);
        add_symbol(struct_table, make_pair(lexinfo, sym));
    }
    node->attributes.set(ATTR_typeid);
}

void vardecl_check(astree* root) {
    symbol_table* table = symbol_stack.back();
    symbol_table::const_iterator got = table->find(root->lexinfo);
    if (got == symbol_stack.back()->end()) {
        return;
    } else {
        fprintf(stderr, "redefinition of '%s'\n",
                root->lexinfo->c_str());
        fprintf(stderr, "location (%lu.%lu.%lu)\n",
                root->filenr, root->linenr, root->offset);
    }
}

void proto_check(const string* lexinfo, symbol* p_sym) {
    symbol* sym;
    symbol_table* table = symbol_stack.back();
    symbol_table::const_iterator got = table->find(lexinfo);
    if (got == symbol_stack.back()->end()) {
        return;
    }
    sym = got->second;
    attr_bitset attributes(sym->attributes);
    if (attributes.test(ATTR_prototype)) {
        if (p_sym->parameters->size() != sym->parameters->size()) {
            /* PARAMETER ERROR */
            fprintf(stderr, "conflicting types for '%s'\n", 
                    lexinfo->c_str());
            fprintf(stderr, "location (%lu.%lu.%lu)\n",
                    p_sym->filenr, p_sym->linenr, p_sym->offset);
            return;
        } else if (attributes != p_sym->attributes) {
            /* TYPE ERROR */
            fprintf(stderr, "conflicting types for '%s'\n", 
                    lexinfo->c_str());
            fprintf(stderr, "location (%lu.%lu.%lu)\n",
                    p_sym->filenr, p_sym->linenr, p_sym->offset);
            return;
        }
        for (unsigned i = 0; i < p_sym->parameters->size(); i++) {
            if (p_sym->parameters->at(i)->attributes 
                    != sym->parameters->at(i)->attributes) {
                /* PARAMTER ERROR */
                fprintf(stderr, "conflicting types for '%s'\n",
                        lexinfo->c_str());
                fprintf(stderr, "location (%lu.%lu.%lu)\n",
                        p_sym->filenr, p_sym->linenr, p_sym->offset);
                return;
            }
        }
    } else if (attributes.test(ATTR_function)) {
        if (p_sym->parameters->size() != sym->parameters->size()) {
            /* PARAMETER ERROR */
            fprintf(stderr, "conflicting types for '%s'\n", 
                    lexinfo->c_str());
            fprintf(stderr, "location (%lu.%lu.%lu)\n",
                    p_sym->filenr, p_sym->linenr, p_sym->offset);
            return;
        }
        attributes.set(ATTR_function, 0);
        attributes.set(ATTR_prototype);
        if (attributes != p_sym->attributes) {
            /* TYPE ERROR */
            fprintf(stderr, "conflicting types for '%s'\n",
                    lexinfo->c_str());
            fprintf(stderr, "location (%lu.%lu.%lu)\n",
                    p_sym->filenr, p_sym->linenr, p_sym->offset);
            return;
        }
        for (unsigned i = 0; i < p_sym->parameters->size(); i++) {
            if (p_sym->parameters->at(i)->attributes 
                    != sym->parameters->at(i)->attributes) {
                /* PARAMTER ERROR */
                fprintf(stderr, "conflicting types for '%s'\n",
                        lexinfo->c_str());
                fprintf(stderr, "location (%lu.%lu.%lu)\n",
                        p_sym->filenr, p_sym->linenr, p_sym->offset);
                return;
            }
        }
    } else {
        /* REDEFINITION ERROR */
        fprintf(stderr, "redefinition of '%s'", lexinfo->c_str());
        fprintf(stderr, "location (%lu.%lu.%lu)\n",
                p_sym->filenr, p_sym->linenr, p_sym->offset);
    }
}

void function_check(const string* lexinfo, symbol* f_sym) {
    symbol* sym;
    symbol_table* table = symbol_stack.back();
    symbol_table::const_iterator got = table->find(lexinfo);
    if (got == symbol_stack.back()->end()) {
        return;
    }
    sym = got->second;
    attr_bitset attributes(sym->attributes);
    if (attributes.test(ATTR_prototype)) {
        if (f_sym->parameters->size() != sym->parameters->size()) {
            /* PARAMETER ERROR */
            fprintf(stderr, "conflicting types for '%s'\n",
                    lexinfo->c_str());
            fprintf(stderr, "location (%lu.%lu.%lu)\n",
                    f_sym->filenr, f_sym->linenr, f_sym->offset);
            return;
        }
        attributes.set(ATTR_prototype, 0);
        attributes.set(ATTR_function);
        if (attributes != f_sym->attributes) {
            /* TYPE ERROR */
            fprintf(stderr, "conflicting types for '%s'\n",
                    lexinfo->c_str());
            fprintf(stderr, "location (%lu.%lu.%lu)\n",
                    f_sym->filenr, f_sym->linenr, f_sym->offset);
            return;
        }
        for (unsigned i = 0; i < f_sym->parameters->size(); i++) {
            if (f_sym->parameters->at(i)->attributes 
                    != sym->parameters->at(i)->attributes) {
                /* PARAMTER ERROR */
                fprintf(stderr, "conflicting types for '%s'\n",
                        lexinfo->c_str());
                fprintf(stderr, "location (%lu.%lu.%lu)\n",
                        f_sym->filenr, f_sym->linenr, f_sym->offset);
                return;
            }
        }
    } else {
        /* REDEFINITION ERROR */
        fprintf(stderr, "redefinition of '%s'", lexinfo->c_str());
        fprintf(stderr, "location (%lu.%lu.%lu)\n",
                f_sym->filenr, f_sym->linenr, f_sym->offset);
    }
}

bool types_compatible(astree* n1, astree* n2) {
    attr_bitset attr1 = n1->attributes;
    attr_bitset attr2 = n2->attributes;
    if ((attr1.test(ATTR_null) &&  (attr2.test(ATTR_string)
                                 || attr2.test(ATTR_struct)
                                 || attr2.test(ATTR_array)))
      || (attr2.test(ATTR_null) &&  (attr1.test(ATTR_string)
                                 || attr1.test(ATTR_struct)
                                 || attr1.test(ATTR_array))))
        return true;
    if ((attr1.test(ATTR_array) && !attr2.test(ATTR_array))
      || (attr1.test(ATTR_array) && !attr2.test(ATTR_array)))
        return false;
    if (attr1.test(ATTR_int)
       && attr2.test(ATTR_int)) {
        return true;
    } else if (attr1.test(ATTR_char)
       && attr2.test(ATTR_char)) {
        return true;
    } else if (attr1.test(ATTR_string)
       && attr2.test(ATTR_string)) {
        return true;
    } else if (attr1.test(ATTR_bool)
       && attr2.test(ATTR_bool)) {
        return true;
    } else if (attr1.test(ATTR_struct)
       && attr2.test(ATTR_struct)) {
        if (n1->struct_name == n2->struct_name)
            return true;
    }
    return false;
}

bool types_compatible(astree* n, symbol* s) {
    attr_bitset attr1 = n->attributes;
    attr_bitset attr2 = s->attributes;
    if ((attr1.test(ATTR_null) &&  (attr2.test(ATTR_string)
                                 || attr2.test(ATTR_struct)
                                 || attr2.test(ATTR_array)))
      || (attr2.test(ATTR_null) &&  (attr1.test(ATTR_string)
                                 || attr1.test(ATTR_struct)
                                 || attr1.test(ATTR_array))))
        return true;
    if ((attr1.test(ATTR_array) && !attr2.test(ATTR_array))
      || (attr1.test(ATTR_array) && !attr2.test(ATTR_array)))
        return false;
    if (attr1.test(ATTR_int)
       && attr2.test(ATTR_int)) {
        return true;
    } else if (attr1.test(ATTR_char)
       && attr2.test(ATTR_char)) {
        return true;
    } else if (attr1.test(ATTR_string)
       && attr2.test(ATTR_string)) {
        return true;
    } else if (attr1.test(ATTR_bool)
       && attr2.test(ATTR_bool)) {
        return true;
    } else if (attr1.test(ATTR_struct)
       && attr2.test(ATTR_struct)) {
        if (n->struct_name == s->struct_name)
            return true;
    }
    return false;
}

bool type_exists(const string* struct_name) {
    symbol_table::const_iterator got = struct_table->find(struct_name);
    if (got == struct_table->end())
        return false;
    return true;
}

void typecheck_vardecl(astree* root) { 
    astree* id_node;
    astree* type_node = root->children.at(0);
    astree* expr_node = root->children.at(1);
    attr_bitset type_bits;

    if (type_node->symbol == TOK_ARRAY) {
        id_node = type_node->children.at(1);
    } else {
        id_node = type_node->children.at(0);
    }

    if (expr_node->attributes.test(ATTR_null)) {
        if (id_node->attributes.test(ATTR_string)
         || id_node->attributes.test(ATTR_struct)
         || id_node->attributes.test(ATTR_array)) {
            return;
        } else {
            fprintf(stderr, "null assignment to non-reference type.\n");
            fprintf(stderr, "location (%lu.%lu.%lu)\n",
                    root->filenr, root->linenr, root->offset);
        }
    } else if (expr_node->attributes.test(ATTR_void)) {
        fprintf(stderr, "void assignment\n");
        fprintf(stderr, "location (%lu.%lu.%lu)\n",
                root->filenr, root->linenr, root->offset);
    } else if (types_compatible(expr_node, id_node)) {
        return;
    }
    fprintf(stderr, "type error in declaration\n");
    fprintf(stderr, "location (%lu.%lu.%lu)\n",
            root->filenr, root->linenr, root->offset);
}
void typecheck_asg(astree* root) { 
    if (!root->children.at(0)->attributes.test(ATTR_lval)) {
        fprintf(stderr, "assignment to immutable element\n");
        fprintf(stderr, "location (%lu.%lu.%lu)\n",
                root->filenr, root->linenr, root->offset);
        return;
    }
    if (!types_compatible(root->children.at(0),
                        root->children.at(1))) {
        fprintf(stderr, "incompatible assignment\n");
        fprintf(stderr, "location (%lu.%lu.%lu)\n",
                root->filenr, root->linenr, root->offset);
        return;
    }
    root->attributes = attr_bitset(root->children.at(0)->attributes);
    root->attributes.set(ATTR_lval, 0);
    root->attributes.set(ATTR_variable, 0);
    root->attributes.set(ATTR_vreg);
    root->attributes.set(ATTR_vaddr, 0);
}
void typecheck_return(astree* root) {
    if (current_function == nullptr) {
        fprintf(stderr, "non-void global return\n");
        fprintf(stderr, "location (%lu.%lu.%lu)\n",
                root->filenr, root->linenr, root->offset);
        return;
    }
    astree* expr_node = root->children.at(0);
    if (expr_node->attributes.test(ATTR_null)) {
        if (current_function->attributes.test(ATTR_string)
         || current_function->attributes.test(ATTR_struct)
         || current_function->attributes.test(ATTR_array))
            return;
    } else if (types_compatible(expr_node,
             current_function)) {
        return;
    } else {
        fprintf(stderr, "return type does not match function type\n");
        fprintf(stderr, "location (%lu.%lu.%lu)\n",
                root->filenr, root->linenr, root->offset);
    }
}
void typecheck_returnvoid(astree* root) {
    if (current_function != nullptr) {
        if (current_function->attributes.test(ATTR_void)) {
            return;
        } else {
            fprintf(stderr, "return from nonvoid function\n");
            fprintf(stderr, "location (%lu.%lu.%lu)\n",
                    root->filenr, root->linenr, root->offset);
        }
    }
}
void typecheck_eq(astree* root) { 
    if (types_compatible(root->children.at(0), root->children.at(1))) {
        root->attributes.set(ATTR_bool);
        root->attributes.set(ATTR_vreg);
    } else {
        fprintf(stderr, "incompatible types for comparison\n");
        fprintf(stderr, "location (%lu.%lu.%lu)\n",
                root->filenr, root->linenr, root->offset);
    }
}
void typecheck_comp(astree* root) { 
    attr_bitset attr1 = root->children.at(0)->attributes;
    attr_bitset attr2 = root->children.at(1)->attributes;
    if (attr1.test(ATTR_int)
       && attr2.test(ATTR_int)) {
        root->attributes.set(ATTR_bool);
        root->attributes.set(ATTR_vreg);
        return;
    } else if (attr1.test(ATTR_char)
       && attr2.test(ATTR_char)) {
        root->attributes.set(ATTR_bool);
        root->attributes.set(ATTR_vreg);
        return;
    } else if (attr1.test(ATTR_bool)
       && attr2.test(ATTR_bool)) {
        root->attributes.set(ATTR_bool);
        root->attributes.set(ATTR_vreg);
        return;
    } else {
        fprintf(stderr, "incompatible types for comparison\n");
        fprintf(stderr, "location (%lu.%lu.%lu)\n", root->filenr,
                root->linenr, root->offset);
    }
}
void typecheck_bin_arithmetic(astree* root) { 
    if (root->children.at(0)->attributes.test(ATTR_int)
    &&  root->children.at(1)->attributes.test(ATTR_int)
    && !root->children.at(0)->attributes.test(ATTR_array)
    && !root->children.at(1)->attributes.test(ATTR_array)) {
        root->attributes.set(ATTR_vreg);
        root->attributes.set(ATTR_int);
        return;
    } else {
        fprintf(stderr, "incompatible types used for arithmetic\n");
        fprintf(stderr, "location (%lu.%lu.%lu)\n", root->filenr,
                root->linenr, root->offset);
    }
}
void typecheck_un_arithmetic(astree* root) { 
    if (root->children.at(0)->attributes.test(ATTR_int)
    && !root->children.at(0)->attributes.test(ATTR_array)) {
        root->attributes.set(ATTR_vreg);
        root->attributes.set(ATTR_int);
    } else {
        fprintf(stderr, "incompatible types used for arithmetic\n");
        fprintf(stderr, "location (%lu.%lu.%lu)\n", root->filenr,
                root->linenr, root->offset);
    }
    return; 
}
void typecheck_negate(astree* root) { 
    if (root->children.at(0)->attributes.test(ATTR_bool)
    && !root->children.at(0)->attributes.test(ATTR_array)) {
        root->attributes.set(ATTR_vreg);
        root->attributes.set(ATTR_bool);
    } else {
        fprintf(stderr, "cannot negate non-bool\n");
        fprintf(stderr, "location (%lu.%lu.%lu)\n", root->filenr,
                root->linenr, root->offset);
    }
    return; 
}
void typecheck_ord(astree* root) { 
    if (root->children.at(0)->attributes.test(ATTR_char)
    && !root->children.at(0)->attributes.test(ATTR_array)) {
        root->attributes.set(ATTR_vreg);
        root->attributes.set(ATTR_int);
    } else {
        fprintf(stderr, "invalid argument to 'ord'\n");
        fprintf(stderr, "location (%lu.%lu.%lu)\n", root->filenr,
                root->linenr, root->offset);
    }
    return; 
}
void typecheck_chr(astree* root) { 
    if (root->children.at(0)->attributes.test(ATTR_int)
    && !root->children.at(0)->attributes.test(ATTR_array)) {
        root->attributes.set(ATTR_vreg);
        root->attributes.set(ATTR_char);
    } else {
        fprintf(stderr, "invalid argument to 'chr'\n");
        fprintf(stderr, "location (%lu.%lu.%lu)\n", root->filenr,
                root->linenr, root->offset);
    }
    return; 
}
void typecheck_select(astree* root) { 
    astree* id_node = root->children.at(0);
    if (!id_node->attributes.test(ATTR_struct)) {
        fprintf(stderr, "accessing non-struct type\n");
        fprintf(stderr, "location (%lu.%lu.%lu)\n", root->filenr,
                root->linenr, root->offset);
        return;
    }
    astree* field_node = root->children.at(1);
    if (id_node->fields == nullptr) {
        fprintf(stderr, "accessing incomplete struct\n");
        fprintf(stderr, "location (%lu.%lu.%lu)\n", root->filenr,
                root->linenr, root->offset);
        return;
    }
    symbol_table::const_iterator got;
    got = id_node->fields->find(field_node->lexinfo);
    if (got == id_node->fields->end()) {
        fprintf(stderr, "accessing invalid struct field\n");
        fprintf(stderr, "location (%lu.%lu.%lu)\n", root->filenr,
                root->linenr, root->offset);
        return;
    }
    field_node->attributes = attr_bitset(got->second->attributes);
    root->attributes       = attr_bitset(got->second->attributes);
    root->attributes.set(ATTR_vaddr);
    root->attributes.set(ATTR_lval);
    root->attributes.set(ATTR_field, 0);
    if (root->attributes.test(ATTR_struct)) {
        root->struct_name = got->second->struct_name;
        root->fields = got->second->fields;
    }
    if (field_node->attributes.test(ATTR_struct)) {
        field_node->struct_name = got->second->struct_name;
        field_node->fields = got->second->fields;
    }
    return;
}
void typecheck_new(astree* root) { 
    astree* typeid_node = root->children.at(0);
    symbol_table::const_iterator got = 
        struct_table->find(typeid_node->lexinfo);
    if (got == struct_table->end()) {
        fprintf(stderr, "allocating incomplete struct\n");
        fprintf(stderr, "location (%lu.%lu.%lu)\n", root->filenr,
                root->linenr, root->offset);
        return;
    }
    root->attributes.set(ATTR_struct);
    root->attributes.set(ATTR_vreg);
    root->struct_name = typeid_node->lexinfo;
    root->fields = got->second->fields;
    return; 
}
void typecheck_array(astree* root) { 
    root->attributes.set(ATTR_array);
    root->attributes.set(ATTR_vreg);
    switch(root->children.at(0)->symbol) {
        case TOK_INT:
            root->attributes.set(ATTR_int);
            break;
        case TOK_CHAR:
            root->attributes.set(ATTR_char);
            break;
        case TOK_BOOL:
            root->attributes.set(ATTR_bool);
            break;
        case TOK_STRING:
            root->attributes.set(ATTR_string);
            break;
        case TOK_TYPEID: 
            if (type_exists(root->children.at(0)->lexinfo)) {
                root->attributes.set(ATTR_typeid);
                root->struct_name = root->children.at(0)->lexinfo;
            } else {
                fprintf(stderr, "unknown type '%s'", 
                        root->children.at(0)->lexinfo->c_str());
                fprintf(stderr, "location (%lu.%lu.%lu)\n",
                        root->filenr, root->linenr, root->offset);
            }
            break;
    }
}
void typecheck_call(astree* root) { 
    symbol_table *table;
    symbol_table::const_iterator got;
    for(int i = (int) symbol_stack.size() - 1; i >= 0; i--) {
        table = symbol_stack.at(i);
        got   = table->find(root->children.at(0)->lexinfo);
        if (got != table->end()) {
            vector<astree*> args    = root->children;
            vector<symbol*>* params = got->second->parameters;
            unsigned arg_len   = args.size() - 1;
            unsigned param_len = params->size();
            if (arg_len != param_len) {
                fprintf(stderr, "function has wrong dimensions\n");
                fprintf(stderr, "location (%lu.%lu.%lu)\n",
                    root->filenr, root->linenr, root->offset);
            } else {
                for (unsigned j = 0; j < param_len; j++) {
                    if(!types_compatible(args.at(j+1),
                                        params->at(j))) {
                        fprintf(stderr, "improper function call\n");
                        fprintf(stderr, "location (%lu.%lu.%lu)\n",
                            root->filenr, root->linenr, root->offset);
                    }
                }
            }
            root->attributes = attr_bitset(got->second->attributes);
            root->attributes.set(ATTR_vreg);
            if (root->attributes.test(ATTR_struct)) {
                root->struct_name = got->second->struct_name;
                root->fields = got->second->fields;
            }
            return;
        }
    }
    fprintf(stderr, "function not found\n");
    fprintf(stderr, "location (%lu.%lu.%lu)\n",
        root->filenr, root->linenr, root->offset);
}
void typecheck_var(astree* root) { 
    symbol_table *table;
    symbol_table::const_iterator got;
    for(int i = (int) symbol_stack.size() - 1; i >= 0; i--) {
        table = symbol_stack.at(i);
        got   = table->find(root->lexinfo);
        if (got != table->end()) {
            root->attributes = attr_bitset(got->second->attributes);
            root->symptr = got->second;
            if (root->attributes.test(ATTR_struct)) {
                root->fields = got->second->fields;
                root->struct_name = got->second->struct_name;
            }
            return;
        }
    }
    fprintf(stderr, "unknown identifier '%s'\n",
            root->lexinfo->c_str());
    fprintf(stderr, "location (%lu.%lu.%lu)\n", root->filenr,
            root->linenr, root->offset);
}
void typecheck_index(astree* root) { 
    if (root->children.at(0)->attributes.test(ATTR_bool)) {
        root->attributes.set(ATTR_bool);
    } else if (root->children.at(0)->attributes.test(ATTR_int)) { 
        root->attributes.set(ATTR_int);
    } else if (root->children.at(0)->attributes.test(ATTR_char)) {
        root->attributes.set(ATTR_char);
    } else if (root->children.at(0)->attributes.test(ATTR_string)) {
        if (root->children.at(0)->attributes.test(ATTR_array)) {
            root->attributes.set(ATTR_string);
        } else
            root->attributes.set(ATTR_char);
    } else if (root->children.at(0)->attributes.test(ATTR_struct)) {
        root->attributes.set(ATTR_struct);
        root->struct_name = root->children.at(0)->lexinfo;
        root->fields = root->children.at(0)->fields;
    }
    root->attributes.set(ATTR_vaddr);
    root->attributes.set(ATTR_lval);
}

void typecheck_ifwhile(astree* root) {
    if (!root->children.at(0)->attributes.test(ATTR_bool)) {
        fprintf(stderr, "control structure statement must be bool\n");
        fprintf(stderr, "location (%lu.%lu.%lu)\n", root->filenr,
                root->linenr, root->offset);
        return;
    }
}
