#!/bin/sh

OUTFILE=ios_wrap.h
INFILE=wrapper.c
INOBJ=wrapper.o

echo "#ifndef _IOS_WRAP_H_" > $OUTFILE
echo "#define _IOS_WRAP_H_" >> $OUTFILE

echo "#include <string.h>" >> $OUTFILE
echo "#include <stdlib.h>" >> $OUTFILE
echo "#include <stdint.h>" >> $OUTFILE
echo "#include <dirent.h>" >> $OUTFILE

echo "#if defined(__APPLE__) && defined(__arm__)" >> $OUTFILE

echo "#ifdef __APPLE__" >> $OUTFILE
echo "  typedef int64_t   off64_t;" >> $OUTFILE
echo "  typedef off_t     __off_t;" >> $OUTFILE
echo "  typedef off64_t   __off64_t;" >> $OUTFILE
echo "  typedef fpos_t    fpos64_t;" >> $OUTFILE
echo "  #define stat64    stat" >> $OUTFILE
echo "  #define statvfs64 statvfs" >> $OUTFILE
echo "  // this is not right, fix later" >> $OUTFILE
echo "  #define _G_va_list va_list" >> $OUTFILE
echo "#endif" >> $OUTFILE

echo "#ifdef _LINUX" >> $OUTFILE
echo "  #define _stat stat" >> $OUTFILE
echo "#endif" >> $OUTFILE

for i in `nm $INOBJ | grep __wrap | awk -F 'T _' '{ print $2}'`
do
  FUN=`echo $i | sed 's/__wrap_//g'`
  echo "#undef $FUN" >> $OUTFILE
  echo "#define $FUN $i" >> $OUTFILE
done

echo "#ifdef __cplusplus" >> $OUTFILE
echo "extern \"C\" {" >> $OUTFILE
echo "#endif" >> $OUTFILE
grep -r __wrap $INFILE | grep -v bash | sed 's/)/);/g' >> $OUTFILE
echo "#ifdef __cplusplus" >> $OUTFILE
echo "}" >> $OUTFILE
echo "#endif" >> $OUTFILE

echo "#endif" >> $OUTFILE
echo "#endif //_IOS_WRAP_H_" >> $OUTFILE

