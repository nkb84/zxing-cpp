/*
 * BoundingBox.h
 *
 *  Created on: Jun 13, 2016
 *      Author: binhnk
 */

#ifndef CORE_SRC_ZXING_PDF417_DECODER_BOUNDINGBOX_H_
#define CORE_SRC_ZXING_PDF417_DECODER_BOUNDINGBOX_H_

#include <zxing/common/BitMatrix.h>
#include <zxing/ResultPoint.h>


namespace zxing {
namespace pdf417 {

class BoundingBox : public Counted {
    Ref<BitMatrix> image;
    Ref<ResultPoint> topLeft;
    Ref<ResultPoint> bottomLeft;
    Ref<ResultPoint> topRight;
    Ref<ResultPoint> bottomRight;
    int minX;
    int maxX;
    int minY;
    int maxY;

public:
    BoundingBox(Ref<BitMatrix> image, Ref<ResultPoint> topLeft,
            Ref<ResultPoint> bottomLeft, Ref<ResultPoint> topRight,
            Ref<ResultPoint> bottomRight);

    ~BoundingBox();

    BoundingBox(Ref<BoundingBox> boundingBox);

    void init(Ref<BitMatrix> image, Ref<ResultPoint> topLeft,
            Ref<ResultPoint> bottomLeft, Ref<ResultPoint> topRight,
            Ref<ResultPoint> bottomRight);

    static Ref<BoundingBox> merge(Ref<BoundingBox> leftBox,
            Ref<BoundingBox> rightBox);

    Ref<BoundingBox> addMissingRows(int missingStartRows, int missingEndRows,
            boolean isLeft);

    void calculateMinMaxValues();

    /*
     void setTopRight(ResultPoint topRight) {
     this.topRight = topRight;
     calculateMinMaxValues();
     }
     void setBottomRight(ResultPoint bottomRight) {
     this.bottomRight = bottomRight;
     calculateMinMaxValues();
     }
     */

    int getMinX() {
        return minX;
    }

    int getMaxX() {
        return maxX;
    }

    int getMinY() {
        return minY;
    }

    int getMaxY() {
        return maxY;
    }

    Ref<ResultPoint> getTopLeft() {
        return topLeft;
    }

    Ref<ResultPoint> getTopRight() {
        return topRight;
    }

    Ref<ResultPoint> getBottomLeft() {
        return bottomLeft;
    }

    Ref<ResultPoint> getBottomRight() {
        return bottomRight;
    }
};

} /* namespace pdf417 */
} /* namespace zxing */

#endif /* CORE_SRC_ZXING_PDF417_DECODER_BOUNDINGBOX_H_ */
