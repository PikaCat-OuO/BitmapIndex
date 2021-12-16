#pragma once
#include "globals.h"
#include "bitmap.h"

class BitmapIndex
{
public:
  BitmapIndex(uint64_t &bitmapLength);

  /** resize all bitmaps */
  void resize();

  /** Set a bitmap bit to 1 on pos */
  void setBitmapBit(const ValueType &value, uint64_t pos);

  /** Set all the bitmap bit to 0 on pos */
  void clearAllBitmapBits(uint64_t pos);

  Bitmap getBitmap(Token comparator, const ValueType &value);

  const std::map<ValueType, Bitmap> &getAllBitmaps() const;

protected:
  bool exist(const ValueType &value);
  /** Set all the bit in a bitmap to 0 on pos */
  void clearBitmapBit(const ValueType &value, uint64_t pos);

private:
  /** Bitmap length */
  uint64_t &m_bitmapLength;
  /** Value to bitmap */
  std::map<ValueType, Bitmap> m_bitmaps;
  /** Not null value bitmap */
  Bitmap m_notNullBitmap;
};
