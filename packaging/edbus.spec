Name:       edbus
Summary:    D-Bus and HAL wrapper libraries for EFL
Version:    1.0.999.svn60291
Release:    1
Group:      TO_BE/FILLED_IN
License:    TO BE FILLED IN
Source0:    http://download.enlightenment.org/releases/edbus-%{version}.tar.bz2
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
%defattr(-,root,root,-)
#%{_libdir}/*.so.*
#%{_bindir}/*
%{_bindir}/e-notify-send
%{_bindir}/e_dbus_bluez_test
%{_bindir}/e_dbus_connman_test
%{_bindir}/e_dbus_notification_daemon
%{_bindir}/e_dbus_notify
%{_bindir}/e_dbus_ofono_test
%{_bindir}/e_dbus_test
%{_bindir}/e_dbus_test_client
%{_bindir}/e_dbus_ukit_test
%{_libdir}/libebluez.so.1
%{_libdir}/libebluez.so.1.0.999
%{_libdir}/libeconnman.so.1
%{_libdir}/libeconnman.so.1.0.999
%{_libdir}/libedbus.so.1
%{_libdir}/libedbus.so.1.0.999
%{_libdir}/libehal.so.1
%{_libdir}/libehal.so.1.0.999
%{_libdir}/libenotify.so.1
%{_libdir}/libenotify.so.1.0.999
%{_libdir}/libeofono.so.1
%{_libdir}/libeofono.so.1.0.999
%{_libdir}/libeukit.so.1
%{_libdir}/libeukit.so.1.0.999


%files devel
%defattr(-,root,root,-)
#%{_libdir}/*.so
#%{_includedir}/*
#%{_libdir}/pkgconfig/*.pc
%{_libdir}/libebluez.so
%{_libdir}/libeconnman.so
%{_libdir}/libedbus.so
%{_libdir}/libehal.so
%{_libdir}/libenotify.so
%{_libdir}/libeofono.so
%{_libdir}/libeukit.so
%{_libdir}/pkgconfig/ebluez.pc
%{_libdir}/pkgconfig/econnman.pc
%{_libdir}/pkgconfig/edbus.pc
%{_libdir}/pkgconfig/ehal.pc
%{_libdir}/pkgconfig/enotify.pc
%{_libdir}/pkgconfig/eofono.pc
%{_libdir}/pkgconfig/eukit.pc

%{_includedir}/e_dbus-1/E_Bluez.h
%{_includedir}/e_dbus-1/E_Connman.h
%{_includedir}/e_dbus-1/E_DBus.h
%{_includedir}/e_dbus-1/E_Hal.h
%{_includedir}/e_dbus-1/E_Notification_Daemon.h
%{_includedir}/e_dbus-1/E_Notify.h
%{_includedir}/e_dbus-1/E_Ofono.h
%{_includedir}/e_dbus-1/E_Ukit.h
