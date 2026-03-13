! This program shows the basic syntax of loops.

define integer x as 5
define integer y as 10

! This loop will print the value of x three times.
loop 3
    print x
endloop

! Loops can be nested. These loops will print the value of y three times, twice.
define integer outerCount as 2
define integer innerCount as 3

loop outerCount
    loop innerCount
        print y
    endloop
endloop

leave 0