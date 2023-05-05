![LOGO](https://github.com/HamishHamiltonSmith/Huffle/blob/main/assets/banner.png)


# Contents

- ***[About](#about)***
- ***[Roadmap](#roadmap)***
- ***[Usage](#usage)***
- ***[Documentation](#documentation)***

# About

Huffle is a programming language built with c++, made for building general purpose (and soon HALEX) applications. Currently it is bare and unoptimised- but features most major programming constructs, and is turing complete. These include:

- For & While loops
- Conditionals
- Functions (naitive and user defined)
- I/O
- Type conversions
- Arithmetic and logical evaluation
- Variables


# Roadmap

Huffle still has a long way to go before becoming a fully fledged and useful langauge. In the future I plan to use it as part of HALEX web infastructure - a sort of internet clone I am working on, but for now this repo has simply the core language features along with core graphics functionality. Here are more immenent plans:

- Add arrays + potentially other containers
- Improve error reporting :heavy_exclamation_mark:
- Add basic GUI controls (via SFML) :computer:
- Add module system for file inclusion 
- Build up standard library with more naitive functions
- Improve type system
- Scoping 
- Add cmake build system

I will update this as the project progresses. 

# Usage

At the moment huffle has no dependencies and no cmake build system - all you need is a c++20 compiler. On linux you can compile simply using:

`g++ interpreter.cpp -std=c++20`

Then to run huffle code, execute with a file parameter, eg:

`./a.out filename.huff/.txt`

# Documentation

I will release a proper docs page in the future, but for now here are the basics:
**Note semi-colons are required**


## Input/Output

Use the `out` keyword to ouput to standard output stream, ie:

```
out("hello world");
out(10+10);
```

You can also concatenate types, ie:

```
out("A cool number is " + toStr(23));
```


The `in` keyword takes a string by default (you can convert types with `toStr()` & `toNum()`), and outputs it before getting console input. This can then be stored or used:

```
udv name = in("Enter name: ");
out("Hello " + name);
```

## Variables & types
 
To define a varaible use the `udv` keyword. There are currently three built in types a variable like this can hold, thse are:
- Literals
- Doubles/Integers
- Booleans
 
```
//Comments can be used like this
 
udv food = in("What is your favourite food?");
out("I like " + toStr(food));
//Assignment
food = "pasta";
```
 
## Loops and conditionals
 
For loops work as standard - with a declarations, condition and increment, ie:
 
```
//For loop with scope
for (udv x=0; x<10; x=x+1) {
  out(x);
}
```
 
While loops are also familiar, with a simple condition:
 
```
while (true) {
  out("I love huffle");
}
```
 
 
As for conditionals, every conditional must start with an `if` statement, this can be extended with `else`, aswell as `elf` statements inbetween, like this:
 
```
udv pet = in("dogs or cats? ");
if (pet == "dogs") {
  out("Cool choice");
} elf (pet == "cats" or pet == "lions") {
  out("Very nice");
} else {
  out("??????");
}
```
 
## Functions
 
Functions now have return values, and can take parameters, including other functions! Here is a basic example:
 
```
func multiply(n1,n2) {
  return n1*n2;
}
 
out(multiply(10,5));
```

## Naitive functions

```
toStr( arg ) - attempts to convert type to string
toNum( arg ) - attempts to convert type to double
contains( str1, str2 ) - checks if a string contains another string
leave() - exits program
type( arg ) - gets type of argument (see c++ type codes)
length( str ) - gets length of string as double
```
 
 
There is a quick walkthough for getting started, soon there will be more detailed documentation!


