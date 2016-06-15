/*
 * DetectionResultColumn.cpp
 *
 *  Created on: Jun 13, 2016
 *      Author: binhnk
 */

#include <zxing/pdf417/decoder/DetectionResultColumn.h>

namespace zxing {
namespace pdf417 {

DetectionResultColumn::~DetectionResultColumn() {
    // TODO Auto-generated destructor stub
}

DetectionResultColumn::DetectionResultColumn(
        Ref<BoundingBox> boundingBox) {
    this->boundingBox = new BoundingBox(boundingBox);
    codewords = ArrayRef<Ref<Codeword> >(
            boundingBox->getMaxY() - boundingBox->getMinY() + 1);
}

Ref<Codeword> DetectionResultColumn::getCodewordNearby(int imageRow) {
    Ref<Codeword> codeword = getCodeword(imageRow);
    if (codeword != NULL) {
        return codeword;
    }
    for (int i = 1; i < MAX_NEARBY_DISTANCE; i++) {
        int nearImageRow = imageRowToCodewordIndex(imageRow) - i;
        if (nearImageRow >= 0) {
            codeword = codewords[nearImageRow];
            if (codeword != NULL) {
                return codeword;
            }
        }
        nearImageRow = imageRowToCodewordIndex(imageRow) + i;
        if (nearImageRow < codewords->size()) {
            codeword = codewords[nearImageRow];
            if (codeword != NULL) {
                return codeword;
            }
        }
    }
    return Ref<Codeword>(NULL);
}

int DetectionResultColumn::imageRowToCodewordIndex(int imageRow) {
    return imageRow - boundingBox->getMinY();
}

void DetectionResultColumn::setCodeword(int imageRow,
        Ref<Codeword> codeword) {
    codewords[imageRowToCodewordIndex(imageRow)] = codeword;
}

Ref<Codeword> DetectionResultColumn::getCodeword(int imageRow) {
    return codewords[imageRowToCodewordIndex(imageRow)];
}

Ref<BoundingBox> DetectionResultColumn::getBoundingBox() {
    return boundingBox;
}

ArrayRef<Ref<Codeword> > DetectionResultColumn::getCodewords() {
    return codewords;
}

Ref<String> DetectionResultColumn::toString() {
    Ref<String> result(new String(1024));
    int row = 0;
    for (Ref<Codeword> codeword : codewords->values()) {
        if (codeword == NULL) {
            // std::sprintf("%3d:    |   %n", row++);
            result->append(std::to_string(row) + ":     |    \n");
            row++;
            continue;
        }
        result->append(
                std::to_string(row) + ": "
                        + std::to_string(codeword->getRowNumber()) + "|"
                        + std::to_string(codeword->getValue()) + "\n");
        // formatter.format("%3d: %3d|%3d%n", row++, codeword.getRowNumber(), codeword.getValue());
        row++;
    }
    return result;
}

} /* namespace pdf417 */
} /* namespace zxing */
