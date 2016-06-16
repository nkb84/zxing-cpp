/*
 *  DataBlock.cpp
 *  zxing
 *
 *  Created by Luiz Silva on 09/02/2010.
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

#include <zxing/datamatrix/decoder/DataBlock.h>
#include <zxing/common/IllegalArgumentException.h>

namespace zxing {
namespace datamatrix {

using namespace std;

DataBlock::DataBlock(int numDataCodewords, ArrayRef<char> codewords) :
    numDataCodewords_(numDataCodewords), codewords_(codewords) {
}

int DataBlock::getNumDataCodewords() {
  return numDataCodewords_;
}

ArrayRef<char> DataBlock::getCodewords() {
  return codewords_;
}

std::vector<Ref<DataBlock> > DataBlock::getDataBlocks(ArrayRef<char> rawCodewords, Version *version) {
  // Figure out the number and size of data blocks used by this version and
  // error correction level
  ECBlocks* ecBlocks = version->getECBlocks();

  // First count the total number of data blocks
  int totalBlocks = 0;
  vector<ECB*> ecBlockArray = ecBlocks->getECBlocks();
  for (size_t i = 0; i < ecBlockArray.size(); i++) {
    totalBlocks += ecBlockArray[i]->getCount();
  }

  // Now establish DataBlocks of the appropriate size and number of data codewords
  std::vector<Ref<DataBlock> > result(totalBlocks);
  int numResultBlocks = 0;
  for (size_t j = 0; j < ecBlockArray.size(); j++) {
    ECB *ecBlock = ecBlockArray[j];
    for (int i = 0; i < ecBlock->getCount(); i++) {
      int numDataCodewords = ecBlock->getDataCodewords();
      int numBlockCodewords = ecBlocks->getECCodewords() + numDataCodewords;
      ArrayRef<char> buffer(numBlockCodewords);
      Ref<DataBlock> blockRef(new DataBlock(numDataCodewords, buffer));
      result[numResultBlocks++] = blockRef;
    }
  }

  // All blocks have the same amount of data, except that the last n
  // (where n may be 0) have 1 less byte. Figure out where these start.
  // TODO(bbrown): There is only one case where there is a difference for Data Matrix for size 144
  int longerBlocksTotalCodewords = result[0]->codewords_->size();
  //int shorterBlocksTotalCodewords = longerBlocksTotalCodewords - 1;

  int longerBlocksNumDataCodewords = longerBlocksTotalCodewords - ecBlocks->getECCodewords();
  int shorterBlocksNumDataCodewords = longerBlocksNumDataCodewords - 1;
  // The last elements of result may be 1 element shorter for 144 matrix
  // first fill out as many elements as all of them have minus 1
  int rawCodewordsOffset = 0;
  for (int i = 0; i < shorterBlocksNumDataCodewords; i++) {
   for (int j = 0; j < numResultBlocks; j++) {
     result[j]->codewords_[i] = rawCodewords[rawCodewordsOffset++];
   }
  }

  // Fill out the last data block in the longer ones
  boolean specialVersion = version->getVersionNumber() == 24;
  int numLongerBlocks = specialVersion ? 8 : numResultBlocks;
  for (int j = 0; j < numLongerBlocks; j++) {
    result[j]->codewords_[longerBlocksNumDataCodewords - 1] = rawCodewords[rawCodewordsOffset++];
  }

  // Now add in error correction blocks
  int max = result[0]->codewords_->size();
  for (int i = longerBlocksNumDataCodewords; i < max; i++) {
    for (int j = 0; j < numResultBlocks; j++) {
      int jOffset = specialVersion ? (j + 8) % numResultBlocks : j;
      int iOffset = specialVersion && jOffset > 7 ? i - 1 : i;
      result[jOffset]->codewords_[iOffset] = rawCodewords[rawCodewordsOffset++];
    }
  }

  if (rawCodewordsOffset != rawCodewords->size()) {
    throw IllegalArgumentException("rawCodewordsOffset != rawCodewords.length");
  }

  return result;
}

}
}
