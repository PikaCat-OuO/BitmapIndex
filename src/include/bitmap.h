#pragma once
#include "globals.h"

class Bitmap;

class BitmapIterator {
public:
  // Iterator traits
  using difference_type = uint64_t;
  using value_type = uint64_t;
  using pointer = const value_type *;
  using reference = const value_type &;
  using iterator_category = std::forward_iterator_tag;

  BitmapIterator(const Bitmap &bitmap, uint64_t pos);
  BitmapIterator &operator++();
  BitmapIterator operator++(int);
  bool operator==(const BitmapIterator &other) const;
  bool operator!=(const BitmapIterator &other) const;
  value_type operator*();

private:
  const Bitmap &m_bitmap;
  uint64_t m_currentPos;
};

class Bitmap
{
public:
  Bitmap(uint64_t &bitmapLength);

  void resize();

  void setBit(uint64_t pos);
  void clearBit(uint64_t pos);

  /** only use this when the bitmap is not engaged in any bit operations */
  uint64_t countBits() const;

  uint64_t popCount() const;

  std::string serialize() const;

  static void deserialize(std::string &bitmapString);

  uint64_t getLength() const;

  bool operator[](uint64_t pos) const;

  Bitmap &operator&=(const Bitmap &rhs);
  Bitmap &operator|=(const Bitmap &rhs);
  Bitmap operator~();
  friend Bitmap operator&(const Bitmap &lhs, const Bitmap &rhs);
  friend Bitmap operator|(const Bitmap &lhs, const Bitmap &rhs);

  BitmapIterator begin() const;
  BitmapIterator end() const;

  static void initBitmap();

protected:
  std::out_of_range outOfRange_helper(uint64_t pos) const;

private:
  /** Bitmap storage */
  std::vector<uint64_t> m_bitmap;
  /** Bitmap length */
  uint64_t &m_bitmapLength;
  /** Bitmap seted bit count */
  uint64_t m_bitCount { 0 };

  /** Bitmap mask */
  inline static uint64_t ms_bitmapMask[64] { };
  /** Bitmap not mask */
  inline static uint64_t ms_bitmapNotMask[64] { };
};
