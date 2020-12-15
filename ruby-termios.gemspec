# frozen_string_literal: true

require_relative 'lib/termios/version'

Gem::Specification.new do |spec|
  spec.name          = 'ruby-termios'
  spec.version       = Termios::VERSION
  spec.authors       = ['akira yamada']
  spec.email         = ['akira@arika.org']

  spec.summary       = 'a simple wrapper of termios(3)'
  spec.description   = <<-E
    Termios module is simple wrapper of termios(3).
    It can be included into IO-family classes and can extend IO-family objects.
    In addition, the methods can use as module function.
  E
  spec.homepage      = 'https://github.com/arika/ruby-termios'
  spec.license       = "Ruby's"
  spec.required_ruby_version = Gem::Requirement.new('>= 2.3.0')

  spec.metadata['homepage_uri'] = spec.homepage
  spec.metadata['source_code_ri'] = spec.homepage
  spec.metadata['bug_tracker_uri'] = spec.homepage + '/issues'

  spec.files = Dir.chdir(File.expand_path(__dir__)) do
    `git ls-files -z`.split("\x0").reject { |f| f.match(%r{\A(?:test|spec|features)/}) }
  end
  spec.bindir        = 'exe'
  spec.executables   = spec.files.grep(%r{\Aexe/}) { |f| File.basename(f) }
  spec.require_paths = ['lib']
  spec.extensions    = ['ext/extconf.rb']
end
