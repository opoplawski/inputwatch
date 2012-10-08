Name:		inputwatch
Version:	1
Release:	1%{?dist}
Summary:	Monitors /dev/input/* and updates /var/spool/input

Group:		System/Daemons
License:	GPLv3+
URL:		http://www.cora.nwra.com/~orion/
Source0:	inputwatch.c
Source1:        inputwatch.service
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

BuildRequires:	systemd-units
Requires(post):	systemd-units
Requires(postun):	systemd-units
Requires(preun):	systemd-units

%description
Monitors /dev/input/* and updates /var/spool/input.


%prep
%setup -c -T


%build
cc %{optflags} -o inputwatch %SOURCE0


%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{_sbindir} $RPM_BUILD_ROOT%{_unitdir}
cp -p inputwatch $RPM_BUILD_ROOT%{_sbindir}/inputwatch
cp -p %SOURCE1 $RPM_BUILD_ROOT%{_unitdir}/inputwatch.service


%clean
rm -rf $RPM_BUILD_ROOT

%post
if [ $1 -eq 1 ] ; then
    # Initial installation
    /bin/systemctl enable inputwatch.service >/dev/null 2>&1 || :
fi

%postun
/bin/systemctl daemon-reload >/dev/null 2>&1 || :
if [ $1 -ge 1 ] ; then
    # Package upgrade, not uninstall
    /bin/systemctl try-restart inputwatch.service >/dev/null 2>&1 || :
fi

%preun
if [ $1 -eq 0 ] ; then
    # Package removal, not upgrade
    /bin/systemctl --no-reload disable inputwatch.service > /dev/null 2>&1 || :
    /bin/systemctl stop inputwatch.service > /dev/null 2>&1 || :
fi


%files
%defattr(-,root,root,-)
%{_unitdir}/inputwatch.service
%{_sbindir}/inputwatch


%changelog
* Thu Oct 13 2011 Orion Poplawski <orion@cora.nwra.com> - 1-1
- Initial package
