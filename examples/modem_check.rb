require 'termios'

DEVICE = '/dev/ttyS3'
BAUDRATE = Termios::B38400


RESCUE = Termios::new_termios(1280, 5, 3261, 35387, 
			      [3, 28, 127, 21, 4, 0, 1, 0, 17, 
				19, 26, 0, 18, 15, 23, 22, 0, 0, 
				115, 80, 138, 6, 8, 48, 236, 
				255, 191, 85, 97, 0, 64, 3], 13, 13)
#p 1
dev = open(DEVICE, 'r+')
#p 1
#sleep 30
#p Termios::getattr(dev)
#Termios::setattr(dev, Termios::TCSANOW, RESCUE)
#p Termios::getattr(dev)
#p 1
#exit

oldtio = Termios::getattr(dev)
p oldtio

newtio = Termios::new_termios()
newtio.c_iflag = Termios::IGNPAR
newtio.c_oflag = 0
newtio.c_cflag = (BAUDRATE | Termios::CRTSCTS | 
		  Termios::CS8 | Termios::CREAD)
newtio.c_lflag = 0
newtio.c_cc[Termios::VTIME] = 0
newtio.c_cc[Termios::VMIN]  = 1

Termios::flush(dev, Termios::TCIOFLUSH)

Termios::setattr(dev, Termios::TCSANOW, newtio)
p Termios::getattr(dev)

"AT\x0d\x0a".each_byte {|c|
  c = c.chr
  dev.putc c
  d = dev.getc.chr
  p [c, d]
}

i = ''
while /OK\x0d\x0a/o !~ i
  i << dev.getc.chr
  p i
end

Termios::setattr(dev, Termios::TCSANOW, oldtio)
p Termios::getattr(dev)
