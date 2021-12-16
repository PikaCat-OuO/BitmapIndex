#pragma once
#include <bits/stdc++.h>
#include <variant>

enum class Token {
  SELECT, INSERT, DELETE, UPDATE, WHERE, COUNT,
  LEFT, RIGHT,
  ATTRIBUTE_NAME, VALUE,
  AND, OR,
  EQUAL, NOT_EQUAL,
  IS_NULL, IS_NOT_NULL,
  GREATER_THAN, GREATER_THAN_OR_EQUAL_TO,
  LESS_THAN, LESS_THAN_OR_EQUAL_TO,
  EOL
};

using RecordIDType = uint64_t;

using ValueType = std::string;

using SubConditionType = std::tuple<std::string, Token, ValueType>;

using ConditionType = std::vector<std::variant<Token, SubConditionType>>;

using AttributeType = std::vector<std::pair<std::string, ValueType>>;

enum class FileType {INVALID, TABLE};
constexpr size_t PAGE_SIZE {4096};
using FileSizeType = uint64_t;
using PageIDType = uint32_t;
using FrameIDType = uint32_t;

constexpr PageIDType INVALID_PAGE_ID { std::numeric_limits<uint32_t>::max() };

using ByteType = char;

enum class Gender { NULL_VALUE, MALE, FEMALE };

enum class Department { NULL_VALUE, COMPUTER_SCIENCE, PHYSICS, CHEMISTRY, FOREIGN_LANG };

struct Record {
  char m_name[20] { 0 } ;
  int m_age { -1 };
  Gender m_gender { Gender::NULL_VALUE };
  Department m_department { Department::NULL_VALUE };
};

constexpr uint8_t MAX_PAGE_RECORD_SIZE { PAGE_SIZE / sizeof(Record)};
