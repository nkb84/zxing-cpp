/*
 * DetectionResultRowIndicatorColumn.h
 *
 *  Created on: Jun 13, 2016
 *      Author: binhnk
 */

#ifndef CORE_SRC_ZXING_PDF417_DECODER_DETECTIONRESULTROWINDICATORCOLUMN_H_
#define CORE_SRC_ZXING_PDF417_DECODER_DETECTIONRESULTROWINDICATORCOLUMN_H_

#include <zxing/pdf417/decoder/DetectionResultColumn.h>
#include <zxing/pdf417/decoder/BarcodeMetadata.h>

namespace zxing {
namespace pdf417 {

class DetectionResultRowIndicatorColumn: public DetectionResultColumn {
    bool isLeft_;

    void removeIncorrectCodewords(ArrayRef<Ref<Codeword> > codewords,
            Ref<BarcodeMetadata> barcodeMetadata);

public:

    DetectionResultRowIndicatorColumn(Ref<BoundingBox> boundingBox,
            bool isLeft_);

    void setRowNumbers();

    // TODO implement properly
    // TODO maybe we should add missing codewords to store the correct row number to make
    // finding row numbers for other columns easier
    // use row height count to make detection of invalid row numbers more reliable
    int adjustCompleteIndicatorColumnRowNumbers(
            Ref<BarcodeMetadata> barcodeMetadata);

    ArrayRef<int> getRowHeights();

    // TODO maybe we should add missing codewords to store the correct row number to make
    // finding row numbers for other columns easier
    // use row height count to make detection of invalid row numbers more reliable
    int adjustIncompleteIndicatorColumnRowNumbers(
            Ref<BarcodeMetadata> barcodeMetadata);

    Ref<BarcodeMetadata> getBarcodeMetadata();


    bool isLeft() {
        return isLeft_;
    }

    Ref<String> toString();

    virtual ~DetectionResultRowIndicatorColumn();
};

} /* namespace pdf417 */
} /* namespace zxing */

#endif /* CORE_SRC_ZXING_PDF417_DECODER_DETECTIONRESULTROWINDICATORCOLUMN_H_ */
