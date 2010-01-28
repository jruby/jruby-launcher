require 'spec'
require 'rbconfig'

if defined?(JRUBY_VERSION)
  require 'jruby'
  JRuby.runtime.instance_config.run_ruby_in_process = false
end

module JRubyLauncherHelper
  JRUBY_EXE = File.expand_path("../../jruby", __FILE__) + Config::CONFIG['EXEEXT']

  def self.check_executable_built
    unless File.executable?(JRUBY_EXE)
      raise "Error: launcher executable not built; type `make' before continuing."
    end
  end

  def jruby_launcher(args)
    `#{JRUBY_EXE} #{args}`
  end

  def jruby_launcher_args(args)
    jruby_launcher("-Xcommand #{args}").split("\n")
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
