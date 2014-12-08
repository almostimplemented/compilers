#include "emit.h"

size_t regcnt;

string oil_type(symbol* sym) {
    string value = "";
    if (sym->attributes.test(ATTR_void)) {
        value.append("void");
        return value;
    }

    if (sym->attributes.test(ATTR_bool)
     || sym->attributes.test(ATTR_char)) {
        value.append("char");
    } else if (sym->attributes.test(ATTR_int)) {
        value.append("int");
    } else if (sym->attributes.test(ATTR_string)) {
        value.append("char*");
    } else if (sym->attributes.test(ATTR_struct)) {
        value.append("struct ");
        value.append(*(sym->struct_name));
        value.append("*");
    }

    if (sym->attributes.test(ATTR_array)) {
        value.append("*");
    }
    return value;
}

void emit_code(astree* root) {
    regcnt = 1;
    emit_structs();
    emit_stringcons();
    emit_vardecls();
    emit_functions();
    emit_statement(root);
}

void emit_structs() {
    for (symtab_it iterator =  struct_table->begin();
                   iterator != struct_table->end();
                   iterator++) {
        fprintf(oilfile, "struct s_%s {\n", iterator->first->c_str());
        symbol_table* field_table = iterator->second->fields;
        for (symtab_it f_iterator =  field_table->begin();
                       f_iterator != field_table->end();
                       f_iterator++) {
            fprintf(oilfile, "        ");
            fprintf(oilfile, "%s f_%s;\n",
                    oil_type(f_iterator->second).c_str(), 
                    f_iterator->first->c_str());
        }
        fprintf(oilfile, "}\n");
    }
}

void emit_stringcons() {
    for (int i = 1; !string_queue.empty(); i++) {
        astree* node  = string_queue.front();
        string_queue.pop();
        const string* value = node->lexinfo;
        fprintf(oilfile, "char* s%d = %s;\n", i, value->c_str());
    }
}

void emit_vardecls() {
    for (symtab_it iterator =  global_table->begin();
                   iterator != global_table->end();
                   iterator++) {
        if (iterator->second->attributes.test(ATTR_variable)) {
            fprintf(oilfile, "%s __%s;\n",
                    oil_type(iterator->second).c_str(),
                    iterator->first->c_str());
        }
    }
}

void emit_functions() {
    for (symtab_it iterator =  global_table->begin();
                   iterator != global_table->end();
                   iterator++) {
        if (iterator->second->attributes.test(ATTR_function)) {
            fprintf(oilfile, "%s __%s (",
                    oil_type(iterator->second).c_str(),
                    iterator->first->c_str());
            vector<symbol*>* params = iterator->second->parameters;
            for (unsigned i = 0; i < params->size(); i++) {
                fprintf(oilfile, "\n");
                fprintf(oilfile, "        ");
                fprintf(oilfile, "%s _%lu_%s",
                        oil_type(params->at(i)).c_str(),
                        params->at(i)->blocknr,
                        params->at(i)->node->lexinfo->c_str());
                if (i < params->size() - 1)
                    fprintf(oilfile, ",");
            }
            fprintf(oilfile, ")\n");
            fprintf(oilfile, "{\n");
            // emit_block
            fprintf(oilfile, "}\n");
        }
    }
}

void emit_statement(astree* root) {
    switch (root->symbol) {
        case TOK_ROOT:
        case TOK_BLOCK:
            for (size_t child = 0; child < root->children.size(); child++) {
                emit_statement(root->children[child]);
            }
        case TOK_WHILE:
            emit_while(root);
            break;
        case TOK_IFELSE:
            emit_ifelse(root);
            break;
        case TOK_IF:
            emit_if(root);
            break;
        case TOK_RETURN:
            emit_return(root);
            break;
        case TOK_VARDECL:
            emit_vardecl(root);
            break;
        default:
            emit_expression(root);
    }
}

void emit_expression(astree* root) {
    for (size_t child = 0; child < root->children.size(); child++) {
        emit_expression(root->children[child]);
    }
    switch (root->symbol) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
            emit_bin_arithmetic(root);
            break;
        case '=':
            emit_assignment(root);
            break;
        case TOK_EQ:
        case TOK_NE:
        case TOK_LT:
        case TOK_LE:
        case TOK_GT:
        case TOK_GE:
            emit_compare(root);
            break;
        case TOK_ORD:
            emit_ord(root);
            break;
        case TOK_CHR:
            emit_chr(root);
            break;
        case TOK_POS:
        case TOK_NEG:
            emit_sign(root);
            break;
        case TOK_NEW:
            emit_new(root);
            break;
        case TOK_NEWARRAY:
            emit_newarray(root);
            break;
        case TOK_NEWSTRING:
            emit_newstring(root);
            break;
        case TOK_INDEX:
            emit_index(root);
            break;
        case '.':
            emit_select(root);
            break;
        case TOK_CALL:
            emit_call(root);
            break;
    }
}


void emit_while(astree* root){
    return;
}
void emit_ifelse(astree* root){
    return;
}
void emit_if(astree* root){
    return;
}
void emit_return(astree* root){
    return;
}

void emit_vardecl(astree* root) {
    astree* ident = get_id(root);
    astree* expr  = root->children.at(1);
    symbol* sym   = ident->symptr;
    if (expr->attributes.test(ATTR_vreg) 
     || expr->attributes.test(ATTR_vaddr))
        emit_expression(expr);

    if (sym->blocknr > 0) {
        fprintf(oilfile, "%s _%lu_%s = ",
                oil_type(sym).c_str(),
                sym->blocknr,
                ident->lexinfo->c_str());
    } else {
        fprintf(oilfile, "__%s = ", ident->lexinfo->c_str());
    }
    emit_operand(expr);
    fprintf(oilfile, ";\n");
}

astree* get_id(astree* vardecl_node) {
    astree* type_node = vardecl_node->children.at(0);
    if (type_node->symbol == TOK_ARRAY) {
        return type_node->children.at(1);
    } else {
        return type_node->children.at(0);
    }
}

void emit_assignment(astree* root) {
    astree* ident = root->children.at(0);
    astree* expr  = root->children.at(1);
    symbol* sym   = ident->symptr;
    fprintf(oilfile, "_");
    if (sym->blocknr > 0)
        fprintf(oilfile, "%lu", sym->blocknr);
    fprintf(oilfile, "_%s = ", ident->lexinfo->c_str());
    emit_operand(expr);
    fprintf(oilfile, ";\n");
}

void emit_bin_arithmetic(astree* root) {
    astree* op1 = root->children.at(0);
    astree* op2 = root->children.at(1);
    fprintf(oilfile, "int i%lu = ", regcnt);
    root->regnr = regcnt; regcnt++;
    emit_int_operand(op1);
    fprintf(oilfile, " %s ", root->lexinfo->c_str());
    emit_int_operand(op2);
    fprintf(oilfile, ";\n");
}

void emit_operand(astree* op) {
    if (op->attributes.test(ATTR_int))
        emit_int_operand(op);
}

void emit_int_operand(astree* op) {
    if (op->attributes.test(ATTR_vreg)) {
        fprintf(oilfile, "i%lu", op->regnr);
    } else if (op->attributes.test(ATTR_variable)) {
        symbol* sym = op->symptr;
        fprintf(oilfile, "_");
        if (sym->blocknr > 0)
            fprintf(oilfile, "%lu", sym->blocknr);
        fprintf(oilfile, "_%s", op->lexinfo->c_str());
    } else if (op->attributes.test(ATTR_const)) {
        string value = "";
        value.append(*(op->lexinfo));
        while (value.at(0) == '0' && value.compare("0") != 0)
            value.erase(0, 1);
        fprintf(oilfile, "%s", value.c_str());
    }
}


void emit_compare(astree* root){
    return;
}
void emit_ord(astree* root){
    return;
}
void emit_chr(astree* root){
    return;
}
void emit_sign(astree* root){
    return;
}
void emit_new(astree* root){
    return;
}
void emit_newarray(astree* root){
    return;
}
void emit_newstring(astree* root){
    return;
}
void emit_index(astree* root){
    return;
}
void emit_select(astree* root){
    return;
}
void emit_call(astree* root){
    return;
}
