#include "bitmap_index_manager.h"
#include "sqlparser.h"
#include "server.h"

void printRecord(const Record &record) {
  if (0 == strlen(record.m_name)) std::cout << "NULL";
  else std::cout << record.m_name;
  std::cout << "\t\t";

  if (-1 == record.m_age) std::cout << "NULL";
  else std::cout << record.m_age;
  std::cout << "\t\t";

  switch (record.m_gender) {
  case Gender::NULL_VALUE: std::cout << "NULL"; break;
  case Gender::MALE: std::cout << "male"; break;
  case Gender::FEMALE: std::cout << "female"; break;
  }
  std::cout << "\t\t";

  switch (record.m_department) {
  case Department::NULL_VALUE: std::cout << "NULL"; break;
  case Department::CHEMISTRY: std::cout << "Chemistry"; break;
  case Department::COMPUTER_SCIENCE: std::cout << "ComputerScience"; break;
  case Department::FOREIGN_LANG: std::cout << "ForeignLanguage"; break;
  case Department::PHYSICS: std::cout << "Physics"; break;
  }

  std::cout << std::endl;
}

int main() {
  Bitmap::initBitmap();
  FileStore fileStore { "testTable" };
  BufferPoolManager bufferPoolManager { 100, &fileStore, 0};
  BitmapIndexManager bitmapIndexManager { "TestTable.txt", bufferPoolManager };

  while (true) {
    std::string sqlString;
    std::cout << "BitmapIndex> ";
    std::getline(std::cin, sqlString);
    if (sqlString == "") continue;
    if (sqlString == "exit") break;

    SQL sql { sqlString };
    if (Token::SELECT == sql.m_operationType) {
      std::cout << "name\t\tage\t\tgender\t\tdepartment" << std::endl;
      uint64_t rowCount { 0 };
      RecordIterator iter { bitmapIndexManager.select(sql.m_conditions) };
      while (iter.hasNext()) {
        ++rowCount;
        printRecord(iter.next());
      }
      std::cout << "Total " << rowCount << " row(s) selected";
    }
    else if (Token::INSERT == sql.m_operationType){
      bitmapIndexManager.insert(sql.m_attributes);
      std::cout << "Insert success.";
    }
    else if (Token::UPDATE == sql.m_operationType) {
      uint64_t rowCount { bitmapIndexManager.update(sql.m_conditions, sql.m_attributes) };
      std::cout << "Update success, " << rowCount << " row(s) affected";
    }
    else if (Token::DELETE == sql.m_operationType) {
      uint64_t rowCount { bitmapIndexManager.remove(sql.m_conditions) };
      std::cout << "Delete success, " << rowCount << " row(s) affected";
    }
    else if (Token::COUNT == sql.m_operationType) {
      std::cout << "There are total " << bitmapIndexManager.count(sql.m_conditions)
                << " row(s) counted";
    }
    else std::cout << "Unrecognized command";

    std::cout << std::endl << std::endl;
  }

  std::cout << "Bye";
  return 0;
}
