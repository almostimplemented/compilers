// Author, Andrew Edwards, ancedwar@ucsc.edu
//
// DESCRIPTION
//    Stringset used for indexing tokens of preprocessed program. 
//

#ifndef __STRINGSET__
#define __STRINGSET__

#include <iostream>
#include <string>

const std::string* intern_stringset (const char*);

void dump_stringset (std::ostream&);

#endif

