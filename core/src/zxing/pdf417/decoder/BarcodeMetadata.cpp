/*
 * BarcodeMetadata.cpp
 *
 *  Created on: Jun 13, 2016
 *      Author: binhnk
 */

#include <zxing/pdf417/decoder/BarcodeMetadata.h>

namespace zxing {
namespace pdf417 {

BarcodeMetadata::~BarcodeMetadata() {
    // TODO Auto-generated destructor stub
}

BarcodeMetadata::BarcodeMetadata(int columnCount, int rowCountUpperPart,
        int rowCountLowerPart, int errorCorrectionLevel) {
    this->columnCount = columnCount;
    this->errorCorrectionLevel = errorCorrectionLevel;
    this->rowCountUpperPart = rowCountUpperPart;
    this->rowCountLowerPart = rowCountLowerPart;
    this->rowCount = rowCountUpperPart + rowCountLowerPart;
}

} /* namespace pdf417 */
} /* namespace zxing */
