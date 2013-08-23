# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'
# $Id: test.pl,v 1.2 2000/07/16 09:24:49 tatuo-y Exp $

BEGIN{
    $| = 1; print "1..4\n";
    $LIB = '../DID';
    unshift(@INC, $LIB, "$LIB/blib/lib", "$LIB/blib/arch");
}
END {print "not ok 1\n" unless $loaded;}

use DID;
$loaded = 1;
print "ok 1\n";

$did = DID->new("./test.txt.did");
if (defined($did)) {
    print "ok 2\n";
} else {
    print "not ok 2\n";
}

($id, $index, $len) = $did->didsearch(0x2f);
# print join(" ",$id, $index, $len), "\n";

if (defined($id) and
    ($id == 0 && $index == 0x2c && $len == 0x36 - 0x2c)) {
    print "ok 3\n";
} else {
    print "not ok 3\n";
}

($id, $index, $len) = $did->didsearch(100000);

if (! defined($id)) {
    print "ok 4\n";
} else {
    print "not ok 4\n";
}

