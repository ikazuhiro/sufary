#-*- perl -*-
# $Id: SUFARY.pm,v 1.10 2000/11/27 09:11:15 tatuo-y Exp $
#
#                                            written by T.Nakayama
package SUFARY;
require 5.000;
require DynaLoader;
@ISA = qw(DynaLoader);

# Constructor
sub new {
    my ($class,$textfile,$aryfile) = @_;
    my $self = {};
    bless $self, $class;

    return undef unless (defined $textfile);

    if (!(defined $aryfile)) {
	$aryfile = $textfile . '.ary';
    }
    $self->{'sufary'} = suf_openfile($textfile, $aryfile);
    if (! $self->{'sufary'}) {
	return undef;
    }

    $self->{'textfile'} = $textfile;
    $self->{'arrayfile'}  = $aryfile;
    $self->{'textsize'} = suf_textsize($self->{'sufary'});
    $self->{'arraysize'} = suf_arraysize($self->{'sufary'});

    return $self;
}

# Destructor
sub DESTROY {
    my $self = shift;

    suf_closefile($self->{'sufary'});
    #print "memory leak!" if (suf_memory_leak_check() == 0);
}

# ���� (�ꥹ���֤�)
sub search {
    my $self = shift;

    return suf_find($self->{'sufary'}, @_);
}

# ����
sub range_search {
    my $self = shift;

    return suf_range_search($self->{'sufary'}, @_);
}

# array ��ź����ƥ����Ȱ���(offset, index point)���Ѵ�
sub get_position {
    my $self = shift;

    return suf_get_position($self->{'sufary'}, @_);
}

# �����ޥå��Ԥν���
sub get_line {
    my $self = shift;

    return suf_getline($self->{'sufary'}, @_);
}

# �����ޥå��Ծ���ν���
sub get_line_info {
    my $self = shift;

    return suf_get_line_info($self->{'sufary'}, @_);
}

# �꡼�����μ���
sub get_region {
    my $self = shift;

    return suf_block($self->{'sufary'}, @_);
}
sub get_block {  # �ߴ����Τ���
    my $self = shift;

    return suf_block($self->{'sufary'}, @_);
}

# �꡼��������μ���
sub get_region_info {
    my $self = shift;

    return suf_get_region_info($self->{'sufary'}, @_);
}

#
sub get_string {
    my $self = shift;

    return suf_getstr($self->{'sufary'}, @_);
}

# regex search (�ꥹ���֤�)
sub regex_search {
    my $self = shift;

    return suf_regex_search($self->{'sufary'}, @_);
}

# ignore case (�ꥹ���֤�)
sub case_insensitive_search {
    my $self = shift;

    return suf_case_insensitive_search($self->{'sufary'}, @_);
}

sub debug_mode {
    suf_set_debug_mode();
}

bootstrap SUFARY;
1;
