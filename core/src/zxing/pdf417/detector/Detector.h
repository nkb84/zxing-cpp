#ifndef __DETECTOR_H__
#define __DETECTOR_H__

/*
 *  Detector.h
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

#include <zxing/common/Point.h>
#include <zxing/common/DetectorResult.h>
#include <zxing/NotFoundException.h>
#include <zxing/BinaryBitmap.h>
#include <zxing/DecodeHints.h>

#include <zxing/pdf417/detector/PDF417DetectorResult.h>

using zxing::pdf417::detector::PDF417DetectorResult;

namespace zxing {
namespace pdf417 {
namespace detector {

class Detector {
private:
  static const int INTEGER_MATH_SHIFT = 8;
  static const int PATTERN_MATCH_RESULT_SCALE_FACTOR = 1 << INTEGER_MATH_SHIFT;
  static const int MAX_AVG_VARIANCE;
  static const int MAX_INDIVIDUAL_VARIANCE;

  static const int START_PATTERN[];
  static const int START_PATTERN_LENGTH;
  static const int START_PATTERN_REVERSE[];
  static const int START_PATTERN_REVERSE_LENGTH;
  static const int STOP_PATTERN[];
  static const int STOP_PATTERN_LENGTH;
  static const int STOP_PATTERN_REVERSE[];
  static const int STOP_PATTERN_REVERSE_LENGTH;

  static const int ROW_STEP;
  static const int BARCODE_MIN_HEIGHT;
  static const int MAX_PIXEL_DRIFT;
  static const int MAX_PATTERN_DRIFT;
  static const int SKIPPED_ROW_COUNT_MAX;
  static const int INDEXES_START_PATTERN[];
  static const int INDEXES_STOP_PATTERN[];

  Ref<BinaryBitmap> image_;
  
  static ArrayRef< Ref<ResultPoint> > findVertices(Ref<BitMatrix> matrix, int rowStep);
  static ArrayRef< Ref<ResultPoint> > findVertices180(Ref<BitMatrix> matrix, int rowStep);

  static ArrayRef<int> findGuardPattern(Ref<BitMatrix> matrix,
                                        int column,
                                        int row,
                                        int width,
                                        bool whiteFirst,
                                        const int pattern[],
                                        int patternSize,
                                        ArrayRef<int>& counters);
  static int patternMatchVariance(ArrayRef<int>& counters, const int pattern[],
                                  int maxIndividualVariance);

  static void correctVertices(Ref<BitMatrix> matrix,
                              ArrayRef< Ref<ResultPoint> >& vertices,
                              bool upsideDown);
  static void findWideBarTopBottom(Ref<BitMatrix> matrix,
                                   ArrayRef< Ref<ResultPoint> >& vertices,
                                   int offsetVertice,
                                   int startWideBar,
                                   int lenWideBar,
                                   int lenPattern,
                                   int nIncrement);
  static void findCrossingPoint(ArrayRef< Ref<ResultPoint> >& vertices,
                                int idxResult,
                                int idxLineA1,int idxLineA2,
                                int idxLineB1,int idxLineB2,
                                Ref<BitMatrix>& matrix);
  static Point intersection(Line a, Line b);
  static float computeModuleWidth(ArrayRef< Ref<ResultPoint> >& vertices);
  static int computeDimension(Ref<ResultPoint> const& topLeft,
                              Ref<ResultPoint> const& topRight,
                              Ref<ResultPoint> const& bottomLeft,
                              Ref<ResultPoint> const& bottomRight,
                              float moduleWidth);
  int computeYDimension(Ref<ResultPoint> const& topLeft,
                        Ref<ResultPoint> const& topRight,
                        Ref<ResultPoint> const& bottomLeft,
                        Ref<ResultPoint> const& bottomRight,
                        float moduleWidth);

  Ref<BitMatrix> sampleLines(ArrayRef< Ref<ResultPoint> > const& vertices, int dimensionY, int dimension);

public:
  Detector(Ref<BinaryBitmap> image);
  Ref<BinaryBitmap> getImage();
  Ref<DetectorResult> detect();
  Ref<DetectorResult> detect(DecodeHints const& hints);

    Ref<PDF417DetectorResult> detect(Ref<BinaryBitmap> image,
            const DecodeHints& hints, bool multiple);

  /**
     * Locate the vertices and the codewords area of a black blob using the Start
     * and Stop patterns as locators.
     *
     * @param matrix the scanned barcode image.
     * @return an array containing the vertices:
     *           vertices[0] x, y top left barcode
     *           vertices[1] x, y bottom left barcode
     *           vertices[2] x, y top right barcode
     *           vertices[3] x, y bottom right barcode
     *           vertices[4] x, y top left codeword area
     *           vertices[5] x, y bottom left codeword area
     *           vertices[6] x, y top right codeword area
     *           vertices[7] x, y bottom right codeword area
     */
    static ArrayRef<Ref<ResultPoint> > findVertices(Ref<BitMatrix> matrix,
            int startRow, int startColumn);

    static void copyToResult(ArrayRef<Ref<ResultPoint> > result,
            ArrayRef<Ref<ResultPoint> > tmpResult,
            const int destinationIndexes[4]);


    static ArrayRef<Ref<ResultPoint> > findRowsWithPattern(
            Ref<BitMatrix> matrix, int height, int width, int startRow,
            int startColumn, const int pattern[], int patternSize);


    static ArrayRef<ArrayRef<Ref<ResultPoint> > > detect(bool multiple,
            Ref<BitMatrix> bitMatrix);
};

}
}
}

#endif // __DETECTOR_H__
