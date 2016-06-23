# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'termios/version'

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

  spec.files         = `git ls-files -z`.split("\x0").reject { |f| f.match(%r{^(test|spec|features)/}) }
  spec.bindir        = 'exe'
  spec.executables   = spec.files.grep(%r{^exe/}) { |f| File.basename(f) }
  spec.require_paths = ['lib']
  spec.extensions    = ['ext/extconf.rb']

  spec.add_development_dependency 'bundler', '~> 1.12'
  spec.add_development_dependency 'rake', '~> 10.0'
  spec.add_development_dependency 'rake-compiler'
end
