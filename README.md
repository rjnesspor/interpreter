# interpreter

  

This program interprets my custom made language. The interpreter currently does **NOT** perform any syntax/semantic checking and assumes the input file is valid. 

__Currently, the acceptable directives are:__

Integer [name] = [value]
- Creates a new integer with specified name and value.

Print <variable/"string literal">
- Prints the specified variable or string literal to stdout.

__You can compile the interpreter using gcc:__
`gcc interp.c -o interp`

__You can run the interpreter like this:__
`./interp <input_file.txt>`
