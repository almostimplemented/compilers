using namespace std;

#include <string.h>
#include "stringset.h"

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
