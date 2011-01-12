#!/bin/sh

if [ ! -d XBMC.app ]; then
  echo "XBMC.app not found! copy it from build dir to here -> `pwd`"
  exit 1
fi

PACKAGE=com.xbmc.xbmc-ios

VERSION=10.0
REVISION=0
ARCHIVE=${PACKAGE}_${VERSION}-${REVISION}_iphoneos-arm.deb

echo Creating $PACKAGE package version $VERSION revision $REVISION
sudo rm -rf $PACKAGE
sudo rm -rf $ARCHIVE

# create debian control file.
mkdir -p $PACKAGE/DEBIAN
echo "Package: $PACKAGE"                          >  $PACKAGE/DEBIAN/control
echo "Name: XBMC-iOS"                             >> $PACKAGE/DEBIAN/control
echo "Version: $VERSION-$REVISION"                >> $PACKAGE/DEBIAN/control
echo "Architecture: iphoneos-arm"                 >> $PACKAGE/DEBIAN/control
echo "Description: XBMC Multimedia center for 4.x iOS devices" >> $PACKAGE/DEBIAN/control
echo "Homepage: http://xbmc.org/"                 >> $PACKAGE/DEBIAN/control
echo "Depiction: http://xbmc.org/"                >> $PACKAGE/DEBIAN/control
echo "Maintainer: Scott Davilla, Edgar Hucek"     >> $PACKAGE/DEBIAN/control
echo "Author: http://xbmc.org/"                   >> $PACKAGE/DEBIAN/control
echo "Section: Multimedia"                        >> $PACKAGE/DEBIAN/control
echo "Icon: file:///Applications/XBMC/XBMC.png"   >> $PACKAGE/DEBIAN/control

# prerm: called on remove and upgrade - get rid of existing bits.
echo "#!/bin/sh"                                  >  $PACKAGE/DEBIAN/prerm
echo "rm -f /Applications/XBMC.app"               >> $PACKAGE/DEBIAN/prerm
chmod +x $PACKAGE/DEBIAN/prerm

# postinst: nothing for now.
echo "#!/bin/sh"                                  >  $PACKAGE/DEBIAN/postinst
chmod +x $PACKAGE/DEBIAN/postinst

# prep XBMC.app
mkdir -p $PACKAGE/Applications
cp -r XBMC.app $PACKAGE/Applications/
find $PACKAGE/Applications/ -name '.svn' -exec rm -rf {} \;
find $PACKAGE/Applications/ -name '.gitignore' -exec rm -rf {} \;

# set ownership to root:root
sudo chown -R 0:0 $PACKAGE

echo Packaging $PACKAGE
export COPYFILE_DISABLE
export COPY_EXTENDED_ATTRIBUTES_DISABLE
dpkg-deb -b $PACKAGE $ARCHIVE
dpkg-deb --info $ARCHIVE
dpkg-deb --contents $ARCHIVE

# clean up by removing package dir
sudo rm -rf $PACKAGE
