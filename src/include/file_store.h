#pragma once

#include "globals.h"

class FileStore final {
public:
  FileStore(const std::string &tableName);
  ~FileStore();
  void readRawPage(FileType fileType, PageIDType pageID, ByteType *raw);
  void writeRawPage(FileType fileType, PageIDType pageID, const ByteType *raw);

private:
  FileSizeType pageIDToOffset(PageIDType pageID);
  
  void readRawPage_helper(std::fstream &fileIO, PageIDType pageID,
                          ByteType *raw, const std::string &fileName);
  void writeRawPage_helper(std::fstream &fileIO, PageIDType pageID,
                           const ByteType *raw, const std::string &fileName);
  void createFile(const std::string &fileName);
  
  std::fstream m_tableFileIO;
};


