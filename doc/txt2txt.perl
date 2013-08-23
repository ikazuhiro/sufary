#!/usr/local/bin/perl
#
# *.txt をみやすいフォーマットに変換


$level = 0;
while(<>){

    if(/^■■■/){
	print "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
	print $_;
	print "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
	$level = 1;
	next;
    }elsif(/^★/){
	print "────────────────────\n";
	print $_;
	print "────────────────────\n";
	$level = 2;
	next;
    }elsif(s/^♪\s+//){
	chomp;
	$l = length($_);
	if($l % 2 == 1){$_ .= " ";$l++;}
	print "┌",('─'x($l/2)),"┐\n";
	print "│$_│\n";
	print "└",('─'x($l/2)),"┘\n";
	$level = 3;
	next;
    }elsif(/^\-\-\-\-\-\-/){
	if($sample_flag == 1){
	    $sample_flag = 0;
	    print "  ";
	}else {
	    $sample_flag = 1;
	}
    }

    if($sample_flag == 1){print "  ";}

    print "  " if($level);
    print;
}

