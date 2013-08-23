# fileselecter.tcl 
#  �ե���������⥸�塼��
#
# $Header: /cvs/sufary/kwicview/fileselecter.tcl,v 1.1.1.1 1999/09/16 09:18:31 tatuo-y Exp $
#
# �� ����ʸ�� ��
# ��������������������������������������������������������
# �ɣӣ£Ρ���4-274-06115-9
# ��̾��������Tcl/Tk �ץ���ߥ����� (FD��)
# ����̾���������Ľ�����˧����ɧ ����
# ���Ǽҡ������������
# �ڡ����������裱���ϣ��� "�ե��������" ( pp.211-235 ) 
# ��������������������������������������������������������

# ������ɥ���ɽ�����ƥե���������򤹤�
# w	������ɥ��Υȥåץ�٥륦�������åȤΥѥ�
# title	�����ȥ�С��ʤɤ�ɽ������ʸ����
# flag	�����Υե�����̾��������ɤ����Υե饰
proc SelectFile {w title flag} {
    global selected_file
    set moto_dir [pwd]
    # ���������åȺ��
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

    # �ꥹ�ȥܥå���������å����줿�Ȥ��ν����ΥХ����
    bind $w.frame.fileList <ButtonRelease-1> "showEntry $w.frame"  
    # �ꥹ�ȥܥå��������֥륯��å����줿�Ȥ��ν����ΥХ����
    bind $w.frame.fileList <Double-ButtonPress-1> "selectFile $w.frame $flag"
    # ����ȥ��Return�����Ϥ��줿�Ȥ��ν����ΥХ����
    bind $w.frame.fileName <Any-Return> "selectFile $w.frame $flag" 
    
    setFileList $w.frame ;     # �ե����������ɽ��

    # �������ꤵ��Ƥ���ե������ǥե���ȤȤ���ɽ��
    $w.frame.fileName delete 0 end; $w.frame.fileName insert 0 $selected_file

    # ������ɥ���ɽ�����֤η׻���ɽ��
    wm withdraw $w
    update idletasks
    set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 \
	       - [winfo vrootx [winfo parent $w]]]
    set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 \
	       - [winfo vrooty [winfo parent $w]]]
    wm geom $w +$x+$y
    wm deiconify $w

    # ����֤ȥե�������������
    set oldFocus [focus]
    grab $w
    focus $w.frame.fileName

    # ������ɥ��ޥ͡����㤫��κ�������ؤ��б�
    wm protocol $w WM_DELETE_WINDOW {set selected_file {}}

    # �ե����뤬���򤵤��Τ��Ԥ�
    tkwait variable selected_file

    # �ե����뤬���򤵤줿�顢������ɥ��������ե����������ִԤ򤷤�
    # ���򤵤줿�ե�����̾���֤�
    destroy $w;  focus $oldFocus

    cd $moto_dir
    return $selected_file
}

# ������ɥ���ɽ���ι���
# f	��Ϣ���륦�������åȤ��ܤäƤ���ե졼�०�������åȤΥѥ�
proc setFileList f {
    set pwd [pwd]; # �����ȥǥ��쥯�ȥ�̾��ɽ��
    $f.pwd configure -text $pwd; # ��٥�˥����ȥǥ��쥯�ȥ�Υѥ���ɽ������
    # �ե����������ɽ��
    # ����ȥ�ȥꥹ�ȥܥå��������ƤΥ��ꥢ
    $f.fileList delete 0 end; $f.fileName delete 0 end
    # �ե�����θ���
    set files {..}
    catch {set files [concat $files [lsort [glob *]]]} 
    # �ե����������ꥹ�ȥܥå�����ɽ������
    foreach file $files { # ���֥ǥ��쥯�ȥ���ä��� ������ "/" ���դ��� 
	if [file isdirectory $file] {set file $file/}
	$f.fileList insert end $file
    }
} 

# ���ꤵ�줿�ե���������򤹤�
# f	��Ϣ���륦�������åȤ��ܤäƤ���ե졼�०�������åȤΥѥ�
# flag	�����Υե�����̾��������ɤ����Υե饰
proc selectFile {f flag} {
    global selected_file
    # ���ꤵ�줿�ե�����̾���������
    set name [$f.fileName get] 
    # �ǥ��쥯�ȥ�Ǥ��뤳�Ȥ򼨤�����ˤĤ��� "/" ��Ϥ���
    if {$name  != "/"} {set name [string trimright $name "/"] }
    # ���ꤵ�줿��Τ��ǥ��쥯�ȥ�ʤ� ������ cd ����ɽ���򹹿�����
    if [file isdirectory $name] {cd $name; setFileList $f; return}
    # ���ꤵ�줿��Τ���¸�Υե�����̾���������Υե�����̾��������Ƥ���ʤ��
    if {[file exists $name] || $flag} {	# �����ѿ� selected_file ����������
	if {[regexp {^[/\~]} $name]} {set selected_file $name
	} else {set selected_file [pwd]/$name}
	return
    }
    # ���ꤵ�줿��Τ�̵����ʸ����ξ��ˤϥ���ȥ�����Ƥ򥯥ꥢ����
    $f.fileName delete 0 end
}

# �ꥹ�ȥܥå��������򤵤�Ƥ�����ܤ򥨥�ȥ��ɽ������
# f	��Ϣ���륦�������åȤ��ܤäƤ���ե졼�०�������åȤΥѥ�
proc showEntry f {
    set index [$f.fileList curselection]
    if {$index == ""} return
    $f.fileName delete 0 end;
    $f.fileName insert 0 [$f.fileList get $index]
}
