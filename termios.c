/*

  A termios library for Ruby.
  Copyright (C) 1999, 2000, 2002 akira yamada.
  $Id: termios.c,v 1.6 2002-10-12 09:53:29 akira Exp $

 */

#include "ruby.h"
#include "rubyio.h"
#include <termios.h>
#include <unistd.h>
#include <string.h>

static VALUE mTermios;
static VALUE cTermios;

static void
free_termios(t)
    struct termios *t;
{
    if (t) {
	free(t);
    }
}

static struct termios *
Termios_to_termios(obj)
    VALUE obj;
{
    struct termios *t;
    Data_Get_Struct(obj, struct termios, t);

    return t;
}


static VALUE
termios_tcgetattr(io)
    VALUE io;
{
    struct termios *t;
    OpenFile *fptr;
    VALUE obj;

    obj = Data_Make_Struct(cTermios, struct termios, 0, free_termios, t);

    GetOpenFile(io, fptr);
    if (tcgetattr(fileno(fptr->f), t) < 0) {
	rb_raise(rb_eRuntimeError, 
		 "can't get terminal parameters (%s)", strerror(errno));
    }

    return obj;
}

static VALUE
termios_s_tcgetattr(obj, io)
    VALUE obj, io;
{
    return termios_tcgetattr(io);
}

static VALUE
termios_tcsetattr(io, opt, param)
    VALUE io, opt, param;
{
    VALUE old;
    OpenFile *fptr;
    struct termios *t;

    Check_Type(io,  T_FILE);
    Check_Type(opt, T_FIXNUM);
    if (CLASS_OF(param) != cTermios) {
	char *type = rb_class2name(CLASS_OF(param));
	rb_raise(rb_eTypeError, 
		 "wrong argument type %s (expected Struct::Termios)", 
		 type);
    }

    old = termios_tcgetattr(io);
    GetOpenFile(io, fptr);
    t = Termios_to_termios(param);
    if (tcsetattr(fileno(fptr->f), FIX2INT(opt), t) < 0) {
	rb_raise(rb_eRuntimeError,
		 "can't set terminal parameters (%s)", strerror(errno));
    }

    return old;
}

static VALUE
termios_s_tcsetattr(obj, io, opt, param)
    VALUE obj, io, opt, param;
{
    return termios_tcsetattr(io, opt, param);
}

static VALUE
termios_tcsendbreak(io, duration)
    VALUE io, duration;
{
    OpenFile *fptr;

    Check_Type(io,       T_FILE);
    Check_Type(duration, T_FIXNUM);

    GetOpenFile(io, fptr);
    if (tcsendbreak(fileno(fptr->f), FIX2INT(duration)) < 0) {
	rb_raise(rb_eRuntimeError, 
		 "can't transmits break (%s)", strerror(errno));
    }

    return Qtrue;
}

static VALUE
termios_s_tcsendbreak(obj, io, duration)
    VALUE obj, io, duration;
{
    return termios_tcsendbreak(io, duration);
}

static VALUE
termios_tcdrain(io)
    VALUE io;
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
termios_s_tcdrain(obj, io)
    VALUE obj, io;
{
    return termios_tcdrain(io);
}

static VALUE
termios_tcflush(io, qs)
    VALUE io, qs;
{
    OpenFile *fptr;
    int queue_selector;

    Check_Type(io, T_FILE);
    Check_Type(qs, T_FIXNUM);
    queue_selector = FIX2INT(qs);
    if (queue_selector != TCIFLUSH &&
	queue_selector != TCOFLUSH &&
	queue_selector != TCIOFLUSH) {
	rb_raise(rb_eTypeError, 
		 "wrong argument value %s (expected Termios::TCIFLUSH, Termios::TCOFLUSH or Termios::TCIOFLUSH)", 
		 queue_selector);
    }

    GetOpenFile(io, fptr);
    if (tcflush(fileno(fptr->f), queue_selector) < 0) {
	rb_raise(rb_eRuntimeError, "can't flush (%s)", strerror(errno));
    }

    return Qtrue;
}

static VALUE
termios_s_tcflush(obj, io, qs)
    VALUE obj, io, qs;
{
    return termios_tcflush(io, qs);
}

static VALUE
termios_tcflow(io, act)
    VALUE io, act;
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
	rb_raise(rb_eTypeError, 
		 "wrong argument value %s (expected Termios::TCOOFF, Termios::TCOON, Termios::TCIOFF or Termios::TCION)", 
		 action);
    }

    GetOpenFile(io, fptr);
    if (tcflow(fileno(fptr->f), action) < 0) {
	rb_raise(rb_eRuntimeError, 
		 "can't control transmitting data flow (%s)", strerror(errno));
    }

    return Qtrue;
}

static VALUE
termios_s_tcflow(obj, io, act)
    VALUE obj, io, act;
{
    return termios_tcflow(io, act);
}

static VALUE
termios_tcgetpgrp(io)
    VALUE io;
{
    OpenFile *fptr;
    int pid;

    Check_Type(io,  T_FILE);
    GetOpenFile(io, fptr);
    if ((pid = tcgetpgrp(fileno(fptr->f))) < 0) {
	rb_raise(rb_eRuntimeError, 
		 "can't get process group id (%s)", strerror(errno));
    }

    return INT2FIX(pid);
}

static VALUE
termios_s_tcgetpgrp(obj, io)
    VALUE obj, io;
{
    return termios_tcgetpgrp(io);
}

static VALUE
termios_tcsetpgrp(io, pgrpid)
    VALUE io, pgrpid;
{
    OpenFile *fptr;

    Check_Type(io,     T_FILE);
    Check_Type(pgrpid, T_FIXNUM);

    GetOpenFile(io, fptr);
    if (tcsetpgrp(fileno(fptr->f), FIX2INT(pgrpid)) < 0) {
	rb_raise(rb_eRuntimeError, 
		 "can't set process group id (%s)", strerror(errno));
    }

    return Qtrue;
}

static VALUE
termios_s_tcsetpgrp(obj, io, pgrpid)
    VALUE obj, io, pgrpid;
{
    return termios_tcsetpgrp(io, pgrpid);
}

static VALUE
termios_s_new(argc, argv, io)
    int argc;
    VALUE *argv;
    VALUE io;
{
    VALUE obj, c_iflag, c_oflag, c_cflag, c_lflag, c_cc, c_ispeed, c_ospeed;
    int i;
    struct termios *t;

    rb_scan_args(argc, argv, "07", 
		 &c_iflag, &c_oflag, &c_cflag, &c_lflag, 
		 &c_cc, &c_ispeed, &c_ospeed);

    obj = Data_Make_Struct(cTermios, struct termios, 0, free_termios, t);

    if (NIL_P(c_iflag)) {
	t->c_iflag = 0;
    }
    else {
	Check_Type(c_iflag, T_FIXNUM);
	t->c_iflag = FIX2INT(c_iflag);
    }

    if (NIL_P(c_iflag)) {
	t->c_oflag = 0;
    }
    else {
	Check_Type(c_oflag, T_FIXNUM);
	t->c_oflag = FIX2INT(c_oflag);
    }

    if (NIL_P(c_iflag)) {
	t->c_cflag = 0;
    }
    else {
	Check_Type(c_cflag, T_FIXNUM);
	t->c_cflag = FIX2INT(c_cflag);
    }

    if (NIL_P(c_iflag)) {
	t->c_lflag = 0;
    }
    else {
	Check_Type(c_lflag, T_FIXNUM);
	t->c_lflag = FIX2INT(c_lflag);
    }

    if (NIL_P(c_cc)) {
	for (i = 0; i < NCCS; i++) {
	    t->c_cc[i] = 0;
	}
    }
    else {
	Check_Type(c_cc, T_ARRAY);
	for (i = 0; i < NCCS && i < RARRAY(c_cc)->len; i++) {
	    t->c_cc[i] = FIX2INT(RARRAY(c_cc)->ptr[i]);
	}
    }

    if (NIL_P(c_iflag)) {
	cfsetispeed(t, 0);
    }
    else {
	Check_Type(c_ispeed, T_FIXNUM);
	cfsetispeed(t, FIX2INT(c_ispeed));
    }

    if (NIL_P(c_iflag)) {
	cfsetospeed(t, 0);
    }
    else {
	Check_Type(c_ospeed, T_FIXNUM);
	cfsetospeed(t, FIX2INT(c_ospeed));
    }

    return obj;
}

static VALUE
termios_iflag(self)
    VALUE self;
{
    struct termios *t;

    Data_Get_Struct(self, struct termios, t);
    return INT2FIX(t->c_iflag);
}

static VALUE
termios_set_iflag(self, value)
    VALUE self, value;
{
    struct termios *t;

    Check_Type(value, T_FIXNUM);
    Data_Get_Struct(self, struct termios, t);
    t->c_iflag = FIX2INT(value);

    return value;
}

static VALUE
termios_oflag(self)
    VALUE self;
{
    struct termios *t;

    Data_Get_Struct(self, struct termios, t);
    return INT2FIX(t->c_oflag);
}

static VALUE
termios_set_oflag(self, value)
    VALUE self, value;
{
    struct termios *t;

    Check_Type(value, T_FIXNUM);
    Data_Get_Struct(self, struct termios, t);
    t->c_oflag = FIX2INT(value);

    return value;
}

static VALUE
termios_cflag(self)
    VALUE self;
{
    struct termios *t;

    Data_Get_Struct(self, struct termios, t);
    return INT2FIX(t->c_cflag);
}

static VALUE
termios_set_cflag(self, value)
    VALUE self, value;
{
    struct termios *t;

    Check_Type(value, T_FIXNUM);
    Data_Get_Struct(self, struct termios, t);
    t->c_cflag = FIX2INT(value);

    return value;
}

static VALUE
termios_lflag(self)
    VALUE self;
{
    struct termios *t;

    Data_Get_Struct(self, struct termios, t);
    return INT2FIX(t->c_lflag);
}

static VALUE
termios_set_lflag(self, value)
    VALUE self, value;
{
    struct termios *t;

    Check_Type(value, T_FIXNUM);
    Data_Get_Struct(self, struct termios, t);
    t->c_lflag = FIX2INT(value);

    return value;
}

static VALUE
termios_cc(argc, argv, self)
    int argc;
    VALUE *argv;
    VALUE self;
{
    int i;
    struct termios *t;
    VALUE ret, index;

    rb_scan_args(argc, argv, "01", &index);
    Data_Get_Struct(self, struct termios, t);

    if (NIL_P(index)) {
      ret = rb_ary_new2(NCCS);
      for (i = 0; i < NCCS; i++) {
  	  rb_ary_push(ret, INT2FIX(t->c_cc[i]));
      }
    }
    else {
      Check_Type(index, T_FIXNUM);
      i = NUM2INT(index);
      if (i < 0 || NCCS <= i) {
	rb_raise(rb_eArgError, "bad number for control charcters");
      }

      ret = INT2NUM(t->c_cc[i]);
    }

    return ret;
}

static VALUE
termios_set_cc(self, value)
    VALUE self, value;
{
    int i;
    struct termios *t;

    Check_Type(value, T_ARRAY);
    Data_Get_Struct(self, struct termios, t);
    for (i = 0; i < NCCS && i < RARRAY(value)->len; i++) {
	t->c_cc[i] = FIX2INT(RARRAY(value)->ptr[i]);
    }

    return value;
}

static VALUE
termios_set_a_cc(argc, argv, self)
    int argc;
    VALUE *argv;
    VALUE self;
{
    VALUE index, value;
    struct termios *t;
    int i;

    rb_scan_args(argc, argv, "11", &index, &value);
    if (NIL_P(value)) {
      Check_Type(index, T_ARRAY);
      value = termios_set_cc(self, index);
    }
    else {
      Check_Type(index, T_FIXNUM);
      Check_Type(value, T_FIXNUM);

      i = NUM2INT(index);
      if (i < 0 || NCCS <= i) {
	rb_raise(rb_eArgError, "bad number for control charcters");
      }

      Data_Get_Struct(self, struct termios, t);
      t->c_cc[i] = NUM2INT(value);
    }

    return value;
}

static VALUE
termios_ispeed(self)
    VALUE self;
{
    struct termios *t;

    Data_Get_Struct(self, struct termios, t);
    return INT2FIX(cfgetispeed(t));
}

static VALUE
termios_set_ispeed(self, value)
    VALUE self, value;
{
    struct termios *t;

    Check_Type(value, T_FIXNUM);
    Data_Get_Struct(self, struct termios, t);
    cfsetispeed(t, FIX2INT(value));

    return value;
}

static VALUE
termios_ospeed(self)
    VALUE self;
{
    struct termios *t;

    Data_Get_Struct(self, struct termios, t);
    return INT2FIX(cfgetospeed(t));
}

static VALUE
termios_set_ospeed(self, value)
    VALUE self, value;
{
    struct termios *t;

    Check_Type(value, T_FIXNUM);
    Data_Get_Struct(self, struct termios, t);
    cfsetospeed(t, FIX2INT(value));

    return value;
}

static VALUE
termios_clone(self)
    VALUE self;
{
    struct termios *old, *new;
    VALUE obj;

    obj = Data_Make_Struct(cTermios, struct termios, 0, free_termios, new);
    Data_Get_Struct(self, struct termios, old);
    memcpy(new, old, sizeof(struct termios));

    return obj;
}

void
Init_termios()
{
    VALUE ccindex, iflags, oflags, cflags, lflags, baud;

    /* module Termios */

    mTermios = rb_define_module("Termios");

    rb_define_module_function(mTermios,"tcgetattr",  termios_s_tcgetattr,  1);
    rb_define_module_function(mTermios,  "getattr",  termios_s_tcgetattr,  1);
    rb_define_method(mTermios,         "tcgetattr",  termios_tcgetattr,    0);

    rb_define_module_function(mTermios,"tcsetattr",  termios_s_tcsetattr,  3);
    rb_define_module_function(mTermios,  "setattr",  termios_s_tcsetattr,  3);
    rb_define_method(mTermios,         "tcsetattr",  termios_tcsetattr,    2);

    rb_define_module_function(mTermios,"tcsendbreak",termios_s_tcsendbreak,2);
    rb_define_module_function(mTermios,  "sendbreak",termios_s_tcsendbreak,2);
    rb_define_method(mTermios,         "tcsendbreak",termios_tcsendbreak,  1);

    rb_define_module_function(mTermios,"tcdrain",    termios_s_tcdrain,    1);
    rb_define_module_function(mTermios,  "drain",    termios_s_tcdrain,    1);
    rb_define_method(mTermios,         "tcdrain",    termios_s_tcdrain,    0);

    rb_define_module_function(mTermios,"tcflush",    termios_s_tcflush,    2);
    rb_define_module_function(mTermios,  "flush",    termios_s_tcflush,    2);
    rb_define_method(mTermios,         "tcflush",    termios_s_tcflush,    1);

    rb_define_module_function(mTermios,"tcflow",     termios_s_tcflow,     2);
    rb_define_module_function(mTermios,  "flow",     termios_s_tcflow,     2);
    rb_define_method(mTermios,         "tcflow",     termios_s_tcflow,     1);

    rb_define_module_function(mTermios,"tcgetpgrp",  termios_s_tcgetpgrp,  1);
    rb_define_module_function(mTermios,  "getpgrp",  termios_s_tcgetpgrp,  1);
    rb_define_method(mTermios,         "tcgetpgrp",  termios_s_tcgetpgrp,  0);

    rb_define_module_function(mTermios,"tcsetpgrp",  termios_s_tcsetpgrp,  2);
    rb_define_module_function(mTermios,  "setpgrp",  termios_s_tcsetpgrp,  2);
    rb_define_method(mTermios,         "tcsetpgrp",  termios_s_tcsetpgrp,  1);

    rb_define_module_function(mTermios,"new_termios",termios_s_new,       -1);

    /* class Termios::Termios */

    cTermios = rb_define_class_under(mTermios, "Termios", rb_cObject);

    rb_define_singleton_method(cTermios, "new", termios_s_new, -1);

    rb_define_method(cTermios,   "iflag",   termios_iflag,      0);
    rb_define_method(cTermios, "c_iflag",   termios_iflag,      0);
    rb_define_method(cTermios,   "iflag=",  termios_set_iflag,  1);
    rb_define_method(cTermios, "c_iflag=",  termios_set_iflag,  1);

    rb_define_method(cTermios,   "oflag",   termios_oflag,      0);
    rb_define_method(cTermios, "c_oflag",   termios_oflag,      0);
    rb_define_method(cTermios,   "oflag=",  termios_set_oflag,  1);
    rb_define_method(cTermios, "c_oflag=",  termios_set_oflag,  1);

    rb_define_method(cTermios,   "cflag",   termios_cflag,      0);
    rb_define_method(cTermios, "c_cflag",   termios_cflag,      0);
    rb_define_method(cTermios,   "cflag=",  termios_set_cflag,  1);
    rb_define_method(cTermios, "c_cflag=",  termios_set_cflag,  1);

    rb_define_method(cTermios,   "lflag",   termios_lflag,      0);
    rb_define_method(cTermios, "c_lflag",   termios_lflag,      0);
    rb_define_method(cTermios,   "lflag=",  termios_set_lflag,  1);
    rb_define_method(cTermios, "c_lflag=",  termios_set_lflag,  1);

    rb_define_method(cTermios,   "cc",      termios_cc,         -1);
    rb_define_method(cTermios, "c_cc",      termios_cc,         -1);
    rb_define_method(cTermios,   "cc=",     termios_set_cc,     1);
    rb_define_method(cTermios, "c_cc=",     termios_set_cc,     1);
    rb_define_method(cTermios, "set_cc",    termios_set_a_cc,   -1);

    rb_define_method(cTermios,   "ispeed",  termios_ispeed,     0);
    rb_define_method(cTermios, "c_ispeed",  termios_ispeed,     0);
    rb_define_method(cTermios,   "ispeed=", termios_set_ispeed, 1);
    rb_define_method(cTermios, "c_ispeed=", termios_set_ispeed, 1);

    rb_define_method(cTermios,   "ospeed",  termios_ospeed,     0);
    rb_define_method(cTermios, "c_ospeed",  termios_ospeed,     0);
    rb_define_method(cTermios,   "ospeed=", termios_set_ospeed, 1);
    rb_define_method(cTermios, "c_ospeed=", termios_set_ospeed, 1);

    rb_define_method(cTermios, "clone",     termios_clone,      0);

    /* constants under Termios module */

    rb_define_const(mTermios, "NCCS",    INT2FIX(NCCS));

    ccindex = rb_hash_new();
    rb_define_const(mTermios, "CCINDEX", ccindex);

    iflags = rb_hash_new();
    rb_define_const(mTermios, "IFLAGS", iflags);

    oflags = rb_hash_new();
    rb_define_const(mTermios, "OFLAGS", oflags);

    cflags = rb_hash_new();
    rb_define_const(mTermios, "CFLAGS", cflags);

    lflags = rb_hash_new();
    rb_define_const(mTermios, "LFLAGS", lflags);

    baud = rb_hash_new();
    rb_define_const(mTermios, "BAUD", baud);

#define define_flag(hash, flag) \
    { \
      rb_define_const(mTermios, #flag, INT2FIX(flag)); \
      rb_hash_aset(hash, rb_const_get(mTermios, rb_intern(#flag)), \
	  ID2SYM(rb_intern(#flag))); \
    }

    /* c_cc characters */
#ifdef VINTR
    define_flag(ccindex, VINTR);
#endif
#ifdef VQUIT
    define_flag(ccindex, VQUIT);
#endif
#ifdef VERASE
    define_flag(ccindex, VERASE);
#endif
#ifdef VKILL
    define_flag(ccindex, VKILL);
#endif
#ifdef VEOF
    define_flag(ccindex, VEOF);
#endif
#ifdef VTIME
    define_flag(ccindex, VTIME);
#endif
#ifdef VMIN
    define_flag(ccindex, VMIN);
#endif
#ifdef VSWTC
    define_flag(ccindex, VSWTC);
#endif
#ifdef VSTART
    define_flag(ccindex, VSTART);
#endif
#ifdef VSTOP
    define_flag(ccindex, VSTOP);
#endif
#ifdef VSUSP
    define_flag(ccindex, VSUSP);
#endif
#ifdef VEOL
    define_flag(ccindex, VEOL);
#endif
#ifdef VREPRINT
    define_flag(ccindex, VREPRINT);
#endif
#ifdef VDISCARD
    define_flag(ccindex, VDISCARD);
#endif
#ifdef VWERASE
    define_flag(ccindex, VWERASE);
#endif
#ifdef VLNEXT
    define_flag(ccindex, VLNEXT);
#endif
#ifdef VEOL2
    define_flag(ccindex, VEOL2);
#endif

    /* c_iflag bits */
#ifdef IGNBRK
    define_flag(iflags, IGNBRK);
#endif
#ifdef BRKINT
    define_flag(iflags, BRKINT);
#endif
#ifdef IGNPAR
    define_flag(iflags, IGNPAR);
#endif
#ifdef PARMRK
    define_flag(iflags, PARMRK);
#endif
#ifdef INPCK
    define_flag(iflags, INPCK);
#endif
#ifdef ISTRIP
    define_flag(iflags, ISTRIP);
#endif
#ifdef INLCR
    define_flag(iflags, INLCR);
#endif
#ifdef IGNCR
    define_flag(iflags, IGNCR);
#endif
#ifdef ICRNL
    define_flag(iflags, ICRNL);
#endif
#ifdef IUCLC
    define_flag(iflags, IUCLC);
#endif
#ifdef IXON
    define_flag(iflags, IXON);
#endif
#ifdef IXANY
    define_flag(iflags, IXANY);
#endif
#ifdef IXOFF
    define_flag(iflags, IXOFF);
#endif
#ifdef IMAXBEL
    define_flag(iflags, IMAXBEL);
#endif

    /* c_oflag bits */
#ifdef OPOST
    define_flag(oflags, OPOST);
#endif
#ifdef OLCUC
    define_flag(oflags, OLCUC);
#endif
#ifdef ONLCR
    define_flag(oflags, ONLCR);
#endif
#ifdef OCRNL
    define_flag(oflags, OCRNL);
#endif
#ifdef ONOCR
    define_flag(oflags, ONOCR);
#endif
#ifdef ONLRET
    define_flag(oflags, ONLRET);
#endif
#ifdef OFILL
    define_flag(oflags, OFILL);
#endif
#ifdef OFDEL
    define_flag(oflags, OFDEL);
#endif
#ifdef NLDLY
    define_flag(oflags, NLDLY);
#endif
#ifdef NL0
    define_flag(oflags, NL0);
#endif
#ifdef NL1
    define_flag(oflags, NL1);
#endif
#ifdef CRDLY
    define_flag(oflags, CRDLY);
#endif
#ifdef CR0
    define_flag(oflags, CR0);
#endif
#ifdef CR1
    define_flag(oflags, CR1);
#endif
#ifdef CR2
    define_flag(oflags, CR2);
#endif
#ifdef CR3
    define_flag(oflags, CR3);
#endif
#ifdef TABDLY
    define_flag(oflags, TABDLY);
#endif
#ifdef TAB0
    define_flag(oflags, TAB0);
#endif
#ifdef TAB1
    define_flag(oflags, TAB1);
#endif
#ifdef TAB2
    define_flag(oflags, TAB2);
#endif
#ifdef TAB3
    define_flag(oflags, TAB3);
#endif
#ifdef XTABS
    define_flag(oflags, XTABS);
#endif
#ifdef BSDLY
    define_flag(oflags, BSDLY);
#endif
#ifdef BS0
    define_flag(oflags, BS0);
#endif
#ifdef BS1
    define_flag(oflags, BS1);
#endif
#ifdef VTDLY
    define_flag(oflags, VTDLY);
#endif
#ifdef VT0
    define_flag(oflags, VT0);
#endif
#ifdef VT1
    define_flag(oflags, VT1);
#endif
#ifdef FFDLY
    define_flag(oflags, FFDLY);
#endif
#ifdef FF0
    define_flag(oflags, FF0);
#endif
#ifdef FF1
    define_flag(oflags, FF1);
#endif

    /* c_cflag bit meaning */
#ifdef CBAUD
    define_flag(cflags, CBAUD);
#endif
#ifdef B0
    define_flag(baud, B0);
#endif
#ifdef B50
    define_flag(baud, B50);
#endif
#ifdef B75
    define_flag(baud, B75);
#endif
#ifdef B110
    define_flag(baud, B110);
#endif
#ifdef B134
    define_flag(baud, B134);
#endif
#ifdef B150
    define_flag(baud, B150);
#endif
#ifdef B200
    define_flag(baud, B200);
#endif
#ifdef B300
    define_flag(baud, B300);
#endif
#ifdef B600
    define_flag(baud, B600);
#endif
#ifdef B1200
    define_flag(baud, B1200);
#endif
#ifdef B1800
    define_flag(baud, B1800);
#endif
#ifdef B2400
    define_flag(baud, B2400);
#endif
#ifdef B4800
    define_flag(baud, B4800);
#endif
#ifdef B9600
    define_flag(baud, B9600);
#endif
#ifdef B19200
    define_flag(baud, B19200);
#endif
#ifdef B38400
    define_flag(baud, B38400);
#endif
#ifdef EXTA
    define_flag(cflags, EXTA);
#endif
#ifdef EXTB
    define_flag(cflags, EXTB);
#endif
#ifdef CSIZE
    define_flag(cflags, CSIZE);
#endif
#ifdef CS5
    define_flag(cflags, CS5);
#endif
#ifdef CS6
    define_flag(cflags, CS6);
#endif
#ifdef CS7
    define_flag(cflags, CS7);
#endif
#ifdef CS8
    define_flag(cflags, CS8);
#endif
#ifdef CSTOPB
    define_flag(cflags, CSTOPB);
#endif
#ifdef CREAD
    define_flag(cflags, CREAD);
#endif
#ifdef PARENB
    define_flag(cflags, PARENB);
#endif
#ifdef PARODD
    define_flag(cflags, PARODD);
#endif
#ifdef HUPCL
    define_flag(cflags, HUPCL);
#endif
#ifdef CLOCAL
    define_flag(cflags, CLOCAL);
#endif
#ifdef CBAUDEX
    define_flag(cflags, CBAUDEX);
#endif
#ifdef B57600
    define_flag(baud, B57600);
#endif
#ifdef B115200
    define_flag(baud, B115200);
#endif
#ifdef B230400
    define_flag(baud, B230400);
#endif
#ifdef B460800
    define_flag(baud, B460800);
#endif
#ifdef CIBAUD
    define_flag(cflags, CIBAUD);
#endif
#ifdef CRTSCTS
    define_flag(cflags, CRTSCTS);
#endif

    /* c_lflag bits */
#ifdef ISIG
    define_flag(lflags, ISIG);
#endif
#ifdef ICANON
    define_flag(lflags, ICANON);
#endif
#ifdef XCASE
    define_flag(lflags, XCASE);
#endif
#ifdef ECHO
    define_flag(lflags, ECHO);
#endif
#ifdef ECHOE
    define_flag(lflags, ECHOE);
#endif
#ifdef ECHOK
    define_flag(lflags, ECHOK);
#endif
#ifdef ECHONL
    define_flag(lflags, ECHONL);
#endif
#ifdef NOFLSH
    define_flag(lflags, NOFLSH);
#endif
#ifdef TOSTOP
    define_flag(lflags, TOSTOP);
#endif
#ifdef ECHOCTL
    define_flag(lflags, ECHOCTL);
#endif
#ifdef ECHOPRT
    define_flag(lflags, ECHOPRT);
#endif
#ifdef ECHOKE
    define_flag(lflags, ECHOKE);
#endif
#ifdef FLUSHO
    define_flag(lflags, FLUSHO);
#endif
#ifdef PENDIN
    define_flag(lflags, PENDIN);
#endif
#ifdef IEXTEN
    define_flag(lflags, IEXTEN);
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
#ifdef TCSASOFT
    rb_define_const(mTermios, "TCSASOFT", INT2FIX(TCSASOFT));
#endif
}
