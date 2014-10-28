#include <string>
#include <vector>
using namespace std;

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "astree.h"
#include "auxlib.h"
#include "lyutils.h"
#include "stringset.h"

string cpp_command = "/usr/bin/cpp";

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
      option = getopt (argc, argv, "@:ely");
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
      errprintf ("Usage: %s [-ly] [filename]\n", get_execname());
      exit (get_exitstatus());
   }
   const char *filename = optind == argc ? "-" : argv[optind];
   cpp_command += " "; 
   cpp_command += filename;
   yyin_cpp_popen();
   DEBUGF ('m', "filename = %s, yyin = %p, fileno (yyin) = %d\n",
           filename, yyin, fileno (yyin));
   scanner_newfilename (filename);
}

int main (int argc, char** argv) {
   int parsecode = 0;
   set_execname (argv[0]);
   DEBUGSTMT ('m',
      for (int argi = 0; argi < argc; ++argi) {
         eprintf ("%s%c", argv[argi], argi < argc - 1 ? ' ' : '\n');
      }
   );
   scan_opts (argc, argv);
   scanner_setecho (want_echo());
   int tok;
   while ((tok=yylex()) > 0) {
       dump_astree(stdout, yylval);
   }
   yyin_cpp_pclose();
   //DEBUGSTMT ('s', dump_stringset (stderr); );
   yylex_destroy();
   return get_exitstatus();
}
