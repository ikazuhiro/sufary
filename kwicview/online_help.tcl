#!/usr/local/bin/wish

tk_bisque; # tk3.6 のような色
option add *highlightThickness 0;
option add *padY 2; option add *padX 2

###########################################################################
# オンラインヘルプ
###########################################################################
proc online_help {hf w {flag ""}} {
    global sufary_path
    if {$flag != ""} { # ウィンドウの作成
	toplevel $w; wm title $w "ON LINE HELP"
	set ysize 30
        frame $w.rst
	text $w.rst.t1 -width 80 -height $ysize -bd 2 -relief sunken -yscrollcommand "$w.rst.sb set" -xscrollcommand ""
	scrollbar $w.rst.sb -relief sunken -command "$w.rst.t1 yview"
	button $w.rst.b -text Close -command "destroy $w"
	pack $w.rst.b -pady 2 -padx 2 -anchor e
	pack $w.rst.t1 -side left -fill both -expand 1
	pack $w.rst.sb -side right -padx 1 -pady 1 -fill y 
	pack $w.rst -fill both -expand 1
    }
    $w.rst.t1 delete 0.0 end

    ### ファイルの読み込み
    set f [open $sufary_path/kwicview/$hf r]
    set line_no 1
    while {1} {
	set in [gets $f]
	if {[regexp {^END} $in]} {break}
	if {[regexp {;} $in]} {
	    eval $in
	    continue;
	}

#	set tag_start [kstring first "<A HREF=\"" $in]
#	regexp {<A HREF=\"(.+)\">} $in g1 file  ; # "
#	regsub {<A HREF=\".+\">} $in {} in  ; # "
#	set tag_end [kstring first "</A>" $in]
#	regsub "</A>" $in {} in

	$w.rst.t1 insert end "$in\n"

#	puts "$line_no $tag_start $tag_end";
# 	if {$tag_start != $tag_end} {
# 	    $w.rst.t1 tag add lk$line_no $line_no.$tag_start $line_no.$tag_end
# 	    $w.rst.t1 tag configure lk$line_no -foreground blue
# 	    $w.rst.t1 tag bind lk$line_no <Button-1> "online_help $file"
# 	}
	incr line_no
    }
    close $f
}





###########################################################################
#online_help help/index.html .a 1









