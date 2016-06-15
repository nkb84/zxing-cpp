/*
 * DetectionResult.h
 *
 *  Created on: Jun 13, 2016
 *      Author: binhnk
 */

#ifndef CORE_SRC_ZXING_PDF417_DECODER_DETECTIONRESULT_H_
#define CORE_SRC_ZXING_PDF417_DECODER_DETECTIONRESULT_H_

#include <zxing/pdf417/decoder/BarcodeMetadata.h>
#include <zxing/pdf417/decoder/BoundingBox.h>
#include <zxing/pdf417/decoder/DetectionResultColumn.h>

namespace zxing {
namespace pdf417 {

class DetectionResult: public Counted {
    static const int ADJUST_ROW_NUMBER_SKIP;

    Ref<BarcodeMetadata> barcodeMetadata;
    ArrayRef<Ref<DetectionResultColumn>> detectionResultColumns;
    Ref<BoundingBox> boundingBox;
    int barcodeColumnCount;

    void adjustIndicatorColumnRowNumbers(
            Ref<DetectionResultColumn> detectionResultColumn);

// TODO ensure that no detected codewords with unknown row number are left
// we should be able to estimate the row height and use it as a hint for the row number
// we should also fill the rows top to bottom and bottom to top
    /**
     * @return number of codewords which don't have a valid row number. Note that the count is not accurate as codewords
     * will be counted several times. It just serves as an indicator to see when we can stop adjusting row numbers
     */
    int adjustRowNumbers();

    int adjustRowNumbersByRow();

    void adjustRowNumbersFromBothRI();

    int adjustRowNumbersFromRRI();

    int adjustRowNumbersFromLRI();

    static int adjustRowNumberIfValid(int rowIndicatorRowNumber,
            int invalidRowCounts, Ref<Codeword> codeword);

    void adjustRowNumbers(int barcodeColumn, int codewordsRow,
            ArrayRef<Ref<Codeword> > codewords);

    /**
     * @return true, if row number was adjusted, false otherwise
     */
    static boolean adjustRowNumber(Ref<Codeword> codeword,
            Ref<Codeword> otherCodeword);

public:
    DetectionResult(Ref<BarcodeMetadata> barcodeMetadata,
            Ref<BoundingBox> boundingBox);

    ArrayRef<Ref<DetectionResultColumn> > getDetectionResultColumns();

    int getBarcodeColumnCount() {
        return barcodeColumnCount;
    }

    int getBarcodeRowCount() {
        return barcodeMetadata->getRowCount();
    }

    int getBarcodeECLevel() {
        return barcodeMetadata->getErrorCorrectionLevel();
    }

    void setBoundingBox(Ref<BoundingBox> boundingBox) {
        this->boundingBox = boundingBox;
    }

    Ref<BoundingBox> getBoundingBox() {
        return boundingBox;
    }

    void setDetectionResultColumn(int barcodeColumn,
            Ref<DetectionResultColumn> detectionResultColumn) {
        detectionResultColumns[barcodeColumn] = detectionResultColumn;
    }

    Ref<DetectionResultColumn> getDetectionResultColumn(int barcodeColumn) {
        return detectionResultColumns[barcodeColumn];
    }

    Ref<String> toString();

    virtual ~DetectionResult();
};

} /* namespace pdf417 */
} /* namespace zxing */

#endif /* CORE_SRC_ZXING_PDF417_DECODER_DETECTIONRESULT_H_ */
