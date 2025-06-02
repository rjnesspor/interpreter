# interpreter

This program interprets a custom made language. The interpreter currently performs basic syntax/semantic checking but mostly assumes the input file is valid.

There is also an [assembler](https://github.com/rjnesspor/assembler) available for this language but doesn't currently support many directives.

Acceptable Directives 
-

`define [integer,string] [name] as [value]`
- Defines a new variable with specified name and value. Currently supports `string` and `integer` types.

`[name] is [name,integer] [+,-,*,/] [name,integer]`
- Performs an arithmetic operation on the specified variable or integers, and assigns the result to a variable.

`[name] is [integer]`
- Assigns the given integer to a variable.

`print [name]`
- Prints the specified variable to stdout.

`if [integer,name] [<,>,=] [integer,name]`
- Executes the code below it if the condition is true.

`endif`
- Signifies the end of an if block.

`leave [success/error]`
- Exits the program with specified status code. Currently supports `success` and `error` codes.

Compiling
-
`gcc interp.c -o interp`
  
__You can run the interpreter like this:__
`./interp [input_file.txt]`

Sample Program
-

*input.txt*
```
define integer x as 10
define integer y as 5
define string message as The value of x minus y is:

define integer z as 0
z is x - y

print message
print z

define string test as Is x minus y less than 7?
define string yes as Yes!
define string no as No!

print test

if z < 7
print yes
endif

if z > 7
print no
endif

leave success
```

*output*
```
The value of x minus y is:
5
Is x minus y less than 7?
Yes!

Program exited with status SUCCESS.
```