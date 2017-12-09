require File.expand_path('../spec_helper.rb', __FILE__)
load File.expand_path('../../lib/jruby-launcher.rb', __FILE__)

describe "JRuby native launcher" do
  it "should run org.jruby.Main" do
    expect(jruby_launcher_args("").last).to eq "org/jruby/Main"
  end

  it "should pass unrecognized arguments to JRuby" do
    expect(jruby_launcher_args("-J-Dsome.option -v --help")[-3..-1]).to eq ["org/jruby/Main", "-v", "--help"]
  end

  it "should print help message" do
    args = jruby_launcher_args("-Xhelp 2>&1")
    expect(args.select {|l| l =~ /JRuby Launcher usage/}).not_to be_empty
    expect(args).to include("-X")
    args = jruby_launcher_args("-X 2>&1")
    expect(args.detect {|l| l =~ /JRuby Launcher usage/}).not_to be_empty
    expect(args).to include("-X")
  end

  it "should use $JAVACMD when JAVACMD is specified" do
    with_environment "JAVACMD" => File.join("jato") do
      if windows?
        expect(jruby_launcher_args("-v 2>&1").join).to match %r{jato}
      else
        expect(jruby_launcher_args("-v").first).to eq File.join("jato")
      end
    end
  end

  it "should use $JAVA_HOME/bin/java when JAVA_HOME is specified" do
    with_environment "JAVA_HOME" => File.join("some", "java", "home") do
      if windows?
        expect(jruby_launcher_args("-v 2>&1").join).to match %r{some/java/home}
      else
        expect(jruby_launcher_args("-v").first).to eq File.join("some", "java", "home", "bin", "java")
      end
    end
  end

  it "should use -Xjdkhome argument above JAVA_HOME" do
    with_environment "JAVA_HOME" => File.join("env", "java", "home") do
      if windows?
        expect(jruby_launcher_args("-Xjdkhome some/java/home 2>&1").join).to match %r{some/java/home}
      else
        expect(jruby_launcher_args("-Xjdkhome some/java/home").first).to eq File.join("some", "java", "home", "bin", "java")
      end
    end
  end

  it "should drop the backslashes at the end of JAVA_HOME" do
    with_environment "JAVA_HOME" => File.join("some", "java", "home\\\\") do
      if windows?
        expect(jruby_launcher_args("").join).to match %r{some/java/home}
      else
        expect(jruby_launcher_args("").first).to eq File.join("some", "java", "home", "bin", "java")
      end
    end
  end

  it "should complain about a missing log argument" do
    expect(jruby_launcher("-Xtrace 2>&1")).to match /Argument is missing for "-Xtrace"/
    expect(jruby_launcher("-Xtrace -- 2>&1")).to match /Argument is missing for "-Xtrace"/
  end

  it "should complain about a missing jdkhome argument" do
    expect(jruby_launcher("-Xjdkhome 2>&1")).to match /Argument is missing/
    expect(jruby_launcher("-Xjdkhome -- 2>&1")).to match /Argument is missing/
  end

  it "should complain about a missing classpath append argument" do
    expect(jruby_launcher("-Xcp:a 2>&1")).to match /Argument is missing/
    expect(jruby_launcher("-Xcp:a -- 2>&1")).to match /Argument is missing/
  end

  it "should run nailgun server with --ng-server option" do
    expect(jruby_launcher_args("--ng-server").last).to eq "com/martiansoftware/nailgun/NGServer"
  end

  it "should run nailgun client with --ng option" do
    expect(jruby_launcher_args('--ng -e "puts 1"')).to eq ["org.jruby.util.NailMain", "-e", "puts 1"]
  end

  it "should handle -J JVM options" do
    expect(jruby_launcher_args("-J-Darg1=value1 -J-Darg2=value2")).to include("-Darg1=value1", "-Darg2=value2")
  end

  it "should pass -Xprop.erty=value as -J-Djruby.prop.erty=value" do
    expect(jruby_launcher_args("-Xprop.erty=value")).to include("-Djruby.prop.erty=value")
  end

  it "should pass -Xproperties as --properties" do
    expect(jruby_launcher_args("-Xproperties")).to include("--properties")
  end

  it "should default to 500m max heap" do
    expect(jruby_launcher_args("")).to include("-Xmx500m")
  end

  it "should allow max heap to be overridden" do
    expect(jruby_launcher_args("-J-Xmx256m")).to include("-Xmx256m")
  end

  it "should default to 2048k max stack" do
    expect(jruby_launcher_args("")).to include("-Xss2048k")
  end

  it "should allow max stack to be overridden" do
    expect(jruby_launcher_args("-J-Xss512k")).to include("-Xss512k")
  end

  it "should add the contents of the CLASSPATH environment variable" do
    with_environment "CLASSPATH" => "some.jar" do
      expect(classpath_arg(jruby_launcher_args(""))).to match /some.jar/
    end
  end

  it "should add the classpath elements in proper order" do
    s = File::PATH_SEPARATOR
    with_environment "CLASSPATH" => "some-env.jar" do
      args = jruby_launcher_args("-Xcp:a some-other.jar -Xcp:p some.jar")
      expect(classpath_arg(args)).to match /some.jar.*#{s}some-env.jar#{s}some-other.jar/
    end
  end

  it "should use the --server compiler" do
    expect(jruby_launcher_args("--server")).to include("-server")
  end

  it "should use the --client compiler" do
    expect(jruby_launcher_args("--client")).to include("-client")
  end

  it "should set the JMX settings when --manage is present" do
    expect(jruby_launcher_args("--manage")).to include("-Dcom.sun.management.jmxremote", "-Djruby.management.enabled=true")
  end

  it "should set the headless flag when --headless is present" do
    expect(jruby_launcher_args("--headless")).to include("-Djava.awt.headless=true")
  end

  it "should pass -Xprof when --sample is present" do
    expect(jruby_launcher_args("--sample")).to include("-Xprof")
  end

  it "should stop argument processing when a -- is seen" do
    expect(jruby_launcher_args("-- -Xhelp -Xtrace --headless")).to include("-Xhelp", "-Xtrace", "--headless")
  end

  # JRUBY-4151
  it "should properly handle single quotes" do
    expect(jruby_launcher_args("-e 'ABC DEF'")).to include("ABC DEF")
  end

  # JRUBY-4581
  it "should prepend JRUBY_OPTS to the start of the argument list to process" do
    with_environment "JRUBY_OPTS" => "--server -J-Dsome.key=val -rubygems" do
      expect(jruby_launcher_args("-e 'ABC DEF'")).to include("-server", "-Dsome.key=val", "-rubygems", "-e", "ABC DEF")
    end
  end

  # JRUBY-4611
  it "stops argument processing on first non-option argument" do
    expect(jruby_launcher_args("foo.rb --sample")[-2..-1]).to eq ["foo.rb", "--sample"]
  end

  # JRUBY-4608
  if RbConfig::CONFIG['target_os'] =~ /darwin/i
    it "includes file.encoding=UTF-8 on Mac if JAVA_ENCODING is not set" do
      expect(jruby_launcher_args("-e true")).to include("-Dfile.encoding=UTF-8")
      with_environment "JAVA_ENCODING" => "MacRoman" do
        expect(jruby_launcher_args("-e true")).not_to include("-Dfile.encoding=UTF-8")
      end
    end
  end

  it "does not crash on empty args" do
    expect(jruby_launcher_args("-e ''")).to include("-e")
    expect(jruby_launcher("-Xtrace '' 2>&1")).to match /-Xtrace/
    expect(jruby_launcher("-Xjdkhome '' 2>&1")).to match /-Xjdkhome/
  end

  # JRUBY-4706
  it "should put JRuby on regular classpath when -Xnobootclasspath is used" do
    args = jruby_launcher_args("-e true")
    expect(args.grep(/Xbootclasspath/)).not_to be_empty
    args = jruby_launcher_args("-Xnobootclasspath -e true")
    expect(args.grep(/Xbootclasspath/)).to be_empty
  end

  it "should put JRuby on regular classpath when VERIFY_JRUBY is set" do
    with_environment "VERIFY_JRUBY" => "true" do
      args = jruby_launcher_args("-e true")
      expect(args.grep(/Xbootclasspath/)).to be_empty
    end
  end

  # JRUBY-4709
  it "should include a bare : or ; at the end of the classpath, to include PWD in the path" do
    end_of_path_pattern = if windows?
      /;$/
    else
      /:$/
    end

    expect(classpath_arg(jruby_launcher_args("-Xnobootclasspath -e true"))).to match end_of_path_pattern
  end

  # JRUBY-6016
  it "should honor JAVA_MEM" do
    with_environment "JAVA_MEM" => "-Xmx768m" do
      expect(jruby_launcher_args("")).to include("-Xmx768m")
    end
  end

  it "should honor JAVA_STACK" do
    with_environment "JAVA_STACK" => "-Xss3072k" do
      expect(jruby_launcher_args("")).to include("-Xss3072k")
    end
  end

  it "should honor JRUBY_HOME" do
    with_environment "JRUBY_HOME" => "/tmp" do
      expect(jruby_launcher_args("")).to include("-Djruby.home=/tmp")
    end
  end

  context "JRUBY_HOME set and JRUBY_HOME/lib/jruby.jar exists" do
    let(:jruby_home) do
      require 'tempfile'
      t = Tempfile.new("jruby_home")
      t.path.tap { t.close! }
    end

    before do
      FileUtils.mkdir_p(File.join(jruby_home, "lib"))
      FileUtils.touch(File.join(jruby_home, "lib", "jruby.jar"))
    end
    after { FileUtils.rm_rf jruby_home }

    it "should add jruby.jar to the bootclasspath" do
      with_environment "JRUBY_HOME" => jruby_home do
        expect(jruby_launcher_args("")).to include("-Xbootclasspath/a:#{jruby_home}/lib/jruby.jar")
      end
    end
  end

  it "should place user-supplied options after default options" do
    args = jruby_launcher_args("-J-Djruby.home=/tmp")
    home_args = args.select {|x| x =~ /^-Djruby\.home/ }
    expect(home_args.length).to eq 2
    expect(home_args.last).to eq "-Djruby.home=/tmp"
  end

  it "should print the version" do
    expect(jruby_launcher("-Xversion 2>&1")).to match /Launcher Version #{JRubyLauncher::VERSION}/
  end

  it "should not crash on format-strings" do
    expect(jruby_launcher_args("-e %s%s%s%s%s 2>&1")).to include('-e', '%s%s%s%s%s')
  end
end
