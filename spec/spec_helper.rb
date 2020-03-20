require 'rspec'
require 'rbconfig'
require 'fileutils'

module JRubyLauncherHelper
  JRUBY_EXE = ''
  WINDOWS = RbConfig::CONFIG['target_os'] =~ /mswin/

  def self.check_executable_built
    exe = File.expand_path("../../jruby", __FILE__) + RbConfig::CONFIG['EXEEXT']
    unless File.executable?(exe)
      raise "Error: launcher executable not built; type `make' before continuing."
    end
    top = File.dirname(exe)
    name = File.basename(exe)
    home = File.join(top, "build/home")
    FileUtils.mkdir_p(File.join(home, "bin"))
    FileUtils.cp(exe, File.join(home, "bin"))
    if JRubyLauncherHelper::WINDOWS
      FileUtils.cp(exe.sub(/exe/, 'dll'), File.join(home, "bin"))
    end
    FileUtils.mkdir_p(File.join(home, "lib"))
    FileUtils.touch(File.join(home, "lib/jruby.jar"))
    JRUBY_EXE.concat File.join(home, "bin", name)
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

  def classpath_arg(args)
    index = args.index("-cp")
    index.should > 0
    args[index + 1]
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

RSpec.configure do |config|
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
