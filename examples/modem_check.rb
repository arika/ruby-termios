require 'fcntl'
require 'termios'

DEVICE = '/dev/modem'
BAUDRATE = Termios::B9600

def dev_open(path)
  dev = open(DEVICE, File::RDWR | File::NONBLOCK)
  mode = dev.fcntl(Fcntl::F_GETFL, 0)
  dev.fcntl(Fcntl::F_SETFL, mode & ~File::NONBLOCK)
  dev
end

def dump_termios(tio, banner)
  puts banner
  puts "  ispeed = #{Termios::BAUD[tio.ispeed]}, ospeed = #{Termios::BAUD[tio.ospeed]}"
  ["iflag", "oflag", "cflag", "lflag"].each do |x|
    flag = tio.send(x)
    flags = []
    eval("Termios::#{x.upcase}S").each do |f, sym|
      flags << sym.to_s if flag & f != 0
    end
    puts "   #{x} = #{flags.sort.join(' | ')}"
  end
  print "      cc ="
  cc = tio.cc
  cc.each_with_index do |x, idx|
    print " #{Termios::CCINDEX[idx]}=#{x}" if Termios::CCINDEX.include?(idx)
  end
  puts
end

dev = dev_open(DEVICE)

oldtio = Termios::getattr(dev)
dump_termios(oldtio, "current tio:")

newtio = Termios::new_termios()
newtio.iflag = Termios::IGNPAR
newtio.oflag = 0
newtio.cflag = (BAUDRATE | Termios::CRTSCTS | 
		Termios::CS8 | Termios::CREAD)
newtio.lflag = 0
newtio.set_cc(Termios::VTIME, 0)
newtio.set_cc(Termios::VMIN, 1)
dump_termios(newtio, "new tio:")

Termios::flush(dev, Termios::TCIOFLUSH)
Termios::setattr(dev, Termios::TCSANOW, newtio)
dump_termios(Termios::getattr(dev), "current tio:")

"AT\x0d".each_byte {|c|
  c = c.chr
  p [:write_char, c]
  dev.putc c
  d = dev.getc
  p [:echo_back, d && d.chr || nil]
}

r = ''
while /OK\x0d\x0a/o !~ r
  r << dev.getc.chr
  p [:response, r]
end

Termios::setattr(dev, Termios::TCSANOW, oldtio)
dump_termios(Termios::getattr(dev), "current tio:")
