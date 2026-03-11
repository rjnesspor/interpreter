! This program shows the basic syntax of functions
! and function arguments.

define func add_nums with (integer x, integer y)
    define integer z as 0
    redefine z as x + y
    print z
endf

define integer a as 5
define integer b as 10

! This will result in "15" being displayed.
call add_nums with (a, b)

leave 0