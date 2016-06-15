// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
/*
 * Copyright 2010 ZXing authors All rights reserved.
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

#include <zxing/pdf417/PDF417Reader.h>
#include <zxing/pdf417/detector/Detector.h>
#include <zxing/pdf417/decoder/PDF417ScanningDecoder.h>

#include <climits>

using zxing::Ref;
using zxing::Result;
using zxing::BitMatrix;
using zxing::pdf417::PDF417Reader;
using zxing::pdf417::detector::Detector;
using zxing::pdf417::detector::PDF417DetectorResult;

// VC++
using zxing::ArrayRef;
using zxing::BinaryBitmap;
using zxing::DecodeHints;

Ref<Result> PDF417Reader::decode(Ref<BinaryBitmap> image, DecodeHints hints) {
#if 0
  Ref<DecoderResult> decoderResult;
  /* 2012-05-30 hfn C++ DecodeHintType does not yet know a type "PURE_BARCODE", */
  /* therefore skip this for now, todo: may be add this type later */
  /*
    if (!hints.isEmpty() && hints.containsKey(DecodeHintType.PURE_BARCODE)) {
    BitMatrix bits = extractPureBits(image.getBlackMatrix());
    decoderResult = decoder.decode(bits);
    points = NO_POINTS;
    } else {
  */
  Detector detector(image);
  Ref<DetectorResult> detectorResult = detector.detect(hints); /* 2012-09-17 hints ("try_harder") */
  ArrayRef< Ref<ResultPoint> > points(detectorResult->getPoints());
  
  if (!hints.isEmpty()) {
    Ref<ResultPointCallback> rpcb = hints.getResultPointCallback();
    /* .get(DecodeHintType.NEED_RESULT_POINT_CALLBACK); */
    if (rpcb != NULL) {
      for (int i = 0; i < points->size(); i++) {
        rpcb->foundPossibleResultPoint(*points[i]);
      }
    }
  }
  decoderResult = decoder.decode(detectorResult->getBits(),hints);
  /*
    }
  */
  Ref<Result> r(new Result(decoderResult->getText(), decoderResult->getRawBytes(), points,
                           BarcodeFormat::PDF_417));
  return r;
#else
  ArrayRef<Ref<Result>> results = decodeMulti(image, hints, false);
  if (results == NULL) {
      throw NotFoundException();
  }
  return results[0];
#endif
}

void PDF417Reader::reset() {
  // do nothing
}

Ref<BitMatrix> PDF417Reader::extractPureBits(Ref<BitMatrix> image) {
  ArrayRef<int> leftTopBlack = image->getTopLeftOnBit();
  ArrayRef<int> rightBottomBlack = image->getBottomRightOnBit();
  /* see BitMatrix::getTopLeftOnBit etc.:
     if (leftTopBlack == null || rightBottomBlack == null) {
     throw NotFoundException.getNotFoundInstance();
     } */
  
  int nModuleSize = moduleSize(leftTopBlack, image);
  
  int top = leftTopBlack[1];
  int bottom = rightBottomBlack[1];
  int left = findPatternStart(leftTopBlack[0], top, image);
  int right = findPatternEnd(leftTopBlack[0], top, image);
  
  int matrixWidth = (right - left + 1) / nModuleSize;
  int matrixHeight = (bottom - top + 1) / nModuleSize;
  if (matrixWidth <= 0 || matrixHeight <= 0) {
    throw NotFoundException("PDF417Reader::extractPureBits: no matrix found!");
  }
  
  // Push in the "border" by half the module width so that we start
  // sampling in the middle of the module. Just in case the image is a
  // little off, this will help recover.
  int nudge = nModuleSize >> 1;
  top += nudge;
  left += nudge;
  
  // Now just read off the bits
  Ref<BitMatrix> bits(new BitMatrix(matrixWidth, matrixHeight));
  for (int y = 0; y < matrixHeight; y++) {
    int iOffset = top + y * nModuleSize;
    for (int x = 0; x < matrixWidth; x++) {
      if (image->get(left + x * nModuleSize, iOffset)) {
        bits->set(x, y);
      }
    }
  }
  return bits;
}

int PDF417Reader::moduleSize(ArrayRef<int> leftTopBlack, Ref<BitMatrix> image) {
  int x = leftTopBlack[0];
  int y = leftTopBlack[1];
  int width = image->getWidth();
  while (x < width && image->get(x, y)) {
    x++;
  }
  if (x == width) {
    throw NotFoundException("PDF417Reader::moduleSize: not found!");
  }
  
  int moduleSize = (int)(((unsigned)(x - leftTopBlack[0])) >> 3); // We've crossed left first bar, which is 8x
  if (moduleSize == 0) {
    throw NotFoundException("PDF417Reader::moduleSize: is zero!");
  }
  
  return moduleSize;
}

int PDF417Reader::findPatternStart(int x, int y, Ref<BitMatrix> image) {
  int width = image->getWidth();
  int start = x;
  // start should be on black
  int transitions = 0;
  bool black = true;
  while (start < width - 1 && transitions < 8) {
    start++;
    bool newBlack = image->get(start, y);
    if (black != newBlack) {
      transitions++;
    }
    black = newBlack;
  }
  if (start == width - 1) {
    throw NotFoundException("PDF417Reader::findPatternStart: no pattern start found!");
  }
  return start;
}

int PDF417Reader::findPatternEnd(int x, int y, Ref<BitMatrix> image) {
  int width = image->getWidth();
  int end = width - 1;
  // end should be on black
  while (end > x && !image->get(end, y)) {
    end--;
  }
  int transitions = 0;
  bool black = true;
  while (end > x && transitions < 9) {
    end--;
    bool newBlack = image->get(end, y);
    if (black != newBlack) {
      transitions++;
    }
    black = newBlack;
  }
  if (end == x) {
    throw NotFoundException("PDF417Reader::findPatternEnd: no pattern end found!");
  }
  return end;
}

int PDF417Reader::getMaxWidth(Ref<ResultPoint> p1, Ref<ResultPoint> p2) {
    if (p1 == NULL || p2 == NULL) {
      return 0;
    }
    return (int) (abs(p1->getX() - p2->getX()));
}

int PDF417Reader::getMinWidth(Ref<ResultPoint> p1, Ref<ResultPoint> p2) {
    if (p1 == NULL || p2 == NULL) {
      return INT_MAX;
    }
    return (int) (abs(p1->getX() - p2->getX()));
}

int PDF417Reader::getMaxCodewordWidth(ArrayRef<Ref<ResultPoint>> p) {
    return std::max(
            std::max(getMaxWidth(p[0], p[4]),
                    getMaxWidth(p[6], p[2])
                            * BitMatrixParser::MODULES_IN_CODEWORD
                            / BitMatrixParser::MODULES_IN_STOP_PATTERN),
            std::max(getMaxWidth(p[1], p[5]),
                    getMaxWidth(p[7], p[3])
                            * BitMatrixParser::MODULES_IN_CODEWORD
                            / BitMatrixParser::MODULES_IN_STOP_PATTERN));
}

int PDF417Reader::getMinCodewordWidth(ArrayRef<Ref<ResultPoint>> p) {
    return std::min(
            std::min(getMinWidth(p[0], p[4]),
                    getMinWidth(p[6], p[2])
                            * BitMatrixParser::MODULES_IN_CODEWORD
                            / BitMatrixParser::MODULES_IN_STOP_PATTERN),
            std::min(getMinWidth(p[1], p[5]),
                    getMinWidth(p[7], p[3])
                            * BitMatrixParser::MODULES_IN_CODEWORD
                            / BitMatrixParser::MODULES_IN_STOP_PATTERN));
}

ArrayRef<Ref<Result>> PDF417Reader::decodeMulti(
        Ref<BinaryBitmap> image, DecodeHints hints, bool multiple) {
    ArrayRef<Ref<Result>> results = new Array<Ref<Result>>();
    Detector detector(image);
    Ref<PDF417DetectorResult> detectorResult = detector.detect(image, hints,
            multiple);
    for (ArrayRef<Ref<ResultPoint>> points : detectorResult->getPoints()->values()) {
        try {
        Ref<DecoderResult> decoderResult = PDF417ScanningDecoder::decode(
                detectorResult->getBits(), points[4], points[5], points[6],
                points[7], getMinCodewordWidth(points),
                getMaxCodewordWidth(points));
        Ref<Result> result(new Result(decoderResult->getText(),
                decoderResult->getRawBytes(), points, BarcodeFormat::PDF_417));
        results->values().push_back(result);
        } catch (Exception ignored) {
            std::cout << "Error: " << ignored.what() << std::endl;
        }
    }
    return results;
}
