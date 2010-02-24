require 'spec/rake/spectask'
require 'rake/gempackagetask'
require 'date'

Spec::Rake::SpecTask.new

task :default => :spec

load './lib/jruby-launcher.rb'

gemspec = Gem::Specification.new do |s|
  s.name = %q{jruby-launcher}
  s.platform = Gem::Platform.new("java")
  s.version = JRubyLauncher::VERSION
  s.authors = ["Nick Sieger", "Vladimir Sizikov"]
  s.date = Date.today.to_s
  s.description = %q{Builds and installs a native launcher for JRuby on your system}
  s.summary = %q{Native launcher for JRuby}
  s.email = ["nick@nicksieger.com", "vsizikov@gmail.com"]
  s.extensions = ["extconf.rb"]
  s.files = FileList["COPYING", "README.txt", "Makefile", "Rakefile", "*.c", "*.cpp", "*.h", "inc/*.*", "**/*.rb", "resources/*.*", "spec/*.rb"]
  s.homepage = %q{http://jruby.org}
  s.rdoc_options = ["--main", "README.txt"]
  s.rubyforge_project = %q{jruby-extras}
end

Rake::GemPackageTask.new(gemspec) do |pkg|
end
