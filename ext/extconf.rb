require 'mkmf'

abort 'termios.h is required' unless have_header('termios.h')
abort 'unistd.h is required' unless have_header('unistd.h')
have_header('sys/ioctl.h')

create_makefile('termios')
