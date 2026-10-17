require 'termios'

Warning[:experimental] = false

r = Ractor.new do
  tio = Termios.tcgetattr($stdin)
  Termios.tcsetattr($stdin, Termios::TCSANOW, tio)
  [
    Termios::BAUDS[tio.ispeed],
    Termios.tcgetpgrp($stdin),
    tio.dup.inspect,
  ]
end
# Ractor#take was replaced by Ractor#value in Ruby 3.5
speed, pgrp, inspect = r.respond_to?(:value) ? r.value : r.take

raise "unexpected ispeed: #{speed.inspect}" unless speed.is_a?(Symbol)
raise "unexpected pgrp: #{pgrp.inspect}" unless pgrp.is_a?(Integer)
raise "unexpected inspect: #{inspect}" unless inspect.start_with?('#<Termios::Termios ')

puts "ok: #{speed}, pgrp=#{pgrp}"
