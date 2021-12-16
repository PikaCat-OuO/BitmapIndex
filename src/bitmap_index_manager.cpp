#include "bitmap_index_manager.h"

RecordIterator::RecordIterator(const Bitmap &bitmap, BufferPoolManager &bufferPoolManager)
    : m_bufferPoolManager { bufferPoolManager } {
  // Save all the recordID
  for (const auto &recordID : bitmap) this->m_recordIDs.emplace(recordID);
}

bool RecordIterator::hasNext() { return not this->m_recordIDs.empty(); }

Record RecordIterator::next() {
  RecordIDType recordID { this->m_recordIDs.top() };
  this->m_recordIDs.pop();

  PageIDType pageID = recordID / MAX_PAGE_RECORD_SIZE;
  char *data { this->m_bufferPoolManager.fetchPage(FileType::TABLE, pageID)->getData() };
  Record record { reinterpret_cast<Record *>(data)[recordID % MAX_PAGE_RECORD_SIZE] };

  this->m_bufferPoolManager.unpinPage(FileType::TABLE, pageID, false);

  return record;
}

BitmapIndexManager::BitmapIndexManager(const std::string &tableName,
                                       BufferPoolManager &bufferPoolManager)
    : m_tableName { tableName }, m_nextRecordID { 0 },
      m_existenceBitmap { m_nextRecordID }, m_bufferPoolManager { bufferPoolManager } {
  // Check if the file exists
  std::ifstream fin { tableName };
  if (not fin.is_open()) { return; }

  // Get the next record id , the attribute count and the existence bitmap
  uint64_t nextRecordID, attributeCount;
  std::string existenceBitmap;
  fin >> nextRecordID >> attributeCount >> existenceBitmap;

  // Resize the existence bitmap
  this->m_nextRecordID = nextRecordID;
  this->m_existenceBitmap.resize();

  // Deserialize the exsitence bitmap string
  Bitmap::deserialize(existenceBitmap);

  // Create existence bitmap
  for (uint64_t pos { 0 }; pos < existenceBitmap.length(); ++pos) {
    if ('1' == existenceBitmap[pos]) this->m_existenceBitmap.setBit(pos);
  }

  for (uint64_t i { 0 }; i < attributeCount; ++i) {
    // Get the attribute name and the value count
    std::string attributeName;
    uint64_t valueCount;
    fin >> attributeName >> valueCount;

    // Create the attribute bitmap index
    this->m_bitmapIndices.emplace(attributeName, this->m_nextRecordID);

    for (uint64_t j {0}; j < valueCount; ++j) {
      // Get the value and the serialized bitmap
      ValueType value;
      std::string bitmap;
      fin >> value >> bitmap;

      // Deserialize bitmap
      Bitmap::deserialize(bitmap);

      // Create bitmap
      for (uint64_t pos { 0 }; pos < bitmap.length(); ++pos) {
        if ('1' == bitmap[pos]) this->m_bitmapIndices.at(attributeName).setBitmapBit(value, pos);
      }
    }
  }
}

BitmapIndexManager::~BitmapIndexManager() {
  // Output the next record id, the attribute count and the existence bitmap
  std::ofstream fout { this->m_tableName };
  fout << this->m_nextRecordID << " " << this->m_bitmapIndices.size() << " "
      << this->m_existenceBitmap.serialize() << " ";

  for (const auto &[attributeName, bitmapIndex] : this->m_bitmapIndices) {
    // Output the attribute name and the value count
    fout << attributeName << " " << bitmapIndex.getAllBitmaps().size() << " ";
    for (const auto &[value, bitmap] : bitmapIndex.getAllBitmaps()) {
      // Output the value and the serialized bitmap
      fout << value << " " << bitmap.serialize() << " ";
    }
  }
}

uint64_t BitmapIndexManager::count(const ConditionType &conditions) {
  // Extract out the bitmap and count the bitmap
  return conditionToBitmap(conditions).popCount();
}

uint64_t BitmapIndexManager::remove(const ConditionType &conditions) {
  // Find the record that need to be removed
  Bitmap removeBitmap { conditionToBitmap(conditions) };
  for (const auto &pos : removeBitmap) {
    // Find one record! Remove all related bits
    for (auto &[attributeName, bitmapIndex] : this->m_bitmapIndices) {
      bitmapIndex.clearAllBitmapBits(pos);
    }

    // Clear the existence bitmap
    this->m_existenceBitmap.clearBit(pos);
  }

  // Return the total record infected
  return removeBitmap.popCount();
}

void BitmapIndexManager::insert(const AttributeType &attributes) {
  // Find a 0 in existence bitmap，If we fine one, then insert the data
  for (const auto &pos : ~this->m_existenceBitmap) { insert_helper(attributes, pos); return; }

  // Check if we need to append a new page
  if (0 == this->m_nextRecordID % MAX_PAGE_RECORD_SIZE) {
    this->m_bufferPoolManager.appendNewPage(FileType::TABLE,
                                            this->m_nextRecordID / MAX_PAGE_RECORD_SIZE);
  }

  // If we do not find any one of it, create a new one
  ++this->m_nextRecordID;
  this->m_existenceBitmap.resize();
  for (auto &[attributeName, bitmapIndex] : this->m_bitmapIndices) bitmapIndex.resize();

  // Insert the data
  insert_helper(attributes, this->m_nextRecordID - 1);
}

uint64_t BitmapIndexManager::update(const ConditionType &conditions,
                                    const AttributeType &attributes) {
  Bitmap needToUpdate { conditionToBitmap(conditions) };

  for (const auto &pos : needToUpdate) {
    for (const auto &[attributeName, value] : attributes) {
      this->m_bitmapIndices.at(attributeName).clearAllBitmapBits(pos);
      this->m_bitmapIndices.at(attributeName).setBitmapBit(value, pos);
    }

    // Update the record to the disk
    update_helper(attributes, pos);
  }

  // Return the total record infected
  return needToUpdate.popCount();
}

RecordIterator BitmapIndexManager::select(const ConditionType &conditions) {
  return RecordIterator { conditionToBitmap(conditions), this->m_bufferPoolManager };
}

bool BitmapIndexManager::exist(const std::string &attributeName) {
  return this->m_bitmapIndices.count(attributeName);
}

void BitmapIndexManager::insert_helper(const AttributeType &attributes, uint64_t pos) {
  // Set the existence bitmap
  this->m_existenceBitmap.setBit(pos);

  PageIDType pageID = pos / MAX_PAGE_RECORD_SIZE;
  char *data { this->m_bufferPoolManager.fetchPage(FileType::TABLE, pageID)->getData() };
  Record &record { reinterpret_cast<Record *>(data)[pos % MAX_PAGE_RECORD_SIZE] };
  record = {};

  // Set related bits by the way
  for (const auto &[attributeName, value] : attributes) {
    if (not exist(attributeName)) {
      this->m_bitmapIndices.emplace(attributeName, this->m_nextRecordID);
    }

    this->m_bitmapIndices.at(attributeName).setBitmapBit(value, pos);
    // Hardcoded not good
    if ("name" == attributeName) strcpy_s(record.m_name, value.c_str());
    else if ("age" == attributeName) record.m_age = std::stoi(value);
    else if ("gender" == attributeName) {
      if ("male" == value) record.m_gender = Gender::MALE;
      else record.m_gender = Gender::FEMALE;
    } else {
      if ("ComputerScience" == value) record.m_department = Department::COMPUTER_SCIENCE;
      else if ("Chemistry" == value) record.m_department = Department::CHEMISTRY;
      else if ("Physics" == value) record.m_department = Department::PHYSICS;
      else record.m_department = Department::FOREIGN_LANG;
    }
  }

  this->m_bufferPoolManager.unpinPage(FileType::TABLE, pageID, true);
}

void BitmapIndexManager::update_helper(const AttributeType &attributes, uint64_t pos) {
  PageIDType pageID = pos / MAX_PAGE_RECORD_SIZE;
  char *data { this->m_bufferPoolManager.fetchPage(FileType::TABLE, pageID)->getData() };
  Record &record { reinterpret_cast<Record *>(data)[pos % MAX_PAGE_RECORD_SIZE] };

  // Set related bits by the way
  for (const auto &[attributeName, value] : attributes) {
    if (not exist(attributeName)) {
      this->m_bitmapIndices.emplace(attributeName, this->m_nextRecordID);
    }

    this->m_bitmapIndices.at(attributeName).setBitmapBit(value, pos);
    // Hardcoded not good
    if ("name" == attributeName) strcpy_s(record.m_name, value.c_str());
    else if ("age" == attributeName) record.m_age = std::stoi(value);
    else if ("gender" == attributeName) {
      if ("male" == value) record.m_gender = Gender::MALE;
      else record.m_gender = Gender::FEMALE;
    } else {
      if ("ComputerScience" == value) record.m_department = Department::COMPUTER_SCIENCE;
      else if ("Chemistry") record.m_department = Department::CHEMISTRY;
      else if ("Physics") record.m_department = Department::PHYSICS;
      else record.m_department = Department::FOREIGN_LANG;
    }
  }

  this->m_bufferPoolManager.unpinPage(FileType::TABLE, pageID, true);
}

Bitmap BitmapIndexManager::conditionToBitmap(const ConditionType &conditions) {
  // If the condition is empty, then returns the existence bitmap
  if (conditions.empty()) return this->m_existenceBitmap;

  std::stack<Bitmap> stack;

  for (const auto &condition : conditions) {
    if (condition.index()) {
      // Case for a = 1
      // Retrieve attribute name comparator value
      auto &[attributeName, comparator, value] { std::get<1>(condition) };
      // Get bitmap index by attribute name, get bitmap by value and then store it into the stack
      stack.emplace(this->m_bitmapIndices.at(attributeName).getBitmap(comparator, value));
      break;
    } else {
      // Case for AND/OR
      // Perform logical operation on the top two bitmaps on the stack
      Bitmap &&bitmap { std::move(stack.top()) };
      stack.pop();
      if (Token::AND == std::get<0>(condition)) stack.top() &= bitmap;
      else stack.top() |= bitmap;
    }
  }

  return stack.top() & this->m_existenceBitmap;
}
