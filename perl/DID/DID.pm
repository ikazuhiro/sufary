#-*- perl -*-
# $Id: DID.pm,v 1.6 2000/11/27 09:11:15 tatuo-y Exp $

package DID;
require 5.000;
require DynaLoader;
@ISA = qw(DynaLoader);

# Constructor
sub new {
    my ($class,$didfile) = @_;
    my $self = {};
    bless $self, $class;

    return undef unless (defined $didfile);

    $self->{'did'} = suf_opendid($didfile);
    if (!($self->{'did'})) {
	return undef;
    }
    $self->{'file'} = $didfile;
    $self->{'size'} = suf_did_size($self->{'did'});

    return $self;
}

# Destructor
sub DESTROY {
    my $self = shift;

    suf_closedid($self->{'did'});
    #print "memory leak!" if (suf_memory_leak_check() == 0);
}

# Search
sub didsearch {
    my ($self, $target) = @_;
    
    return suf_didsearch($self->{'did'}, $target);
}
bootstrap DID;
1;
