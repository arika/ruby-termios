/*

  A termios library for Ruby.
  Copyright (C) 1999, 2000, 2002 akira yamada.

 */

#include "ruby.h"
#if defined(HAVE_RUBY_IO_H)
#include "ruby/io.h"
#else
#include "rubyio.h"
#endif
#include <termios.h>
#include <sys/ioctl.h>
#if defined(HAVE_SYS_IOCTL_H)
#include <unistd.h>
#endif
#include <string.h>

#if defined(HAVE_TYPE_RB_IO_T) && !defined(HAVE_MACRO_OPENFILE)
typedef rb_io_t OpenFile;
#endif

#if defined(HAVE_ST_FD)
#define FILENO(fptr) (fptr->fd)
#else
#define FILENO(fptr) fileno(fptr->f)
#endif

#define validate_ulong(v) ULONG2NUM(NUM2ULONG(v))

static VALUE mTermios;
static VALUE cTermios;
static VALUE tcsetattr_opt, tcflush_qs, tcflow_act;
static ID id_iflag, id_oflag, id_cflag, id_lflag, id_cc, id_ispeed, id_ospeed;

/*
 * Document-class: Termios::Termios
 *
 * Encupsalates termios parameters.
 *
 * See also: termios(3)
 */

/*
 * call-seq:
 *   termios.iflag = flag
 *
 * Updates input modes of the object.
 */
static VALUE
termios_set_iflag(self, value)
    VALUE self, value;
{
    rb_ivar_set(self, id_iflag, validate_ulong(value));

    return value;
}

/*
 * call-seq:
 *   termios.oflag = flag
 *
 * Updates output modes of the object.
 */
static VALUE
termios_set_oflag(self, value)
    VALUE self, value;
{
    rb_ivar_set(self, id_oflag, validate_ulong(value));

    return value;
}

/*
 * call-seq:
 *   termios.cflag = flag
 *
 * Updates control modes of the object.
 */
static VALUE
termios_set_cflag(self, value)
    VALUE self, value;
{
    rb_ivar_set(self, id_cflag, validate_ulong(value));

    return value;
}

/*
 * call-seq:
 *   termios.lflag = flag
 *
 * Updates local modes of the object.
 */
static VALUE
termios_set_lflag(self, value)
    VALUE self, value;
{
    rb_ivar_set(self, id_lflag, validate_ulong(value));

    return value;
}

/*
 * call-seq:
 *   termios.cc = value
 *
 * Updates control characters of the object.
 */
static VALUE
termios_set_cc(self, value)
    VALUE self, value;
{
    Check_Type(value, T_ARRAY);
    rb_ivar_set(self, id_cc, value);

    return value;
}

/*
 * call-seq:
 *   termios.ispeed = speed
 *
 * Updates input baud rate of the object.
 */
static VALUE
termios_set_ispeed(self, value)
    VALUE self, value;
{
    rb_ivar_set(self, id_ispeed, validate_ulong(value));

    return value;
}

/*
 * call-seq:
 *   termios.ospeed = speed
 *
 * Updates output baud rate of the object.
 */
static VALUE
termios_set_ospeed(self, value)
    VALUE self, value;
{
    rb_ivar_set(self, id_ospeed, validate_ulong(value));

    return value;
}

/*
 * call-seq:
 *   Termios.new
 *
 * Returns new Termios::Termios object.
 */
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

/*
 * Document-module: Termios
 *
 * = Description
 *
 * Termios module is simple wrapper of termios(3).  It can be included
 * into IO-family classes and can extend IO-family objects.  In addition,
 * the methods can use as module function.
 *
 * You can call termios(3) function as module methods.  Or you can use these
 * methods as instance method by including Termios module to the target IO
 * object.
 *
 * == Constants
 *
 * Many constants which are derived from "termios.h" are defined on Termios
 * module.  You can use these constants as the same name in "termios.h"
 * basically.
 *
 * IFLAGS, OFLAGS, CFLAGS and LFLAGS are Hash object.  They contains Symbols
 * of constants for c_iflag, c_oflag, c_cflag and c_lflag.  CCINDEX and BAUDS
 * are Hash object too.  They contains Symbols of constats for c_cc or ispeed
 * and ospeed.
 *
 * == See also
 *
 * termios(3)
 */

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

/*
 * call-seq:
 *   Termios.tcgetattr(io)
 *   io.tcgetattr
 *
 * Returns new Termios::Termios object which stores termios parameters
 * associated with the io.
 *
 *   require 'termios'
 *
 *   Termios.tcgetattr($stdin)
 *     #=> #<Termios::Termios speed 38400 baud; intr=^C ... >
 *
 *   $stdout.extend(Termios)
 *   $stdout.tcgetattr
 *     #=> #<Termios::Termios speed 38400 baud; intr=^C ... >
 *
 * See also: tcgetattr(3)
 */
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

/*
 * call-seq:
 *   Termios.tcsetattr(io, option, termios)
 *   io.tcsetattr(option, termios)
 *
 * Sets the Termios::Termios object as the termios paramter to the io
 * and returns the old termios parameter.
 *
 * Option are specifies when the parameter is changed.  What option are
 * available is plathome dependent, but usually Termios::TCSANOW,
 * Termios::TCSADRAIN and Termios::TCSAFLUSH are provided.
 *
 *   require 'termios'
 *
 *   oldt = Termios.tcgetattr($stdin)
 *   newt = oldt.dup
 *   newt.lflag &= ~Termios::ECHO
 *
 *   secret = nil
 *   begin
 *     Termios.tcsetattr($stdin, Termios::TCSANOW, newt)
 *     print "noecho> "
 *     secret = $stdin.gets
 *     print "\n"
 *   ensure
 *     Termios.tcsetattr($stdin, Termios::TCSANOW, oldt)
 *   end
 *
 *   puts secret
 *
 * See also: tcsetattr(3)
 */
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
	const char *type = rb_class2name(CLASS_OF(param));
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

/*
 * call-seq:
 *   Termios.tcsendbreak(io, duration)
 *   io.tcsendbreak(duration)
 *
 * Sends a continuous stream of 0-bits for a specific duration.
 *
 * See also: tcsendbreak(3)
 */
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

/*
 * call-seq:
 *   Termios.tcdrain(io)
 *   io.tcdrain
 *
 * Waits until all output to the object has been sent.
 *
 * See also: tcdrain(3)
 */
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

/*
 * call-seq:
 *   Termios.tcflush(io, qs)
 *   io.tcflush(qs)
 *
 * Cancels data written to the object but not send or data received but not
 * read.
 *
 * See also: tcflush(3)
 */
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

/*
 * call-seq:
 *   Termios.tcflow(io, action)
 *   io.tcflow(action)
 *
 * Suspends write or read of data on the object.
 *
 * See also: tcflow(3)
 */
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

/*
 * call-seq:
 *   Termios.tcgetpgrp(io)
 *   io.tcgetpgrp
 *
 * Returns the process group ID of the foreground process group  the
 * terminal associated to the object.
 *
 * See also: tcgetpgrp(3)
 */
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

/*
 * call-seq:
 *   Termios.tcsetpgrp(io, pgrpid)
 *   io.tcsetpgrp(pgrpid)
 *
 * Makes the process group with pgrpid the foreground process group on the
 * terminal associated to the object.
 *
 * See also: tcsetpgrp(3)
 */
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

/*
 * call-seq:
 *   Termios.new_termios
 *
 * Returns new Termios::Termios object.
 */
static VALUE
termios_s_newtermios(argc, argv, klass)
    int argc;
    VALUE *argv;
    VALUE klass;
{
    return rb_funcall2(cTermios, rb_intern("new"), argc, argv);
}

/*
 * call-seq:
 *   termios.dup
 *
 * Produces a shallow copy of the object.
 */
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
    VALUE ioctl_commands, ioctl_commands_names;
    VALUE modem_signals, modem_signals_names;
    VALUE pty_pkt_options, pty_pkt_options_names;
    VALUE line_disciplines, line_disciplines_names;

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

    /* input modes */
    rb_define_attr(cTermios, "iflag",  1, 0);
    /* output modes */
    rb_define_attr(cTermios, "oflag",  1, 0);
    /* control modes */
    rb_define_attr(cTermios, "cflag",  1, 0);
    /* local modes */
    rb_define_attr(cTermios, "lflag",  1, 0);
    /* control characters */
    rb_define_attr(cTermios, "cc",     1, 0);
    /* input baud rate */
    rb_define_attr(cTermios, "ispeed", 1, 0);
    /* output baud rate */
    rb_define_attr(cTermios, "ospeed", 1, 0);

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

    /* number of control characters */
    rb_define_const(mTermios, "NCCS",    INT2FIX(NCCS));
    rb_define_const(mTermios, "POSIX_VDISABLE", INT2FIX(_POSIX_VDISABLE));

    ccindex = rb_hash_new();
    ccindex_names = rb_ary_new();
    /* Hash of control character index and control character names */
    rb_define_const(mTermios, "CCINDEX", ccindex);
    /* List of control character names */
    rb_define_const(mTermios, "CCINDEX_NAMES", ccindex_names);

    iflags = rb_hash_new();
    iflags_names = rb_ary_new();
    /* Hash of input mode names and values */
    rb_define_const(mTermios, "IFLAGS", iflags);
    /* List of input mode names */
    rb_define_const(mTermios, "IFLAG_NAMES", iflags_names);

    oflags = rb_hash_new();
    oflags_names = rb_ary_new();
    oflags_choices = rb_hash_new();
    /* Hash of output mode names and values */
    rb_define_const(mTermios, "OFLAGS", oflags);
    /* List of output mode names */
    rb_define_const(mTermios, "OFLAG_NAMES", oflags_names);
    rb_define_const(mTermios, "OFLAG_CHOICES", oflags_choices);

    cflags = rb_hash_new();
    cflags_names = rb_ary_new();
    cflags_choices = rb_hash_new();
    /* Hash of control mode names and values */
    rb_define_const(mTermios, "CFLAGS", cflags);
    /* List of control mode names */
    rb_define_const(mTermios, "CFLAG_NAMES", cflags_names);
    rb_define_const(mTermios, "CFLAG_CHOICES", cflags_choices);

    lflags = rb_hash_new();
    lflags_names = rb_ary_new();
    /* Hash of local mode names and values */
    rb_define_const(mTermios, "LFLAGS", lflags);
    /* List of local mode names */
    rb_define_const(mTermios, "LFLAG_NAMES", lflags_names);

    bauds = rb_hash_new();
    bauds_names = rb_ary_new();
    /* List of baud rates */
    rb_define_const(mTermios, "BAUDS", bauds);
    /* List of baud rate names */
    rb_define_const(mTermios, "BAUD_NAMES", bauds_names);

    tcsetattr_opt = rb_ary_new();
    /* List of tcsetattr options */
    rb_define_const(mTermios, "SETATTR_OPTS", tcsetattr_opt);

    tcflush_qs = rb_ary_new();
    /* List of tcflush qselectors */
    rb_define_const(mTermios, "FLUSH_QSELECTORS", tcflush_qs);

    tcflow_act = rb_ary_new();
    /* List of tcflow actions */
    rb_define_const(mTermios, "FLOW_ACTIONS", tcflow_act);

    ioctl_commands = rb_hash_new();
    ioctl_commands_names = rb_ary_new();
    rb_define_const(mTermios, "IOCTL_COMMANDS", ioctl_commands);
    rb_define_const(mTermios, "IOCTL_COMMAND_NAMES", ioctl_commands_names);

    modem_signals = rb_hash_new();
    modem_signals_names = rb_ary_new();
    rb_define_const(mTermios, "MODEM_SIGNALS", modem_signals);
    rb_define_const(mTermios, "MODEM_SIGNAL_NAMES", modem_signals_names);

    pty_pkt_options = rb_hash_new();
    pty_pkt_options_names = rb_ary_new();
    rb_define_const(mTermios, "PTY_PACKET_OPTIONS", pty_pkt_options);
    rb_define_const(mTermios, "PTY_PACKET_OPTION_NAMES", pty_pkt_options_names);

    line_disciplines = rb_hash_new();
    line_disciplines_names = rb_ary_new();
    rb_define_const(mTermios, "LINE_DISCIPLINES", line_disciplines);
    rb_define_const(mTermios, "LINE_DISCIPLINE_NAMES", line_disciplines_names);

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

    /* Constants useful to ioctl for controlling lines */
#ifdef TIOCMODG
	define_flag(ioctl_commands, TIOCMODG)
#endif
#ifdef TIOCMODS
	define_flag(ioctl_commands, TIOCMODS)
#endif
#ifdef TIOCM_LE
	define_flag(modem_signals, TIOCM_LE)
#endif
#ifdef TIOCM_DTR
	define_flag(modem_signals, TIOCM_DTR)
#endif
#ifdef TIOCM_RTS
	define_flag(modem_signals, TIOCM_RTS)
#endif
#ifdef TIOCM_ST
	define_flag(modem_signals, TIOCM_ST)
#endif
#ifdef TIOCM_SR
	define_flag(modem_signals, TIOCM_SR)
#endif
#ifdef TIOCM_CTS
	define_flag(modem_signals, TIOCM_CTS)
#endif
#ifdef TIOCM_CAR
	define_flag(modem_signals, TIOCM_CAR)
#endif
#ifdef TIOCM_CD
	define_flag(modem_signals, TIOCM_CD)
#endif
#ifdef TIOCM_RNG
	define_flag(modem_signals, TIOCM_RNG)
#endif
#ifdef TIOCM_RI
	define_flag(modem_signals, TIOCM_RI)
#endif
#ifdef TIOCM_DSR
	define_flag(modem_signals, TIOCM_DSR)
#endif
#ifdef TIOCEXCL
	define_flag(ioctl_commands, TIOCEXCL)
#endif
#ifdef TIOCNXCL
	define_flag(ioctl_commands, TIOCNXCL)
#endif
#ifdef TIOCFLUSH
	define_flag(ioctl_commands, TIOCFLUSH)
#endif
#ifdef TIOCGETA
	define_flag(ioctl_commands, TIOCGETA)
#endif
#ifdef TIOCSETA
	define_flag(ioctl_commands, TIOCSETA)
#endif
#ifdef TIOCSETAW
	define_flag(ioctl_commands, TIOCSETAW)
#endif
#ifdef TIOCSETAF
	define_flag(ioctl_commands, TIOCSETAF)
#endif
#ifdef TIOCGETD
	define_flag(ioctl_commands, TIOCGETD)
#endif
#ifdef TIOCSETD
	define_flag(ioctl_commands, TIOCSETD)
#endif
#ifdef TIOCIXON
	define_flag(ioctl_commands, TIOCIXON)
#endif
#ifdef TIOCIXOFF
	define_flag(ioctl_commands, TIOCIXOFF)
#endif
#ifdef TIOCSBRK
	define_flag(ioctl_commands, TIOCSBRK)
#endif
#ifdef TIOCCBRK
	define_flag(ioctl_commands, TIOCCBRK)
#endif
#ifdef TIOCSDTR
	define_flag(ioctl_commands, TIOCSDTR)
#endif
#ifdef TIOCCDTR
	define_flag(ioctl_commands, TIOCCDTR)
#endif
#ifdef TIOCGPGRP
	define_flag(ioctl_commands, TIOCGPGRP)
#endif
#ifdef TIOCSPGRP
	define_flag(ioctl_commands, TIOCSPGRP)
#endif
#ifdef TIOCOUTQ
	define_flag(ioctl_commands, TIOCOUTQ)
#endif
#ifdef TIOCSTI
	define_flag(ioctl_commands, TIOCSTI)
#endif
#ifdef TIOCNOTTY
	define_flag(ioctl_commands, TIOCNOTTY)
#endif
#ifdef TIOCPKT
	define_flag(ioctl_commands, TIOCPKT)
#endif
#ifdef TIOCPKT_DATA
	define_flag(pty_pkt_options, TIOCPKT_DATA)
#endif
#ifdef TIOCPKT_FLUSHREAD
	define_flag(pty_pkt_options, TIOCPKT_FLUSHREAD)
#endif
#ifdef TIOCPKT_FLUSHWRITE
	define_flag(pty_pkt_options, TIOCPKT_FLUSHWRITE)
#endif
#ifdef TIOCPKT_STOP
	define_flag(pty_pkt_options, TIOCPKT_STOP)
#endif
#ifdef TIOCPKT_START
	define_flag(pty_pkt_options, TIOCPKT_START)
#endif
#ifdef TIOCPKT_NOSTOP
	define_flag(pty_pkt_options, TIOCPKT_NOSTOP)
#endif
#ifdef TIOCPKT_DOSTOP
	define_flag(pty_pkt_options, TIOCPKT_DOSTOP)
#endif
#ifdef TIOCPKT_IOCTL
	define_flag(pty_pkt_options, TIOCPKT_IOCTL)
#endif
#ifdef TIOCSTOP
	define_flag(ioctl_commands, TIOCSTOP)
#endif
#ifdef TIOCSTART
	define_flag(ioctl_commands, TIOCSTART)
#endif
#ifdef TIOCMSET
	define_flag(ioctl_commands, TIOCMSET)
#endif
#ifdef TIOCMBIS
	define_flag(ioctl_commands, TIOCMBIS)
#endif
#ifdef TIOCMBIC
	define_flag(ioctl_commands, TIOCMBIC)
#endif
#ifdef TIOCMGET
	define_flag(ioctl_commands, TIOCMGET)
#endif
#ifdef TIOCREMOTE
	define_flag(ioctl_commands, TIOCREMOTE)
#endif
#ifdef TIOCGWINSZ
	define_flag(ioctl_commands, TIOCGWINSZ)
#endif
#ifdef TIOCSWINSZ
	define_flag(ioctl_commands, TIOCSWINSZ)
#endif
#ifdef TIOCUCNTL
	define_flag(ioctl_commands, TIOCUCNTL)
#endif
#ifdef TIOCSTAT
	define_flag(ioctl_commands, TIOCSTAT)
#endif
#ifdef TIOCSCONS
	define_flag(ioctl_commands, TIOCSCONS)
#endif
#ifdef TIOCCONS
	define_flag(ioctl_commands, TIOCCONS)
#endif
#ifdef TIOCSCTTY
	define_flag(ioctl_commands, TIOCSCTTY)
#endif
#ifdef TIOCEXT
	define_flag(ioctl_commands, TIOCEXT)
#endif
#ifdef TIOCSIG
	define_flag(ioctl_commands, TIOCSIG)
#endif
#ifdef TIOCDRAIN
	define_flag(ioctl_commands, TIOCDRAIN)
#endif
#ifdef TIOCMSDTRWAIT
	define_flag(ioctl_commands, TIOCMSDTRWAIT)
#endif
#ifdef TIOCMGDTRWAIT
	define_flag(ioctl_commands, TIOCMGDTRWAIT)
#endif
#ifdef TIOCTIMESTAMP
	define_flag(ioctl_commands, TIOCTIMESTAMP)
#endif
#ifdef TIOCDCDTIMESTAMP
	define_flag(ioctl_commands, TIOCDCDTIMESTAMP)
#endif
#ifdef TIOCSDRAINWAIT
	define_flag(ioctl_commands, TIOCSDRAINWAIT)
#endif
#ifdef TIOCGDRAINWAIT
	define_flag(ioctl_commands, TIOCGDRAINWAIT)
#endif
#ifdef TIOCDSIMICROCODE
	define_flag(ioctl_commands, TIOCDSIMICROCODE)
#endif
#ifdef TIOCPTYGRANT
	define_flag(ioctl_commands, TIOCPTYGRANT)
#endif
#ifdef TIOCPTYGNAME
	define_flag(ioctl_commands, TIOCPTYGNAME)
#endif
#ifdef TIOCPTYUNLK
	define_flag(ioctl_commands, TIOCPTYUNLK)
#endif
#ifdef TTYDISC
	define_flag(line_disciplines, TTYDISC)
#endif
#ifdef TABLDISC
	define_flag(line_disciplines, TABLDISC)
#endif
#ifdef SLIPDISC
	define_flag(line_disciplines, SLIPDISC)
#endif
#ifdef PPPDISC
	define_flag(line_disciplines, PPPDISC)
#endif
}
