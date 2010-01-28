require 'spec'
require 'rbconfig'

if defined?(JRUBY_VERSION)
  require 'jruby'
  JRuby.runtime.instance_config.run_ruby_in_process = false
end

module JRubyLauncherHelper
  WINDOWS = Config::CONFIG['host_os'] =~ /Windows|mswin/
  EXE = JRubyLauncherHelper::WINDOWS ? "./jruby.exe" : "./jruby"

  def self.check_executable_built
    unless File.executable?(EXE)
      raise "Error: launcher executable not built; type `make' before continuing."
    end
  end

  def jruby_launcher(args)
    `./jruby #{args}`
  end

  def jruby_launcher_args(args)
    jruby_launcher("-Xcommand #{args}").split
  end

  def last_exit_code
    $?.exitstatus
  end
end

Spec::Runner.configure do |config|
  config.before(:all) do
    JRubyLauncherHelper.check_executable_built
  end
  config.include(JRubyLauncherHelper)
end
