# fileselecter.tcl 
#  ファイル選択モジュール
#
# $Header: /cvs/sufary/kwicview/fileselecter.tcl,v 1.1.1.1 1999/09/16 09:18:31 tatuo-y Exp $
#
# ● 参考文献 ●
# ─────┬──────────────────────
# ＩＳＢＮ　│4-274-06115-9
# 書名　　　│Tcl/Tk プログラミング入門 (FD付)
# 著者名　　│宮田重明、芳賀敏彦 共著
# 出版社　　│オーム社
# ページ　　│第１２章５節 "ファイル操作" ( pp.211-235 ) 
# ─────┴──────────────────────

# ウィンドウを表示してファイルを選択する
# w	ウィンドウのトップレベルウィジェットのパス
# title	タイトルバーなどに表示する文字列
# flag	新規のファイル名を許すかどうかのフラグ
proc SelectFile {w title flag} {
    global selected_file
    set moto_dir [pwd]
    # ウィジェット作り
    toplevel $w
    wm title $w $title
    frame $w.frame
    label $w.frame.dir -text "Directry:" -fg blue
    label $w.frame.pwd
    label $w.frame.file -text "FileName:" -fg blue
    entry $w.frame.fileName -relief sunken -bd 2
    listbox $w.frame.fileList -width 60 -height 20 -relief sunken \
	-yscrollcommand "$w.frame.scrollbar set"
    scrollbar $w.frame.scrollbar -orient vertical -relief sunken \
	-command "$w.frame.fileList yview"
    frame $w.click
    button $w.click.select -width 10 -text "OK" -command "selectFile $w.frame $flag"
    button $w.click.cansel -width 10 -text "CANCEL" -command {set selected_file {}}

    pack $w.click -anchor s -pady 5
    pack $w.click.select -side left -padx 10
    pack $w.click.cansel -padx 10
    pack $w.frame -side top -padx 10 -pady 10
    pack $w.frame.dir -anchor w
    pack $w.frame.pwd -anchor w
    pack $w.frame.file -anchor w
    pack $w.frame.fileName -fill x 
    pack $w.frame.scrollbar -side right -fill y
    pack $w.frame.fileList -side left -fill x -expand 1

    # リストボックスがクリックされたときの処理のバインド
    bind $w.frame.fileList <ButtonRelease-1> "showEntry $w.frame"  
    # リストボックスがダブルクリックされたときの処理のバインド
    bind $w.frame.fileList <Double-ButtonPress-1> "selectFile $w.frame $flag"
    # エントリでReturnが入力されたときの処理のバインド
    bind $w.frame.fileName <Any-Return> "selectFile $w.frame $flag" 
    
    setFileList $w.frame ;     # ファイル一覧の表示

    # 現在設定されているファイルをデフォルトとして表示
    $w.frame.fileName delete 0 end; $w.frame.fileName insert 0 $selected_file

    # ウィンドウの表示位置の計算と表示
    wm withdraw $w
    update idletasks
    set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 \
	       - [winfo vrootx [winfo parent $w]]]
    set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 \
	       - [winfo vrooty [winfo parent $w]]]
    wm geom $w +$x+$y
    wm deiconify $w

    # グラブとフォーカスの設定
    set oldFocus [focus]
    grab $w
    focus $w.frame.fileName

    # ウィンドウマネージャからの削除要請への対応
    wm protocol $w WM_DELETE_WINDOW {set selected_file {}}

    # ファイルが選択されるのを待つ
    tkwait variable selected_file

    # ファイルが選択されたら、ウィンドウを削除、フォーカスを返還をして
    # 選択されたファイル名を返す
    destroy $w;  focus $oldFocus

    cd $moto_dir
    return $selected_file
}

# ウィンドウの表示の更新
# f	関連するウィジェットが載っているフレームウィジェットのパス
proc setFileList f {
    set pwd [pwd]; # カレントディレクトリ名の表示
    $f.pwd configure -text $pwd; # ラベルにカレントディレクトリのパスを表示する
    # ファイル一覧の表示
    # エントリとリストボックスの内容のクリア
    $f.fileList delete 0 end; $f.fileName delete 0 end
    # ファイルの検索
    set files {..}
    catch {set files [concat $files [lsort [glob *]]]} 
    # ファイル一覧をリストボックスへ表示する
    foreach file $files { # サブディレクトリだったら 末尾に "/" を付ける 
	if [file isdirectory $file] {set file $file/}
	$f.fileList insert end $file
    }
} 

# 指定されたファイルを選択する
# f	関連するウィジェットが載っているフレームウィジェットのパス
# flag	新規のファイル名を許すかどうかのフラグ
proc selectFile {f flag} {
    global selected_file
    # 指定されたファイル名を取得する
    set name [$f.fileName get] 
    # ディレクトリであることを示すためにつけた "/" をはずす
    if {$name  != "/"} {set name [string trimright $name "/"] }
    # 指定されたものがディレクトリなら そこへ cd し、表示を更新する
    if [file isdirectory $name] {cd $name; setFileList $f; return}
    # 指定されたものが既存のファイル名か、新規のファイル名が許されているならば
    if {[file exists $name] || $flag} {	# 広域変数 selected_file に代入する
	if {[regexp {^[/\~]} $name]} {set selected_file $name
	} else {set selected_file [pwd]/$name}
	return
    }
    # 指定されたものが無効な文字列の場合にはエントリの内容をクリアする
    $f.fileName delete 0 end
}

# リストボックスの選択されている項目をエントリに表示する
# f	関連するウイジェットが載っているフレームウィジェットのパス
proc showEntry f {
    set index [$f.fileList curselection]
    if {$index == ""} return
    $f.fileName delete 0 end;
    $f.fileName insert 0 [$f.fileList get $index]
}
