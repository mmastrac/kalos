## Bootstrap Strategy

We want to have a folder containing a "known-good" bootstrap version that can be used to compile the current development
version. This will be a less painful version of the current `make gen` process that can break easily.

 - [ ] Create a bootstrap folder to contain the bootstrap C and other files
 - [ ] Create a main.kalos that is used to bootstrap the compiler
 - [ ] Always use bootstrap to compile development version
 - [ ] Create a makefile task that promotes the current development version to a bootstrap
