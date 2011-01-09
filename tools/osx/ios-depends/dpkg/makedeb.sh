#!/bin/sh

find XBMC -name '.svn' -exec rm -rf {} \;
chown -R root:wheel XBMC/Applications/XBMC.app
dpkg-deb -b XBMC XBMC.deb
