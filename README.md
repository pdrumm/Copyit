# Copyit
========

This Copyit repository contains my project1 assingment for Operating Systems (cse 30341).
This simple program was created just to get familiar with unix system calls, signals, and error checking.

After compiling using `make`, two binary files are created.

- copyit
- copyit_extracredit

### copyit

**copyit** performs the simple functionality of copying a file.
**copyit** will read in the source file and then copy it over to the destination path specified (relative to the current directory). 
The usage is as follows:

`$ copyit <sourcefile> <targetfile>`

### copyit_extracredit

**copyit_extracredit** provided an extra challenge of copying directories recursively!
If the sourcefile is a regular file, it performs identically to **copyit**.
However, if the sourcefile is a directory, the program creates a new directory with a name and path specified by the targetfile. 
**copyit_extracredit** then copies all files, subdirectories, files within those subdiretories, etc.
The usage is as follows:

`$ copyit_extracredit <sourcefile> <targetfile>`
