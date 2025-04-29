#!/bin/bash
set -e

APP_NAME="Inwentaryzacja"
VERSION="1.2.0"
ARCH="x86_64"
BUILD_DIR="deploy"
RPM_DIR="rpm_build"

mkdir -p $RPM_DIR/BUILD $RPM_DIR/RPMS $RPM_DIR/SOURCES $RPM_DIR/SPECS $RPM_DIR/SRPMS

# PRZYGOTOWANIE poprawnej struktury tar.gz
mkdir -p $RPM_DIR/tmp/${APP_NAME}-${VERSION}
cp -r $BUILD_DIR/* $RPM_DIR/tmp/${APP_NAME}-${VERSION}/
tar czvf $RPM_DIR/SOURCES/${APP_NAME}-${VERSION}.tar.gz -C $RPM_DIR/tmp ${APP_NAME}-${VERSION}
rm -rf $RPM_DIR/tmp

cat <<EOF > $RPM_DIR/SPECS/${APP_NAME}.spec
Name:           $APP_NAME
Version:        $VERSION
Release:        1%{?dist}
Summary:        Program do inwentaryzacji retro komputerów
License:        MIT
URL:            http://example.com
Source0:        %{name}-%{version}.tar.gz
BuildArch:      $ARCH

%description
Program do inwentaryzacji retro komputerów.

%prep
%setup -q

%install
mkdir -p %{buildroot}/usr/bin
cp usr/bin/$APP_NAME %{buildroot}/usr/bin/

%files
/usr/bin/$APP_NAME

%changelog
* Mon Apr 28 2025 Twoje Imię <twoj.email@example.com> - 1.2.0-1
- Initial RPM release
EOF

rpmbuild --define "_topdir $(pwd)/$RPM_DIR" -bb $RPM_DIR/SPECS/${APP_NAME}.spec

echo "✅ Utworzono paczkę RPM w katalogu: $RPM_DIR/RPMS/$ARCH/"
