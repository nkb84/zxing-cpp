/*
 * DetectionResult.cpp
 *
 *  Created on: Jun 13, 2016
 *      Author: binhnk
 */

#include <zxing/pdf417/decoder/DetectionResult.h>
#include <zxing/pdf417/decoder/BitMatrixParser.h>
#include <zxing/pdf417/decoder/DetectionResultRowIndicatorColumn.h>

using zxing::pdf417::decoder::BitMatrixParser;

namespace zxing {
namespace pdf417 {

const int DetectionResult::ADJUST_ROW_NUMBER_SKIP = 2;

DetectionResult::~DetectionResult() {
    // TODO Auto-generated destructor stub
}

void DetectionResult::adjustIndicatorColumnRowNumbers(
        Ref<DetectionResultColumn> detectionResultColumn) {
    DetectionResultColumn* column = detectionResultColumn;
    if (detectionResultColumn != NULL) {
        ((DetectionResultRowIndicatorColumn*) column)->adjustCompleteIndicatorColumnRowNumbers(
                barcodeMetadata);
    }
}

int DetectionResult::adjustRowNumbersByRow() {
    adjustRowNumbersFromBothRI();
    // TODO we should only do full row adjustments if row numbers of left and right row indicator column match.
    // Maybe it's even better to calculated the height (in codeword rows) and divide it by the number of barcode
    // rows. This, together with the LRI and RRI row numbers should allow us to get a good estimate where a row
    // number starts and ends.
    int unadjustedCount = adjustRowNumbersFromLRI();
    return unadjustedCount + adjustRowNumbersFromRRI();
}

void DetectionResult::adjustRowNumbersFromBothRI() {
    if (detectionResultColumns[0] == NULL
            || detectionResultColumns[barcodeColumnCount + 1] == NULL) {
        return;
    }
    ArrayRef<Ref<Codeword> > LRIcodewords =
            detectionResultColumns[0]->getCodewords();
    ArrayRef<Ref<Codeword> > RRIcodewords =
            detectionResultColumns[barcodeColumnCount + 1]->getCodewords();
    for (int codewordsRow = 0; codewordsRow < LRIcodewords->size();
            codewordsRow++) {
        if (LRIcodewords[codewordsRow] != NULL
                && RRIcodewords[codewordsRow] != NULL
                && LRIcodewords[codewordsRow]->getRowNumber()
                        == RRIcodewords[codewordsRow]->getRowNumber()) {
            for (int barcodeColumn = 1; barcodeColumn <= barcodeColumnCount;
                    barcodeColumn++) {
                Ref<Codeword> codeword =
                        detectionResultColumns[barcodeColumn]->getCodewords()[codewordsRow];
                if (codeword == NULL) {
                    continue;
                }
                codeword->setRowNumber(
                        LRIcodewords[codewordsRow]->getRowNumber());
                if (!codeword->hasValidRowNumber()) {
                    detectionResultColumns[barcodeColumn]->getCodewords()[codewordsRow] =
                            NULL;
                }
            }
        }
    }
}

int DetectionResult::adjustRowNumbersFromRRI() {
    if (detectionResultColumns[barcodeColumnCount + 1] == NULL) {
        return 0;
    }
    int unadjustedCount = 0;
    ArrayRef<Ref<Codeword> > codewords =
            detectionResultColumns[barcodeColumnCount + 1]->getCodewords();
    for (int codewordsRow = 0; codewordsRow < codewords->size();
            codewordsRow++) {
        if (codewords[codewordsRow] == NULL) {
            continue;
        }
        int rowIndicatorRowNumber = codewords[codewordsRow]->getRowNumber();
        int invalidRowCounts = 0;
        for (int barcodeColumn = barcodeColumnCount + 1;
                barcodeColumn > 0 && invalidRowCounts < ADJUST_ROW_NUMBER_SKIP;
                barcodeColumn--) {
            Ref<Codeword> codeword =
                    detectionResultColumns[barcodeColumn]->getCodewords()[codewordsRow];
            if (codeword != NULL) {
                invalidRowCounts = adjustRowNumberIfValid(rowIndicatorRowNumber,
                        invalidRowCounts, codeword);
                if (!codeword->hasValidRowNumber()) {
                    unadjustedCount++;
                }
            }
        }
    }
    return unadjustedCount;
}

int DetectionResult::adjustRowNumbersFromLRI() {
    if (detectionResultColumns[0] == NULL) {
        return 0;
    }
    int unadjustedCount = 0;
    ArrayRef<Ref<Codeword> > codewords =
            detectionResultColumns[0]->getCodewords();
    for (int codewordsRow = 0; codewordsRow < codewords->size();
            codewordsRow++) {
        if (codewords[codewordsRow] == NULL) {
            continue;
        }
        int rowIndicatorRowNumber = codewords[codewordsRow]->getRowNumber();
        int invalidRowCounts = 0;
        for (int barcodeColumn = 1;
                barcodeColumn < barcodeColumnCount + 1
                        && invalidRowCounts < ADJUST_ROW_NUMBER_SKIP;
                barcodeColumn++) {
            Ref<Codeword> codeword =
                    detectionResultColumns[barcodeColumn]->getCodewords()[codewordsRow];
            if (codeword != NULL) {
                invalidRowCounts = adjustRowNumberIfValid(rowIndicatorRowNumber,
                        invalidRowCounts, codeword);
                if (!codeword->hasValidRowNumber()) {
                    unadjustedCount++;
                }
            }
        }
    }
    return unadjustedCount;
}

int DetectionResult::adjustRowNumberIfValid(int rowIndicatorRowNumber,
        int invalidRowCounts, Ref<Codeword> codeword) {
    if (codeword == NULL) {
        return invalidRowCounts;
    }
    if (!codeword->hasValidRowNumber()) {
        if (codeword->isValidRowNumber(rowIndicatorRowNumber)) {
            codeword->setRowNumber(rowIndicatorRowNumber);
            invalidRowCounts = 0;
        } else {
            ++invalidRowCounts;
        }
    }
    return invalidRowCounts;
}

void DetectionResult::adjustRowNumbers(int barcodeColumn,
        int codewordsRow, ArrayRef<Ref<Codeword> > codewords) {
    Ref<Codeword> codeword = codewords[codewordsRow];
    ArrayRef<Ref<Codeword> > previousColumnCodewords =
            detectionResultColumns[barcodeColumn - 1]->getCodewords();
    ArrayRef<Ref<Codeword> > nextColumnCodewords = previousColumnCodewords;
    if (detectionResultColumns[barcodeColumn + 1] != NULL) {
        nextColumnCodewords =
                detectionResultColumns[barcodeColumn + 1]->getCodewords();
    }
    ArrayRef<Ref<Codeword> > otherCodewords(14);
    otherCodewords[2] = previousColumnCodewords[codewordsRow];
    otherCodewords[3] = nextColumnCodewords[codewordsRow];
    if (codewordsRow > 0) {
        otherCodewords[0] = codewords[codewordsRow - 1];
        otherCodewords[4] = previousColumnCodewords[codewordsRow - 1];
        otherCodewords[5] = nextColumnCodewords[codewordsRow - 1];
    }
    if (codewordsRow > 1) {
        otherCodewords[8] = codewords[codewordsRow - 2];
        otherCodewords[10] = previousColumnCodewords[codewordsRow - 2];
        otherCodewords[11] = nextColumnCodewords[codewordsRow - 2];
    }
    if (codewordsRow < codewords->size() - 1) {
        otherCodewords[1] = codewords[codewordsRow + 1];
        otherCodewords[6] = previousColumnCodewords[codewordsRow + 1];
        otherCodewords[7] = nextColumnCodewords[codewordsRow + 1];
    }
    if (codewordsRow < codewords->size() - 2) {
        otherCodewords[9] = codewords[codewordsRow + 2];
        otherCodewords[12] = previousColumnCodewords[codewordsRow + 2];
        otherCodewords[13] = nextColumnCodewords[codewordsRow + 2];
    }
    for (Ref<Codeword> otherCodeword : otherCodewords->values()) {
        if (adjustRowNumber(codeword, otherCodeword)) {
            return;
        }
    }
}

int DetectionResult::adjustRowNumbers() {
    int unadjustedCount = adjustRowNumbersByRow();
    if (unadjustedCount == 0) {
        return 0;
    }
    for (int barcodeColumn = 1; barcodeColumn < barcodeColumnCount + 1;
            barcodeColumn++) {
        ArrayRef<Ref<Codeword>> codewords =
                detectionResultColumns[barcodeColumn]->getCodewords();
        for (int codewordsRow = 0; codewordsRow < codewords->size();
                codewordsRow++) {
            if (codewords[codewordsRow] == NULL) {
                continue;
            }
            if (!codewords[codewordsRow]->hasValidRowNumber()) {
                adjustRowNumbers(barcodeColumn, codewordsRow, codewords);
            }
        }
    }
    return unadjustedCount;
}

boolean DetectionResult::adjustRowNumber(Ref<Codeword> codeword,
        Ref<Codeword> otherCodeword) {
    if (otherCodeword == NULL) {
        return false;
    }
    if (otherCodeword->hasValidRowNumber()
            && otherCodeword->getBucket() == codeword->getBucket()) {
        codeword->setRowNumber(otherCodeword->getRowNumber());
        return true;
    }
    return false;
}

DetectionResult::DetectionResult(Ref<BarcodeMetadata> barcodeMetadata,
        Ref<BoundingBox> boundingBox) {
    this->barcodeMetadata = barcodeMetadata;
    this->barcodeColumnCount = barcodeMetadata->getColumnCount();
    this->boundingBox = boundingBox;
    detectionResultColumns = ArrayRef<Ref<DetectionResultColumn> >(
            barcodeColumnCount + 2);
}

ArrayRef<Ref<DetectionResultColumn> > DetectionResult::getDetectionResultColumns() {
    adjustIndicatorColumnRowNumbers(detectionResultColumns[0]);
    adjustIndicatorColumnRowNumbers(
            detectionResultColumns[barcodeColumnCount + 1]);
    // int unadjustedCodewordCount = BitMatrixParser::MAX_CODEWORDS_IN_BARCODE;
    int unadjustedCodewordCount = BitMatrixParser::BARS_IN_MODULE;
    int previousUnadjustedCount;
    do {
        previousUnadjustedCount = unadjustedCodewordCount;
        unadjustedCodewordCount = adjustRowNumbers();
    } while (unadjustedCodewordCount > 0
            && unadjustedCodewordCount < previousUnadjustedCount);
    return detectionResultColumns;
}

Ref<String> DetectionResult::toString() {
    Ref<String> result(new String(1024));
    Ref<DetectionResultColumn> rowIndicatorColumn = detectionResultColumns[0];
    if (rowIndicatorColumn == NULL) {
        rowIndicatorColumn = detectionResultColumns[barcodeColumnCount + 1];
    }
    for (int codewordsRow = 0;
            codewordsRow < rowIndicatorColumn->getCodewords()->size();
            codewordsRow++) {
        // formatter.format("CW %3d:", codewordsRow);
        result->append(std::string("CW ") + std::to_string(codewordsRow));
        for (int barcodeColumn = 0; barcodeColumn < barcodeColumnCount + 2;
                barcodeColumn++) {
            if (detectionResultColumns[barcodeColumn] == NULL) {
                // formatter.format("    |   ");
                result->append(std::string("    |   "));
                continue;
            }
            Ref<Codeword> codeword =
                    detectionResultColumns[barcodeColumn]->getCodewords()[codewordsRow];
            if (codeword == NULL) {
                // formatter.format("    |   ");
                result->append(std::string("    |   "));
                continue;
            }
            // formatter.format(" %3d|%3d", codeword.getRowNumber(), codeword.getValue());
            result->append(
                    std::string(" ") + std::to_string(codeword->getRowNumber())
                            + "|" + std::to_string(codeword->getValue()));
        }
        // formatter.format("%n");
        result->append("\n");
    }
    // String result = formatter.toString();
    // formatter.close();
    return result;
}

} /* namespace pdf417 */
} /* namespace zxing */
