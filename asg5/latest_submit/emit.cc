#include "emit.h"

size_t regcnt;

string oil_type(symbol* sym);
string oil_type(astree* node);

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
        value.append("struct s_");
        value.append(*(sym->struct_name));
        value.append("*");
    }

    if (sym->attributes.test(ATTR_array)) {
        value.append("*");
    }
    return value;
}

string oil_type(astree* node) {
    string value = "";
    if (node->attributes.test(ATTR_void)) {
        value.append("void");
        return value;
    }

    if (node->attributes.test(ATTR_bool)
     || node->attributes.test(ATTR_char)) {
        value.append("char");
    } else if (node->attributes.test(ATTR_int)) {
        value.append("int");
    } else if (node->attributes.test(ATTR_string)) {
        value.append("char*");
    } else if (node->attributes.test(ATTR_struct)) {
        value.append("struct s_");
        value.append(*(node->struct_name));
        value.append("*");
    }

    if (node->attributes.test(ATTR_array)) {
        value.append("*");
    }
    return value;
}

void emit_code(astree* root) {
    regcnt = 1;
    emit_prologue();
    emit_structs();
    emit_stringcons();
    emit_vardecls();
    emit_functions();
    emit_statement(root);
}

void emit_prologue() {
    fprintf(oilfile, "#define __OCLIB_C__\n");
    fprintf(oilfile, "#include \"oclib.oh\"\n");
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
            fprintf(oilfile, "%s f_%s_%s;\n",
                    oil_type(f_iterator->second).c_str(), 
                    iterator->first->c_str(),
                    f_iterator->first->c_str());
        }
        fprintf(oilfile, "};\n");
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
    for (int i = 1; !function_queue.empty(); i++) {
        symbol *sym  = function_queue.front();
        function_queue.pop();
        fprintf(oilfile, "%s __%s (",
                oil_type(sym).c_str(),
                sym->node->lexinfo->c_str());
        vector<symbol*>* params = sym->parameters;
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
        emit_statement(sym->block);
        fprintf(oilfile, "}\n");
    }
}

void emit_statement(astree* root) {
    switch (root->symbol) {
        case TOK_FUNCTION:
            break;
        case TOK_ROOT:
            fprintf(oilfile, "void __ocmain (void)\n");
            fprintf(oilfile, "{\n");
            for (size_t child = 0; child < root->children.size(); child++) {
                emit_statement(root->children[child]);
            }
            fprintf(oilfile, "}\n");
            break;
        case TOK_BLOCK:
            for (size_t child = 0; child < root->children.size(); child++) {
                emit_statement(root->children[child]);
            }
            break;
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
        case TOK_RETURNVOID:
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
        case '!':
            emit_negate(root);
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
    fprintf(oilfile, "while_%lu_%lu_%lu:;\n",
            root->filenr, root->linenr, root->offset);
    emit_expression(root->children.at(0));
    fprintf(oilfile, "        ");
    fprintf(oilfile, "if (!");
    emit_operand(root->children.at(0));
    fprintf(oilfile, ") goto break_%lu_%lu_%lu;\n",
            root->filenr, root->linenr, root->offset);
    emit_statement(root->children.at(1));
    fprintf(oilfile, "        ");
    fprintf(oilfile, "goto while_%lu_%lu_%lu;\n",
            root->filenr, root->linenr, root->offset);
    fprintf(oilfile, "break_%lu_%lu_%lu:;\n",
            root->filenr, root->linenr, root->offset);
}
void emit_ifelse(astree* root){
    emit_expression(root->children.at(0));
    fprintf(oilfile, "        ");
    fprintf(oilfile, "if (!");
    emit_operand(root->children.at(0));
    fprintf(oilfile, ") goto else_%lu_%lu_%lu;\n",
            root->filenr, root->linenr, root->offset);
    emit_statement(root->children.at(1));
    fprintf(oilfile, "        ");
    fprintf(oilfile, "goto fi_%lu_%lu_%lu;\n",
            root->filenr, root->linenr, root->offset);
    fprintf(oilfile, "else_%lu_%lu_%lu:;\n",
            root->filenr, root->linenr, root->offset);
    emit_statement(root->children.at(2));
    fprintf(oilfile, "fi_%lu_%lu_%lu:;\n",
            root->filenr, root->linenr, root->offset);
}
void emit_if(astree* root){
    emit_expression(root->children.at(0));
    fprintf(oilfile, "        ");
    fprintf(oilfile, "if (!");
    emit_operand(root->children.at(0));
    fprintf(oilfile, ") goto fi_%lu_%lu_%lu;\n",
            root->filenr, root->linenr, root->offset);
    emit_statement(root->children.at(1));
    fprintf(oilfile, "fi_%lu_%lu_%lu:;\n",
            root->filenr, root->linenr, root->offset);
}
void emit_return(astree* root){
    fprintf(oilfile, "        ");
    if (root->symbol == TOK_RETURN) {
        emit_expression(root->children.at(0));
        fprintf(oilfile, "return ");
        emit_operand(root->children.at(0));
        fprintf(oilfile, ";\n");
    } else {
        fprintf(oilfile, "return;\n");
    }
}

void emit_vardecl(astree* root) {
    astree* ident = get_id(root);
    astree* expr  = root->children.at(1);
    symbol* sym   = ident->symptr;
    if (expr->attributes.test(ATTR_vreg) 
     || expr->attributes.test(ATTR_vaddr))
        emit_expression(expr);

    fprintf(oilfile, "        ");
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
    fprintf(oilfile, "        ");
    if (ident->attributes.test(ATTR_variable)) {
        symbol* sym   = ident->symptr;
        fprintf(oilfile, "_");
        if (sym->blocknr > 0)
            fprintf(oilfile, "%lu", sym->blocknr);
        fprintf(oilfile, "_%s = ", ident->lexinfo->c_str());
    } else {
        emit_operand(ident);
        fprintf(oilfile, " = ");
    }
    emit_operand(expr);
    fprintf(oilfile, ";\n");
}

void emit_bin_arithmetic(astree* root) {
    astree* op1 = root->children.at(0);
    astree* op2 = root->children.at(1);
    fprintf(oilfile, "        ");
    fprintf(oilfile, "int i%lu = ", regcnt);
    root->regnr = regcnt; regcnt++;
    emit_operand(op1);
    fprintf(oilfile, " %s ", root->lexinfo->c_str());
    emit_operand(op2);
    fprintf(oilfile, ";\n");
}

void emit_operand(astree* op) {
    if (op->attributes.test(ATTR_vreg)) {
        fprintf(oilfile, "%c%lu", 
                register_category(op),
                op->regnr);
    } else if (op->attributes.test(ATTR_vaddr)) {
        fprintf(oilfile, "*%c%lu", 
                register_category(op),
                op->regnr);
    } else if (op->attributes.test(ATTR_variable)) {
        symbol* sym = op->symptr;
        fprintf(oilfile, "_");
        if (sym->blocknr > 0)
            fprintf(oilfile, "%lu", sym->blocknr);
        fprintf(oilfile, "_%s", op->lexinfo->c_str());
    } else if (op->attributes.test(ATTR_const)) {
        string value = "";
        value.append(*(op->lexinfo));
        if (op->attributes.test(ATTR_int)) {
            while (value.at(0) == '0' && value.compare("0") != 0)
                value.erase(0, 1);
        }
        fprintf(oilfile, "%s", value.c_str());
    }
}

void emit_compare(astree* root){
    astree* op1 = root->children.at(0);
    astree* op2 = root->children.at(1);
    fprintf(oilfile, "        ");
    fprintf(oilfile, "char b%lu = ", regcnt);
    root->regnr = regcnt; regcnt++;
    emit_operand(op1);
    fprintf(oilfile, " %s ", root->lexinfo->c_str());
    emit_operand(op2);
    fprintf(oilfile, ";\n");
}
void emit_negate(astree* root){
    astree* op1 = root->children.at(0);
    fprintf(oilfile, "        ");
    fprintf(oilfile, "char b%lu = !", regcnt);
    root->regnr = regcnt; regcnt++;
    emit_operand(op1);
    fprintf(oilfile, ";\n");
}
void emit_ord(astree* root){
    fprintf(oilfile, "        ");
    fprintf(oilfile, "int i%lu = (int) ", regcnt);
    root->regnr = regcnt; regcnt++;
    emit_operand(root->children.at(0));
    fprintf(oilfile, ";\n");
}
void emit_chr(astree* root){
    fprintf(oilfile, "        ");
    fprintf(oilfile, "char c%lu = (char) ", regcnt);
    root->regnr = regcnt; regcnt++;
    emit_operand(root->children.at(0));
    fprintf(oilfile, ";\n");
}
void emit_sign(astree* root){
    fprintf(oilfile, "        ");
    if (root->symbol == TOK_POS)
        fprintf(oilfile, "int i%lu = +", regcnt);
    else 
        fprintf(oilfile, "int i%lu = -", regcnt);
    root->regnr = regcnt; regcnt++;
    emit_operand(root->children.at(0));
    fprintf(oilfile, ";\n");
}
void emit_new(astree* root){
    astree* type = root->children.at(0);
    fprintf(oilfile, "        ");
    fprintf(oilfile, "struct s_%s* p%lu = ", 
            type->lexinfo->c_str(), regcnt);
    root->regnr = regcnt; regcnt++;
    fprintf(oilfile, "xcalloc (1, sizeof (struct s_%s));\n",
            type->lexinfo->c_str());
}
void emit_newarray(astree* root){
    astree* type   = root->children.at(0);
    astree* size   = root->children.at(1);
    string oiltype = "";
    switch (type->symbol) {
        case TOK_INT:
            oiltype.append("int");
            break;
        case TOK_BOOL:
        case TOK_CHAR:
            oiltype.append("char");
            break;
        case TOK_STRING:
            oiltype.append("char*");
            break;
        case TOK_TYPEID:
            oiltype.append("struct s_");
            oiltype.append(*(type->struct_name));
            oiltype.append("*");
            break;
    }
    fprintf(oilfile, "        ");
    fprintf(oilfile, "%s* p%lu = ",
            oiltype.c_str(),
            regcnt);
    root->regnr = regcnt; regcnt++;
    fprintf(oilfile, "xcalloc (");
    emit_operand(size);
    fprintf(oilfile, ", sizeof (%s));\n",
            oiltype.c_str());
}
void emit_newstring(astree* root){
    astree* size = root->children.at(0);
    fprintf(oilfile, "        ");
    fprintf(oilfile, "char* p%lu = xcalloc (", regcnt);
    root->regnr = regcnt; regcnt++;
    emit_operand(size);
    fprintf(oilfile, ", sizeof (char));\n");
}
void emit_index(astree* root) {
    astree* ident = root->children.at(0);
    astree* index = root->children.at(1);
    fprintf(oilfile, "        ");
    fprintf(oilfile, "%s a%lu = ", 
            oil_type(ident).c_str(), regcnt);
    root->regnr = regcnt; regcnt++;
    if (ident->attributes.test(ATTR_vaddr)
     || (ident->attributes.test(ATTR_vreg)
      && ident->attributes.test(ATTR_array))) {
        fprintf(oilfile, "&a%lu", ident->regnr);
    } else {
        symbol* sym = ident->symptr;
        fprintf(oilfile, "&_");
        if (sym->blocknr > 0)
            fprintf(oilfile, "%lu", sym->blocknr);
        fprintf(oilfile, "_%s", ident->lexinfo->c_str());
    }

    fprintf(oilfile, "[");
    emit_operand(index);
    fprintf(oilfile, "]");
    fprintf(oilfile, ";\n");
}
void emit_select(astree* root){
    // create intermediate register
    astree* ident = root->children.at(0);
    astree* field = root->children.at(1);
    fprintf(oilfile, "        ");
    fprintf(oilfile, "%s* a%lu = ", 
            oil_type(field).c_str(), regcnt);
    root->regnr = regcnt; regcnt++;
    if (ident->attributes.test(ATTR_vaddr)) {
        fprintf(oilfile, "&((*a%lu)->f_%s_%s)", ident->regnr,
                ident->struct_name->c_str(),
                field->lexinfo->c_str());
    } else {
        symbol* sym = ident->symptr;
        fprintf(oilfile, "&_");
        if (sym->blocknr > 0)
            fprintf(oilfile, "%lu", sym->blocknr);
        fprintf(oilfile, "_%s", ident->lexinfo->c_str());
        fprintf(oilfile, "->f_%s_%s",
                ident->struct_name->c_str(),
                field->lexinfo->c_str());
    }
    fprintf(oilfile, ";\n");
}
void emit_call(astree* root){
    astree* ident = root->children.at(0);
    fprintf(oilfile, "        ");
    if (!root->attributes.test(ATTR_void)) {
        fprintf(oilfile, "%s %c%lu = ", 
                oil_type(ident).c_str(),
                register_category(ident),
                regcnt);
        root->regnr = regcnt; regcnt++;
    }
    fprintf(oilfile, "__%s(", ident->lexinfo->c_str());
    for (unsigned i = 1; i < root->children.size(); i++) {
        emit_operand(root->children.at(i));
        if (i < root->children.size() - 1)
            fprintf(oilfile, ",");
    }
    fprintf(oilfile, ");\n");
}
char register_category(astree* node) {
    if (node->symbol == TOK_NEW
     || node->symbol == TOK_NEWARRAY
     || node->symbol == TOK_NEWSTRING) {
        return 'p';
    }
    if (node->attributes.test(ATTR_vaddr)
     || node->attributes.test(ATTR_array)) {
        return 'a';
    }
    if (node->attributes.test(ATTR_int)) {
        return 'i';
    } else if (node->attributes.test(ATTR_char)) {
        return 'c';
    } else if (node->attributes.test(ATTR_bool)) {
        return 'b';
    } else {
        return 's';
    }
}
