Assignment 3
=========
Student Name:   Andrew C. Edwards

Student Email:  ancedwar@ucsc.edu

Student ID:     1253060

Assignment:     Project 3, Parser

Deadline:       Monday, 17 Nov 2014


Documentation:
----
This project implements a scanner for the oc compiler
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

The program generates the same .str file created by project 1, but
also creates a .tok file that dumps the tokens of the program.
