#include "bitmap_index_manager.h"
#include "sqlparser.h"
#include <benchmark/benchmark.h>

/** This is a benchmark for insertions
 *  This insertions inserts 10000 records into the file
 *  All the record's datafields are set
 */
static void Insert(benchmark::State& state) {
  Bitmap::initBitmap();
  FileStore fileStore { "testTable" };
  BufferPoolManager bufferPoolManager { 200, &fileStore, 0};
  BitmapIndexManager bitmapIndexManager { "TestTable.txt", bufferPoolManager };
  std::vector<SQL> sqls;

  for (int i = 0; i < 10000; ++i) {
    std::string name = "lihu" + std::to_string(i);
    std::string gender = i % 2 ? "male" : "female";
    std::string age = std::to_string(i % 150);
    std::string department;
    switch (i % 4) {
    case 0: department = "Chemistry"; break;
    case 1: department = "ComputerScience"; break;
    case 2: department = "Physics"; break;
    case 3: department = "ForeignLanguage"; break;
    }
    sqls.emplace_back("insert name=" + name + " gender=" + gender +
                      " age=" + age + " department=" + department);
  }

  for (auto _ : state) {
    for (const auto &sql : sqls) bitmapIndexManager.insert(sql.m_attributes);
  }
}

/** This is a benchmark for selections
 *  After the insertion benchmark executed, all of the data are inserted into the database
 *  Now we are trying to get all of them back using the same arguments
 *  Select 10000 records back from the database
 */
static void Select(benchmark::State& state) {
  Bitmap::initBitmap();
  FileStore fileStore { "testTable" };
  BufferPoolManager bufferPoolManager { 200, &fileStore, 0};
  BitmapIndexManager bitmapIndexManager { "TestTable.txt", bufferPoolManager };
  std::vector<SQL> sqls;

  for (int i = 0; i < 10000; ++i) {
    std::string name = "lihu" + std::to_string(i);
    std::string gender = i % 2 ? "male" : "female";
    std::string age = std::to_string(i % 150);
    std::string department;
    switch (i % 4) {
    case 0: department = "Chemistry"; break;
    case 1: department = "ComputerScience"; break;
    case 2: department = "Physics"; break;
    case 3: department = "ForeignLanguage"; break;
    }
    sqls.emplace_back("select name=" + name + " and gender=" + gender +
                      " and age=" + age + " and department=" + department);
  }

  for (auto _ : state) {
    for (const auto &sql : sqls) {
      auto iter { bitmapIndexManager.select(sql.m_conditions) };
      while (iter.hasNext()) iter.next();
    }
  }
}

/** This is a selection benchmark
 *  Different from the above one, this one performs 10000 selections on the database
 *  Each time selects 67 records from the database
 */
static void SelectLarge(benchmark::State& state) {
  Bitmap::initBitmap();
  FileStore fileStore { "testTable" };
  BufferPoolManager bufferPoolManager { 200, &fileStore, 0};
  BitmapIndexManager bitmapIndexManager { "TestTable.txt", bufferPoolManager };
  std::vector<SQL> sqls;

  for (int i = 0; i < 10000; ++i) {
    std::string age = std::to_string(i % 150);
    sqls.emplace_back("select age=" + age);
  }

  for (auto _ : state) {
    for (const auto &sql : sqls) {
      auto iter { bitmapIndexManager.select(sql.m_conditions) };
      while (iter.hasNext()) iter.next();
    }
  }
}

/** This is a counting benchmark.
 *  Performs 1 count operations on the database.
 *  Counting the number of records on the database.
 */
static void Count(benchmark::State& state) {
  Bitmap::initBitmap();
  FileStore fileStore { "testTable" };
  BufferPoolManager bufferPoolManager { 200, &fileStore, 0};
  BitmapIndexManager bitmapIndexManager { "TestTable.txt", bufferPoolManager };
  std::vector<SQL> sqls;

  for (auto _ : state) bitmapIndexManager.count({});
}

/** This is a counting benchmark.
 *  Performs 10000 count operations on the database.
 *  Each time count 67 records from the database.
 */
static void CountLarge(benchmark::State& state) {
  Bitmap::initBitmap();
  FileStore fileStore { "testTable" };
  BufferPoolManager bufferPoolManager { 200, &fileStore, 0};
  BitmapIndexManager bitmapIndexManager { "TestTable.txt", bufferPoolManager };
  std::vector<SQL> sqls;

  for (int i = 0; i < 10000; ++i) {
    std::string age = std::to_string(i % 150);
    sqls.emplace_back("count age=" + age);
  }

  for (auto _ : state) {
    for (const auto &sql : sqls) bitmapIndexManager.count(sql.m_conditions);
  }
}

/** This is a update benchmark
 *  Perform 150 updates on the database
 *  Update all the oldAge to new Age
 */
static void Update(benchmark::State& state) {
  Bitmap::initBitmap();
  FileStore fileStore { "testTable" };
  BufferPoolManager bufferPoolManager { 200, &fileStore, 0};
  BitmapIndexManager bitmapIndexManager { "TestTable.txt", bufferPoolManager };
  std::vector<SQL> sqls;

  for (int i = 0; i < 150; ++i) {
    std::string oldAge = std::to_string(i % 150 + 1);
    std::string age = std::to_string(i % 150);
    sqls.emplace_back("update age=" + age + " where age=" + oldAge);
  }

  for (auto _ : state) {
    for (const auto &sql : sqls) bitmapIndexManager.count(sql.m_conditions);
  }
}

/** This is a update benchmark
 *  Update all the age, total 10000 records updated
 */
static void UpdateLarge(benchmark::State& state) {
  Bitmap::initBitmap();
  FileStore fileStore { "testTable" };
  BufferPoolManager bufferPoolManager { 200, &fileStore, 0};
  BitmapIndexManager bitmapIndexManager { "TestTable.txt", bufferPoolManager };
  SQL sql { "update age=15" };

  for (auto _ : state) {
    bitmapIndexManager.update(sql.m_conditions, sql.m_attributes);
  }
}

/** This is a delete benchmark
 *  Delete all the record where department=Chemistry (2500 records)
 */
static void Delete(benchmark::State& state) {
  Bitmap::initBitmap();
  FileStore fileStore { "testTable" };
  BufferPoolManager bufferPoolManager { 200, &fileStore, 0};
  BitmapIndexManager bitmapIndexManager { "TestTable.txt", bufferPoolManager };
  SQL sql { "delete department=Chemistry" };

  for (auto _ : state) {
    bitmapIndexManager.remove(sql.m_conditions);
  }
}

/** This is a delete benchmark,
 *  It remove all the records from the database. (7500 records)
 */
static void DeleteAll(benchmark::State& state) {
  Bitmap::initBitmap();
  FileStore fileStore { "testTable" };
  BufferPoolManager bufferPoolManager { 200, &fileStore, 0};
  BitmapIndexManager bitmapIndexManager { "TestTable.txt", bufferPoolManager };

  for (auto _ : state) {
    bitmapIndexManager.remove({});
  }
}

BENCHMARK(Insert);
BENCHMARK(Select);
BENCHMARK(SelectLarge);
BENCHMARK(Count);
BENCHMARK(CountLarge);
BENCHMARK(Update);
BENCHMARK(UpdateLarge);
BENCHMARK(Delete);
BENCHMARK(DeleteAll);
BENCHMARK_MAIN();
