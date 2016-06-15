/*
 * PDF417DetectorResult.h
 *
 *  Created on: Jun 13, 2016
 *      Author: binhnk
 */

#ifndef CORE_SRC_ZXING_PDF417_DETECTOR_PDF417DETECTORRESULT_H_
#define CORE_SRC_ZXING_PDF417_DETECTOR_PDF417DETECTORRESULT_H_

#include <zxing/common/BitMatrix.h>
#include <zxing/ResultPoint.h>

namespace zxing {
namespace pdf417 {
namespace detector {

class PDF417DetectorResult : public Counted {
    Ref<BitMatrix> bits;
    ArrayRef<ArrayRef<Ref<ResultPoint>>> points;

public:
    PDF417DetectorResult(Ref<BitMatrix> bits, ArrayRef<ArrayRef<Ref<ResultPoint>>> points) {
      this->bits = bits;
      this->points = points;
    }

    Ref<BitMatrix> getBits() {
      return bits;
    }

    ArrayRef<ArrayRef<Ref<ResultPoint>>> getPoints() {
      return points;
    }

    virtual ~PDF417DetectorResult();
};

} /* namespace detector */
} /* namespace pdf417 */
} /* namespace zxing */

#endif /* CORE_SRC_ZXING_PDF417_DETECTOR_PDF417DETECTORRESULT_H_ */
