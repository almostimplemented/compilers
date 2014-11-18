// Author, Andrew Edwards, ancedwar@ucsc.edu
// 
// NAME
//    oc - main program for the oc compiler
//
// SYNOPSIS
//    oc [-ly] [-@ <flags...>] [-D <string>] <program>.oc
//
// DESCRIPTION
//    Part three of the ongoing compiler project.
//    Generates <prog>.ast, <prog>.tok, and <prog>.str
//    The first file is a visualization of the AST.
//    The second file shows the contents of each token.
//    The third file shows the contents of the maintained
//    dictionary of encountered tokens.
//

#include <string>
#include <vector>
#include <fstream>
using namespace std;

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "astree.h"
#include "auxlib.h"
#include "lyutils.h"
#include "stringset.h"

string cpp_command = "/usr/bin/cpp";
string filename = "";

FILE *astfile;
FILE *tokfile;
FILE *strfile;

// Open a pipe from the C preprocessor.
// Exit failure if can't.
// Assigns opened pipe to FILE* yyin.
void yyin_cpp_popen () {
    yyin = popen (cpp_command.c_str(), "r");
    if (yyin == NULL) {
        syserrprintf (cpp_command.c_str());
        exit (get_exitstatus());
    }
}

void yyin_cpp_pclose (void) {
    int pclose_rc = pclose (yyin);
    eprint_status (cpp_command.c_str(), pclose_rc);
    if (pclose_rc != 0) set_exitstatus (EXIT_FAILURE);
}

bool want_echo () {
    return not (isatty (fileno (stdin)) and isatty (fileno (stdout)));
}

void scan_opts (int argc, char** argv) {
    int option;
    opterr = 0;
    yy_flex_debug = 0;
    yydebug = 0;
    for(;;) {
        option = getopt (argc, argv, "@:D:ly");
        if (option == EOF) break;
        switch (option) {
            case '@': set_debugflags (optarg);   break;
            case 'D': cpp_command += " -D " + *optarg; 
                      break;
            case 'l': yy_flex_debug = 1;         break;
            case 'y': yydebug = 1;               break;
            default:  errprintf ("%:bad option (%c)\n", optopt); break;
        }
    }
    if (optind > argc) {
        errprintf ("Usage: %s [-@Dly] [filename]\n", get_execname());
        exit (get_exitstatus());
    }
    const char *fname = optind == argc ? "-" : argv[optind];

    // Ensure that file ends in .oc
    struct stat buffer;
    if (stat(fname, &buffer) != 0) {
        syserrprintf(fname);
        exit(get_exitstatus());
    } else {
        int length = strlen(fname);
        if (fname[length - 3] != '.' || \
                fname[length - 2] != 'o' || \
                fname[length - 1] != 'c') {
            fprintf(stderr, "oc: %s: file must have .oc suffix\n", fname);
            exit(get_exitstatus());
        }
    }
    // Copy over filename, remove suffix
    filename = string(basename(fname));
    filename = filename.substr(0, filename.length() - 3);

    // Open cpp pipe
    cpp_command += " "; 
    cpp_command += fname;
    yyin_cpp_popen();
    DEBUGF ('m', "filename = %s, yyin = %p, fileno (yyin) = %d\n",
            fname, yyin, fileno (yyin));
}

int main (int argc, char** argv) {
    set_execname (argv[0]);
    DEBUGSTMT ('m',
            for (int argi = 0; argi < argc; ++argi) {
            eprintf ("%s%c", argv[argi], argi < argc - 1 ? ' ' : '\n');
            }
            );
    // read in options
    scan_opts(argc, argv);


    // initialize output files 
    string strfilename = filename + ".str";
    strfile = fopen(strfilename.c_str(), "w");

    string tokfilename = filename + ".tok";
    tokfile = fopen(tokfilename.c_str(), "w");

    string astfilename = filename + ".ast";
    astfile = fopen(astfilename.c_str(), "w");

    // parse
    yyparse();
    yyin_cpp_pclose();

    // generate .str file
    dump_stringset(strfile);

    // generate .ast file
    dump_astree(astfile, yyparse_astree);
    free_ast (yyparse_astree);

    // close tokfile and strfile
    fclose(astfile);
    fclose(tokfile);
    fclose(strfile);

    yylex_destroy();
    return get_exitstatus();
}
