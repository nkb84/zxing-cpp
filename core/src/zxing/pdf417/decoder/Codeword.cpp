/*
 * Codeword.cpp
 *
 *  Created on: Jun 13, 2016
 *      Author: binhnk
 */

#include <zxing/pdf417/decoder/Codeword.h>

namespace zxing {
namespace pdf417 {

Codeword::Codeword(int startX, int endX, int bucket, int value) {
    this->startX = startX;
    this->endX = endX;
    this->bucket = bucket;
    this->value = value;
}

void Codeword::setRowNumber(int rowNumber) {
    this->rowNumber = rowNumber;
}

bool Codeword::hasValidRowNumber() {
    return isValidRowNumber(rowNumber);
}

bool Codeword::isValidRowNumber(int rowNumber) {
    return rowNumber != BARCODE_ROW_UNKNOWN && bucket == (rowNumber % 3) * 3;
}

void Codeword::setRowNumberAsRowIndicatorColumn() {
    rowNumber = (value / 30) * 3 + bucket / 3;
}

Ref<String> Codeword::toString() {
    return Ref<String>(new String(rowNumber + "|" + value));
}

Codeword::~Codeword() {
    // TODO Auto-generated destructor stub
}

} /* namespace pdf417 */
} /* namespace zxing */
