require 'termios'

DEVICE = '/dev/ttyS3'
EOL = "\x0d\x0a"
BAUDRATE = Termios::B38400
TIMEOUT = 1 #[sec]

class Chat
  include Termios

  CRTSCTS =  1
  XONXOFF = -2
  NONE    = -1
  def initialize_device(io, flow, speed, is_modem)
    @oldt = getattr(io)
    @newt = new_termios()

    # from ppp-2.3.5/pppd/sys-linux.c (set_up_tty)
    @newt.c_iflag  = IGNBRK | IGNPAR
    @newt.c_oflag  = 0
    @newt.c_cflag  = @oldt.c_cflag & 		# -(CSIZE, CSTOPB, 
      ~(CSIZE | CSTOPB | PARENB | CLOCAL)	#   PARENB, CLOCAL)
    @newt.c_cflag |= (CS8 | CREAD | HUPCL)	# +(CS8, CREAD, HUPCL)
    @newt.c_lflag  = 0
    @newt.c_line   = @oldt.c_line

    @oldt.c_cc.each_index {|i| @newt.c_cc[i] = @oldt.c_cc[i]}
    @newt.c_cc[VMIN]  = 1
    @newt.c_cc[VTIME] = 0

    unless is_modem
      @newt.c_cflag ^= (CLOCAL | HUPCL)		# +CLOCAL, -HUPCL
    end

    case flow
    when CRTSCTS
      @newt.c_cflag |= CRTSCTS			# +CRTSCTS
    when XONXOFF
      @newt.c_iflag |= (IXON | IXOFF)		# +(IXON, IXOFF)
      @newt.c_cc[VSTOP]  = "\x13"		# ^S
      @newt.c_cc[VSTART] = "\x11"		# ^Q
    when NONE
      @newt.c_cflag &= ~CRTSCTS			# -CRTSCTS
    end

    if speed
      @newt.c_ispeed = speed
      @newt.c_ospeed = speed
    else
      @newt.c_ispeed = @oldt.c_ispeed
      @newt.c_ospeed = @oldt.c_ospeed
    end

    setattr(io, TCSANOW, @newt)
    flush(io, TCIOFLUSH)
  end
  private :initialize_device

  def initialize(device, flow = CRTSCTS, speed = nil, is_modem = true)
    @oldt = nil
    @newt = nil

    @io = open(device, 'r+')
    begin
      initialize_device(@io, flow, speed, is_modem)
    rescue
      self.close
      raise
    end
  end

  def close
    flush(@io, TCIOFLUSH)
    setattr(@io, TCSANOW, @oldt)
    @io.close
  end

  def expect_send(expect, send)
    recv = ''
    if expect
      while expect !~ recv
	if s = @io.getc
	  recv << s.chr
	else
	  raise "can not get expect string (got: #{recv})"
	end
      end
    end

    sent = ''
    if send && send.size > 0
      send.chomp!
      send << EOL

      send.each_byte {|c|
	c = c.chr
	@io.putc c
	if s = @io.getc
	  sent << s.chr
	else
	  raise "can not send string (sent: #{sent})"
	end
      }
    end

    return recv, sent
  end
end #Chat


if $0 == __FILE__
  begin
    modem = Chat.new(DEVICE)
  ensure
    modem.close if modem
  end
end
