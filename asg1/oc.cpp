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

const string CPP = "/usr/bin/cpp";

int main (int argc, char **argv) {
    set_execname (argv[0]);
    int opt, lflag = 0, yflag = 0, err = 0;
    string name;
    string cpp_opts = "", debugflags = "";
    while ((opt = getopt(argc, argv, "@:D:ly")) != -1) {
        switch (opt) {
            case '@':
                name = optarg;
                debugflags += name;
                set_debugflags(debugflags.c_str());
                break;
            case 'D':
                name = optarg;
                cpp_opts += "-D " + name;
                break;
            case 'l':
                lflag = 1;
                break;
            case 'y':
                yflag = 1;
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
    }

    // if (lflag) printf("lflag set\n"); 
    // if (yflag) printf("yflag set\n"); 

    char* filename = argv[optind];
    struct stat buffer;
    if (stat(filename, &buffer) != 0) {
        syserrprintf(filename);
        exit(1);
    } else {
        int length = strlen(filename);
        if (filename[length - 3] != '.' || filename[length - 2] != 'o' || filename[length - 1] != 'c') {
            fprintf(stderr, "oc: error: file must have .oc suffix\n");
        }
    }
    string command = CPP + " " + cpp_opts + " " + filename;
    FILE* pipe = popen (command.c_str(), "r");
    if (pipe == NULL) {
        syserrprintf(command.c_str());
        exit(1);
    } else {
        build_stringset(pipe, filename);
        int pclose_rc = pclose(pipe);
    }
    ofstream outfile;
    set_localname(filename);
    string outfilename(get_localname());
    outfile.open(outfilename.substr(0, outfilename.length() - 3) + ".str");
    dump_stringset(outfile);
    return get_exitstatus();
}
