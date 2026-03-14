! This program shows the basic syntax of functions
! and function arguments.

! Functions are defined using the 'define' keyword similar to regular variables.
! Parameters can be specified inside the parentheses.
define func add_nums with (integer x, integer y)
    define integer z as x + y
    
    ! The 'leave' keyword is used to return values.
    leave z
endf

define integer a as 5
define integer b as 10

! The 'call' keyword is used to call a function.
! An optional return value can be captured by using the keyword 'as'.
call add_nums with (a, b) as c

print c

leave 0