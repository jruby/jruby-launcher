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
    jruby_launcher_args("").should include("-Xmx500m")
  end

  it "should allow max heap to be overridden" do
    jruby_launcher_args("-J-Xmx256m").should include("-Xmx256m")
  end

  it "should default to 1024k max stack" do
    jruby_launcher_args("").should include("-Xss1024k")
  end

  it "should allow max heap to be overridden" do
    jruby_launcher_args("-J-Xss512k").should include("-Xss512k")
  end
end
