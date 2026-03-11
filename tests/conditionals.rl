! This program shows the basic syntax of conditional 
! statements and handling logic.

define integer myInput as 0

print "Please enter a number:"
input integer as myInput

if myInput = 5
    print "Your number was exactly 5!"
endif

if myInput > 5
    print "Your number was greater than 5!"
endif

if myInput < 5
    print "Your number was less than 5!"
endif

leave 0