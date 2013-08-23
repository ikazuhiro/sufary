#!/bin/sh

dlink='y'
sufary='y'
current=`pwd`
str_current=`echo $current | sed 's/\//\\\\\//g' `

cat << 'EOM'
-----------------------------------------------------------
                 --- DID Perl Module ---

このスクリプトは，Perl(ver.5以上) から DID を
使うためモジュールをセットアップするものです．
-----------------------------------------------------------


-----------------------------------------------------------
Perl ver.5 以上が必要ですが，どこにインストールされてい
ますか？ フルパス名で答えて下さい．
-----------------------------------------------------------
EOM
jperl=`which perl`
if test -x "$jperl"; then
	echo -n
else
	jperl='perl'
fi
str_jperl=''
while test "$str_jperl" = "" ; do
	echo -n "? [$jperl]: "
	read ans
	case "$ans" in
	'')
		if test -r "$jperl"; then
			echo "ok."
			str_jperl=`echo $jperl | sed 's/\//\\\\\//g' `
		fi
		;;
	*)
		if test -r "$ans"; then
			echo "了解．"
			jperl=$ans
			str_jperl=`echo $jperl | sed 's/\//\\\\\//g' `
		else
			echo "見付かりません " $ans
		fi
		;;
	esac
done

cat << 'EOM'
-----------------------------------------------------------
ダイナミックリンクライブラリ(libsufary.so.*)を使用しますか？
No を選択した場合は libsufary.a を使用します．
-----------------------------------------------------------
EOM
echo -n "ダイナミックリンクライブラリの使用 [y]: "
read ans
case "$ans" in
n*)
	dlink='n'
	;;
*)
	dlink='y'
	;;
esac

echo -----------------------------------------------------------
echo Perl = $jperl
echo ダイナミックリンクライブラリの使用 = $dlink
echo カレントディレクトリ =
echo "	$current"
echo -----------------------------------------------------------

echo -n "これでよろしいですか？ [y]: "
read ans
case "$ans" in
	n*)
		echo 中断
		exit
		;;
esac

# sufary
echo "----- module 作製中 -----"
case "$dlink" in
y*)
	$jperl ./Makefile.PL
	;;
*)
	$jperl ./Makefile.static.PL
	;;
esac
make

echo 終了しました．
echo
echo    $jperl ./test.pl
echo でテストしましょう．

echo
