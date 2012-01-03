require 'rbconfig'

mf = File.read('Makefile')
mf = mf.gsub(/^BINDIR\s*=.*$/, "BINDIR = #{Config::CONFIG['bindir']}")
mf = mf.gsub(/^PREFIX\s*=.*$/, "PREFIX = #{File.dirname(Config::CONFIG['libdir'])}")
puts mf
File.open('Makefile', 'wb') {|f| f << mf}
