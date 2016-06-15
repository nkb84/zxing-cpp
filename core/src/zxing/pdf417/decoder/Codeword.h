/*
 * Codeword.h
 *
 *  Created on: Jun 13, 2016
 *      Author: binhnk
 */

#ifndef CORE_SRC_ZXING_PDF417_DECODER_CODEWORD_H_
#define CORE_SRC_ZXING_PDF417_DECODER_CODEWORD_H_

#include <zxing/common/Str.h>
namespace zxing {
namespace pdf417 {

class Codeword : public Counted {
    static const int BARCODE_ROW_UNKNOWN = -1;

    int startX;
    int endX;
    int bucket;
    int value;
    int rowNumber = BARCODE_ROW_UNKNOWN;

public:

    bool hasValidRowNumber();

    bool isValidRowNumber(int rowNumber);

    void setRowNumberAsRowIndicatorColumn();

    int getWidth() {
        return endX - startX;
    }

    int getStartX() {
        return startX;
    }

    int getEndX() {
        return endX;
    }

    int getBucket() {
        return bucket;
    }

    int getValue() {
        return value;
    }

    int getRowNumber() {
        return rowNumber;
    }

    void setRowNumber(int rowNumber);

    Ref<String> toString();

    Codeword(int startX, int endX, int bucket, int value);
    virtual ~Codeword();
};

} /* namespace pdf417 */
} /* namespace zxing */

#endif /* CORE_SRC_ZXING_PDF417_DECODER_CODEWORD_H_ */
