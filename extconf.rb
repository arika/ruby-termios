require 'mkmf'

if have_header('termios.h') &&
    have_header('unistd.h')

  unless RUBY_VERSION < '1.7' || have_type("rb_io_t", ["ruby.h", "rubyio.h"])
    have_struct_member("OpenFile", "fd", ["ruby.h", "rubyio.h"])
  end

  create_makefile('termios')
end
