#include "file_store.h"

#include "fileapi.h"

FileStore::FileStore(const std::string &tableName) {
  std::string tableFileName {tableName + ".db"};
  
  m_tableFileIO.open(tableFileName, std::ios::binary | std::ios::in | std::ios::out);
  if (!m_tableFileIO.is_open()) {
    createFile(tableFileName);
    m_tableFileIO.open(tableFileName, std::ios::binary | std::ios::in | std::ios::out);
    if (!m_tableFileIO.is_open()) {
      throw std::runtime_error("fail to open index file");
    }
  }
}

FileStore::~FileStore() {
  m_tableFileIO.close();
}

FileSizeType FileStore::pageIDToOffset(PageIDType pageID) {
  // pageID * 4096
  return pageID << 12;
}

void FileStore::readRawPage(FileType fileType, PageIDType pageID, ByteType *raw) {
  switch (fileType) {
  case FileType::TABLE:
    readRawPage_helper(m_tableFileIO, pageID, raw, "table file");
    break;
  default:
    throw std::runtime_error("wrong file type");
    break;
  }
}

void FileStore::writeRawPage(FileType fileType, PageIDType pageID, const ByteType *raw) {
  switch (fileType) {
  case FileType::TABLE:
    writeRawPage_helper(m_tableFileIO, pageID, raw, "table file");
    break;
  default:
    throw std::runtime_error("wrong file type");
    break;
  }
}

void FileStore::readRawPage_helper(std::fstream &fileIO, PageIDType pageID,
                                   ByteType *raw, const std::string &fileName) {
  FileSizeType offset = pageIDToOffset(pageID);
  
  fileIO.seekg(offset, std::ios::beg);
  fileIO.read(raw, PAGE_SIZE);
  if (fileIO.bad()) {
    throw std::runtime_error("IO error while reading " + fileName);
  }
  
  int read_bits = fileIO.gcount();
  if (read_bits < PAGE_SIZE) {
    throw std::runtime_error("read less than one page while reading " + fileName);
  }
  
}

void FileStore::writeRawPage_helper(std::fstream &fileIO, PageIDType pageID,
                                    const ByteType *raw, const std::string &fileName) {
  FileSizeType offset = pageIDToOffset(pageID);
  
  fileIO.seekp(offset, std::ios::beg);
  fileIO.write(raw, PAGE_SIZE);
  if (fileIO.bad()) {
    throw std::runtime_error("IO error while writing " + fileName);
  }
  
  fileIO.flush();
}

void FileStore::createFile(const std::string &fileName) {
  std::fstream fs{fileName, std::ios::out};
  fs.close();
}
