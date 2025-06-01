
# interpreter

  
  

This program interprets a custom made language. The interpreter currently performs basic syntax/semantic checking but mostly assumes the input file is valid.

  

__Currently, the acceptable directives are:__

  

`define <integer/string> <name> as <value`
- Defines a new variable with specified name and value. Currently supports `string` and `integer` types.
  

`print <name>`
- Prints the specified variable to stdout.

`leave <success/error>`
- Exits the program with specified status code. Currently supports `success` and `error` codes.

__You can compile the interpreter using gcc:__

`gcc interp.c -o interp`

  

__You can run the interpreter like this:__

`./interp <input_file.txt>`