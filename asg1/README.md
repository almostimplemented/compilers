Assignment 1
=========
Student Name:   Andrew C. Edwards

Student Email:  ancedwar@ucsc.edu

Student ID:     1253060

Assignment:     Project 1, String Set and Preprocessor

Deadline:       Monday, 20 Oct 2014


Documentation:
----
This project implements a skeleton for the oc compiler
I will be building this quarter. To build do:

    make all

To use do:

    oc [-ly] [-@ <flags...>] [-D <string>] <program>.oc

The options are as follows:

    -@ <flags...>   Call set_debugflags, and use DEBUG
                    and DEBUGSTMT for debugging.

    -D <string>     Pass this option and its argument
                    to cpp.

    -l              Debug yylex() with yy_flex_debug = 1

    -y              Debug yyparse() with yydebug = 1

The program flow is as follows:

    (1) Parse the command line arguments.
    (2) Use cpp to preprocess the input.
    (3) Tokenize the output from (2) with strtok_r().
    (4) Intern tokens into string set.
    (5) Dump the string set into a file <program>.str.
