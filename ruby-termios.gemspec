Gem::Specification.new do |s|
  s.name = %q{ruby-termios}
  s.version = "0.9.5"
  s.date = %q{2009-02-05}

  s.authors = ["akira yamada"]
  s.email = %q{akira@arika.org}
  s.description = <<E
Termios module is simple wrapper of termios(3).  It can be included into
IO-family classes and can extend IO-family objects.  In addition, the methods
can use as module function.
E
  s.extra_rdoc_files = ["README", "ChangeLog"]
  s.extensions = ["extconf.rb"]
  s.files = ["README", "ChangeLog", "test/test0.rb", "lib/termios.rb", "examples/secret_input1.rb", "examples/modem_check2.rb", "examples/modem_check0.rb", "examples/secret_input2.rb", "examples/modem_check1.rb", "termios.c"]
  s.bindir = false
  s.has_rdoc = true
  s.homepage = %q{http://arika.org/ruby/termios}
  s.rdoc_options = ["--title", "ruby-termios documentation", "--charset", "utf-8", "--opname", "index.html", "--line-numbers", "--main", "README", "--inline-source", "--exclude", "^(examples|extras)/"]
  s.require_paths = ["lib", "."]
  s.rubyforge_project = %q{termios}
  s.summary = %q{a simple wrapper of termios(3)}

  s.required_rubygems_version = Gem::Requirement.new(">= 0") if s.respond_to? :required_rubygems_version=
  if s.respond_to? :specification_version then
    current_version = Gem::Specification::CURRENT_SPECIFICATION_VERSION
    s.specification_version = 2

    if current_version >= 3 then
    else
    end
  else
  end
end
