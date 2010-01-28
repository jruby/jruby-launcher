require File.expand_path('../spec_helper.rb', __FILE__)

describe "JRuby native launcher" do
  it "should run org.jruby.Main" do
    jruby_launcher_args("").last.should == "org/jruby/Main"
  end

  it "should pass unrecognized arguments to JRuby" do
    jruby_launcher_args("-J-Dsome.option -v --help")[-3..-1].should == ["org/jruby/Main", "-v", "--help"]
  end

  it "should print help message" do
    jruby_launcher("-Xhelp").should =~ /JRuby Launcher usage/
  end
end
