// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
#ifndef __BIT_MATRIX_H__
#define __BIT_MATRIX_H__

/*
 *  BitMatrix.h
 *  zxing
 *
 *  Copyright 2010 ZXing authors All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <zxing/common/Counted.h>
#include <zxing/common/BitArray.h>
#include <zxing/common/Array.h>
#include <limits>

namespace zxing {

class BitMatrix : public Counted {
public:
  static const int bitsPerWord = std::numeric_limits<unsigned int>::digits;

private:
  int width;
  int height;
  int rowSize;
  ArrayRef<int> bits;

#define ZX_LOG_DIGITS(digits) \
    ((digits == 8) ? 3 : \
     ((digits == 16) ? 4 : \
      ((digits == 32) ? 5 : \
       ((digits == 64) ? 6 : \
        ((digits == 128) ? 7 : \
         (-1))))))

  static const int logBits = ZX_LOG_DIGITS(bitsPerWord);
  static const int bitsMask = (1 << logBits) - 1;

public:
  BitMatrix(int dimension);
  BitMatrix(int width, int height);

  ~BitMatrix();

  bool get(int x, int y) const {
    int offset = y * rowSize + (x >> logBits);
    return ((((unsigned)bits[offset]) >> (x & bitsMask)) & 1) != 0;
  }

  void set(int x, int y) {
    int offset = y * rowSize + (x >> logBits);
    bits[offset] |= 1 << (x & bitsMask);
  }

  void flip(int x, int y);
  void clear();
  void setRegion(int left, int top, int width, int height);
  Ref<BitArray> getRow(int y, Ref<BitArray> row);

  /**
    * @param y row to set
    * @param row {@link BitArray} to copy from
    */
   void setRow(int y, Ref<BitArray> row) {
     int offset = y * rowSize;
     std::vector<int>& vrow = row->getBitArray();
     std::copy(vrow.begin(), vrow.end(), bits->values().begin() + offset);
   }

  /**
    * Modifies this {@code BitMatrix} to represent the same but rotated 180 degrees
    */
  void rotate180() {
    int width = getWidth();
    int height = getHeight();
    Ref<BitArray> topRow(new BitArray(width));
    Ref<BitArray> bottomRow(new BitArray(width));
    for (int i = 0; i < (height + 1) / 2; i++) {
        topRow = getRow(i, topRow);
        bottomRow = getRow(height - 1 - i, bottomRow);
        topRow->reverse();
        bottomRow->reverse();
        setRow(i, bottomRow);
        setRow(height - 1 - i, topRow);
    }
  }

  int getWidth() const;
  int getHeight() const;

  ArrayRef<int> getTopLeftOnBit() const;
  ArrayRef<int> getBottomRightOnBit() const;

  friend std::ostream& operator<<(std::ostream &out, const BitMatrix &bm);
  const char *description();

private:
  inline void init(int, int);

  BitMatrix(const BitMatrix&);
  BitMatrix& operator =(const BitMatrix&);
};

}

#endif // __BIT_MATRIX_H__
