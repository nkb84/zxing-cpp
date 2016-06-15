/*
 * DetectionResultColumn.h
 *
 *  Created on: Jun 13, 2016
 *      Author: binhnk
 */

#ifndef CORE_SRC_ZXING_PDF417_DECODER_DETECTIONRESULTCOLUMN_H_
#define CORE_SRC_ZXING_PDF417_DECODER_DETECTIONRESULTCOLUMN_H_

#include <zxing/pdf417/decoder/Codeword.h>
#include <zxing/pdf417/decoder/BoundingBox.h>

namespace zxing {
namespace pdf417 {

class DetectionResultColumn : public Counted {

    static const int MAX_NEARBY_DISTANCE = 5;

    Ref<BoundingBox> boundingBox;
    ArrayRef<Ref<Codeword>> codewords;

public:
    DetectionResultColumn(Ref<BoundingBox> boundingBox);

    Ref<Codeword> getCodewordNearby(int imageRow);

    int imageRowToCodewordIndex(int imageRow);

    void setCodeword(int imageRow, Ref<Codeword> codeword);

    Ref<Codeword> getCodeword(int imageRow);

    Ref<BoundingBox> getBoundingBox();

    ArrayRef<Ref<Codeword> > getCodewords();

    Ref<String> toString();

    virtual ~DetectionResultColumn();
};

} /* namespace pdf417 */
} /* namespace zxing */

#endif /* CORE_SRC_ZXING_PDF417_DECODER_DETECTIONRESULTCOLUMN_H_ */
