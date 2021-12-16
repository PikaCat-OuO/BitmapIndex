#pragma once
#include "globals.h"
#include "bitmap_index.h"
#include "buffer_pool_manager.h"
#include <fstream>

class RecordIterator {
public:
  RecordIterator(const Bitmap &bitmap, BufferPoolManager &bufferPoolManager);
  bool hasNext();
  Record next();

private:
  std::stack<RecordIDType> m_recordIDs;
  BufferPoolManager &m_bufferPoolManager;
};

class BitmapIndexManager
{
public:
  BitmapIndexManager(const std::string &tableName, BufferPoolManager &bufferPoolManager);
  ~BitmapIndexManager();
  uint64_t count(const ConditionType &conditions);
  uint64_t remove(const ConditionType &conditions);
  void insert(const AttributeType &attributes);
  uint64_t update(const ConditionType &conditions, const AttributeType &attributes);
  RecordIterator select(const ConditionType &conditions);

protected:
  bool exist(const std::string &attributeName);
  void writeRecord(uint64_t pos, Record &&record);
  void insert_helper(const AttributeType &attributes, uint64_t pos);
  void update_helper(const AttributeType &attributes, uint64_t pos);
  Bitmap conditionToBitmap(const ConditionType &conditions);

private:
  /** Table name */
  std::string m_tableName;
  /** Next record ID */
  RecordIDType m_nextRecordID;
  /** Existence bitmap */
  Bitmap m_existenceBitmap;
  /** Attribute name to bitmap index */
  std::map<std::string, BitmapIndex> m_bitmapIndices;

  /** Buffer pool manager */
  BufferPoolManager &m_bufferPoolManager;
};
