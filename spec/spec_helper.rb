require 'spec'
require 'rbconfig'

if defined?(JRUBY_VERSION)
  require 'jruby'
  JRuby.runtime.instance_config.run_ruby_in_process = false
end

module JRubyLauncherHelper
  JRUBY_EXE = File.expand_path("../../jruby", __FILE__) + Config::CONFIG['EXEEXT']
  WINDOWS = Config::CONFIG['target_os'] =~ /mswin/

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

  def windows?
    WINDOWS
  end

  def with_environment(pairs = {})
    prev_env = {}
    pairs.each_pair do |k,v|
      prev_env[k] = ENV[k] if ENV.has_key?(k)
      ENV[k] = v
    end
    begin
      yield
    ensure
      pairs.keys.each {|k| ENV.delete(k)}
      ENV.update(prev_env)
    end
  end
end

Spec::Runner.configure do |config|
  config.before(:all) do
    JRubyLauncherHelper.check_executable_built
    # clear environment for better control
    ENV.delete("JAVA_HOME")
    ENV.delete("JRUBY_HOME")
    ENV.delete("JAVA_OPTS")
    ENV.delete("JRUBY_OPTS")
    ENV.delete("CLASSPATH")
    ENV.delete("JAVA_ENCODING")
  end
  config.include(JRubyLauncherHelper)
end
