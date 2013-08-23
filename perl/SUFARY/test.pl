# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'
# $Id: test.pl,v 1.5 2000/09/10 10:34:33 tatuo-y Exp $

BEGIN {
    $| = 1; print "1..10\n";
    $LIB = '.';
    unshift(@INC, $LIB, "$LIB/blib/lib", "$LIB/blib/arch");
}
END {print "not ok 1\n" unless $loaded;}

use SUFARY;
$loaded = 1;
print "ok 1\n";

###
$text = "./test.txt";

$suf = SUFARY->new($text);
if (defined($suf)) {
    print "ok 2\n";
} else {
    print "not ok 2\n";
}

# $suf->debug_mode;

if ($suf->{'textfile'} == $text
    && $suf->{'arrayfile'} == "./text.txt.ary"
    && $suf->{'textsize'} == 88
    && $suf->{'arraysize'} == 88) {
    print "ok 3\n";
} else {
    print "not ok 3\n";
}

@res = $suf->search("foo");
if ($suf->get_line(@res[0]) == "<a>foo</a><a>bar</a>\n") {
    print "ok 4\n";
} else {
    print "not ok 4\n";
}
if ($suf->get_region($index, '<a>', '</a>') == "<a>foo</a>") {
    print "ok 5\n";
} else {
    print "not ok 5\n";
}

@res = $suf->regex_search("fo[a-z]");
if ($res[0] == 47) {
    print "ok 6\n";
} else {
    print "not ok 6\n";
}

if ($suf->get_string(2, 5) == "racad") {
    print "ok 7\n";;
} else {
    print "not ok 7\n";
}

@res = $suf->case_insensitive_search("dabra");
#print $_," ",$suf->get_string($_,5),"\n" for(@res);
if (@res == 4 && $res[0] == 37) {
    print "ok 8\n";
} else {
    print "not ok 8\n";
}

my ($from, $to) = $suf->range_search("a");
my $i;
for ($i = $from; $i <= $to; $i++) {
    my $pos =  $suf->get_position($i);
    #print "$i $pos ", $suf->get_line($pos);
}
if ($i == $to + 1 and $to - $from + 1 == 27) {
    print "ok 9\n";
} else {
    print "not ok 9\n";
}


### trie search
($from, $to) = (0, $suf->{'arraysize'} - 1);
my $skip = 0;
my $key = "daB";
while ($skip < length($key)) {
    ($from, $to) =
	$suf->range_search(substr($key, $skip, 1), $from, $to, $skip);
#    print "." x $skip, substr($key, $skip, 1), " $from <= range <=$to\n";
    last if (!defined $from);
    for ($i = $from; $i <= $to; $i++) {
	my $pos =  $suf->get_position($i);
#	print "$i $pos ", $suf->get_line($pos);
    }
    $skip++;
}
if ($skip == 3 and $from == 69 and $to == 69) {
    print "ok 10\n";
} else {
    print "not ok 10\n";
}
