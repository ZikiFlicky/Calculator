# Calculator
This is a simple command line calculator program written in C (C99).

It can also be used as a library, if one imports just `calculator.c` and `calculator.h`.

It currently supports basic operators like + - * / () % ^.

This program was created because I wanted to think of an algorithm to do calculations, so I tried to think of my own one and came up with this.

## Build instructions
First, `git clone` the repository
### Linux or any other unix-like operating systems
#### Dependencies
* GNU make
* GCC
#### Instructions
* `cd` into the program's directory.
* Type `make`.
* Type `./calculator` to run the program

### Windows
#### Dependencies
* Visual Studio 2019
#### Instructions
* `cd` into the program's directory.
* Type `.\make`.
* Type `.\calculator` to run the program

## How it works
This calculator works by moving linearly on an array of tokens and creating a tree of nodes, that are later recursively evaluated, and at the end return a number.

### This is a node tree created from 1+2*3^4-5
```
       (-)
     /     \
   (+)     (5)
  /   \
(1)   (*)
     /   \
   (2)   (^)
        /   \
      (3)   (4)
```
It will later be evaluated recursivaly to finally return 158.
