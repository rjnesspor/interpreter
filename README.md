# interpreter

This program interprets a custom made language. The interpreter currently performs syntax/semantic checking and will produce syntax/runtime errors where appropriate.

Syntax Rules
-
- `string`s must be wrapped in double quotes (i.e. "Hello, world!").
- Only single binary operations are supported (i.e. `redefine x as x + y + z` is **not** valid (2 binary ops)).
- `loop` conditions are **not** supported currently.
- Variable scope is enforced.
- Arguments passed to `call` must be variables, not literals.

Data Types
-
- `integer`
- `string`
- ~~`float`~~ (Coming soon)

Acceptable Directives 
-
`{}` is required. \
`[]` is optional.

`!`
- This character defines a comment. Anything after it on the same line will be ignored.

`define {type} {name} as {literal|name} [+|-|*|/] [literal|name]`
- Defines a new variable to be a given value or arithmetic result.

`define func {name} with ([type] [name], ...)`
- Defines a new function with the specified name and optional parameters.

`endf`
- Signifies the end of a function block.

`call {name} with ([name], ...)`
- Executes the given function with optional parameters.

`call {name} with ([name], ...) as [name]`
- Executes the given function with optional parameters and store the return value.

`leave [literal|name]`
- Returns a value or variable from the current function. If done in the global scope, exits the program with a code.

`redefine {name} as {literal|name} [+|-|*|/] [literal|name]`
- Redefines a variable to be a given value or arithmetic result.

`print {literal|name}`
- Prints the specified variable or literal to `stdout`.

`input {type} as {name}`
- Reads an input from `stdin` and stores the result in a variable.

`if {literal|name} {<|>|=} {literal|name}`
- Executes the block if the condition is true.

`endif`
- Signifies the end of an if block.

`loop {integer literal|name}`
- Executes the block a number of times.

`endloop`
- Signifies the end of a loop block.

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

- Allow for more than one binary operation per statement
- Allow loop conditions similar to if statements
- Allow for string concatenation with the + operator
- ~~Add new data types/create a generic data type framework~~
  - Arrays (In progress)
  - Floating point numbers (In progress)
- ~~Allow for binary operations to be performed in define statements, to allow for declaring/defining a variable in one line~~
- ~~Allow direct printing of string literals~~
- ~~Type check parameters passed to functions to make sure they are correct~~
- ~~Let functions be able to return values~~