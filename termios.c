/*

  a termios library for Ruby 1.2.
  Copyright (C) 1999,2000 akira yamada.
  $Id: termios.c,v 1.1.1.1 2000-11-15 10:09:17 akira Exp $

 */

#include "ruby.h"
#include "rubyio.h"
#include <termios.h>
#include <unistd.h>
#include <string.h>

static VALUE sTermios;

/*
  struct termios -> Termios::Termios
 */
static VALUE
rb_termios_to_Termios(t)
     struct termios *t;
{
  VALUE c_cc;
  int i;
  c_cc = rb_ary_new2(NCCS);

  for (i = 0; i < NCCS; i++) {
    rb_ary_push(c_cc, INT2FIX(t->c_cc[i]));
  }

  return rb_struct_new(sTermios,
		    INT2FIX(t->c_iflag),
		    INT2FIX(t->c_oflag),
		    INT2FIX(t->c_cflag),
		    INT2FIX(t->c_lflag),
		    c_cc,
		    INT2FIX(cfgetispeed(t)),
		    INT2FIX(cfgetospeed(t)));
}

/*
  Termios::Termios -> struct termios
 */
static int
rb_Termios_to_termios0(value, name)
     VALUE value;
     char *name;
{
  if (NIL_P(value)) {
    return 0;
  } else if (FIXNUM_P(value)) {
    return FIX2INT(value);
  } else {
    rb_raise(rb_eTypeError, "wrong structure value %s (expected Fixnum or Nil)", 
	      name);
  }
}

static void
rb_Termios_to_termios(rt, t)
     VALUE rt;
     struct termios *t;
{
  int i;
  char c_cc[sizeof("c_cc[xx]") + 1];

  t->c_iflag  = rb_Termios_to_termios0(RSTRUCT(rt)->ptr[0], "c_iflag");
  t->c_oflag  = rb_Termios_to_termios0(RSTRUCT(rt)->ptr[1], "c_oflag");
  t->c_cflag  = rb_Termios_to_termios0(RSTRUCT(rt)->ptr[2], "c_cflag");
  t->c_lflag  = rb_Termios_to_termios0(RSTRUCT(rt)->ptr[3], "c_lflag");
  for (i = 0; i < RARRAY(RSTRUCT(rt)->ptr[4])->len; i++) {
    sprintf(c_cc, "c_cc[%d]", i);
    t->c_cc[i] = rb_Termios_to_termios0(RARRAY(RSTRUCT(rt)->ptr[4])->ptr[i],
					c_cc);
  }
  cfsetispeed(t, rb_Termios_to_termios0(RSTRUCT(rt)->ptr[5], "c_ispeed"));
  cfsetospeed(t, rb_Termios_to_termios0(RSTRUCT(rt)->ptr[6], "c_ospeed"));

  /*
  t->c_iflag  = FIX2INT(RSTRUCT(rt)->ptr[0]);
  t->c_oflag  = FIX2INT(RSTRUCT(rt)->ptr[1]);
  t->c_cflag  = FIX2INT(RSTRUCT(rt)->ptr[2]);
  t->c_lflag  = FIX2INT(RSTRUCT(rt)->ptr[3]);
  t->c_line   = FIX2INT(RSTRUCT(rt)->ptr[4]);
  for (i = 0; i < RARRAY(RSTRUCT(rt)->ptr[5])->len; i++) {
    t->c_cc[i] = FIX2INT(RARRAY(RSTRUCT(rt)->ptr[5])->ptr[i]);
  }
  cfsetispeed(t, FIX2INT(RSTRUCT(rt)->ptr[6]));
  cfsetospeed(t, FIX2INT(RSTRUCT(rt)->ptr[7]));
  */
}


static VALUE
rb_tcgetattr0(io)
     VALUE io;
{
  OpenFile *fptr;
  struct termios t;

  GetOpenFile(io, fptr);
  if (tcgetattr(fileno(fptr->f), &t) < 0) {
    rb_raise(rb_eRuntimeError, "can't get terminal parameters (%s)", strerror(errno));
  }
  return rb_termios_to_Termios(&t);
}

static VALUE
rb_tcgetattr(obj, io)
     VALUE obj, io;
{
  Check_Type(io, T_FILE);
  return rb_tcgetattr0(io);
}

static VALUE
rb_tcsetattr(obj, io, opt, param)
     VALUE obj, io, opt, param;
{
  VALUE old;
  OpenFile *fptr;
  struct termios t;
  char *type;

  Check_Type(io,    T_FILE);
  Check_Type(opt,   T_FIXNUM);
  Check_Type(param, T_STRUCT);
  type = rb_class2name(CLASS_OF(param));
  if (strcmp(type, "Struct::Termios") != 0) {
    rb_raise(rb_eTypeError, "wrong argument type %s (expected Struct::Termios)", 
	      type);
  }

  old = rb_tcgetattr0(io);
  GetOpenFile(io, fptr);
  rb_Termios_to_termios(param, &t);
  if (tcsetattr(fileno(fptr->f), FIX2INT(opt), &t) < 0) {
    rb_raise(rb_eRuntimeError, "can't set terminal parameters (%s)", strerror(errno));
  }

  return old;
}

static VALUE
rb_tcsendbreak(obj, io, duration)
     VALUE obj, io, duration;
{
  OpenFile *fptr;

  Check_Type(io,       T_FILE);
  Check_Type(duration, T_FIXNUM);

  GetOpenFile(io, fptr);
  if (tcsendbreak(fileno(fptr->f), FIX2INT(duration)) < 0) {
    rb_raise(rb_eRuntimeError, "can't transmits break (%s)", strerror(errno));
  }

  return Qtrue;
}

static VALUE
rb_tcdrain(obj, io)
     VALUE obj, io;
{
  OpenFile *fptr;

  Check_Type(io, T_FILE);

  GetOpenFile(io, fptr);
  if (tcdrain(fileno(fptr->f)) < 0) {
    rb_raise(rb_eRuntimeError, "can't drain (%s)", strerror(errno));
  }

  return Qtrue;
}


static VALUE
rb_tcflush(obj, io, qs)
     VALUE obj, io, qs;
{
  OpenFile *fptr;
  int queue_selector;

  Check_Type(io, T_FILE);
  Check_Type(qs, T_FIXNUM);
  queue_selector = FIX2INT(qs);
  if (queue_selector != TCIFLUSH &&
      queue_selector != TCOFLUSH &&
      queue_selector != TCIOFLUSH) {
    rb_raise(rb_eTypeError, "wrong argument value %s (expected Termios::TCIFLUSH, Termios::TCOFLUSH or Termios::TCIOFLUSH)", 
	      queue_selector);
  }

  GetOpenFile(io, fptr);
  if (tcflush(fileno(fptr->f), queue_selector) < 0) {
    rb_raise(rb_eRuntimeError, "can't flush (%s)", strerror(errno));
  }

  return Qtrue;
}


static VALUE
rb_tcflow(obj, io, act)
     VALUE obj, io, act;
{
  OpenFile *fptr;
  int action;

  Check_Type(io,  T_FILE);
  Check_Type(act, T_FIXNUM);
  action = FIX2INT(act);
  if (action != TCOOFF &&
      action != TCOON &&
      action != TCIOFF &&
      action != TCION) {
    rb_raise(rb_eTypeError, "wrong argument value %s (expected Termios::TCOOFF, Termios::TCOON, Termios::TCIOFF or Termios::TCION)", 
	      action);
  }

  GetOpenFile(io, fptr);
  if (tcflow(fileno(fptr->f), action) < 0) {
    rb_raise(rb_eRuntimeError, "can't control transmitting data flow (%s)", strerror(errno));
  }

  return Qtrue;
}


static VALUE
rb_tcgetpgrp(obj, io)
     VALUE obj, io;
{
  OpenFile *fptr;
  int pid;

  Check_Type(io,  T_FILE);
  GetOpenFile(io, fptr);
  if (pid = tcgetpgrp(fileno(fptr->f)) < 0) {
    rb_raise(rb_eRuntimeError, "can't get process group id (%s)", strerror(errno));
  }

  return INT2FIX(pid);
}


static VALUE
rb_tcsetpgrp(obj, io, pgrpid)
     VALUE obj, io, pgrpid;
{
  OpenFile *fptr;

  Check_Type(io,     T_FILE);
  Check_Type(pgrpid, T_FIXNUM);

  GetOpenFile(io, fptr);
  if (tcsetpgrp(fileno(fptr->f), FIX2INT(pgrpid)) < 0) {
    rb_raise(rb_eRuntimeError, "can't set process group id (%s)", strerror(errno));
  }

  return Qtrue;
}


static VALUE
rb_new_termios(argc, argv, self)
     int argc;
     VALUE *argv;
     VALUE self;
{
  VALUE c_iflag, c_oflag, c_cflag, c_lflag, 
    c_cc, c_ispeed, c_ospeed;
  int i;

  rb_scan_args(argc, argv, "07", 
	       &c_iflag, &c_oflag, &c_cflag, &c_lflag, 
	       &c_cc, &c_ispeed, &c_ospeed);
  if (NIL_P(c_cc)) {
    c_cc = rb_ary_new2(NCCS);
    for (i = 0; i < NCCS; i++) {
      rb_ary_push(c_cc, Qnil);
    }
  } else {
    Check_Type(c_cc, T_ARRAY);
  }

  return rb_struct_new(sTermios,
		    c_iflag, c_oflag, c_cflag, c_lflag,
		    c_cc, c_ispeed, c_ospeed);
}


void
Init_termios()
{
  VALUE mTermios = rb_define_module("Termios");

  rb_define_module_function(mTermios, "getattr",   rb_tcgetattr,   1);
  rb_define_module_function(mTermios, "setattr",   rb_tcsetattr,   3);
  rb_define_module_function(mTermios, "sendbreak", rb_tcsendbreak, 2);
  rb_define_module_function(mTermios, "drain",     rb_tcdrain,     1);
  rb_define_module_function(mTermios, "flush",     rb_tcflush,     2);
  rb_define_module_function(mTermios, "flow",      rb_tcflow,      2);
  rb_define_module_function(mTermios, "getpgrp",   rb_tcgetpgrp,   1);
  rb_define_module_function(mTermios, "setpgrp",   rb_tcsetpgrp,   2);

  rb_define_module_function(mTermios, "new_termios", rb_new_termios, -1);

  sTermios = rb_struct_define("Termios",
			   "c_iflag",	/* input modes */
			   "c_oflag",	/* output modes */
			   "c_cflag",	/* control modes */
			   "c_lflag",	/* local modes */
			   "c_cc",	/* control chars */
			   "c_ispeed",	/* input speed */
			   "c_ospeed",	/* output speed */
			   0);
  /* rb_define_const(mTermios, "Termios", sTermios); */
  rb_global_variable(&sTermios);

#ifdef NCCS
  rb_define_const(mTermios, "NCCS", INT2FIX(NCCS));
#endif

  /* c_cc characters */
#ifdef VINTR
  rb_define_const(mTermios, "VINTR", INT2FIX(VINTR));
#endif
#ifdef VQUIT
  rb_define_const(mTermios, "VQUIT", INT2FIX(VQUIT));
#endif
#ifdef VERASE
  rb_define_const(mTermios, "VERASE", INT2FIX(VERASE));
#endif
#ifdef VKILL
  rb_define_const(mTermios, "VKILL", INT2FIX(VKILL));
#endif
#ifdef VEOF
  rb_define_const(mTermios, "VEOF", INT2FIX(VEOF));
#endif
#ifdef VTIME
  rb_define_const(mTermios, "VTIME", INT2FIX(VTIME));
#endif
#ifdef VMIN
  rb_define_const(mTermios, "VMIN", INT2FIX(VMIN));
#endif
#ifdef VSWTC
  rb_define_const(mTermios, "VSWTC", INT2FIX(VSWTC));
#endif
#ifdef VSTART
  rb_define_const(mTermios, "VSTART", INT2FIX(VSTART));
#endif
#ifdef VSTOP
  rb_define_const(mTermios, "VSTOP", INT2FIX(VSTOP));
#endif
#ifdef VSUSP
  rb_define_const(mTermios, "VSUSP", INT2FIX(VSUSP));
#endif
#ifdef VEOL
  rb_define_const(mTermios, "VEOL", INT2FIX(VEOL));
#endif
#ifdef VREPRINT
  rb_define_const(mTermios, "VREPRINT", INT2FIX(VREPRINT));
#endif
#ifdef VDISCARD
  rb_define_const(mTermios, "VDISCARD", INT2FIX(VDISCARD));
#endif
#ifdef VWERASE
  rb_define_const(mTermios, "VWERASE", INT2FIX(VWERASE));
#endif
#ifdef VLNEXT
  rb_define_const(mTermios, "VLNEXT", INT2FIX(VLNEXT));
#endif
#ifdef VEOL2
  rb_define_const(mTermios, "VEOL2", INT2FIX(VEOL2));
#endif

  /* c_iflag bits */
#ifdef IGNBRK
  rb_define_const(mTermios, "IGNBRK", INT2FIX(IGNBRK));
#endif
#ifdef BRKINT
  rb_define_const(mTermios, "BRKINT", INT2FIX(BRKINT));
#endif
#ifdef IGNPAR
  rb_define_const(mTermios, "IGNPAR", INT2FIX(IGNPAR));
#endif
#ifdef PARMRK
  rb_define_const(mTermios, "PARMRK", INT2FIX(PARMRK));
#endif
#ifdef INPCK
  rb_define_const(mTermios, "INPCK", INT2FIX(INPCK));
#endif
#ifdef ISTRIP
  rb_define_const(mTermios, "ISTRIP", INT2FIX(ISTRIP));
#endif
#ifdef INLCR
  rb_define_const(mTermios, "INLCR", INT2FIX(INLCR));
#endif
#ifdef IGNCR
  rb_define_const(mTermios, "IGNCR", INT2FIX(IGNCR));
#endif
#ifdef ICRNL
  rb_define_const(mTermios, "ICRNL", INT2FIX(ICRNL));
#endif
#ifdef IUCLC
  rb_define_const(mTermios, "IUCLC", INT2FIX(IUCLC));
#endif
#ifdef IXON
  rb_define_const(mTermios, "IXON", INT2FIX(IXON));
#endif
#ifdef IXANY
  rb_define_const(mTermios, "IXANY", INT2FIX(IXANY));
#endif
#ifdef IXOFF
  rb_define_const(mTermios, "IXOFF", INT2FIX(IXOFF));
#endif
#ifdef IMAXBEL
  rb_define_const(mTermios, "IMAXBEL", INT2FIX(IMAXBEL));
#endif

  /* c_oflag bits */
#ifdef OPOST
  rb_define_const(mTermios, "OPOST", INT2FIX(OPOST));
#endif
#ifdef OLCUC
  rb_define_const(mTermios, "OLCUC", INT2FIX(OLCUC));
#endif
#ifdef ONLCR
  rb_define_const(mTermios, "ONLCR", INT2FIX(ONLCR));
#endif
#ifdef OCRNL
  rb_define_const(mTermios, "OCRNL", INT2FIX(OCRNL));
#endif
#ifdef ONOCR
  rb_define_const(mTermios, "ONOCR", INT2FIX(ONOCR));
#endif
#ifdef ONLRET
  rb_define_const(mTermios, "ONLRET", INT2FIX(ONLRET));
#endif
#ifdef OFILL
  rb_define_const(mTermios, "OFILL", INT2FIX(OFILL));
#endif
#ifdef OFDEL
  rb_define_const(mTermios, "OFDEL", INT2FIX(OFDEL));
#endif
#ifdef NLDLY
  rb_define_const(mTermios, "NLDLY", INT2FIX(NLDLY));
#endif
#ifdef NL0
  rb_define_const(mTermios, "NL0", INT2FIX(NL0));
#endif
#ifdef NL1
  rb_define_const(mTermios, "NL1", INT2FIX(NL1));
#endif
#ifdef CRDLY
  rb_define_const(mTermios, "CRDLY", INT2FIX(CRDLY));
#endif
#ifdef CR0
  rb_define_const(mTermios, "CR0", INT2FIX(CR0));
#endif
#ifdef CR1
  rb_define_const(mTermios, "CR1", INT2FIX(CR1));
#endif
#ifdef CR2
  rb_define_const(mTermios, "CR2", INT2FIX(CR2));
#endif
#ifdef CR3
  rb_define_const(mTermios, "CR3", INT2FIX(CR3));
#endif
#ifdef TABDLY
  rb_define_const(mTermios, "TABDLY", INT2FIX(TABDLY));
#endif
#ifdef TAB0
  rb_define_const(mTermios, "TAB0", INT2FIX(TAB0));
#endif
#ifdef TAB1
  rb_define_const(mTermios, "TAB1", INT2FIX(TAB1));
#endif
#ifdef TAB2
  rb_define_const(mTermios, "TAB2", INT2FIX(TAB2));
#endif
#ifdef TAB3
  rb_define_const(mTermios, "TAB3", INT2FIX(TAB3));
#endif
#ifdef XTABS
  rb_define_const(mTermios, "XTABS", INT2FIX(XTABS));
#endif
#ifdef BSDLY
  rb_define_const(mTermios, "BSDLY", INT2FIX(BSDLY));
#endif
#ifdef BS0
  rb_define_const(mTermios, "BS0", INT2FIX(BS0));
#endif
#ifdef BS1
  rb_define_const(mTermios, "BS1", INT2FIX(BS1));
#endif
#ifdef VTDLY
  rb_define_const(mTermios, "VTDLY", INT2FIX(VTDLY));
#endif
#ifdef VT0
  rb_define_const(mTermios, "VT0", INT2FIX(VT0));
#endif
#ifdef VT1
  rb_define_const(mTermios, "VT1", INT2FIX(VT1));
#endif
#ifdef FFDLY
  rb_define_const(mTermios, "FFDLY", INT2FIX(FFDLY));
#endif
#ifdef FF0
  rb_define_const(mTermios, "FF0", INT2FIX(FF0));
#endif
#ifdef FF1
  rb_define_const(mTermios, "FF1", INT2FIX(FF1));
#endif

  /* c_cflag bit meaning */
#ifdef CBAUD
  rb_define_const(mTermios, "CBAUD", INT2FIX(CBAUD));
#endif
#ifdef B0
  rb_define_const(mTermios, "B0", INT2FIX(B0));
#endif
#ifdef B50
  rb_define_const(mTermios, "B50", INT2FIX(B50));
#endif
#ifdef B75
  rb_define_const(mTermios, "B75", INT2FIX(B75));
#endif
#ifdef B110
  rb_define_const(mTermios, "B110", INT2FIX(B110));
#endif
#ifdef B134
  rb_define_const(mTermios, "B134", INT2FIX(B134));
#endif
#ifdef B150
  rb_define_const(mTermios, "B150", INT2FIX(B150));
#endif
#ifdef B200
  rb_define_const(mTermios, "B200", INT2FIX(B200));
#endif
#ifdef B300
  rb_define_const(mTermios, "B300", INT2FIX(B300));
#endif
#ifdef B600
  rb_define_const(mTermios, "B600", INT2FIX(B600));
#endif
#ifdef B1200
  rb_define_const(mTermios, "B1200", INT2FIX(B1200));
#endif
#ifdef B1800
  rb_define_const(mTermios, "B1800", INT2FIX(B1800));
#endif
#ifdef B2400
  rb_define_const(mTermios, "B2400", INT2FIX(B2400));
#endif
#ifdef B4800
  rb_define_const(mTermios, "B4800", INT2FIX(B4800));
#endif
#ifdef B9600
  rb_define_const(mTermios, "B9600", INT2FIX(B9600));
#endif
#ifdef B19200
  rb_define_const(mTermios, "B19200", INT2FIX(B19200));
#endif
#ifdef B38400
  rb_define_const(mTermios, "B38400", INT2FIX(B38400));
#endif
#ifdef EXTA
  rb_define_const(mTermios, "EXTA", INT2FIX(EXTA));
#endif
#ifdef EXTB
  rb_define_const(mTermios, "EXTB", INT2FIX(EXTB));
#endif
#ifdef CSIZE
  rb_define_const(mTermios, "CSIZE", INT2FIX(CSIZE));
#endif
#ifdef CS5
  rb_define_const(mTermios, "CS5", INT2FIX(CS5));
#endif
#ifdef CS6
  rb_define_const(mTermios, "CS6", INT2FIX(CS6));
#endif
#ifdef CS7
  rb_define_const(mTermios, "CS7", INT2FIX(CS7));
#endif
#ifdef CS8
  rb_define_const(mTermios, "CS8", INT2FIX(CS8));
#endif
#ifdef CSTOPB
  rb_define_const(mTermios, "CSTOPB", INT2FIX(CSTOPB));
#endif
#ifdef CREAD
  rb_define_const(mTermios, "CREAD", INT2FIX(CREAD));
#endif
#ifdef PARENB
  rb_define_const(mTermios, "PARENB", INT2FIX(PARENB));
#endif
#ifdef PARODD
  rb_define_const(mTermios, "PARODD", INT2FIX(PARODD));
#endif
#ifdef HUPCL
  rb_define_const(mTermios, "HUPCL", INT2FIX(HUPCL));
#endif
#ifdef CLOCAL
  rb_define_const(mTermios, "CLOCAL", INT2FIX(CLOCAL));
#endif
#ifdef CBAUDEX
  rb_define_const(mTermios, "CBAUDEX", INT2FIX(CBAUDEX));
#endif
#ifdef B57600
  rb_define_const(mTermios, "B57600", INT2FIX(B57600));
#endif
#ifdef B115200
  rb_define_const(mTermios, "B115200", INT2FIX(B115200));
#endif
#ifdef B230400
  rb_define_const(mTermios, "B230400", INT2FIX(B230400));
#endif
#ifdef B460800
  rb_define_const(mTermios, "B460800", INT2FIX(B460800));
#endif
#ifdef CIBAUD
  rb_define_const(mTermios, "CIBAUD", INT2FIX(CIBAUD));
#endif
#ifdef CRTSCTS
  rb_define_const(mTermios, "CRTSCTS", INT2FIX(CRTSCTS));
#endif

  /* c_lflag bits */
#ifdef ISIG
  rb_define_const(mTermios, "ISIG", INT2FIX(ISIG));
#endif
#ifdef ICANON
  rb_define_const(mTermios, "ICANON", INT2FIX(ICANON));
#endif
#ifdef XCASE
  rb_define_const(mTermios, "XCASE", INT2FIX(XCASE));
#endif
#ifdef ECHO
  rb_define_const(mTermios, "ECHO", INT2FIX(ECHO));
#endif
#ifdef ECHOE
  rb_define_const(mTermios, "ECHOE", INT2FIX(ECHOE));
#endif
#ifdef ECHOK
  rb_define_const(mTermios, "ECHOK", INT2FIX(ECHOK));
#endif
#ifdef ECHONL
  rb_define_const(mTermios, "ECHONL", INT2FIX(ECHONL));
#endif
#ifdef NOFLSH
  rb_define_const(mTermios, "NOFLSH", INT2FIX(NOFLSH));
#endif
#ifdef TOSTOP
  rb_define_const(mTermios, "TOSTOP", INT2FIX(TOSTOP));
#endif
#ifdef ECHOCTL
  rb_define_const(mTermios, "ECHOCTL", INT2FIX(ECHOCTL));
#endif
#ifdef ECHOPRT
  rb_define_const(mTermios, "ECHOPRT", INT2FIX(ECHOPRT));
#endif
#ifdef ECHOKE
  rb_define_const(mTermios, "ECHOKE", INT2FIX(ECHOKE));
#endif
#ifdef FLUSHO
  rb_define_const(mTermios, "FLUSHO", INT2FIX(FLUSHO));
#endif
#ifdef PENDIN
  rb_define_const(mTermios, "PENDIN", INT2FIX(PENDIN));
#endif
#ifdef IEXTEN
  rb_define_const(mTermios, "IEXTEN", INT2FIX(IEXTEN));
#endif

  /* tcflow() and TCXONC use these */
#ifdef TCOOFF
  rb_define_const(mTermios, "TCOOFF", INT2FIX(TCOOFF));
#endif
#ifdef TCOON
  rb_define_const(mTermios, "TCOON", INT2FIX(TCOON));
#endif
#ifdef TCIOFF
  rb_define_const(mTermios, "TCIOFF", INT2FIX(TCIOFF));
#endif
#ifdef TCION
  rb_define_const(mTermios, "TCION", INT2FIX(TCION));
#endif

  /* tcflush() and TCFLSH use these */
#ifdef TCIFLUSH
  rb_define_const(mTermios, "TCIFLUSH", INT2FIX(TCIFLUSH));
#endif
#ifdef TCOFLUSH
  rb_define_const(mTermios, "TCOFLUSH", INT2FIX(TCOFLUSH));
#endif
#ifdef TCIOFLUSH
  rb_define_const(mTermios, "TCIOFLUSH", INT2FIX(TCIOFLUSH));
#endif

  /* tcsetattr uses these */
#ifdef TCSANOW
  rb_define_const(mTermios, "TCSANOW", INT2FIX(TCSANOW));
#endif
#ifdef TCSADRAIN
  rb_define_const(mTermios, "TCSADRAIN", INT2FIX(TCSADRAIN));
#endif
#ifdef TCSAFLUSH
  rb_define_const(mTermios, "TCSAFLUSH", INT2FIX(TCSAFLUSH));
#endif
}
