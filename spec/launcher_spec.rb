require File.expand_path('../spec_helper.rb', __FILE__)

describe "JRuby native launcher" do
  it "should run org.jruby.Main" do
    jruby_launcher_args("").last.should == "org/jruby/Main"
  end

  it "should pass unrecognized arguments to JRuby" do
    jruby_launcher_args("-J-Dsome.option -v --help")[-3..-1].should == ["org/jruby/Main", "-v", "--help"]
  end

  it "should print help message" do
    args = jruby_launcher_args("-Xhelp 2>&1")
    args.detect{|l| l =~ /JRuby Launcher usage/}.should be_true
    args.should include("-X")
    args = jruby_launcher_args("-X 2>&1")
    args.detect{|l| l =~ /JRuby Launcher usage/}.should be_true
    args.should include("-X")
  end

  it "should use $JAVACMD when JAVACMD is specified" do
    with_environment "JAVACMD" => File.join("jato") do
      if windows?
        jruby_launcher_args("-v 2>&1").join.should =~ %r{jato}
      else
        jruby_launcher_args("-v").first.should == File.join("jato")
      end
    end
  end

  it "should use $JAVA_HOME/bin/java when JAVA_HOME is specified" do
    with_environment "JAVA_HOME" => File.join("some", "java", "home") do
      if windows?
        jruby_launcher_args("-v 2>&1").join.should =~ %r{some/java/home}
      else
        jruby_launcher_args("-v").first.should == File.join("some", "java", "home", "bin", "java")
      end
    end
  end

  it "should use -Xjdkhome argument above JAVA_HOME" do
    with_environment "JAVA_HOME" => File.join("env", "java", "home") do
      if windows?
        jruby_launcher_args("-Xjdkhome some/java/home 2>&1").join.should =~ %r{some/java/home}
      else
        jruby_launcher_args("-Xjdkhome some/java/home").first.should == File.join("some", "java", "home", "bin", "java")
      end
    end
  end

  it "should complain about a missing log argument" do
    jruby_launcher("-Xtrace 2>&1").should =~ /Argument is missing for "-Xtrace"/
    jruby_launcher("-Xtrace -- 2>&1").should =~ /Argument is missing for "-Xtrace"/
  end

  it "should complain about a missing jdkhome argument" do
    jruby_launcher("-Xjdkhome 2>&1").should =~ /Argument is missing/
    jruby_launcher("-Xjdkhome -- 2>&1").should =~ /Argument is missing/
  end

  it "should complain about a missing classpath append argument" do
    jruby_launcher("-Xcp:a 2>&1").should =~ /Argument is missing/
    jruby_launcher("-Xcp:a -- 2>&1").should =~ /Argument is missing/
  end

  it "should run nailgun server with --ng-server option" do
    jruby_launcher_args("--ng-server").last.should == "com/martiansoftware/nailgun/NGServer"
  end

  it "should run nailgun client with --ng option" do
    jruby_launcher_args('--ng -e "puts 1"').should == ["org.jruby.util.NailMain", "-e", "puts 1"]
  end

  it "should handle -J JVM options" do
    jruby_launcher_args("-J-Darg1=value1 -J-Darg2=value2").should include("-Darg1=value1", "-Darg2=value2")
  end

  it "should pass -Xprop.erty=value as -J-Djruby.prop.erty=value" do
    jruby_launcher_args("-Xprop.erty=value").should include("-Djruby.prop.erty=value")
  end

  it "should pass -Xproperties as --properties" do
    jruby_launcher_args("-Xproperties").should include("--properties")
  end

  it "should default to 500m max heap" do
    jruby_launcher_args("").should include("-Xmx500m", "-Djruby.memory.max=500m")
  end

  it "should allow max heap to be overridden" do
    jruby_launcher_args("-J-Xmx256m").should include("-Xmx256m", "-Djruby.memory.max=256m")
  end

  it "should default to 2048k max stack" do
    jruby_launcher_args("").should include("-Xss2048k", "-Djruby.stack.max=2048k")
  end

  it "should allow max stack to be overridden" do
    jruby_launcher_args("-J-Xss512k").should include("-Xss512k", "-Djruby.stack.max=512k")
  end

  it "should add the contents of the CLASSPATH environment variable" do
    with_environment "CLASSPATH" => "some.jar" do
      classpath_arg = jruby_launcher_args("").detect{|a| a =~ /java\.class\.path/}
      classpath_arg.should =~ /-Djava.class.path=.*some.jar/
    end
  end

  it "should add the classpath elements in proper order" do
    s = File::PATH_SEPARATOR
    with_environment "CLASSPATH" => "some-env.jar" do
      classpath_arg = jruby_launcher_args("-Xcp:a some-other.jar -Xcp:p some.jar").detect{|a| a =~ /java\.class\.path/}
      classpath_arg.should =~ /-Djava.class.path=some.jar.*#{s}some-env.jar#{s}some-other.jar/
    end
  end

  it "should use the --server compiler" do
    jruby_launcher_args("--server").should include("-server")
  end

  it "should use the --client compiler" do
    jruby_launcher_args("--client").should include("-client")
  end

  it "should set the JMX settings when --manage is present" do
    jruby_launcher_args("--manage").should include("-Dcom.sun.management.jmxremote", "-Djruby.management.enabled=true")
  end

  it "should set the headless flag when --headless is present" do
    jruby_launcher_args("--headless").should include("-Djava.awt.headless=true")
  end

  it "should pass -Xprof when --sample is present" do
    jruby_launcher_args("--sample").should include("-Xprof")
  end

  it "should stop argument processing when a -- is seen" do
    jruby_launcher_args("-- -Xhelp -Xtrace --headless").should include("-Xhelp", "-Xtrace", "--headless")
  end

  # JRUBY-4151
  it "should properly handle single quotes" do
    jruby_launcher_args("-e 'ABC DEF'").should include("ABC DEF")
  end

  # JRUBY-4581
  it "should prepend JRUBY_OPTS to the start of the argument list to process" do
    with_environment "JRUBY_OPTS" => "--server -J-Dsome.key=val -rubygems" do
      jruby_launcher_args("-e 'ABC DEF'").should include("-server", "-Dsome.key=val", "-rubygems", "-e", "ABC DEF")
    end
  end

  # JRUBY-4611
  it "stops argument processing on first non-option argument" do
    jruby_launcher_args("foo.rb --sample")[-2..-1].should == ["foo.rb", "--sample"]
  end

  # JRUBY-4608
  if Config::CONFIG['target_os'] =~ /darwin/i
    it "includes file.encoding=UTF-8 on Mac if JAVA_ENCODING is not set" do
      jruby_launcher_args("-e true").should include("-Dfile.encoding=UTF-8")
      with_environment "JAVA_ENCODING" => "MacRoman" do
        jruby_launcher_args("-e true").should_not include("-Dfile.encoding=UTF-8")
      end
    end
  end

  it "does not crash on empty args" do
    jruby_launcher_args("-e ''").should include("-e")
    jruby_launcher("-Xtrace '' 2>&1").should =~ /-Xtrace/
    jruby_launcher("-Xjdkhome '' 2>&1").should =~ /-Xjdkhome/
  end

  # JRUBY-4706
  it "should put JRuby on regular classpath when -Xnobootclasspath is used" do
    args = jruby_launcher_args("-e true")
    args.grep(/Xbootclasspath/).should_not be_empty
    args = jruby_launcher_args("-Xnobootclasspath -e true")
    args.grep(/Xbootclasspath/).should be_empty
  end

  it "should put JRuby on regular classpath when VERIFY_JRUBY is set" do
    with_environment "VERIFY_JRUBY" => "true" do
      args = jruby_launcher_args("-e true")
      args.grep(/Xbootclasspath/).should be_empty
    end
  end

  # JRUBY-4709
  it "should include a bare : or ; at the end of the classpath, to include PWD in the path" do
    jruby_launcher_args("-Xnobootclasspath -e true").grep(/java\.class\.path/).first.should =~
      if windows?
        /;$/
      else
        /:$/
      end
  end

  # JRUBY-6016
  it "should honor JAVA_MEM" do
    with_environment "JAVA_MEM" => "-Xmx768m" do
      jruby_launcher_args("").should include("-Xmx768m", "-Djruby.memory.max=768m")
    end
  end

  it "should honor JAVA_STACK" do
    with_environment "JAVA_STACK" => "-Xss2048k" do
      jruby_launcher_args("").should include("-Xss2048k", "-Djruby.stack.max=2048k")
    end
  end

  it "should print the version" do
    jruby_launcher("-Xversion 2>&1").should =~ /Launcher Version [0-9.]+/
  end
end
