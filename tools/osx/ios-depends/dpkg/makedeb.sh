#!/bin/sh

find XBMC_iOS -name '.svn' -exec rm -rf {} \;
chown -R root:wheel XBMC_iOS/Applications/XBMC.app
dpkg-deb -b XBMC_iOS com.xbmx.xbmc_ios.deb
