# BTOA - Binary File to Assembly Code Converter

*License:* GNU GPL v3.

## Description

A simple but very useful tool helped me many times. Not all assembly
language translators support a special command for inclusion of binary
files to your code. This utility helps to do such thing the simplest
possible way - by usge of basic data definition mnemonic which can be
found in any assembler.

For definition of a byte we often use something like "db" in intel-like
assemblers and ".byte" in AT&T ones. So, it is possible to define any data
as an array of bytes, no matter what you want to inject to your final
binary - some image, text or other kind of BLOB. It is necessary also to
know the size of the included data.

btoa defines two symbols. The first one is a label to acces the first byte
in the data array. The symbol name is the name of your input file + a
postfix "\_file". Of course, symbols not supported in assembly language are
replaced simply by '\_'. The second symbol is a long value containing a size
of the data. Its name looks similar to the data label but with a little
difference - a postfix "\_size".

## An Example

If we have executed btoa on the file named login-screen.bmp we will see two
symbols in the output: login\_screen\_bmp\_file, login\_screen\_bmp\_file\_size.

