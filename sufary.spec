%define prefix /usr
%define version 2.3.8

Summary: SUFARY is a package of libraries using the Suffix array 
Name: sufary
Version: %{version}
Release: 2
Copyright: GPL
Group: local
Packager: Taku Kudoh <taku-ku@is.aist-nara.ac.jp>
Source: sufary-%{version}.tar.gz
URL: http://cl.aist-nara.ac.jp/lab/nlt/ss/
BuildRoot: /var/tmp/sufary

%description
SUFARY is a package of libraries using the Suffix array 

%package devel
Summary: Libraries and header files for sufary
Group: Development/Libraries
Requires: sufary = %{version}

%package perl
Summary: sufary Perl Module
Group: Development/Libraries
Requires: perl >= 5.6 sufary = %{version}

%package ruby
Summary: sufary ruby Module
Group: Development/Libraries
Requires: ruby >= 1.4 sufary = %{version}

%description devel
Libraries and header files for sufary

%description perl
sufary Perl Module

%description ruby
sufary Ruby Module

%prep

%setup

%build
./configure --prefix=%{prefix}
make CFLAGS=$RPM_OPT_FLAGS

cd perl/SUFARY
perl Makefile.PL
make LCFLAGS="$RPM_OPT_FLAGS" LDDLFLAGS="-shared -L/usr/lib" LDFLAGS="-L/usr/lib"
cd ../..

cd perl/DID
perl Makefile.PL
make LCFLAGS="$RPM_OPT_FLAGS" LDDLFLAGS="-shared -L/usr/lib" LDFLAGS="-L/usr/lib"
cd ../..

%install
make prefix=$RPM_BUILD_ROOT%{prefix} install

cd perl/SUFARY
make PREFIX=$RPM_BUILD_ROOT%{prefix} install
cd ../..

cd perl/DID
make PREFIX=$RPM_BUILD_ROOT%{prefix} install
cd ../..

cd ruby
ruby extconf.rb
make
sed s#/usr#$RPM_BUILD_ROOT%{prefix}#g < Makefile > Makefile.new
mv -f Makefile.new Makefile
make install
cd ..

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%doc ChangeLog INSTALL README* TODO doc/*
%{prefix}/man/*
%{prefix}/lib/*.so*
%{prefix}/bin/*
%{prefix}/share/sufary/kwicview/*
%{prefix}/share/sufary/misc/*

%files devel
%defattr(-, root, root)
%{prefix}/include/*
%{prefix}/lib/*.a
%{prefix}/lib/*.la

%files perl
%defattr(-, root, root)
%{prefix}/lib/perl5/*

%files ruby
%defattr(-, root, root)
%{prefix}/lib/ruby/*
