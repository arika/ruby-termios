# frozen_string_literal: true

require_relative 'lib/termios/version'

Gem::Specification.new do |spec|
  spec.name          = 'ruby-termios'
  spec.version       = Termios::VERSION
  spec.authors       = ['akira yamada']
  spec.email         = ['akira@arika.org']

  spec.summary       = 'a simple wrapper of termios(3)'
  spec.description   = <<~E
    Termios module is simple wrapper of termios(3).
    It can be included into IO-family classes and can extend IO-family objects.
    In addition, the methods can use as module function.
  E
  spec.homepage      = 'https://github.com/arika/ruby-termios'
  spec.license       = 'Ruby'
  spec.required_ruby_version = '>= 3.2.0'

  spec.metadata['source_code_uri'] = spec.homepage
  spec.metadata['bug_tracker_uri'] = "#{spec.homepage}/issues"
  spec.metadata['rubygems_mfa_required'] = 'true'

  gemspec = File.basename(__FILE__)
  spec.files = IO.popen(%w[git ls-files -z], chdir: __dir__, err: IO::NULL) do |ls|
    ls.readlines("\x0", chomp: true).reject do |f|
      (f == gemspec) ||
        f.start_with?(*%w[bin/ test/ spec/ features/ .git .github Gemfile])
    end
  end
  spec.require_paths = ['lib']
  spec.extensions    = ['ext/extconf.rb']
end
