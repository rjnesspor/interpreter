# interpreter

This program interprets a custom made language. The interpreter currently performs syntax/semantic checking and will produce syntax/runtime errors where appropriate.

Syntax Rules
-
- `string`s must be wrapped in double quotes (i.e. "Hello, world!").
- Only single binary operations are supported (i.e. `redefine x as x + y + z` is **not** valid (2 binary ops)).
- `loop` conditions are **not** supported currently. Only integer literals.
- Variable scope is enforced.
- Arguments passed to `call` must be variables, not literals.


Acceptable Directives 
-

`!`
- This character defines a comment. Anything after it on the same line will be ignored.

`define [integer,string] [name] as [value]`
- Defines a new variable with specified name and value. Currently supports `string` and `integer` types.

`define func [name] with ([integer,string] [name], ...)`
- Defines a new function with the specified name and optional parameters.

`endf`
- Signifies the end of a function block.

`call [name] with ([name], ...)`
- Executes the given function with optional parameters.

`redefine [name] as [integer,string] [value]`
- Redefines a variable to be a given string or integer.

`redefine [name] as [integer,name] [+,-,*,/] [integer,name]`
- Performs an arithmetic operation and stores the result in a variable.

`print [string/name]`
- Prints the specified variable or string to `stdout`.

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
- Exits the program with specified status code. Supports `0` and `1` codes for success and error, respectively.

Compiling/Executing
-

`gcc main.c parser.c tokenizer.c interpreter.c utils.c -o interp`

`./interp [input_file]`

There is also a provided Makefile.

Sample Programs
-
Many sample programs can be found in `/tests`, for your convenience. These aim to cover the basic applications of the language. 

To-Do List
-
- Allow for binary operations to be performed in define statements, to allow for declaring/defining a variable in one line (The current syntax only parses arithmetic within redefine statements).
- Allow for more than one binary operation per statement.
- ~~Type check parameters passed to functions to make sure they are correct~~
- Allow loop conditions similar to if statements
- Let functions be able to return values
- ~~Allow direct printing of string literals~~