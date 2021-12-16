#include "bitmap_index.h"

BitmapIndex::BitmapIndex(uint64_t &bitmapLength)
    : m_bitmapLength { bitmapLength }, m_notNullBitmap { bitmapLength } { }

void BitmapIndex::resize() {
  for (auto &[value, bitmap] : this->m_bitmaps) bitmap.resize();
  this->m_notNullBitmap.resize();
}

void BitmapIndex::setBitmapBit(const ValueType &value, uint64_t pos) {
  // If the bitmap does not exist, create one
  if (not exist(value)) this->m_bitmaps.emplace(value, this->m_bitmapLength);

  // Set the bit
  this->m_bitmaps.at(value).setBit(pos);

  // Set the bit in the not null bitmap
  this->m_notNullBitmap.setBit(pos);
}

void BitmapIndex::clearAllBitmapBits(uint64_t pos) {
  std::vector<ValueType> needToRemove;

  for (auto &[value, bitmap] : this->m_bitmaps) {
    // Set the bit to 0
    bitmap.clearBit(pos);
    // If the bitmap is empty, record it for removal later
    if (0 == bitmap.countBits()) needToRemove.emplace_back(value);
  }

  // remove the empty bitmap
  for (const auto &value : needToRemove) this->m_bitmaps.erase(value);

  // Clear the bit in the not null bitmap
  this->m_notNullBitmap.clearBit(pos);
}

Bitmap BitmapIndex::getBitmap(Token comparator, const std::string &value) {
  Bitmap resultBitmap { this->m_bitmapLength };

  switch (comparator) {
  case Token::IS_NULL: return ~this->m_notNullBitmap;
  case Token::IS_NOT_NULL: return this->m_notNullBitmap;
  case Token::EQUAL:
    // If the value exist, returns directly
    if (exist(value)) return this->m_bitmaps.at(value);
    // If the value does not exist, returns empty bitmap
    break;
  case Token::NOT_EQUAL:
    // If the value exist, returns the not of it AND not null bitmap
    if (exist(value)) return ~this->m_bitmaps.at(value) & this->m_notNullBitmap;
    // If the value does not exist, returns the OR of all the bitmaps
    for (const auto &[value, bitmap] : this->m_bitmaps) resultBitmap |= bitmap;
    break;
  case Token::GREATER_THAN:
    for (auto iter { this->m_bitmaps.upper_bound(value) };
         iter != end(this->m_bitmaps); ++iter) resultBitmap |= iter->second;
    break;
  case Token::GREATER_THAN_OR_EQUAL_TO:
    for (auto iter { this->m_bitmaps.lower_bound(value) };
         iter != end(this->m_bitmaps); ++iter) resultBitmap |= iter->second;
    break;
  case Token::LESS_THAN:
    for (auto iter { begin(this->m_bitmaps) };
         iter != this->m_bitmaps.lower_bound(value); ++iter) resultBitmap |= iter->second;
    break;
  case Token::LESS_THAN_OR_EQUAL_TO:
    for (auto iter { begin(this->m_bitmaps) };
         iter != this->m_bitmaps.upper_bound(value); ++iter) resultBitmap |= iter->second;
    break;
  default: break;
  }
  return resultBitmap;
}

const std::map<ValueType, Bitmap> &BitmapIndex::getAllBitmaps() const { return this->m_bitmaps; }

bool BitmapIndex::exist(const ValueType &value) { return this->m_bitmaps.count(value); }
