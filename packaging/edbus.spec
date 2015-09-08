Name:       edbus
Summary:    D-Bus and HAL wrapper libraries for EFL
Version:    1.6.0+svn.76526slp2+build10
Release:    1
Group:      System/Libraries
License:    BSD 2-Clause
URL:        http://www.enlightenment.org/
Source0:    %{name}-%{version}.tar.gz
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  ecore-devel
BuildRequires:  eina-devel
BuildRequires:  evas-devel
BuildRequires:  dbus-devel


%description
D-Bus and HAL wrapper libraries for use with the EFL


%package devel
Summary:    D-Bus and HAL wrapper libraries for EFL (devel)
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}


%description devel
D-Bus and HAL wrapper libraries for use with the EFL (devel)


%package tools
Summary:    D-Bus and HAL wrapper libraries for EFL (tools)
Group:      Development/Tools
Requires:   %{name} = %{version}-%{release}
Provides:   %{name}-bin
Obsoletes:  %{name}-bin


%description tools
D-Bus and HAL wrapper libraries for use with the EFL (tools)


%prep
%setup -q


%build
export CFLAGS+=" -fvisibility=hidden -fPIC -Wall"
export LDFLAGS+=" -fvisibility=hidden -Wl,--hash-style=both -Wl,--as-needed"

%autogen --disable-static

make %{?jobs:-j%jobs}


%install
%make_install
mkdir -p %{buildroot}/%{_datadir}/license
cp %{_builddir}/%{buildsubdir}/COPYING %{buildroot}/%{_datadir}/license/%{name}
cp %{_builddir}/%{buildsubdir}/COPYING %{buildroot}/%{_datadir}/license/%{name}-tools


%post -p /sbin/ldconfig


%postun -p /sbin/ldconfig


%files
%defattr(-,root,root,-)
%{_libdir}/libe*.so.*
%{_datadir}/license/%{name}
%manifest %{name}.manifest


%files devel
%defattr(-,root,root,-)
%{_bindir}/e_dbus_async_client_test
%{_bindir}/e_dbus_async_server_test
%{_bindir}/e_dbus_bluez_test
%{_bindir}/e_dbus_connman0_7x_test
%{_bindir}/e_dbus_ofono_test
%{_bindir}/e_dbus_test
%{_bindir}/e_dbus_test_client
%{_bindir}/e_dbus_ukit_test
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


%files tools
%{_bindir}/e-notify-send
%{_bindir}/e_dbus_notification_daemon
%{_bindir}/e_dbus_notify
%{_datadir}/e_dbus/logo.png
%{_datadir}/license/%{name}-tools
%manifest %{name}-tools.manifest
