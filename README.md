# interpreter

This program interprets a custom made language. The interpreter currently performs basic syntax/semantic checking.

There is also an [compiler](https://github.com/rjnesspor/compiler) available for this language but doesn't currently support many directives.

Acceptable Directives 
-

`define [integer,string] [name] as [value]`
- Defines a new variable with specified name and value. Currently supports `string` and `integer` types.

`redefine [name] as [integer,string] [value]`
- Redefines a variable to be a given string or integer.

`redefine [name] as [integer,name] [+,-,*,/] [integer,name]`
- Performs the given arithmetic operation and stores the result in a variable.

`print [name]`
- Prints the specified variable to `stdout`.

`input [integer,string] as [name]`
- Reads an input from `stdin` and stores the result in a variable.

`if [integer,name] [<,>,=] [integer,name]`
- Executes the code below it if the condition is true.

`endif`
- Signifies the end of an if block.

`leave [success/error]`
- Exits the program with specified status code. Currently supports `success` and `error` codes.

Compiling/Executing
-
Make sure to include `utils.c` when compiling.

`gcc interp.c utils.c -o interp`

`./interp [input_file.txt]`

Sample Program
-

*input.txt*
```
define string message as Enter a number:
print message

input integer as val

define integer y as 0
redefine y as 5 + val

define string response as Your number plus 5 equals:

print response
print y

if y > 10
define string greater as The result is greater than 10!
print greater
endif

if y < 10
define string less as The result is less than 10!
print less
endif

if y = 10
define string equal as The result equals 10!
print equal
endif

leave success
```

*output*
```
Enter a number:
1   <== user input
Your number plus 5 equals:
6
The result is less than 10!

Program exited with status SUCCESS.
```