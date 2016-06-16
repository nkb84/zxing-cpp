// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
/*
 *  DataMatrixReader.cpp
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

#include <zxing/NotFoundException.h>
#include <zxing/datamatrix/DataMatrixReader.h>
#include <zxing/datamatrix/detector/Detector.h>
#include <iostream>

namespace zxing {
namespace datamatrix {

using namespace std;

const ArrayRef< Ref<ResultPoint> > DataMatrixReader::NO_POINTS;
DataMatrixReader::DataMatrixReader() :
    decoder_() {
}

Ref<Result> DataMatrixReader::decode(Ref<BinaryBitmap> image, DecodeHints hints) {
  Ref<DecoderResult> decoderResult;
  ArrayRef< Ref<ResultPoint> > points;
  if (hints.containsKey(DecodeHints::PURE_BARCODE)) {
    Ref<BitMatrix> bits = extractPureBits(image->getBlackMatrix());
    decoderResult = decoder_.decode(bits);
    points = NO_POINTS;
  } else {
    Detector detector(image->getBlackMatrix());
    Ref<DetectorResult> detectorResult(detector.detect());
    decoderResult = decoder_.decode(detectorResult->getBits());
    points = detectorResult->getPoints();
  }

  Ref<Result> result(
    new Result(decoderResult->getText(), decoderResult->getRawBytes(), points, BarcodeFormat::DATA_MATRIX));

  return result;
}

DataMatrixReader::~DataMatrixReader() {
}

Ref<BitMatrix> DataMatrixReader::extractPureBits(Ref<BitMatrix> image) {
    ArrayRef<int> leftTopBlack = image->getTopLeftOnBit();
    ArrayRef<int> rightBottomBlack = image->getBottomRightOnBit();
    if (leftTopBlack == NULL || rightBottomBlack == NULL) {
        throw NotFoundException();
    }
    int moduleSize_ = moduleSize(leftTopBlack, image);
    int top = leftTopBlack[1];
    int bottom = rightBottomBlack[1];
    int left = leftTopBlack[0];
    int right = rightBottomBlack[0];
    int matrixWidth = (right - left + 1) / moduleSize_;
    int matrixHeight = (bottom - top + 1) / moduleSize_;
    if (matrixWidth <= 0 || matrixHeight <= 0) {
        throw NotFoundException();
    }
    // Push in the "border" by half the module width so that we start
    // sampling in the middle of the module. Just in case the image is a
    // little off, this will help recover.
    int nudge = moduleSize_ / 2;
    top += nudge;
    left += nudge;
    // Now just read off the bits
    Ref<BitMatrix> bits(new BitMatrix(matrixWidth, matrixHeight));
    for (int y = 0; y < matrixHeight; y++) {
        int iOffset = top + y * moduleSize_;
        for (int x = 0; x < matrixWidth; x++) {
            if (image->get(left + x * moduleSize_, iOffset)) {
                bits->set(x, y);
            }
        }
    }
    return bits;
}

int DataMatrixReader::moduleSize(ArrayRef<int> leftTopBlack,
        Ref<BitMatrix> image) {
    int width = image->getWidth();
    int x = leftTopBlack[0];
    int y = leftTopBlack[1];
    while (x < width && image->get(x, y)) {
        x++;
    }
    if (x == width) {
        throw NotFoundException();
    }
    int moduleSize = x - leftTopBlack[0];
    if (moduleSize == 0) {
        throw NotFoundException();
    }
    return moduleSize;
}

}
}
