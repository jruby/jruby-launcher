require File.expand_path('../spec_helper.rb', __FILE__)

describe "JRuby native launcher" do
  it "should run org.jruby.Main" do
    jruby_launcher_args("").last.should == "org/jruby/Main"
  end

  it "should pass unrecognized arguments to JRuby" do
    jruby_launcher_args("-J-Dsome.option -v --help")[-3..-1].should == ["org/jruby/Main", "-v", "--help"]
  end

  it "should print help message" do
    jruby_launcher("-Xhelp 2>&1").should =~ /JRuby Launcher usage/
  end

  it "should complain about a missing log argument" do
    jruby_launcher("-Xtrace 2>&1").should =~ /Argument is missing/
    jruby_launcher("-Xtrace -- 2>&1").should =~ /Argument is missing/
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

  it "should default to 500m max heap" do
    jruby_launcher_args("").should include("-Xmx500m", "-Djruby.memory.max=500m")
  end

  it "should allow max heap to be overridden" do
    jruby_launcher_args("-J-Xmx256m").should include("-Xmx256m", "-Djruby.memory.max=256m")
  end

  it "should default to 1024k max stack" do
    jruby_launcher_args("").should include("-Xss1024k", "-Djruby.stack.max=1024k")
  end

  it "should allow max heap to be overridden" do
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
end
