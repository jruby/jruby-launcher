mf = File.read('Makefile')
mf = mf.gsub(/^BINDIR\s*=.*$/, "BINDIR = #{RbConfig::CONFIG['bindir']}")
mf = mf.gsub(/^PREFIX\s*=.*$/, "PREFIX = #{File.dirname(RbConfig::CONFIG['libdir'])}")
mf = mf.gsub(/^JRUBY_VERSION\s*=.*$/, "JRUBY_VERSION = #{JRUBY_VERSION}")

# Launcher will use .module_opts file if present, otherwise hardcoded add-opens for this module.
# Pre-9.2.1: ALL-UNNAMED because no name was exported
# 9.2.1 and higher: org.jruby.dist
if JRUBY_VERSION !~ /(^1)|(^9\.[01])|(^9\.2\.0\.0)/
  mf = mf.gsub(/^JRUBY_MODULE\s*=.*$/, "JRUBY_MODULE = 1")
end
puts mf
File.open('Makefile', 'wb') {|f| f << mf}
