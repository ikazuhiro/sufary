#!/bin/sh

dlink='y'
sufary='y'
current=`pwd`
str_current=`echo $current | sed 's/\//\\\\\//g' `

cat << 'EOM'
-----------------------------------------------------------
                 --- DID Perl Module ---

���Υ�����ץȤϡ�Perl(ver.5�ʾ�) ���� DID ��
�Ȥ�����⥸�塼��򥻥åȥ��åפ����ΤǤ���
-----------------------------------------------------------


-----------------------------------------------------------
Perl ver.5 �ʾ夬ɬ�פǤ������ɤ��˥��󥹥ȡ��뤵��Ƥ�
�ޤ����� �ե�ѥ�̾�������Ʋ�������
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
			echo "λ��"
			jperl=$ans
			str_jperl=`echo $jperl | sed 's/\//\\\\\//g' `
		else
			echo "���դ���ޤ��� " $ans
		fi
		;;
	esac
done

cat << 'EOM'
-----------------------------------------------------------
�����ʥߥå���󥯥饤�֥��(libsufary.so.*)����Ѥ��ޤ�����
No �����򤷤����� libsufary.a ����Ѥ��ޤ���
-----------------------------------------------------------
EOM
echo -n "�����ʥߥå���󥯥饤�֥��λ��� [y]: "
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
echo �����ʥߥå���󥯥饤�֥��λ��� = $dlink
echo �����ȥǥ��쥯�ȥ� =
echo "	$current"
echo -----------------------------------------------------------

echo -n "����Ǥ�����Ǥ����� [y]: "
read ans
case "$ans" in
	n*)
		echo ����
		exit
		;;
esac

# sufary
echo "----- module ������ -----"
case "$dlink" in
y*)
	$jperl ./Makefile.PL
	;;
*)
	$jperl ./Makefile.static.PL
	;;
esac
make

echo ��λ���ޤ�����
echo
echo    $jperl ./test.pl
echo �ǥƥ��Ȥ��ޤ��礦��

echo
