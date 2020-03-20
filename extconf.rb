mf = File.read('Makefile')
mf = mf.gsub(/^BINDIR\s*=.*$/, "BINDIR = #{RbConfig::CONFIG['bindir']}")
mf = mf.gsub(/^PREFIX\s*=.*$/, "PREFIX = #{File.dirname(RbConfig::CONFIG['libdir'])}")
mf = mf.gsub(/^JRUBY_VERSION\s*=.*$/, "JRUBY_VERSION = #{JRUBY_VERSION}")

# Launcher will use .module_opts file if present, otherwise hardcoded add-opens for this module.
# Module options are only supported on JRuby 9.2.1 or higher.
if JRUBY_VERSION =~ /(^1)|(^9\.[01])|(^9\.2\.0\.0)/
  mf = mf.gsub(/^JRUBY_MODULE\s*=.*1$/, "JRUBY_MODULE =")
end
puts mf
File.open('Makefile', 'wb') {|f| f << mf}
