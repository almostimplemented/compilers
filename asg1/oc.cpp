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

const string CPP = "/usr/bin/cpp";
const size_t LINESIZE = 1024;

void chomp(char *string, char delim) {
    size_t len = strlen(string);
    if (len == 0) return;
    char *nlpos = string + len - 1;
    if (*nlpos == delim) *nlpos = '\0';
}

void build_stringset(FILE* pipe, char* filename) {
    int linenr = 1;
    char inputname[LINESIZE];
    strcpy (inputname, filename);
    for (;;) {
        char buffer[LINESIZE];
        char* fgets_rc = fgets (buffer, LINESIZE, pipe);
        if (fgets_rc == NULL) break;
        chomp (buffer, '\n');
        // printf ("%s:line %d: [%s]\n", filename, linenr, buffer);
        int sscanf_rc = sscanf (buffer, "# %d \"%[^\"]\"", &linenr, filename);
        if (sscanf_rc == 2) {
            // printf ("DIRECTIVE: line %d file \"%s\"\n", linenr, filename);
            continue;
        }
        char* savepos = NULL;
        char* bufptr = buffer;
        for (int tokenct = 1;; ++tokenct) {
            char* token = strtok_r(bufptr, " \t\n", &savepos);
            bufptr = NULL;
            if (token == NULL) break;
            //printf ("token %d.%d: [%s]\n", linenr, tokenct, token);
            const string* str = intern_stringset(token);
        }
        ++linenr;
    }
}

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
        perror("oc: error: no input file");
    }

    // if (lflag) printf("lflag set\n"); 
    // if (yflag) printf("yflag set\n"); 

    char* filename = argv[optind];
    struct stat buffer;
    if (stat(filename, &buffer) != 0) {
        syserrprintf(filename);
        exit(1);
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
