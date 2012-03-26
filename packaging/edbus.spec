#sbs-git:slp/pkgs/e/edbus edbus 1.0.0.001+svn.69045slp2+build01 706f4acfdaf1b818e2387701a993eca929051358
Name:       edbus
Summary:    D-Bus and HAL wrapper libraries for EFL
Version:    1.0.0.001+svn.69376slp2+build02
Release:    1
Group:      TO_BE/FILLED_IN
License:    TO BE FILLED IN
Source0:    %{name}-%{version}.tar.gz
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
export CFLAGS+=" -fvisibility=hidden -fPIC"
export LDFLAGS+=" -fvisibility=hidden -Wl,--hash-style=both -Wl,--as-needed"

%autogen --disable-static
%configure --disable-static

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install


%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_bindir}/e-*
%{_bindir}/e_*
%{_libdir}/libe*.so.*
/usr/share/e_dbus/logo.png

%files devel
%defattr(-,root,root,-)
%{_libdir}/libebluez.so
%{_libdir}/libeconnman0_7x.so
%{_libdir}/libedbus.so
%{_libdir}/libehal.so
%{_libdir}/libenotify.so
%{_libdir}/libeofono.so
%{_libdir}/libeukit.so
%{_libdir}/pkgconfig/ebluez.pc
%{_libdir}/pkgconfig/econnman-0.7x.pc
%{_libdir}/pkgconfig/edbus.pc
%{_libdir}/pkgconfig/ehal.pc
%{_libdir}/pkgconfig/enotify.pc
%{_libdir}/pkgconfig/eofono.pc
%{_libdir}/pkgconfig/eukit.pc

%{_includedir}/e_dbus-1/E_Bluez.h
%{_includedir}/e_dbus-1/connman0_7x/E_Connman.h
%{_includedir}/e_dbus-1/E_DBus.h
%{_includedir}/e_dbus-1/E_Hal.h
%{_includedir}/e_dbus-1/E_Notification_Daemon.h
%{_includedir}/e_dbus-1/E_Notify.h
%{_includedir}/e_dbus-1/E_Ofono.h
%{_includedir}/e_dbus-1/E_Ukit.h
