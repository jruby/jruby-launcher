require 'spec'
require 'rbconfig'

if defined?(JRUBY_VERSION)
  require 'jruby'
  JRuby.runtime.instance_config.run_ruby_in_process = false
end

module JRubyLauncherHelper
  WINDOWS = Config::CONFIG['host_os'] =~ /Windows|mswin/

  def jruby_launcher(args)
    command = "./jruby #{args}"
    command += " 2>&1" unless WINDOWS
    `#{command}`
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
    exe = JRubyLauncherHelper::WINDOWS ? "./jruby.exe" : "./jruby"
    unless File.executable?(exe)
      raise "Error: launcher executable not built; type `make' before continuing."
    end
  end
  config.include(JRubyLauncherHelper)
end
