// Author, Andrew Edwards, ancedwar@ucsc.edu

using namespace std;

#include <string.h>
#include "auxlib.h"
#include "stringset.h"

const size_t LINESIZE = 1024;

// Replace delimiter with null terminator
void chomp(char *string, char delim) {
    size_t len = strlen(string);
    if (len == 0) return;
    char *nlpos = string + len - 1;
    if (*nlpos == delim) *nlpos = '\0';
}

// Invokes CPP and generates stringset
void preprocess(string command, char* filename) {
    FILE* pipe = popen(command.c_str(), "r");
    if (pipe == NULL) {
        syserrprintf(command.c_str());
        exit(1);
    } else {
        int linenr = 1;
        for (;;) {
            char buffer[LINESIZE];
            char* fgets_rc = fgets(buffer, LINESIZE, pipe);
            if (fgets_rc == NULL) break;
            chomp (buffer, '\n');
            int sscanf_rc = sscanf(buffer, "# %d \"%[^\"]\"", \
                                   &linenr, filename);
            if (sscanf_rc == 2) {
                continue;
            }
            char* savepos = NULL;
            char* bufptr = buffer;
            for (int tokenct = 1;; ++tokenct) {
                char* token = strtok_r(bufptr, " \t\n", &savepos);
                bufptr = NULL;
                if (token == NULL) break;
                intern_stringset(token);
            }
            ++linenr;
        }
        pclose(pipe);
    }
}
