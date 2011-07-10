require 'rake/gempackagetask'
require 'date'

begin
  gem 'rspec'
  require 'rspec/core/rake_task'
  desc "Runs Java Integration Specs"
  RSpec::Core::RakeTask.new
  task :default => :spec
rescue LoadError
  task :default do
    puts "rspec 2.0.0 or higher is not installed; skipping jruby-launcher specs"
  end
end

file './lib/jruby-launcher.rb' => 'version.h' do |t|
  version = nil
  IO.readlines(t.prerequisites.first).grep(/LAUNCHER_VERSION\s+"([^"]+)"/) {|l| version = $1 }
  ruby = IO.readlines(t.name)
  File.open(t.name, "wb") do |f|
    ruby.each do |l|
      f << l.sub(/VERSION\s*=\s*"([^"]*)"/, "VERSION = \"#{version}\"")
    end
  end
end

task :gemspec => './lib/jruby-launcher.rb' do
  @gemspec ||= Gem::Specification.new do |s|
    load './lib/jruby-launcher.rb'
    s.name = %q{jruby-launcher}
    s.platform = Gem::Platform.new("java")
    s.version = JRubyLauncher::VERSION
    s.authors = ["Nick Sieger", "Vladimir Sizikov"]
    s.date = Date.today.to_s
    s.description = %q{Builds and installs a native launcher for JRuby on your system}
    s.summary = %q{Native launcher for JRuby}
    s.email = ["nick@nicksieger.com", "vsizikov@gmail.com"]
    s.extensions = ["extconf.rb"]
    s.files = FileList["COPYING", "README.txt", "Makefile", "Rakefile", "*.c", "*.cpp", "*.h", "inc/*.*", "**/*.rb", "resources/*.*"]
    s.homepage = %q{http://jruby.org}
    s.rubyforge_project = %q{jruby-extras}
  end
end

task :package => :gemspec do
  Rake::GemPackageTask.new(@gemspec) do |pkg|
  end
  Rake::Task['gem'].invoke
end
