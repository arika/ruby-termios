require 'termios.so'

module Termios
  VISIBLE_CHAR = {}
  [
    "^@", "^A", "^B", "^C", "^D", "^E", "^F", "^G",
    "^H", "^I", "^J", "^K", "^L", "^M", "^N", "^O",
    "^P", "^Q", "^R", "^S", "^T", "^U", "^V", "^W",
    "^X", "^Y", "^Z", "^[", "^\\", "^]", "^^", "^_",
    "<sp>", "!", "\"", "#", "$", "%", "&", "'",
    "(", ")", "*", "+", ",", "-", ".", "/",
    "0", "1", "2", "3", "4", "5", "6", "7",
    "8", "9", ":", ";", "<", "=", ">", "?",
    "@", "A", "B", "C", "D", "E", "F", "G",
    "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W",
    "X", "Y", "Z", "[", "\\", "]", "^", "_",
    "`", "a", "b", "c", "d", "e", "f", "g",
    "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w",
    "x", "y", "z", "{", "|", "}", "~", "^?",
    "M-^@", "M-^A", "M-^B", "M-^C", "M-^D", "M-^E", "M-^F", "M-^G",
    "M-^H", "M-^I", "M-^J", "M-^K", "M-^L", "M-^M", "M-^N", "M-^O",
    "M-^P", "M-^Q", "M-^R", "M-^S", "M-^T", "M-^U", "M-^V", "M-^W",
    "M-^X", "M-^Y", "M-^Z", "M-^[", "M-^\\", "M-^]", "M-^^", "M-^_",
    "M-<sp>", "M-!", "M-\"", "M-#", "M-$", "M-%", "M-&", "M-'",
    "M-(", "M-)", "M-*", "M-+", "M-,", "M--", "M-.", "M-/",
    "M-0", "M-1", "M-2", "M-3", "M-4", "M-5", "M-6", "M-7",
    "M-8", "M-9", "M-:", "M-;", "M-<", "M-=", "M->", "M-?",
    "M-@", "M-A", "M-B", "M-C", "M-D", "M-E", "M-F", "M-G",
    "M-H", "M-I", "M-J", "M-K", "M-L", "M-M", "M-N", "M-O",
    "M-P", "M-Q", "M-R", "M-S", "M-T", "M-U", "M-V", "M-W",
    "M-X", "M-Y", "M-Z", "M-[", "M-\\", "M-]", "M-^", "M-_",
    "M-`", "M-a", "M-b", "M-c", "M-d", "M-e", "M-f", "M-g",
    "M-h", "M-i", "M-j", "M-k", "M-l", "M-m", "M-n", "M-o",
    "M-p", "M-q", "M-r", "M-s", "M-t", "M-u", "M-v", "M-w",
    "M-x", "M-y", "M-z", "M-{", "M-|", "M-}", "M-~", "M-^?",
  ].each_with_index {|s, i|
    VISIBLE_CHAR[i] = s
    VISIBLE_CHAR[[i].pack("C")] = s
  }
  VISIBLE_CHAR[POSIX_VDISABLE] = "<undef>"
  VISIBLE_CHAR[[POSIX_VDISABLE].pack("C")] = "<undef>"

  class Termios
    def inspect
      str = "\#<#{self.class}"
      if self.ispeed == self.ospeed
        speed = (BAUDS[self.ispeed] || "B???").to_s[1..-1]
        str << " speed #{speed} baud;"
      else
        ispeed = (BAUDS[self.ispeed] || "B???").to_s[1..-1]
        ospeed = (BAUDS[self.ospeed] || "B???").to_s[1..-1]
        str << " ispeed #{ispeed} baud; ospeed #{ospeed} baud;"
      end

      CCINDEX_NAMES.each {|ccindex|
        next if ccindex == :VMIN || ccindex == :VTIME
        str << " #{ccindex.to_s[1..-1].downcase}"
        str << "=#{VISIBLE_CHAR[self.cc[::Termios.const_get(ccindex)]]}"
      }
      str << " min=#{self.cc[VMIN]}"
      str << " time=#{self.cc[VTIME]}"

      [
        [:cflag,
         CFLAG_NAMES-[:CBAUD, :CBAUDEX, :CIBAUD, :EXTA, :EXTB],
         CFLAG_CHOICES],
        [:iflag, IFLAG_NAMES, nil],
        [:oflag, OFLAG_NAMES, OFLAG_CHOICES],
        [:lflag, LFLAG_NAMES, nil]
      ].each {|l|
        str << ";"
        flag_type, flag_names, choices = l
        flags = self.send(flag_type)
        choice_names = choices ? choices.values.flatten : []
        (flag_names-choice_names).each {|name|
          str << " "
          if choices and ns = choices[name]
            mask = ::Termios.const_get(name)
            ns.each {|n|
              if (flags & mask) == ::Termios.const_get(n)
                str << n.to_s.downcase
                break
              end
            }
          else
            str << "-" if (flags & ::Termios.const_get(name)) == 0
            str << name.to_s.downcase
          end
        }
      }

      str << ">"
      str
    end

    def pretty_print(q)	# :nodoc:
      q.object_group(self) {
        if self.ispeed == self.ospeed
          speed = (BAUDS[self.ispeed] || "B???").to_s[1..-1]
          q.fill_breakable; q.text "speed #{speed} baud;"
        else
          ispeed = (BAUDS[self.ispeed] || "B???").to_s[1..-1]
          ospeed = (BAUDS[self.ospeed] || "B???").to_s[1..-1]
          q.fill_breakable; q.text "ispeed #{ispeed} baud;"
          q.fill_breakable; q.text "ospeed #{ospeed} baud;"
        end
        q.breakable

        q.seplist(CCINDEX_NAMES-[:VMIN, :VTIME],
                  lambda { q.fill_breakable }) {|ccindex|
          q.text ccindex.to_s[1..-1].downcase
          q.text "=#{VISIBLE_CHAR[self.cc[::Termios.const_get(ccindex)]]}"
        }
        q.breakable; q.text "min=#{self.cc[VMIN]}"
        q.fill_breakable; q.text "time=#{self.cc[VTIME]}"

        [
          [:cflag,
           CFLAG_NAMES-[:CBAUD, :CBAUDEX, :CIBAUD, :EXTA, :EXTB],
           CFLAG_CHOICES],
          [:iflag, IFLAG_NAMES, nil],
          [:oflag, OFLAG_NAMES, OFLAG_CHOICES],
          [:lflag, LFLAG_NAMES, nil]
        ].each {|l|
          q.text ";"
          q.breakable
          flag_type, flag_names, choices = l
          flags = self.send(flag_type)
          choice_names = choices ? choices.values.flatten : []
          q.seplist(flag_names-choice_names,
                    lambda { q.fill_breakable }) {|name|
            if choices and ns = choices[name]
              mask = ::Termios.const_get(name)
              ns.each {|n|
                if (flags & mask) == ::Termios.const_get(n)
                  q.text n.to_s.downcase
                  break
                end
              }
            else
              q.text "-" if (flags & ::Termios.const_get(name)) == 0
              q.text name.to_s.downcase
            end
          }
        }
      }
    end
  end
end
