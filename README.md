General Usage
====================

The Acronym Search Tool is a semi-GUI, semi-command line based tool to help find, add, delete, and extract acronyms.
It is a standalone application that will work on Windows 7 and above.

It can be downloaded from the following link if you are only interested in using the application:

https://sourceforge.net/projects/ladywriter/files/ast.zip/download


See the section below if you wish to compile it yourself.


Compiling with mingw-GCC
====================

You will need to link to the following Windows libraries:
  -gdi32
  -user32
  -kernel32
  -comctl32
  -comdlg32
  
Your installation of mingw may not have the following files, and might be required:
  -winapifamily.h
  -_mingw-unicode.h
  -wincon.h
  -virtdisk.h
  -stralign.h
  
  The binary in the link above has been compiled with the following flags and -static-libgcc
  
  _-Wall -g -O3 -Winline -Wunreachable-code -pedantic-errors -pedantic -Wextra -Winline -Wmain -Wfatal-errors -Wextra
