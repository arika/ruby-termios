require 'mkmf'

Dir.glob('./lib/**/*.rb') do |f|
  $INSTALLFILES << [f, '$(RUBYLIBDIR)', 'lib']
end

if have_header('termios.h') &&
    have_header('unistd.h')
  have_header('sys/ioctl.h')

  if RUBY_VERSION >= '1.7'
    if have_header('ruby/io.h')
      have_type("rb_io_t", ["ruby/io.h"])
      have_struct_member("rb_io_t", "fd", ["ruby/io.h"])
    else
      if have_type("rb_io_t", ["ruby.h", "rubyio.h"])
        have_struct_member("rb_io_t", "fd", ["ruby.h", "rubyio.h"])
      else
        have_struct_member("OpenFile", "fd", ["ruby.h", "rubyio.h"])
      end
      if have_macro("OpenFile", ["ruby.h", "rubyio.h"])
        $defs.push("-DHAVE_MACRO_OPENFILE")
      end
    end
  end

  create_makefile('termios')
end
