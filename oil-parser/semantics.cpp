// $Id: semantics.cpp,v 1.3 2014-11-25 17:14:38-08 - - $

#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <iostream>
#include <libgen.h>
#include <string>
using namespace std;

#include "semantics.h"

string this_progname; // Name of this program.
string oil_filename; // Name of the program.oil to be compiled.
string oil_execname; // Executable name created from oil & oclib.

void yyerror (const char* error) {
   cerr << oil_filename << ":" << lex_linenr << ":" << lex_offset
        << ": error: " << error << endl;
}

void error (initializer_list<string> args, int status = EXIT_FAILURE) {
   cerr << this_progname << ": ";
   for (const string& str: args) cerr << str;
   cerr << endl;
   if (status != 0) exit (status);
}

void status_report (int status) {
   if (status == -1) {
      error ({"gcc: ", strerror (errno)});
   }else if (status & 0x7F) {
      status &= 0x7F;
      error ({"gcc: caught signal ", to_string (status)}, status + 128);
   }else {
      status >>= 8;
      error ({"gcc: exit status ", to_string (status)}, status);
   }
}

void set_oil_execname() {
   oil_execname = oil_filename;
   size_t size = oil_execname.size();
   if (size < 4 or oil_execname.substr (size - 4) != ".oil") {
      error ({oil_filename, ": suffix not .oil"});
   }
   oil_execname = oil_filename.substr (0, size - 4);
}


int main (int argc, char** argv) {
   this_progname = basename (argv[0]);
   yy_flex_debug = 0;
   yydebug = 0;
   if (argc != 2) error ({"Usage: ", this_progname, " oil_filename"});
   cout << this_progname << ": version " << __DATE__ << " " << __TIME__
        << endl;
   oil_filename = argv[1];
   set_oil_execname();
   yyin = fopen (oil_filename.c_str(), "r");
   if (yyin == NULL) error ({oil_filename, ": ", strerror (errno)});
   int yycode = yyparse();
   if (yycode != 0) error ({"parse of ", oil_filename, " returned ",
                           to_string (yycode)});
   string command = string ("gcc -g -o ") + oil_execname + " -x c "
                  + oil_filename + " oclib.c";
   cout << this_progname << ": running: " << command << endl;
   int status = system (command.c_str());
   status_report (status);
   return EXIT_SUCCESS;
}

