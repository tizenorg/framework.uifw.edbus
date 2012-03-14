Name:       edbus
Summary:    D-Bus and HAL wrapper libraries for EFL
Version:    1.0.0.001+svn.67284slp2
Release:    1.1
Group:      TO_BE/FILLED_IN
License:    TO BE FILLED IN
Source0:    http://download.enlightenment.org/releases/edbus-%{version}.tar.gz
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(ecore)
BuildRequires:  pkgconfig(eina)
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(dbus-1)


%description
D-Bus and HAL wrapper libraries for use with the EFL



%package devel
Summary:    D-Bus and HAL wrapper libraries for EFL (devel)
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
D-Bus and HAL wrapper libraries for use with the EFL (devel)


%prep
%setup -q


%build

%autogen --disable-static
%configure --disable-static
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install


%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%{_bindir}/*
%{_libdir}/lib*.so.*
/usr/share/e_dbus/logo.png


%files devel
%defattr(-,root,root,-)
%{_libdir}/*.so
%{_libdir}/pkgconfig/*.pc
%{_includedir}/e_dbus-1/*.h
/usr/include/e_dbus-1/connman0_7x/E_Connman.h

