// Author, Andrew Edwards, ancedwar@ucsc.edu
// 
// NAME
//    oc - main program for the oc compiler
//
// SYNOPSIS
//    oc [-ly] [-@ <flags...>] [-D <string>] <program>.oc
//
// DESCRIPTION
//    At this stage the program preprocesses the input program,
//    does a number of preliminary checks, and initializes the
//    stringset, and dumps its contents into <program>.str
//

#include <string>
#include <fstream>
using namespace std;

#include <errno.h>
#include <libgen.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "auxlib.h"
#include "stringset.h"
#include "preprocess.h"

// Base of preprocessor pipe
const string CPP = "/usr/bin/cpp";

// Debugging variables 
// Need to make extern in later assignments
int yy_flex_debug = 0;
int yydebug = 0;

int main (int argc, char **argv) {
    set_execname (argv[0]);
    int opt, err = 0;
    string name;
    string cpp_opts = "", debugflags = "";
    // Process options
    while ((opt = getopt(argc, argv, "@:D:ly")) != -1) {
        switch (opt) {
            case '@':
                name = optarg;
                debugflags += name;
                set_debugflags(debugflags.c_str());
                break;
            case 'D':
                // Extra option for CPP
                name = optarg;
                cpp_opts += " -D " + name;
                break;
            case 'l':
                yy_flex_debug = 1;
                break;
            case 'y':
                yydebug = 1;
                break;
            case '?':
                err = 1;
                break;
        }
    }
    if (err) {
        syserrprintf(optarg);
    } else if ((optind + 1) > argc) {
        fprintf(stderr, "oc: error: no input file\n");
        exit(1);
    } else if ((optind + 1) < argc) {
        fprintf(stderr, "oc: error: more than one input file\n");
        exit(1);
    }

    // Make sure the file to compile exists and has .oc suffix
    char* filename = argv[optind];
    struct stat buffer;
    if (stat(filename, &buffer) != 0) {
        syserrprintf(filename);
        exit(1);
    } else {
        int length = strlen(filename);
        if (filename[length - 3] != '.' || \
            filename[length - 2] != 'o' || \
            filename[length - 1] != 'c') {
            fprintf(stderr, "oc: error: file must have .oc suffix\n");
            exit(1);
        }
    }

    // Preprocess the file and generate the string set
    string command = CPP + " " + cpp_opts + " " + filename;
    preprocess(command, filename);

    // Create <program>.str file
    ofstream outfile;
    set_localname(filename);
    string outname(get_localname());
    outfile.open(outname.substr(0, outname.length() - 3) + ".str");
    dump_stringset(outfile);
    return get_exitstatus();
}
