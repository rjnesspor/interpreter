# interpreter

This program interprets a custom made language. The interpreter currently performs basic syntax/semantic checking.

There is also an [compiler](https://github.com/rjnesspor/compiler) available for this language but doesn't currently support many directives.

Syntax Rules
-
- Strings must be wrapped in quotes (i.e. "Hello, world!").
- Only single binary operations are supported (i.e. redefine x as x + y + z is NOT valid (2 binary ops)).
- Loop conditions are **not** supported currently. Only integer literals.
- Variable scope (block) is **not** supported. All variables are treated as global.


Acceptable Directives 
-

`define [integer,string] [name] as [value]`
- Defines a new variable with specified name and value. Currently supports `string` and `integer` types.

`define func [name]`
- Defines a new function with the specified name.

`call [name]`
- Executes the given function name.

`redefine [name] as [integer,string] [value]`
- Redefines a variable to be a given string or integer.

`redefine [name] as [integer,name] [+,-,*,/] [integer,name]`
- Performs an arithmetic operation and stores the result in a variable.

`print [name]`
- Prints the specified variable to `stdout`.

`input [integer,string] as [name]`
- Reads an input from `stdin` and stores the result in a variable.

`if [integer,name] [<,>,=] [integer,name]`
- Executes the block if the condition is true.

`endif`
- Signifies the end of an if block.

`loop [count]`
- Executes the block a number of times, specified by `count`.

`endloop`
- Signifies the end of a loop block.

`leave [0/1]`
- Exits the program with specified status code. Currently supports `0` and `1` codes for success and error, respectively.

Compiling/Executing
-

`gcc main.c parser.c tokenizer.c interpreter.c utils.c -o interp`

`./interp [input_file.txt]`

There is also a provided Makefile.

Sample Program
-

*input.txt*
```
define string message as "Enter a number:"
print message

input integer as val

define integer y as 0
redefine y as 5 + val

define string response as "Your number plus 5 equals:"

print response
print y

if y > 10
define string greater as "The result is greater than 10!"
print greater
endif

if y < 10
define string less as "The result is less than 10!"
print less
endif

if y = 10
define string equal as "The result equals 10!"
print equal
endif

define integer zz as 1
define func incrementZZ
    redefine zz as 1 + zz
endf

loop 5
print zz
call incrementZZ
endloop

leave 0
```

*output*
```
Enter a number:
1 <== User input
Your number plus 5 equals:
6
The result is less than 10!
1
2
3
4
5
```