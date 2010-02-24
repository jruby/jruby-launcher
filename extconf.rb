require 'rbconfig'

mf = File.read('Makefile')
mf = mf.gsub(/^BINDIR\s*=.*$/, "BINDIR = #{Config::CONFIG['bindir']}")
mf = mf.gsub(/^SITELIBDIR\s*=.*$/, "SITELIBDIR = #{Config::CONFIG['sitelibdir']}")
File.open('Makefile', 'wb') {|f| f << mf}
