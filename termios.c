/*

  A termios library for Ruby.
  Copyright (C) 1999, 2000, 2002 akira yamada.
  $Id: termios.c,v 1.10 2007-05-30 04:09:04 akira Exp $

 */

#include "ruby.h"
#include "rubyio.h"
#include <termios.h>
#include <unistd.h>
#include <string.h>

#ifdef HAVE_TYPE_RB_IO_T
typedef rb_io_t OpenFile;
#endif

#if defined(HAVE_TYPE_RB_IO_T) ||  defined(HAVE_ST_FD)
# define FILENO(fptr) (fptr->fd)
#else
# define FILENO(fptr) fileno(fptr->f)
#endif

#define validate_ulong(v) ULONG2NUM(NUM2ULONG(v))

static VALUE mTermios;
static VALUE cTermios;
static VALUE tcsetattr_opt, tcflush_qs, tcflow_act;
static ID id_iflag, id_oflag, id_cflag, id_lflag, id_cc, id_ispeed, id_ospeed;

static VALUE
termios_set_iflag(self, value)
    VALUE self, value;
{
    rb_ivar_set(self, id_iflag, validate_ulong(value));

    return value;
}

static VALUE
termios_set_oflag(self, value)
    VALUE self, value;
{
    rb_ivar_set(self, id_oflag, validate_ulong(value));

    return value;
}

static VALUE
termios_set_cflag(self, value)
    VALUE self, value;
{
    rb_ivar_set(self, id_cflag, validate_ulong(value));

    return value;
}

static VALUE
termios_set_lflag(self, value)
    VALUE self, value;
{
    rb_ivar_set(self, id_lflag, validate_ulong(value));

    return value;
}

static VALUE
termios_set_cc(self, value)
    VALUE self, value;
{
    Check_Type(value, T_ARRAY);
    rb_ivar_set(self, id_cc, value);

    return value;
}

static VALUE
termios_set_ispeed(self, value)
    VALUE self, value;
{
    rb_ivar_set(self, id_ispeed, validate_ulong(value));

    return value;
}

static VALUE
termios_set_ospeed(self, value)
    VALUE self, value;
{
    rb_ivar_set(self, id_ospeed, validate_ulong(value));

    return value;
}

static VALUE
termios_initialize(argc, argv, self)
    int argc;
    VALUE *argv;
    VALUE self;
{
    VALUE c_iflag, c_oflag, c_cflag, c_lflag, c_cc, c_ispeed, c_ospeed;
    VALUE cc_ary;
    int i;

    cc_ary = rb_ary_new2(NCCS);
    for (i = 0; i < NCCS; i++) {
	rb_ary_store(cc_ary, i, INT2FIX(0));
    }

    rb_ivar_set(self, id_iflag,  INT2FIX(0));
    rb_ivar_set(self, id_oflag,  INT2FIX(0));
    rb_ivar_set(self, id_cflag,  INT2FIX(0));
    rb_ivar_set(self, id_lflag,  INT2FIX(0));
    rb_ivar_set(self, id_cc,     cc_ary);
    rb_ivar_set(self, id_ispeed, INT2FIX(0));
    rb_ivar_set(self, id_ospeed, INT2FIX(0));

    rb_scan_args(argc, argv, "07", 
		 &c_iflag, &c_oflag, &c_cflag, &c_lflag, 
		 &c_cc, &c_ispeed, &c_ospeed);

    if (!NIL_P(c_iflag))
	termios_set_iflag(self, c_iflag);

    if (!NIL_P(c_oflag))
	termios_set_oflag(self, c_oflag);

    if (!NIL_P(c_cflag))
	termios_set_cflag(self, c_cflag);

    if (!NIL_P(c_lflag))
	termios_set_lflag(self, c_lflag);

    if (!NIL_P(c_cc))
	termios_set_cc(self, c_cc);

    if (!NIL_P(c_ispeed))
	termios_set_ispeed(self, c_ispeed);

    if (!NIL_P(c_ospeed))
	termios_set_ispeed(self, c_ospeed);

    return self;
}

static VALUE
termios_to_Termios(t)
    struct termios *t;
{
    int i;
    VALUE obj, cc_ary;

    obj = rb_funcall(cTermios, rb_intern("new"), 0);

    termios_set_iflag(obj, ULONG2NUM(t->c_iflag));
    termios_set_oflag(obj, ULONG2NUM(t->c_oflag));
    termios_set_cflag(obj, ULONG2NUM(t->c_cflag));
    termios_set_lflag(obj, ULONG2NUM(t->c_lflag));

    cc_ary = rb_ary_new2(NCCS);
    for (i = 0; i < NCCS; i++) {
	rb_ary_store(cc_ary, i, CHR2FIX(t->c_cc[i]));
    }
    termios_set_cc(obj, cc_ary);

    termios_set_ispeed(obj, ULONG2NUM(cfgetispeed(t)));
    termios_set_ospeed(obj, ULONG2NUM(cfgetospeed(t)));

    return obj;
}

static void
Termios_to_termios(obj, t)
    VALUE obj;
    struct termios *t;
{
    int i;
    VALUE cc_ary;

    t->c_iflag = NUM2ULONG(rb_ivar_get(obj, id_iflag));
    t->c_oflag = NUM2ULONG(rb_ivar_get(obj, id_oflag));
    t->c_cflag = NUM2ULONG(rb_ivar_get(obj, id_cflag));
    t->c_lflag = NUM2ULONG(rb_ivar_get(obj, id_lflag));

    cc_ary = rb_ivar_get(obj, id_cc);
    for (i = 0; i < NCCS; i++) {
        VALUE elt = rb_ary_entry(cc_ary, i);
        t->c_cc[i] = NUM2CHR(elt);
    }

    cfsetispeed(t, NUM2ULONG(rb_ivar_get(obj, id_ispeed)));
    cfsetospeed(t, NUM2ULONG(rb_ivar_get(obj, id_ospeed)));
}


static VALUE
termios_tcgetattr(io)
    VALUE io;
{
    struct termios t;
    OpenFile *fptr;

    Check_Type(io, T_FILE);
    GetOpenFile(io, fptr);
    if (tcgetattr(FILENO(fptr), &t) < 0) {
        rb_sys_fail("tcgetattr");
    }

    return termios_to_Termios(&t);
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
    struct termios t;
    int tcsetattr_option;

    Check_Type(io,  T_FILE);
    Check_Type(opt, T_FIXNUM);
    if (CLASS_OF(param) != cTermios) {
	char *type = rb_class2name(CLASS_OF(param));
	rb_raise(rb_eTypeError, 
		 "wrong argument type %s (expected Termios::Termios)", 
		 type);
    }

    tcsetattr_option = FIX2INT(opt);
    if (rb_ary_includes(tcsetattr_opt, opt) != Qtrue) {
	rb_raise(rb_eArgError, 
		 "wrong option value %d", tcsetattr_option);
    }

    old = termios_tcgetattr(io);
    GetOpenFile(io, fptr);
    Termios_to_termios(param, &t);
    if (tcsetattr(FILENO(fptr), tcsetattr_option, &t) < 0) {
        rb_sys_fail("tcsetattr");
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
    if (tcsendbreak(FILENO(fptr), FIX2INT(duration)) < 0) {
        rb_sys_fail("tcsendbreak");
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
    if (tcdrain(FILENO(fptr)) < 0) {
        rb_sys_fail("tcdrain");
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
    if (rb_ary_includes(tcflush_qs, qs) != Qtrue) {
	rb_raise(rb_eArgError, 
		 "wrong queue-selector value %d", queue_selector);
    }

    GetOpenFile(io, fptr);
    if (tcflush(FILENO(fptr), queue_selector) < 0) {
        rb_sys_fail("tcflush");
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
    if (rb_ary_includes(tcflow_act, act) != Qtrue) {
	rb_raise(rb_eArgError, 
		 "wrong action value %d", action);
    }

    GetOpenFile(io, fptr);
    if (tcflow(FILENO(fptr), action) < 0) {
        rb_sys_fail("tcflow");
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
    pid_t pid;

    Check_Type(io,  T_FILE);
    GetOpenFile(io, fptr);
    if ((pid = tcgetpgrp(FILENO(fptr))) < 0) {
        rb_sys_fail("tcgetpgrp");
    }

    return LONG2NUM(pid);
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
    pid_t pgrp;

    Check_Type(io,     T_FILE);
    pgrp = NUM2LONG(pgrpid);

    GetOpenFile(io, fptr);
    if (tcsetpgrp(FILENO(fptr), pgrp) < 0) {
        rb_sys_fail("tcsetpgrp");
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
termios_s_newtermios(argc, argv, klass)
    int argc;
    VALUE *argv;
    VALUE klass;
{
    return rb_funcall2(cTermios, rb_intern("new"), argc, argv);
}

static VALUE
termios_dup(self)
    VALUE self;
{
    VALUE result;
    VALUE cc_ary;
    
    result = rb_call_super(0, 0);
    cc_ary = rb_ivar_get(self, id_cc);
    rb_ivar_set(result, id_cc, rb_ary_dup(cc_ary));

    return result;
}

void
Init_termios()
{
    VALUE ccindex, ccindex_names;
    VALUE iflags, iflags_names;
    VALUE oflags, oflags_names, oflags_choices;
    VALUE cflags, cflags_names, cflags_choices;
    VALUE lflags, lflags_names;
    VALUE bauds, bauds_names;

    /* module Termios */

    mTermios = rb_define_module("Termios");

    rb_define_singleton_method(mTermios,"tcgetattr",  termios_s_tcgetattr,  1);
    rb_define_module_function(mTermios,   "getattr",  termios_s_tcgetattr,  1);
    rb_define_method(mTermios,          "tcgetattr",  termios_tcgetattr,    0);

    rb_define_singleton_method(mTermios,"tcsetattr",  termios_s_tcsetattr,  3);
    rb_define_module_function(mTermios,   "setattr",  termios_s_tcsetattr,  3);
    rb_define_method(mTermios,          "tcsetattr",  termios_tcsetattr,    2);

    rb_define_singleton_method(mTermios,"tcsendbreak",termios_s_tcsendbreak,2);
    rb_define_module_function(mTermios,   "sendbreak",termios_s_tcsendbreak,2);
    rb_define_method(mTermios,          "tcsendbreak",termios_tcsendbreak,  1);

    rb_define_singleton_method(mTermios,"tcdrain",    termios_s_tcdrain,    1);
    rb_define_module_function(mTermios,   "drain",    termios_s_tcdrain,    1);
    rb_define_method(mTermios,          "tcdrain",    termios_tcdrain,      0);

    rb_define_singleton_method(mTermios,"tcflush",    termios_s_tcflush,    2);
    rb_define_module_function(mTermios,   "flush",    termios_s_tcflush,    2);
    rb_define_method(mTermios,          "tcflush",    termios_tcflush,      1);

    rb_define_singleton_method(mTermios,"tcflow",     termios_s_tcflow,     2);
    rb_define_module_function(mTermios,   "flow",     termios_s_tcflow,     2);
    rb_define_method(mTermios,          "tcflow",     termios_tcflow,       1);

    rb_define_singleton_method(mTermios,"tcgetpgrp",  termios_s_tcgetpgrp,  1);
    rb_define_module_function(mTermios,   "getpgrp",  termios_s_tcgetpgrp,  1);
    rb_define_method(mTermios,          "tcgetpgrp",  termios_tcgetpgrp,    0);

    rb_define_singleton_method(mTermios,"tcsetpgrp",  termios_s_tcsetpgrp,  2);
    rb_define_module_function(mTermios,   "setpgrp",  termios_s_tcsetpgrp,  2);
    rb_define_method(mTermios,          "tcsetpgrp",  termios_tcsetpgrp,    1);

    rb_define_module_function(mTermios,"new_termios",termios_s_newtermios, -1);

    /* class Termios::Termios */

    cTermios = rb_define_class_under(mTermios, "Termios", rb_cObject);

    id_iflag  = rb_intern("@iflag");
    id_oflag  = rb_intern("@oflag");
    id_cflag  = rb_intern("@cflag");
    id_lflag  = rb_intern("@lflag");
    id_cc     = rb_intern("@cc");
    id_ispeed = rb_intern("@ispeed");
    id_ospeed = rb_intern("@ospeed");

    rb_attr(cTermios, rb_intern("iflag"),  1, 0, Qfalse);
    rb_attr(cTermios, rb_intern("oflag"),  1, 0, Qfalse);
    rb_attr(cTermios, rb_intern("cflag"),  1, 0, Qfalse);
    rb_attr(cTermios, rb_intern("lflag"),  1, 0, Qfalse);
    rb_attr(cTermios, rb_intern("cc"),     1, 0, Qfalse);
    rb_attr(cTermios, rb_intern("ispeed"), 1, 0, Qfalse);
    rb_attr(cTermios, rb_intern("ospeed"), 1, 0, Qfalse);

    rb_define_private_method(cTermios, "initialize", termios_initialize, -1);
    rb_define_method(cTermios, "dup", termios_dup, 0);
    rb_define_method(cTermios, "clone", termios_dup, 0);

    rb_define_method(cTermios, "iflag=",  termios_set_iflag,  1);
    rb_define_method(cTermios, "oflag=",  termios_set_oflag,  1);
    rb_define_method(cTermios, "cflag=",  termios_set_cflag,  1);
    rb_define_method(cTermios, "lflag=",  termios_set_lflag,  1);
    rb_define_method(cTermios, "cc=",     termios_set_cc,     1);
    rb_define_method(cTermios, "ispeed=", termios_set_ispeed, 1);
    rb_define_method(cTermios, "ospeed=", termios_set_ospeed, 1);

    rb_define_alias(cTermios, "c_iflag",   "iflag");
    rb_define_alias(cTermios, "c_iflag=",  "iflag=");
    rb_define_alias(cTermios, "c_oflag",   "oflag");
    rb_define_alias(cTermios, "c_oflag=",  "oflag=");
    rb_define_alias(cTermios, "c_cflag",   "cflag");
    rb_define_alias(cTermios, "c_cflag=",  "cflag=");
    rb_define_alias(cTermios, "c_lflag",   "lflag");
    rb_define_alias(cTermios, "c_lflag=",  "lflag=");
    rb_define_alias(cTermios, "c_cc",      "cc");
    rb_define_alias(cTermios, "c_cc=",     "cc=");
    rb_define_alias(cTermios, "c_ispeed",  "ispeed");
    rb_define_alias(cTermios, "c_ispeed=", "ispeed=");
    rb_define_alias(cTermios, "c_ospeed",  "ospeed");
    rb_define_alias(cTermios, "c_ospeed=", "ospeed=");

    /* constants under Termios module */

    rb_define_const(mTermios, "NCCS",    INT2FIX(NCCS));
    rb_define_const(mTermios, "POSIX_VDISABLE", INT2FIX(_POSIX_VDISABLE));

    ccindex = rb_hash_new();
    ccindex_names = rb_ary_new();
    rb_define_const(mTermios, "CCINDEX", ccindex);
    rb_define_const(mTermios, "CCINDEX_NAMES", ccindex_names);

    iflags = rb_hash_new();
    iflags_names = rb_ary_new();
    rb_define_const(mTermios, "IFLAGS", iflags);
    rb_define_const(mTermios, "IFLAG_NAMES", iflags_names);

    oflags = rb_hash_new();
    oflags_names = rb_ary_new();
    oflags_choices = rb_hash_new();
    rb_define_const(mTermios, "OFLAGS", oflags);
    rb_define_const(mTermios, "OFLAG_NAMES", oflags_names);
    rb_define_const(mTermios, "OFLAG_CHOICES", oflags_choices);

    cflags = rb_hash_new();
    cflags_names = rb_ary_new();
    cflags_choices = rb_hash_new();
    rb_define_const(mTermios, "CFLAGS", cflags);
    rb_define_const(mTermios, "CFLAG_NAMES", cflags_names);
    rb_define_const(mTermios, "CFLAG_CHOICES", cflags_choices);

    lflags = rb_hash_new();
    lflags_names = rb_ary_new();
    rb_define_const(mTermios, "LFLAGS", lflags);
    rb_define_const(mTermios, "LFLAG_NAMES", lflags_names);

    bauds = rb_hash_new();
    bauds_names = rb_ary_new();
    rb_define_const(mTermios, "BAUDS", bauds);
    rb_define_const(mTermios, "BAUD_NAMES", bauds_names);

    tcsetattr_opt = rb_ary_new();
    rb_define_const(mTermios, "SETATTR_OPTS", tcsetattr_opt);

    tcflush_qs = rb_ary_new();
    rb_define_const(mTermios, "FLUSH_QSELECTORS", tcflush_qs);

    tcflow_act = rb_ary_new();
    rb_define_const(mTermios, "FLOW_ACTIONS", tcflow_act);

#define define_flag(hash, flag) \
    { \
      rb_define_const(mTermios, #flag, ULONG2NUM(flag)); \
      rb_hash_aset(hash, rb_const_get(mTermios, rb_intern(#flag)), \
	  ID2SYM(rb_intern(#flag))); \
      rb_ary_push(hash##_names, ID2SYM(rb_intern(#flag)));\
    }
#define define_flag2(ary, flag) \
    { \
      rb_define_const(mTermios, #flag, INT2FIX(flag)); \
      rb_ary_push(ary, rb_const_get(mTermios, rb_intern(#flag)));\
    }
#define define_choice(hash, mask, value) \
    { \
      VALUE a; \
      rb_define_const(mTermios, #value, ULONG2NUM(value)); \
      rb_hash_aset(hash, rb_const_get(mTermios, rb_intern(#value)), \
	  ID2SYM(rb_intern(#value))); \
      rb_ary_push(hash##_names, ID2SYM(rb_intern(#value)));\
      a = rb_hash_aref(hash##_choices, ID2SYM(rb_intern(#mask))); \
      if (a == Qnil) { \
          a = rb_ary_new(); \
          rb_hash_aset(hash##_choices, ID2SYM(rb_intern(#mask)), a); \
      } \
      rb_ary_push(a, ID2SYM(rb_intern(#value))); \
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
#ifdef VEOL
    define_flag(ccindex, VEOL);
#endif
#ifdef VEOL2
    define_flag(ccindex, VEOL2);
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
#ifdef VDSUSP
    define_flag(ccindex, VDSUSP);
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
#ifdef VSTATUS
    define_flag(ccindex, VSTATUS);
#endif
#ifdef VTIME
    define_flag(ccindex, VTIME);
#endif
#ifdef VMIN
    define_flag(ccindex, VMIN);
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
#ifdef IXON
    define_flag(iflags, IXON);
#endif
#ifdef IXOFF
    define_flag(iflags, IXOFF);
#endif
#ifdef IUCLC
    define_flag(iflags, IUCLC);
#endif
#ifdef IXANY
    define_flag(iflags, IXANY);
#endif
#ifdef IMAXBEL
    define_flag(iflags, IMAXBEL);
#endif
#ifdef IUTF8
    define_flag(iflags, IUTF8);
#endif

    /* c_oflag bits */
#ifdef OPOST
    define_flag(oflags, OPOST);
#endif
#ifdef OLCUC
    define_flag(oflags, OLCUC);
#endif
#ifdef OCRNL
    define_flag(oflags, OCRNL);
#endif
#ifdef ONLCR
    define_flag(oflags, ONLCR);
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
#ifdef ONOEOT
    define_flag(oflags, ONOEOT);
#endif
#ifdef OXTABS
    define_flag(oflags, OXTABS);
#endif
#ifdef NLDLY
    define_flag(oflags, NLDLY);
#endif
#ifdef NL0
    define_choice(oflags, NLDLY, NL0);
#endif
#ifdef NL1
    define_choice(oflags, NLDLY, NL1);
#endif
#ifdef CRDLY
    define_flag(oflags, CRDLY);
#endif
#ifdef CR0
    define_choice(oflags, CRDLY, CR0);
#endif
#ifdef CR1
    define_choice(oflags, CRDLY, CR1);
#endif
#ifdef CR2
    define_choice(oflags, CRDLY, CR2);
#endif
#ifdef CR3
    define_choice(oflags, CRDLY, CR3);
#endif
#ifdef TABDLY
    define_flag(oflags, TABDLY);
#endif
#ifdef TAB0
    define_choice(oflags, TABDLY, TAB0);
#endif
#ifdef TAB1
    define_choice(oflags, TABDLY, TAB1);
#endif
#ifdef TAB2
    define_choice(oflags, TABDLY, TAB2);
#endif
#ifdef TAB3
    define_choice(oflags, TABDLY, TAB3);
#endif
#ifdef XTABS
    define_choice(oflags, TABDLY, XTABS);
#endif
#ifdef BSDLY
    define_flag(oflags, BSDLY);
#endif
#ifdef BS0
    define_choice(oflags, BSDLY, BS0);
#endif
#ifdef BS1
    define_choice(oflags, BSDLY, BS1);
#endif
#ifdef VTDLY
    define_flag(oflags, VTDLY);
#endif
#ifdef VT0
    define_choice(oflags, VTDLY, VT0);
#endif
#ifdef VT1
    define_choice(oflags, VTDLY, VT1);
#endif
#ifdef FFDLY
    define_flag(oflags, FFDLY);
#endif
#ifdef FF0
    define_choice(oflags, FFDLY, FF0);
#endif
#ifdef FF1
    define_choice(oflags, FFDLY, FF1);
#endif

    /* c_cflag bit meaning */
#ifdef CBAUD
    define_flag(cflags, CBAUD);
#endif
#ifdef B0
    define_flag(bauds, B0);
#endif
#ifdef B50
    define_flag(bauds, B50);
#endif
#ifdef B75
    define_flag(bauds, B75);
#endif
#ifdef B110
    define_flag(bauds, B110);
#endif
#ifdef B134
    define_flag(bauds, B134);
#endif
#ifdef B150
    define_flag(bauds, B150);
#endif
#ifdef B200
    define_flag(bauds, B200);
#endif
#ifdef B300
    define_flag(bauds, B300);
#endif
#ifdef B600
    define_flag(bauds, B600);
#endif
#ifdef B1200
    define_flag(bauds, B1200);
#endif
#ifdef B1800
    define_flag(bauds, B1800);
#endif
#ifdef B2400
    define_flag(bauds, B2400);
#endif
#ifdef B4800
    define_flag(bauds, B4800);
#endif
#ifdef B9600
    define_flag(bauds, B9600);
#endif
#ifdef B19200
    define_flag(bauds, B19200);
#endif
#ifdef B38400
    define_flag(bauds, B38400);
#endif
#ifdef EXTA
    define_flag(cflags, EXTA);
#endif
#ifdef EXTB
    define_flag(cflags, EXTB);
#endif
#ifdef PARENB
    define_flag(cflags, PARENB);
#endif
#ifdef PARODD
    define_flag(cflags, PARODD);
#endif
#ifdef CSIZE
    define_flag(cflags, CSIZE);
#endif
#ifdef CS5
    define_choice(cflags, CSIZE, CS5);
#endif
#ifdef CS6
    define_choice(cflags, CSIZE, CS6);
#endif
#ifdef CS7
    define_choice(cflags, CSIZE, CS7);
#endif
#ifdef CS8
    define_choice(cflags, CSIZE, CS8);
#endif
#ifdef HUPCL
    define_flag(cflags, HUPCL);
#endif
#ifdef CSTOPB
    define_flag(cflags, CSTOPB);
#endif
#ifdef CREAD
    define_flag(cflags, CREAD);
#endif
#ifdef CLOCAL
    define_flag(cflags, CLOCAL);
#endif
#ifdef CBAUDEX
    define_flag(cflags, CBAUDEX);
#endif
#ifdef B57600
    define_flag(bauds, B57600);
#endif
#ifdef B115200
    define_flag(bauds, B115200);
#endif
#ifdef B230400
    define_flag(bauds, B230400);
#endif
#ifdef B460800
    define_flag(bauds, B460800);
#endif
#ifdef B500000
    define_flag(bauds, B500000);
#endif
#ifdef B576000
    define_flag(bauds, B576000);
#endif
#ifdef B921600
    define_flag(bauds, B921600);
#endif
#ifdef B1000000
    define_flag(bauds, B1000000);
#endif
#ifdef B1152000
    define_flag(bauds, B1152000);
#endif
#ifdef B1500000
    define_flag(bauds, B1500000);
#endif
#ifdef B2000000
    define_flag(bauds, B2000000);
#endif
#ifdef B2500000
    define_flag(bauds, B2500000);
#endif
#ifdef B3000000
    define_flag(bauds, B3000000);
#endif
#ifdef B3500000
    define_flag(bauds, B3500000);
#endif
#ifdef B4000000
    define_flag(bauds, B4000000);
#endif
#ifdef CIBAUD
    define_flag(cflags, CIBAUD);
#endif
#ifdef CRTSCTS
    define_flag(cflags, CRTSCTS);
#endif
#ifdef MDMBUF
    define_flag(cflags, MDMBUF);
#endif

    /* c_lflag bits */
#ifdef ISIG
    define_flag(lflags, ISIG);
#endif
#ifdef ICANON
    define_flag(lflags, ICANON);
#endif
#ifdef IEXTEN
    define_flag(lflags, IEXTEN);
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
#ifdef XCASE
    define_flag(lflags, XCASE);
#endif
#ifdef TOSTOP
    define_flag(lflags, TOSTOP);
#endif
#ifdef ECHOPRT
    define_flag(lflags, ECHOPRT);
#endif
#ifdef ECHOCTL
    define_flag(lflags, ECHOCTL);
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
#ifdef ALTWERASE
    define_flag(lflags, ALTWERASE);
#endif
#ifdef EXTPROC
    define_flag(lflags, EXTPROC);
#endif
#ifdef NOKERNINFO
    define_flag(lflags, NOKERNINFO);
#endif

    /* tcflow() and TCXONC use these */
#ifdef TCOOFF
    define_flag2(tcflow_act, TCOOFF);
#endif
#ifdef TCOON
    define_flag2(tcflow_act, TCOON);
#endif
#ifdef TCIOFF
    define_flag2(tcflow_act, TCIOFF);
#endif
#ifdef TCION
    define_flag2(tcflow_act, TCION);
#endif

    /* tcflush() and TCFLSH use these */
#ifdef TCIFLUSH
    define_flag2(tcflush_qs, TCIFLUSH);
#endif
#ifdef TCOFLUSH
    define_flag2(tcflush_qs, TCOFLUSH);
#endif
#ifdef TCIOFLUSH
    define_flag2(tcflush_qs, TCIOFLUSH);
#endif

    /* tcsetattr uses these */
#ifdef TCSANOW
    define_flag2(tcsetattr_opt, TCSANOW);
#endif
#ifdef TCSADRAIN
    define_flag2(tcsetattr_opt, TCSADRAIN);
#endif
#ifdef TCSAFLUSH
    define_flag2(tcsetattr_opt, TCSAFLUSH);
#endif
#ifdef TCSASOFT
    define_flag2(tcsetattr_opt, TCSASOFT);
#endif
}
