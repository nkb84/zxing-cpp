/*
 * DetectionResultRowIndicatorColumn.cpp
 *
 *  Created on: Jun 13, 2016
 *      Author: binhnk
 */

#include <zxing/pdf417/decoder/BarcodeValue.h>
#include <zxing/pdf417/decoder/BitMatrixParser.h>
#include <zxing/pdf417/decoder/DetectionResultRowIndicatorColumn.h>

using zxing::pdf417::decoder::BitMatrixParser;
namespace zxing {
namespace pdf417 {

DetectionResultRowIndicatorColumn::~DetectionResultRowIndicatorColumn() {
    // TODO Auto-generated destructor stub
}

void DetectionResultRowIndicatorColumn::removeIncorrectCodewords(
        ArrayRef<Ref<Codeword> > codewords,
        Ref<BarcodeMetadata> barcodeMetadata) {
    // Remove codewords which do not match the metadata
    // TODO Maybe we should keep the incorrect codewords for the start and end positions?
    for (int codewordRow = 0; codewordRow < codewords->size(); codewordRow++) {
        Ref<Codeword> codeword = codewords[codewordRow];
        if (codewords[codewordRow] == NULL) {
            continue;
        }
        int rowIndicatorValue = codeword->getValue() % 30;
        int codewordRowNumber = codeword->getRowNumber();
        if (codewordRowNumber > barcodeMetadata->getRowCount()) {
            codewords[codewordRow] = NULL;
            continue;
        }
        if (!isLeft_) {
            codewordRowNumber += 2;
        }
        switch (codewordRowNumber % 3) {
        case 0:
            if (rowIndicatorValue * 3 + 1
                    != barcodeMetadata->getRowCountUpperPart()) {
                codewords[codewordRow] = NULL;
            }
            break;
        case 1:
            if (rowIndicatorValue / 3
                    != barcodeMetadata->getErrorCorrectionLevel()
                    || rowIndicatorValue % 3
                            != barcodeMetadata->getRowCountLowerPart()) {
                codewords[codewordRow] = NULL;
            }
            break;
        case 2:
            if (rowIndicatorValue + 1 != barcodeMetadata->getColumnCount()) {
                codewords[codewordRow] = NULL;
            }
            break;
        }
    }
}

DetectionResultRowIndicatorColumn::DetectionResultRowIndicatorColumn(
        Ref<BoundingBox> boundingBox, bool isLeft_) :
        DetectionResultColumn(boundingBox) {
    this->isLeft_ = isLeft_;
}

void DetectionResultRowIndicatorColumn::setRowNumbers() {
    for (Ref<Codeword> codeword : getCodewords()->values()) {
        if (codeword != NULL) {
            codeword->setRowNumberAsRowIndicatorColumn();
        }
    }
}

int DetectionResultRowIndicatorColumn::adjustCompleteIndicatorColumnRowNumbers(
        Ref<BarcodeMetadata> barcodeMetadata) {
    ArrayRef<Ref<Codeword> > codewords = getCodewords();
    setRowNumbers();
    removeIncorrectCodewords(codewords, barcodeMetadata);
    Ref<BoundingBox> boundingBox = getBoundingBox();
    Ref<ResultPoint> top =
            isLeft_ ? boundingBox->getTopLeft() : boundingBox->getTopRight();
    Ref<ResultPoint> bottom =
            isLeft_ ?
                    boundingBox->getBottomLeft() :
                    boundingBox->getBottomRight();
    int firstRow = imageRowToCodewordIndex((int) (top->getY()));
    int lastRow = imageRowToCodewordIndex((int) (bottom->getY()));
    // We need to be careful using the average row height. Barcode could be skewed so that we have smaller and
    // taller rows
    float averageRowHeight = (lastRow - firstRow)
            / (float) (barcodeMetadata->getRowCount());
    int barcodeRow = -1;
    int maxRowHeight = 1;
    int currentRowHeight = 0;
    for (int codewordsRow = firstRow; codewordsRow < lastRow; codewordsRow++) {
        if (codewords[codewordsRow] == NULL) {
            continue;
        }
        Ref<Codeword> codeword = codewords[codewordsRow];

        //      float expectedRowNumber = (codewordsRow - firstRow) / averageRowHeight;
        //      if (Math.abs(codeword.getRowNumber() - expectedRowNumber) > 2) {
        //        SimpleLog.log(LEVEL.WARNING,
        //            "Removing codeword, rowNumberSkew too high, codeword[" + codewordsRow + "]: Expected Row: " +
        //                expectedRowNumber + ", RealRow: " + codeword.getRowNumber() + ", value: " + codeword.getValue());
        //        codewords[codewordsRow] = null;
        //      }

        int rowDifference = codeword->getRowNumber() - barcodeRow;

        // TODO improve handling with case where first row indicator doesn't start with 0

        if (rowDifference == 0) {
            currentRowHeight++;
        } else if (rowDifference == 1) {
            maxRowHeight = std::max(maxRowHeight, currentRowHeight);
            currentRowHeight = 1;
            barcodeRow = codeword->getRowNumber();
        } else if (rowDifference < 0
                || codeword->getRowNumber() >= barcodeMetadata->getRowCount()
                || rowDifference > codewordsRow) {
            codewords[codewordsRow] = NULL;
        } else {
            int checkedRows;
            if (maxRowHeight > 2) {
                checkedRows = (maxRowHeight - 2) * rowDifference;
            } else {
                checkedRows = rowDifference;
            }
            bool closePreviousCodewordFound = checkedRows >= codewordsRow;
            for (int i = 1; i <= checkedRows && !closePreviousCodewordFound;
                    i++) {
                // there must be (height * rowDifference) number of codewords missing. For now we assume height = 1.
                // This should hopefully get rid of most problems already.
                closePreviousCodewordFound =
                        codewords[codewordsRow - i] != NULL;
            }
            if (closePreviousCodewordFound) {
                codewords[codewordsRow] = NULL;
            } else {
                barcodeRow = codeword->getRowNumber();
                currentRowHeight = 1;
            }
        }
    }
    return (int) ((averageRowHeight + 0.5));
}

ArrayRef<int> DetectionResultRowIndicatorColumn::getRowHeights() {
    Ref<BarcodeMetadata> barcodeMetadata = getBarcodeMetadata();
    if (barcodeMetadata == NULL) {
        return NULL;
    }
    adjustIncompleteIndicatorColumnRowNumbers(barcodeMetadata);
    ArrayRef<int> result(barcodeMetadata->getRowCount());
    for (Ref<Codeword> codeword : getCodewords()->values()) {
        if (codeword != NULL) {
            int rowNumber = codeword->getRowNumber();
            if (rowNumber >= result->size()) {
                // We have more rows than the barcode metadata allows for, ignore them.
                continue;
            }
            result[rowNumber]++;
        } // else throw exception?
    }
    return result;
}

int DetectionResultRowIndicatorColumn::adjustIncompleteIndicatorColumnRowNumbers(
        Ref<BarcodeMetadata> barcodeMetadata) {
    Ref<BoundingBox> boundingBox = getBoundingBox();
    Ref<ResultPoint> top =
            isLeft_ ? boundingBox->getTopLeft() : boundingBox->getTopRight();
    Ref<ResultPoint> bottom =
            isLeft_ ?
                    boundingBox->getBottomLeft() :
                    boundingBox->getBottomRight();
    int firstRow = imageRowToCodewordIndex((int) (top->getY()));
    int lastRow = imageRowToCodewordIndex((int) (bottom->getY()));
    float averageRowHeight = (lastRow - firstRow)
            / (float) (barcodeMetadata->getRowCount());
    ArrayRef<Ref<Codeword> > codewords = getCodewords();
    int barcodeRow = -1;
    int maxRowHeight = 1;
    int currentRowHeight = 0;
    for (int codewordsRow = firstRow; codewordsRow < lastRow; codewordsRow++) {
        if (codewords[codewordsRow] == NULL) {
            continue;
        }
        Ref<Codeword> codeword = codewords[codewordsRow];

        codeword->setRowNumberAsRowIndicatorColumn();

        int rowDifference = codeword->getRowNumber() - barcodeRow;

        // TODO improve handling with case where first row indicator doesn't start with 0

        if (rowDifference == 0) {
            currentRowHeight++;
        } else if (rowDifference == 1) {
            maxRowHeight = std::max(maxRowHeight, currentRowHeight);
            currentRowHeight = 1;
            barcodeRow = codeword->getRowNumber();
        } else if (codeword->getRowNumber() >= barcodeMetadata->getRowCount()) {
            codewords[codewordsRow] = NULL;
        } else {
            barcodeRow = codeword->getRowNumber();
            currentRowHeight = 1;
        }
    }
    return (int) ((averageRowHeight + 0.5));
}

Ref<BarcodeMetadata> DetectionResultRowIndicatorColumn::getBarcodeMetadata() {
    ArrayRef<Ref<Codeword> > codewords = getCodewords();
    Ref<BarcodeValue> barcodeColumnCount(new BarcodeValue());
    Ref<BarcodeValue> barcodeRowCountUpperPart(new BarcodeValue());
    Ref<BarcodeValue> barcodeRowCountLowerPart(new BarcodeValue());
    Ref<BarcodeValue> barcodeECLevel(new BarcodeValue());
    for (Ref<Codeword> codeword : codewords->values()) {
        if (codeword == NULL) {
            continue;
        }
        codeword->setRowNumberAsRowIndicatorColumn();
        int rowIndicatorValue = codeword->getValue() % 30;
        int codewordRowNumber = codeword->getRowNumber();
        if (!isLeft_) {
            codewordRowNumber += 2;
        }
        switch (codewordRowNumber % 3) {
        case 0:
            barcodeRowCountUpperPart->setValue(rowIndicatorValue * 3 + 1);
            break;
        case 1:
            barcodeECLevel->setValue(rowIndicatorValue / 3);
            barcodeRowCountLowerPart->setValue(rowIndicatorValue % 3);
            break;
        case 2:
            barcodeColumnCount->setValue(rowIndicatorValue + 1);
            break;
        }
    }
    // Maybe we should check if we have ambiguous values?
    if ((barcodeColumnCount->getValue()->size() == 0)
            || (barcodeRowCountUpperPart->getValue()->size() == 0)
            || (barcodeRowCountLowerPart->getValue()->size() == 0)
            || (barcodeECLevel->getValue()->size() == 0)
            || barcodeColumnCount->getValue()[0] < 1
            || barcodeRowCountUpperPart->getValue()[0]
                    + barcodeRowCountLowerPart->getValue()[0]
                    < BitMatrixParser::MIN_ROWS_IN_BARCODE
            || barcodeRowCountUpperPart->getValue()[0]
                    + barcodeRowCountLowerPart->getValue()[0]
                    > BitMatrixParser::MAX_ROWS_IN_BARCODE) {
        return Ref<BarcodeMetadata>(NULL);
    }
    Ref<BarcodeMetadata> barcodeMetadata(
            new BarcodeMetadata(barcodeColumnCount->getValue()[0],
                    barcodeRowCountUpperPart->getValue()[0],
                    barcodeRowCountLowerPart->getValue()[0],
                    barcodeECLevel->getValue()[0]));
    removeIncorrectCodewords(codewords, barcodeMetadata);
    return barcodeMetadata;
}

Ref<String> DetectionResultRowIndicatorColumn::toString() {
    Ref<String> result(new String("IsLeft: "));
    result->append(
            std::to_string(isLeft_) + "\n"
                    + DetectionResultColumn::toString()->getText());
    return result;
}

} /* namespace pdf417 */
} /* namespace zxing */
