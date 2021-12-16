#pragma once
#include "sqlscannar.h"

struct Where {
  Where();
  void A();
  void B();
  void C();
  ConditionType m_conditions;
};

struct Attributes {
  Attributes();
  void A();
  AttributeType m_attributes;
};

struct SQL {
  SQL(const std::string &sql);
  void getConditions();
  void getAttributes();
  Token m_operationType;
  AttributeType m_attributes;
  ConditionType m_conditions;
};
