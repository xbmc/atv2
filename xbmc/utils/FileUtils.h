#pragma once
#include "FileItem.h"

class CFileUtils
{
public:
  static bool DeleteItem(const CFileItemPtr &item);
  static bool RenameFile(const CStdString &strFile);
  static bool SubtitleFileSizeAndHash(const CStdString &path, CStdString &size, CStdString &hash);
};
