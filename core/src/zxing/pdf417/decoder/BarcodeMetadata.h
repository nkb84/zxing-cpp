/*
 * BarcodeMetadata.h
 *
 *  Created on: Jun 13, 2016
 *      Author: binhnk
 */

#ifndef CORE_SRC_ZXING_PDF417_DECODER_BARCODEMETADATA_H_
#define CORE_SRC_ZXING_PDF417_DECODER_BARCODEMETADATA_H_

#include <zxing/common/Counted.h>

namespace zxing {
namespace pdf417 {

class BarcodeMetadata : public Counted {
    int columnCount;
    int errorCorrectionLevel;
    int rowCountUpperPart;
    int rowCountLowerPart;
    int rowCount;

public:
    BarcodeMetadata(int columnCount, int rowCountUpperPart,
            int rowCountLowerPart, int errorCorrectionLevel);

    int getColumnCount() {
        return columnCount;
    }

    int getErrorCorrectionLevel() {
        return errorCorrectionLevel;
    }

    int getRowCount() {
        return rowCount;
    }

    int getRowCountUpperPart() {
        return rowCountUpperPart;
    }

    int getRowCountLowerPart() {
        return rowCountLowerPart;
    }
    virtual ~BarcodeMetadata();
};

} /* namespace pdf417 */
} /* namespace zxing */

#endif /* CORE_SRC_ZXING_PDF417_DECODER_BARCODEMETADATA_H_ */
