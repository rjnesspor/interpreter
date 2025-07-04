# interpreter

This program interprets a custom made language. The interpreter currently performs syntax/semantic checking and will produce syntax/runtime errors where appropriate.

~~There is also an [compiler](https://github.com/rjnesspor/compiler) available for this language but doesn't currently support many directives.~~ This compiler is old and outdated. I advise against even looking at it until I update it to the newest syntax of this language.

Syntax Rules
-
- `string`s must be wrapped in quotes (i.e. "Hello, world!").
- Only single binary operations are supported (i.e. `redefine x as x + y + z` is **not** valid (2 binary ops)).
- `loop` conditions are **not** supported currently. Only integer literals.
- Variable scope is enforced.
- Function parameter typechecking is non-existent as of now.
- Arguments passed to `call` must be variables, not literals.


Acceptable Directives 
-

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
define func main with ()
    define string message as "Enter a number:"
    print message

    define integer val as 0
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

    loop 5
      print zz
      redefine zz as zz + 1
    endloop
endf

call main with ()
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