#!/bin/sh

if [ ! -d XBMC.frappliance ]; then
  echo "XBMC.frappliance not found! copy it from build dir to here -> `pwd`"
  exit 1
fi
PACKAGE=com.xbmc.xbmc-atv2

VERSION=10.0
REVISION=0
ARCHIVE=${PACKAGE}_${VERSION}-${REVISION}_iphoneos-arm.deb

echo Creating $PACKAGE package version $VERSION revision $REVISION
sudo rm -rf $PACKAGE
sudo rm -rf $ARCHIVE

# create debian control file.
mkdir -p $PACKAGE/DEBIAN
echo "Package: $PACKAGE"                          >  $PACKAGE/DEBIAN/control
echo "Name: XBMC-ATV2"                            >> $PACKAGE/DEBIAN/control
echo "Depends: beigelist"                         >> $PACKAGE/DEBIAN/control
echo "Version: $VERSION-$REVISION"                >> $PACKAGE/DEBIAN/control
echo "Architecture: iphoneos-arm"                 >> $PACKAGE/DEBIAN/control
echo "Description: XBMC Multimedia center for AppleTV 2" >> $PACKAGE/DEBIAN/control
echo "Homepage: http://xbmc.org/"                 >> $PACKAGE/DEBIAN/control
echo "Maintainer: Scott Davilla, Edgar Hucek"     >> $PACKAGE/DEBIAN/control
echo "Author: TeamXBMC"                           >> $PACKAGE/DEBIAN/control
echo "Section: Multimedia"                        >> $PACKAGE/DEBIAN/control

# prerm: called on remove and upgrade - get rid of existing bits.
echo "#!/bin/sh"                                  >  $PACKAGE/DEBIAN/prerm
echo "rm -rf /Applications/XBMC.frappliance"      >> $PACKAGE/DEBIAN/prerm
echo "if [ "`uname -r`" = "10.3.1" ]; then"       >> $PACKAGE/DEBIAN/prerm
echo "  rm -rf /Applications/Lowtide.app/Appliances/XBMC.frappliance" >> $PACKAGE/DEBIAN/prerm
echo "else"                                       >> $PACKAGE/DEBIAN/prerm
echo "  rm -rf /Applications/AppleTV.app/Appliances/XBMC.frappliance" >> $PACKAGE/DEBIAN/prerm
echo "fi"                                         >> $PACKAGE/DEBIAN/prerm
chmod +x $PACKAGE/DEBIAN/prerm

# postinst: symlink XBMC.frappliance into correct location and reload Lowtide/AppleTV.
echo "#!/bin/sh"                                  >  $PACKAGE/DEBIAN/postinst
echo "if [ "`uname -r`" = "10.3.1" ]; then"       >> $PACKAGE/DEBIAN/postinst
echo "  ln -sf /Applications/XBMC.frappliance /Applications/Lowtide.app/Appliances/XBMC.frappliance" >> $PACKAGE/DEBIAN/postinst
echo "  killall Lowtide"                          >> $PACKAGE/DEBIAN/postinst
echo "else"                                       >> $PACKAGE/DEBIAN/postinst
echo "  ln -sf /Applications/XBMC.frappliance /Applications/AppleTV.app/Appliances/XBMC.frappliance" >> $PACKAGE/DEBIAN/postinst
echo "  killall AppleTV"                          >> $PACKAGE/DEBIAN/postinst
echo "fi"                                         >> $PACKAGE/DEBIAN/postinst
chmod +x $PACKAGE/DEBIAN/postinst

# prep XBMC.frappliance
mkdir -p $PACKAGE/Applications
cp -r XBMC.frappliance $PACKAGE/Applications/
find $PACKAGE/Applications/ -name '.svn' -exec rm -rf {} \;
find $PACKAGE/Applications/ -name '.gitignore' -exec rm -rf {} \;

# set ownership to root:root
sudo chown -R 0:0 $PACKAGE

echo Packaging $PACKAGE
export COPYFILE_DISABLE
export COPY_EXTENDED_ATTRIBUTES_DISABLE
../../ios-depends/build/bin/dpkg-deb -b $PACKAGE $ARCHIVE
../../ios-depends/build/bin/dpkg-deb --info $ARCHIVE
../../ios-depends/build/bin/dpkg-deb --contents $ARCHIVE

# clean up by removing package dir
sudo rm -rf $PACKAGE
