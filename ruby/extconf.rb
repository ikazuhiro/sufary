# $Id: extconf.rb,v 1.4 2000/06/23 08:10:28 kazuma-t Exp $
require 'mkmf'

$CFLAGS = "-I../lib"
$LDFLAGS = "-L../lib/.libs"

if have_library("sufary")
    create_makefile("sufary")
end
