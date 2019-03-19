require 'rbconfig'
mf = File.read('Makefile')
mf = mf.gsub(/^BINDIR\s*=.*$/, "BINDIR = #{RbConfig::CONFIG['bindir']}")
mf = mf.gsub(/^PREFIX\s*=.*$/, "PREFIX = #{File.dirname(RbConfig::CONFIG['libdir'])}")
puts mf
File.open('Makefile', 'wb') {|f| f << mf}
