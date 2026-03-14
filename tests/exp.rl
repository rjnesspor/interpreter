! This program defines a simple function to
! calculate the result of a number raised to another.

define func pow with (integer base, integer exponent)
    define integer res as 1

    ! 'loop' will execute a block as many times as 'exponent'
    loop exponent
        redefine res as res * base
    endloop

    leave res
endf

define integer x as 3
define integer y as 2

! Calculate 3^2 = 9
call pow with (x, y) as result
print result

leave 0