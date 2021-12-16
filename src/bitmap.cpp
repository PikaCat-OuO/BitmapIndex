#include "bitmap.h"

BitmapIterator::BitmapIterator(const Bitmap &bitmap, uint64_t pos)
    : m_bitmap { bitmap }, m_currentPos { pos } {
  // Find the first 1
  while (this->m_currentPos < this->m_bitmap.getLength() and
         0 == this->m_bitmap[this->m_currentPos]) ++this->m_currentPos;
}

BitmapIterator &BitmapIterator::operator++() {
  while (++this->m_currentPos < this->m_bitmap.getLength() and
         0 == this->m_bitmap[this->m_currentPos]);
  return *this;
}

BitmapIterator BitmapIterator::operator++(int) {
  BitmapIterator ret { *this };
  while (++this->m_currentPos < this->m_bitmap.getLength() and
         0 == this->m_bitmap[this->m_currentPos]);
  return ret;
}

bool BitmapIterator::operator==(const BitmapIterator &rhs) const {
  return this->m_currentPos == rhs.m_currentPos;
}

bool BitmapIterator::operator!=(const BitmapIterator &rhs) const { return !(*this == rhs); }

BitmapIterator::value_type BitmapIterator::operator*() { return this->m_currentPos; }

Bitmap::Bitmap(uint64_t &bitmapLength) : m_bitmapLength { bitmapLength } { resize(); }

void Bitmap::resize() {
  this->m_bitmap.resize((int64_t(this->m_bitmapLength) - 1) / 64 + 1);
}

void Bitmap::setBit(uint64_t pos) {
  if (pos < this->m_bitmapLength) {
    if (not (*this)[pos]) ++this->m_bitCount;
    this->m_bitmap[pos / 64] |= Bitmap::ms_bitmapMask[pos % 64];
  }
  else throw outOfRange_helper(pos);
}

void Bitmap::clearBit(uint64_t pos) {
  if (pos < this->m_bitmapLength) {
    if ((*this)[pos]) --this->m_bitCount;
    this->m_bitmap[pos / 64] &= Bitmap::ms_bitmapNotMask[pos % 64];
  }
  else throw outOfRange_helper(pos);
}

uint64_t Bitmap::countBits() const { return this->m_bitCount; }

uint64_t Bitmap::popCount() const {
  uint64_t bitCounter { 0 };
  for (const auto &bits : this->m_bitmap) bitCounter += __builtin_popcountll(bits);
  return bitCounter;
}

std::string Bitmap::serialize() const {
  // Construct bitmap string
  std::string bitmapString;
  for (const auto &bitmapPart : this->m_bitmap) {
    std::string bin { std::bitset<64> { bitmapPart }.to_string() };
    std::reverse(std::begin(bin), std::end(bin));
    bitmapString += bin;
  }
  // Cut out the needed part
  bitmapString.substr(0, this->m_bitmapLength);
  // Add a terminate sign
  bitmapString += '1';

  // The convertion result
  std::string result;

  uint64_t zeroCount { 0 };
  for (const auto &bit : bitmapString) {
    // Count to see there are how many zero before a 1
    if ('0' == bit) ++zeroCount;
    else {
      // Convert the zero count into string format
      char compressBody[65] { 0 };
      ulltoa(zeroCount, compressBody, 2);

      // make compress header
      std::string compressHeader { compressBody };
      for (auto &bit : compressHeader) bit = '1';
      compressHeader.back() = '0';

      // Append to the result
      result += compressHeader + compressBody;

      // Reset the counter
      zeroCount = 0;
    }
  }

  return result;
}

void Bitmap::deserialize(std::string &bitmapString) {
  // Decompress result
  std::string result;

  uint64_t oneCount { 0 }, index { 0 };
  while (index < bitmapString.length()) {
    // Find the first 0
    while (index < bitmapString.length() and '1' == bitmapString[index++]) ++oneCount;

    if (index < bitmapString.length()) {
      // Get the zero count
      uint64_t zeroCount { std::stoull(bitmapString.substr(index, ++oneCount), nullptr, 2) };
      // Append to the result
      result += std::string(zeroCount, '0') + '1';

      // Move the pointer forward
      index += oneCount;

      // Reset counter
      oneCount = 0;
    }
  }

  // Get rid of the last 1
  if (0 not_eq index) result.pop_back();

  bitmapString = result;
}

uint64_t Bitmap::getLength() const { return this->m_bitmapLength; }

bool Bitmap::operator[](uint64_t pos) const {
  if (pos < this->m_bitmapLength) {
    return this->m_bitmap[pos / 64] & Bitmap::ms_bitmapMask[pos % 64];
  }
  else throw outOfRange_helper(pos);
}

Bitmap &Bitmap::operator&=(const Bitmap &rhs) {
  for (uint64_t index { 0 }; index < rhs.m_bitmap.size(); ++index) {
    this->m_bitmap[index] &= rhs.m_bitmap[index];
  }
  return *this;
}

Bitmap &Bitmap::operator|=(const Bitmap &rhs) {
  for (uint64_t index { 0 }; index < rhs.m_bitmap.size(); ++index) {
    this->m_bitmap[index] |= rhs.m_bitmap[index];
  }
  return *this;
}

Bitmap Bitmap::operator~() {
  Bitmap result { *this };
  for (auto &bits : result.m_bitmap) bits = ~bits;
  return result;
}

BitmapIterator Bitmap::begin() const { return { *this, 0 }; }

BitmapIterator Bitmap::end() const { return { *this, this->m_bitmapLength }; }

void Bitmap::initBitmap() {
  for (uint64_t i { 0 }; i < 64; ++i) {
    Bitmap::ms_bitmapMask[i] = 1ULL << i;
    Bitmap::ms_bitmapNotMask[i] = ~Bitmap::ms_bitmapMask[i];
  }
}

std::out_of_range Bitmap::outOfRange_helper(uint64_t pos) const {
  std::stringstream message;
  message << "Bitmap index out of range: bitmap length is " << this->m_bitmapLength
          << ", but requested index is " << pos << ".";
  return std::out_of_range { message.str() };
}

Bitmap operator&(const Bitmap &lhs, const Bitmap &rhs) {
  Bitmap result { lhs };
  for (uint64_t index { 0 }; index < result.m_bitmap.size(); ++index) {
    result.m_bitmap[index] &= rhs.m_bitmap[index];
  }
  return result;
}

Bitmap operator|(const Bitmap &lhs, const Bitmap &rhs) {
  Bitmap result { lhs };
  for (uint64_t index { 0 }; index < result.m_bitmap.size(); ++index) {
    result.m_bitmap[index] |= rhs.m_bitmap[index];
  }
  return result;
}
