### WGREP Project

This repository holds the source code and a set of test cases for a program named `wgrep` that simulates the Unix `grep` command

> Description

`Grep `is a **Linux / Unix command-line tool used to search for a string of characters in a specified file**. When it finds a match, it prints the line with the result. The grep command is handy when searching through large log files.

The `wgrep` command consists of three parts in its most basic form. The first part starts with **`wgrep`**, followed by the pattern that you are searching for. After the string comes the file name that the grep searches through.

```bash
prompet> ./wgrep "Term To Search For" filename
```

wgrep will display every line where there is a match of the term that you are searching for.

> Usage

Firstly we compile the code file as follows

``` bash
prompt> gcc -o wgrep wgrep.c -Wall 
```

this will produce a binary file named `wgrep` which we will use.

To run the tests we firstly have to give a running permission for the bash script `test-wgrep.sh`

```bash
prompt> chmod u+x ./test-wgrep.sh
prompt> ./test-wgrep.sh
```

the output will be in the following form

```bash
test 1: passed
test 2: passed
test 3: passed
test 4: passed
test 5: passed
test 6: passed
test 7: passed
```

The wgrep utility expression take the following form:

```sh
./wgrep "Term To Search For" file1 [FILE2]
```

**To Search a File**

The most basic and common usage of the `wgrep` command is to print any line from a file that contains a specific pattern of characters.

For example, the following command will print out any line containing "Hello" in the `testfile.txt` file on the terminal:

```bash
./wgrep "Hello" testfile.txt
```

**To Search Multiple Files**

To search multiple files with the `wgrep` command, insert the filenames you want to search, separated with a space character.

```bash
./wgrep "pattern" file1 file2
```
