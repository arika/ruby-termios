require 'bundler/gem_tasks'
require 'rake/extensiontask'

task build: :compile

Rake::ExtensionTask.new('termios') do |ext|
  ext.ext_dir = 'ext'
  ext.lib_dir = 'lib'
end

task default: [:clobber, :compile]
