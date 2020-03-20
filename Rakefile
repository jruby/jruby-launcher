require 'rubygems/package_task'
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

desc "Generate gemspec file"
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
    s.files = FileList["COPYING", "README.md", "Makefile", "Rakefile", "*.c", "*.cpp", "*.h", "inc/*.*", "**/*.rb", "resources/*.*"]
    s.homepage = %q{http://jruby.org}
  end
end

desc "Create gem file"
task :package => [:update_version, :gemspec] do
  Gem::PackageTask.new(@gemspec) do |pkg|
  end
  Rake::Task['gem'].invoke
end

desc "Update version.h based on information in lib/jruby-launcher.rb"
task :update_version do
  load File.join(File.dirname(__FILE__), "lib", "jruby-launcher.rb")
  version_file = File.join(File.dirname(__FILE__), "version.h")
  version_file_content = File.read(version_file)
  version_file_content.gsub! /JRUBY_LAUNCHER_VERSION\s+"[^"]+"/, "JRUBY_LAUNCHER_VERSION \"#{JRubyLauncher::VERSION}\""
  File.open(version_file, "w") do |f|
    f.puts version_file_content
  end
end
