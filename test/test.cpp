#include "gtest/gtest.h"
#include "bitmap_index_manager.h"
#include "sqlparser.h"

class BitmapIndexTest : public testing::Test {
public:
  void SetUp() override { Bitmap::initBitmap(); }

protected:
  FileStore fileStore { "testTable" };
  BufferPoolManager bufferPoolManager { 200, &fileStore, 0};
  BitmapIndexManager bitmapIndexManager { "TestTable.txt", bufferPoolManager };
};

TEST_F(BitmapIndexTest, InsertTest) {
  // Insert
  SQL sql { "insert name=lihua age=3 gender=male department=Chemistry" };
  bitmapIndexManager.insert(sql.m_attributes);
}

TEST_F(BitmapIndexTest, SelectTest) {
  // Select
  SQL sql2 { "select name=lihua and age=3 and gender=male and department=Chemistry" };
  Record record { bitmapIndexManager.select(sql2.m_conditions).next() };
  ASSERT_EQ(strcmp(record.m_name, "lihua"), 0);
  ASSERT_EQ(record.m_age, 3);
  ASSERT_EQ(record.m_gender, Gender::MALE);
  ASSERT_EQ(record.m_department, Department::CHEMISTRY);
}

TEST_F(BitmapIndexTest, UpdateTest) {
  // Update
  SQL sql3 { "update name=liuhai where name=lihua" };
  ASSERT_EQ(bitmapIndexManager.update(sql3.m_conditions, sql3.m_attributes), 1);
  SQL sql4 { "select name=liuhai" };
  Record record2 { bitmapIndexManager.select(sql4.m_conditions).next() };
  ASSERT_EQ(strcmp(record2.m_name, "liuhai"), 0);

}

TEST_F(BitmapIndexTest, CountTest) {
  // Count
  ASSERT_EQ(bitmapIndexManager.count({}), 1);
}

TEST_F(BitmapIndexTest, DeleteTest) {
  // Delete
  ASSERT_EQ(bitmapIndexManager.remove({}), 1);
}

TEST_F(BitmapIndexTest, LargeIntegratedTest) {
  for (size_t i { 0 }; i < 10000; ++i) {
    std::string name = "lihua" + std::to_string(i);
    std::string gender = i % 2 ? "male" : "female";
    std::string age = std::to_string(i % 150);
    std::string department;
    switch (i % 4) {
    case 0: department = "Chemistry"; break;
    case 1: department = "ComputerScience"; break;
    case 2: department = "Physics"; break;
    case 3: department = "ForeignLanguage"; break;
    }

    // Insert
    SQL sql { "insert name=" + name + " gender=" + gender +
            " age=" + age + " department=" + department };
    bitmapIndexManager.insert(sql.m_attributes);

    // Select
    SQL sql2 { "select name=" + name + " gender=" + gender +
            " age=" + age + " department=" + department };
    Record record { bitmapIndexManager.select(sql2.m_conditions).next() };
    assert(record.m_name == name);
    ASSERT_EQ(record.m_age, i % 150);
    ASSERT_EQ(record.m_gender, i % 2 ? Gender::MALE : Gender::FEMALE);
    Department testDepartment;
    switch (i % 4) {
    case 0: testDepartment = Department::CHEMISTRY; break;
    case 1: testDepartment = Department::COMPUTER_SCIENCE; break;
    case 2: testDepartment = Department::PHYSICS; break;
    case 3: testDepartment = Department::FOREIGN_LANG; break;
    }
    ASSERT_EQ(record.m_department, testDepartment);

    // Update
    SQL sql3 { "update name=" + name + "a where name=" + name };
    ASSERT_EQ(bitmapIndexManager.update(sql3.m_conditions, sql3.m_attributes), 1);
    SQL sql4 { "select name=" + name + "a" };
    assert(bitmapIndexManager.select(sql4.m_conditions).next().m_name == name + "a");

    // Count
    ASSERT_EQ(bitmapIndexManager.count(sql4.m_conditions), 1);
    ASSERT_EQ(bitmapIndexManager.count({}), i + 1);
  }

  // Delete
  ASSERT_EQ(bitmapIndexManager.remove({}), 10000);
}
