# shadows input then reports [ruby-list:15968]
require 'termios'
include Termios

oldt = getattr($stdin)
newt = oldt.dup
newt.c_lflag &= ~ECHO
setattr($stdin, TCSANOW, newt)
print "noecho> "
a = $stdin.gets
setattr($stdin, TCSANOW, oldt)
print "\n"
p a
