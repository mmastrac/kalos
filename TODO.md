## Bootstrap Strategy

We want to have a folder containing a "known-good" bootstrap version that can be used to compile the current development
version. This will be a less painful version of the current `make gen` process that can break easily.

 - [X] Ensure that compiler_main.c calls into compiler.kalos
 - [X] Fix redirect hack with compiler.kalos `dispatch`
 - [X] Create a bootstrap folder to contain the bootstrap C and other files
 - [X] Always use bootstrap to compile development version
 - [X] Create a makefile task that promotes the current development version to a bootstrap
 - [X] Update compiler to use imports
