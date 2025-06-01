
  

# interpreter

  

  

This program interprets a custom made language. The interpreter currently performs basic syntax/semantic checking but mostly assumes the input file is valid.

Acceptable Directives 
-

`define <integer,string> <name> as <value`
- Defines a new variable with specified name and value. Currently supports `string` and `integer` types.

`<name> is <name,integer> <+,-,*,/> <name,integer>`
- Performs an arithmetic operation on the specified variable or integers, and assigns the result to a variable.

`<name> is <integer>`
- Assigns the given integer to a variable.

`print <name>`
- Prints the specified variable to stdout.

`leave <success/error>`
- Exits the program with specified status code. Currently supports `success` and `error` codes.

Compiling
-
`gcc interp.c -o interp`
  
__You can run the interpreter like this:__
`./interp <input_file.txt>`

Sample Program
-

*input.txt*
```
define integer x as 5
define string message as The value of x is:
print message
print x
define integer y as 2
define integer z as 0
define string message2 as The value of y is:
define string message3 as The value of x times y is:
print message2
print y
z is x * y
print message3
print z
leave success
```

*output*
```
The value of x is: 
5
The value of y is: 
2
The value of x times y is: 
10

Program exited with status SUCCESS.
```