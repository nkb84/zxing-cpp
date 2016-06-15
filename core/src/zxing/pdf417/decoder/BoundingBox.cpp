/*
 * BoundingBox.cpp
 *
 *  Created on: Jun 13, 2016
 *      Author: binhnk
 */

#include <zxing/pdf417/decoder/BoundingBox.h>
#include <zxing/NotFoundException.h>

namespace zxing {
namespace pdf417 {

BoundingBox::BoundingBox(Ref<BoundingBox> boundingBox) {
    init(boundingBox->image, boundingBox->topLeft, boundingBox->bottomLeft,
            boundingBox->topRight, boundingBox->bottomRight);
}

BoundingBox::BoundingBox(Ref<BitMatrix> image, Ref<ResultPoint> topLeft,
        Ref<ResultPoint> bottomLeft, Ref<ResultPoint> topRight,
        Ref<ResultPoint> bottomRight) {
    if ((topLeft == NULL && topRight == NULL)
            || (bottomLeft == NULL && bottomRight == NULL)
            || (topLeft != NULL && bottomLeft == NULL)
            || (topRight != NULL && bottomRight == NULL)) {
        throw NotFoundException();
    }
    init(image, topLeft, bottomLeft, topRight, bottomRight);
}

BoundingBox::~BoundingBox() {
}

void BoundingBox::init(Ref<BitMatrix> image, Ref<ResultPoint> topLeft,
        Ref<ResultPoint> bottomLeft, Ref<ResultPoint> topRight,
        Ref<ResultPoint> bottomRight) {
    this->image = image;
    this->topLeft = topLeft;
    this->bottomLeft = bottomLeft;
    this->topRight = topRight;
    this->bottomRight = bottomRight;
    calculateMinMaxValues();
}

Ref<BoundingBox> BoundingBox::merge(Ref<BoundingBox> leftBox,
        Ref<BoundingBox> rightBox) {
    if (leftBox == NULL) {
        return rightBox;
    }
    if (rightBox == NULL) {
        return leftBox;
    }

    Ref<BoundingBox> result(new BoundingBox(leftBox->image, leftBox->topLeft,
            leftBox->bottomLeft, rightBox->topRight, rightBox->bottomRight));

    return result;
}

Ref<BoundingBox> BoundingBox::addMissingRows(int missingStartRows,
        int missingEndRows, boolean isLeft) {
    Ref<ResultPoint> newTopLeft = topLeft;
    Ref<ResultPoint> newBottomLeft = bottomLeft;
    Ref<ResultPoint> newTopRight = topRight;
    Ref<ResultPoint> newBottomRight = bottomRight;
    if (missingStartRows > 0) {
        Ref<ResultPoint> top = isLeft ? topLeft : topRight;
        int newMinY = (int) ((top->getY())) - missingStartRows;
        if (newMinY < 0) {
            newMinY = 0;
        }
        Ref<ResultPoint> newTop(
                new ResultPoint(top->getX(), (float) ((newMinY))));
        if (isLeft) {
            newTopLeft = newTop;
        } else {
            newTopRight = newTop;
        }
    }
    if (missingEndRows > 0) {
        Ref<ResultPoint> bottom = isLeft ? bottomLeft : bottomRight;
        int newMaxY = (int) ((bottom->getY())) + missingEndRows;
        if (newMaxY >= image->getHeight()) {
            newMaxY = image->getHeight() - 1;
        }
        Ref<ResultPoint> newBottom(
                new ResultPoint(bottom->getX(), (float) ((newMaxY))));
        if (isLeft) {
            newBottomLeft = newBottom;
        } else {
            newBottomRight = newBottom;
        }
    }
    calculateMinMaxValues();
    Ref<BoundingBox> result(new BoundingBox(
            image, newTopLeft, newBottomLeft, newTopRight, newBottomRight));
    return result;
}

void BoundingBox::calculateMinMaxValues() {
    if (topLeft == NULL) {
        topLeft = new ResultPoint(0.0, topRight->getY());
        bottomLeft = new ResultPoint(0.0, bottomRight->getY());
    } else if (topRight == NULL) {
        topRight = new ResultPoint((float)image->getWidth() - 1, topLeft->getY());
        bottomRight = new ResultPoint((float)image->getWidth() - 1,
                bottomLeft->getY());
    }

    minX = (int) ((std::min(topLeft->getX(), bottomLeft->getX())));
    maxX = (int) ((std::max(topRight->getX(), bottomRight->getX())));
    minY = (int) ((std::min(topLeft->getY(), topRight->getY())));
    maxY = (int) ((std::max(bottomLeft->getY(), bottomRight->getY())));
}

} /* namespace pdf417 */
} /* namespace zxing */
