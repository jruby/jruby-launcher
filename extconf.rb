require 'rbconfig'

mf = File.read('Makefile')
mf = mf.gsub(/^BINDIR\s*=.*$/, "BINDIR = #{Config::CONFIG['bindir']}")
File.open('Makefile', 'wb') {|f| f << mf}
