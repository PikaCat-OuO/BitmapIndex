#include "sqlparser.h"

Where::Where() { A(); }

void Where::A() {
  B();
  while (true) {
    if (Token::OR not_eq CURRENT_TOKEN) break;
    yylex();
    B();
    this->m_conditions.emplace_back(Token::OR);
  }
}

void Where::B() {
  C();
  while (true) {
    if (Token::AND not_eq CURRENT_TOKEN) break;
    yylex();
    C();
    this->m_conditions.emplace_back(Token::AND);
  }
}

void Where::C() {
  SubConditionType condition;

  switch (CURRENT_TOKEN) {
  case Token::LEFT: yylex(); A(); yylex(); break;
  case Token::ATTRIBUTE_NAME:
    std::get<0>(condition) = yytext;
    yylex();
    std::get<1>(condition) = CURRENT_TOKEN;
    yylex();
    if (Token::IS_NULL not_eq CURRENT_TOKEN and Token::IS_NOT_NULL not_eq CURRENT_TOKEN) {
      std::get<2>(condition) = yytext;
      yylex();
    }
    break;
  case Token::VALUE:
    std::get<2>(condition) = yytext;
    yylex();
    std::get<1>(condition) = CURRENT_TOKEN;
    yylex();
    std::get<0>(condition) = yytext;
    yylex();
    break;
  default: return;
  }

  this->m_conditions.emplace_back(condition);
}

Attributes::Attributes() { A(); }

void Attributes::A() {
  std::string attributeName;
  ValueType value;

  while (true) {
    switch (CURRENT_TOKEN) {
    case Token::ATTRIBUTE_NAME:
      attributeName = yytext;
      yylex();
      yylex();
      value = yytext;
      break;
    case Token::VALUE:
      value = yytext;
      yylex();
      yylex();
      attributeName = yytext;
      break;
    default: return;
    }

    yylex();
    this->m_attributes.emplace_back(attributeName, value);
  }
}

SQL::SQL(const std::string &sql) {
  yy_scan_string(sql.c_str());
  yylex();
  this->m_operationType = CURRENT_TOKEN;
  switch (CURRENT_TOKEN) {
  case Token::SELECT: yylex(); getConditions(); break;
  case Token::INSERT: yylex(); getAttributes(); break;
  case Token::DELETE: yylex(); getConditions(); break;
  case Token::UPDATE: yylex(); getAttributes(); yylex(); getConditions(); break;
  case Token::COUNT: yylex(); getConditions(); break;
  default: break;
  }

  // Special cases for age
  for (auto &[attributeName, value] : this->m_attributes) {
    if ("age" == attributeName && 3 > value.length()) {
      value = std::string(3 - value.length(), '0') + value;
    }
  }
  for (auto &condition : this->m_conditions) {
    if (1 == condition.index()) {
      auto &[attributeName, comparator, value] { std::get<1>(condition) };
      if ("age" == attributeName and 3 > value.length()){
        value = std::string(3 - value.length(), '0') + value;
      }
    }
  }
}

void SQL::getConditions() { this->m_conditions = Where{}.m_conditions; }
void SQL::getAttributes() { this->m_attributes = Attributes{}.m_attributes; }
