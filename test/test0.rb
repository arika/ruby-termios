require 'termios'

now = Termios::getattr($stdin)
old = Termios::setattr($stdin, Termios::TCSANOW, now)
new = Termios::getattr($stdin)

p now
p old
p new

p Termios::getpgrp($stdin)
