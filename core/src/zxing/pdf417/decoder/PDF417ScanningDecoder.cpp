/*
 * PDF417ScanningDecoder.cpp
 *
 *  Created on: Jun 13, 2016
 *      Author: binhnk
 */

#include <zxing/pdf417/decoder/PDF417ScanningDecoder.h>

namespace zxing {
namespace pdf417 {


const int PDF417ScanningDecoder::CODEWORD_SKEW_SIZE = 2;
const int PDF417ScanningDecoder::MAX_ERRORS = 3;
const int PDF417ScanningDecoder::MAX_EC_CODEWORDS = 512;
const Ref<ErrorCorrection> PDF417ScanningDecoder::errorCorrection(new ErrorCorrection());

// final ErrorCorrection errorCorrection = new ErrorCorrection();

PDF417ScanningDecoder::PDF417ScanningDecoder() {
    // TODO Auto-generated constructor stub

}

PDF417ScanningDecoder::~PDF417ScanningDecoder() {
    // TODO Auto-generated destructor stub
}

Ref<DecoderResult> PDF417ScanningDecoder::decode(Ref<BitMatrix> image,
        Ref<ResultPoint> imageTopLeft, Ref<ResultPoint> imageBottomLeft,
        Ref<ResultPoint> imageTopRight, Ref<ResultPoint> imageBottomRight,
        int minCodewordWidth, int maxCodewordWidth) {
    Ref<BoundingBox> boundingBox(
            new BoundingBox(image, imageTopLeft, imageBottomLeft, imageTopRight,
                    imageBottomRight));
    Ref<DetectionResultRowIndicatorColumn> leftRowIndicatorColumn(NULL);
    Ref<DetectionResultRowIndicatorColumn> rightRowIndicatorColumn(NULL);
    Ref<DetectionResult> detectionResult(NULL);
    for (int i = 0; i < 2; i++) {
        if (imageTopLeft != NULL) {
            leftRowIndicatorColumn = getRowIndicatorColumn(image, boundingBox,
                    imageTopLeft, true, minCodewordWidth, maxCodewordWidth);
        }
        if (imageTopRight != NULL) {
            rightRowIndicatorColumn = getRowIndicatorColumn(image, boundingBox,
                    imageTopRight, false, minCodewordWidth, maxCodewordWidth);
        }
        detectionResult = merge(leftRowIndicatorColumn,
                rightRowIndicatorColumn);
        if (detectionResult == NULL) {
            throw NotFoundException();
        }
        if (i == 0 && detectionResult->getBoundingBox() != NULL
                && (detectionResult->getBoundingBox()->getMinY()
                        < boundingBox->getMinY()
                        || detectionResult->getBoundingBox()->getMaxY()
                                > boundingBox->getMaxY())) {
            boundingBox = detectionResult->getBoundingBox();
        } else {
            detectionResult->setBoundingBox(boundingBox);
            break;
        }
    }
    int maxBarcodeColumn = detectionResult->getBarcodeColumnCount() + 1;
    detectionResult->setDetectionResultColumn(0, leftRowIndicatorColumn);
    detectionResult->setDetectionResultColumn(maxBarcodeColumn,
            rightRowIndicatorColumn);
    bool leftToRight = leftRowIndicatorColumn != NULL;
    for (int barcodeColumnCount = 1; barcodeColumnCount <= maxBarcodeColumn;
            barcodeColumnCount++) {
        int barcodeColumn =
                leftToRight ?
                        barcodeColumnCount :
                        maxBarcodeColumn - barcodeColumnCount;
        if (detectionResult->getDetectionResultColumn(barcodeColumn) != NULL) {
            // This will be the case for the opposite row indicator column, which doesn't need to be decoded again.
            continue;
        }
        Ref<DetectionResultColumn> detectionResultColumn;
        if (barcodeColumn == 0 || barcodeColumn == maxBarcodeColumn) {
            detectionResultColumn = new DetectionResultRowIndicatorColumn(
                    boundingBox, barcodeColumn == 0);
        } else {
            detectionResultColumn = new DetectionResultColumn(boundingBox);
        }
        detectionResult->setDetectionResultColumn(barcodeColumn,
                detectionResultColumn);
        int startColumn = -1;
        int previousStartColumn = startColumn;
        // TODO start at a row for which we know the start position, then detect upwards and downwards from there.
        for (int imageRow = boundingBox->getMinY();
                imageRow <= boundingBox->getMaxY(); imageRow++) {
            startColumn = getStartColumn(detectionResult, barcodeColumn,
                    imageRow, leftToRight);
            if (startColumn < 0 || startColumn > boundingBox->getMaxX()) {
                if (previousStartColumn == -1) {
                    continue;
                }
                startColumn = previousStartColumn;
            }
            Ref<Codeword> codeword = detectCodeword(image, boundingBox->getMinX(),
                    boundingBox->getMaxX(), leftToRight, startColumn, imageRow,
                    minCodewordWidth, maxCodewordWidth);
            if (codeword != NULL) {
                detectionResultColumn->setCodeword(imageRow, codeword);
                previousStartColumn = startColumn;
                minCodewordWidth = std::min(minCodewordWidth,
                        codeword->getWidth());
                maxCodewordWidth = std::max(maxCodewordWidth,
                        codeword->getWidth());
            }
        }
    }
    return createDecoderResult(detectionResult);
}

Ref<DetectionResult> PDF417ScanningDecoder::merge(
        Ref<DetectionResultRowIndicatorColumn> leftRowIndicatorColumn,
        Ref<DetectionResultRowIndicatorColumn> rightRowIndicatorColumn) {
    if (leftRowIndicatorColumn == NULL && rightRowIndicatorColumn == NULL) {
        return Ref<DetectionResult>(NULL);
    }
    Ref<BarcodeMetadata> barcodeMetadata = getBarcodeMetadata(leftRowIndicatorColumn,
            rightRowIndicatorColumn);
    if (barcodeMetadata == NULL) {
        return Ref<DetectionResult>(NULL);
    }
    Ref<BoundingBox> boundingBox = BoundingBox::merge(
            adjustBoundingBox(leftRowIndicatorColumn),
            adjustBoundingBox(rightRowIndicatorColumn));
    return Ref<DetectionResult>(new DetectionResult(barcodeMetadata, boundingBox));
}

Ref<BoundingBox> PDF417ScanningDecoder::adjustBoundingBox(
        Ref<DetectionResultRowIndicatorColumn> rowIndicatorColumn) {
    if (rowIndicatorColumn == NULL) {
        return Ref<BoundingBox>(NULL);
    }
    ArrayRef<int> rowHeights = rowIndicatorColumn->getRowHeights();
    if (rowHeights == NULL) {
        return Ref<BoundingBox>(NULL);
    }
    int maxRowHeight = getMax(rowHeights);
    int missingStartRows = 0;
    for (int rowHeight : rowHeights->values()) {
        missingStartRows += maxRowHeight - rowHeight;
        if (rowHeight > 0) {
            break;
        }
    }
    ArrayRef<Ref<Codeword> > codewords = rowIndicatorColumn->getCodewords();
    for (int row = 0; missingStartRows > 0 && codewords[row] == NULL; row++) {
        missingStartRows--;
    }
    int missingEndRows = 0;
    for (int row = rowHeights->size() - 1; row >= 0; row--) {
        missingEndRows += maxRowHeight - rowHeights[row];
        if (rowHeights[row] > 0) {
            break;
        }
    }
    for (int row = codewords->size() - 1;
            missingEndRows > 0 && codewords[row] == NULL; row--) {
        missingEndRows--;
    }
    return rowIndicatorColumn->getBoundingBox()->addMissingRows(
            missingStartRows, missingEndRows, rowIndicatorColumn->isLeft());
}

int PDF417ScanningDecoder::getMax(ArrayRef<int> values) {
    int maxValue = -1;
    for (int value : values->values()) {
        maxValue = std::max(maxValue, value);
    }
    return maxValue;
}

Ref<BarcodeMetadata> PDF417ScanningDecoder::getBarcodeMetadata(
        Ref<DetectionResultRowIndicatorColumn> leftRowIndicatorColumn,
        Ref<DetectionResultRowIndicatorColumn> rightRowIndicatorColumn) {
    Ref<BarcodeMetadata> leftBarcodeMetadata;
    if (leftRowIndicatorColumn == NULL || (leftBarcodeMetadata =
            leftRowIndicatorColumn->getBarcodeMetadata()) == NULL) {
        return rightRowIndicatorColumn == NULL ?
                Ref<BarcodeMetadata>(NULL) :
                rightRowIndicatorColumn->getBarcodeMetadata();
    }
    Ref<BarcodeMetadata> rightBarcodeMetadata;
    if (rightRowIndicatorColumn == NULL || (rightBarcodeMetadata =
            rightRowIndicatorColumn->getBarcodeMetadata()) == NULL) {
        return leftBarcodeMetadata;
    }
    if (leftBarcodeMetadata->getColumnCount()
            != rightBarcodeMetadata->getColumnCount()
            && leftBarcodeMetadata->getErrorCorrectionLevel()
                    != rightBarcodeMetadata->getErrorCorrectionLevel()
            && leftBarcodeMetadata->getRowCount()
                    != rightBarcodeMetadata->getRowCount()) {
        return Ref<BarcodeMetadata>(NULL);
    }
    return leftBarcodeMetadata;
}

Ref<DetectionResultRowIndicatorColumn> PDF417ScanningDecoder::getRowIndicatorColumn(
        Ref<BitMatrix> image, Ref<BoundingBox> boundingBox,
        Ref<ResultPoint> startPoint, bool leftToRight, int minCodewordWidth,
        int maxCodewordWidth) {
    Ref<DetectionResultRowIndicatorColumn> rowIndicatorColumn(
            new DetectionResultRowIndicatorColumn(boundingBox, leftToRight));
    for (int i = 0; i < 2; i++) {
        int increment = i == 0 ? 1 : -1;
        int startColumn = (int) startPoint->getX();
        for (int imageRow = (int) startPoint->getY();
                imageRow <= boundingBox->getMaxY()
                        && imageRow >= boundingBox->getMinY(); imageRow +=
                        increment) {
            Ref<Codeword> codeword = detectCodeword(image, 0, image->getWidth(),
                    leftToRight, startColumn, imageRow, minCodewordWidth,
                    maxCodewordWidth);
            if (codeword != NULL) {
                rowIndicatorColumn->setCodeword(imageRow, codeword);
                if (leftToRight) {
                    startColumn = codeword->getStartX();
                } else {
                    startColumn = codeword->getEndX();
                }
            }
        }
    }
    return rowIndicatorColumn;
}

void PDF417ScanningDecoder::adjustCodewordCount(
        Ref<DetectionResult> detectionResult,
        ArrayRef<ArrayRef<Ref<BarcodeValue> > > barcodeMatrix) {
    ArrayRef<int> numberOfCodewords = barcodeMatrix[0][1]->getValue();
    int calculatedNumberOfCodewords = detectionResult->getBarcodeColumnCount()
            * detectionResult->getBarcodeRowCount()
            - getNumberOfECCodeWords(detectionResult->getBarcodeECLevel());
    if (numberOfCodewords->size() == 0) {
        if (calculatedNumberOfCodewords < 1
                || calculatedNumberOfCodewords
                        > BitMatrixParser::MAX_CODEWORDS_IN_BARCODE) {
            throw NotFoundException();
        }
        barcodeMatrix[0][1]->setValue(calculatedNumberOfCodewords);
    } else if (numberOfCodewords[0] != calculatedNumberOfCodewords) {
        // The calculated one is more reliable as it is derived from the row indicator columns
        barcodeMatrix[0][1]->setValue(calculatedNumberOfCodewords);
    }
}

Ref<DecoderResult> PDF417ScanningDecoder::createDecoderResult(
        Ref<DetectionResult> detectionResult) {
    ArrayRef<ArrayRef<Ref<BarcodeValue> > > barcodeMatrix = createBarcodeMatrix(
            detectionResult);
    adjustCodewordCount(detectionResult, barcodeMatrix);
    ArrayRef<int> erasures = new Array<int>();
    ArrayRef<int> codewords(
            detectionResult->getBarcodeRowCount()
                    * detectionResult->getBarcodeColumnCount());
    ArrayRef<ArrayRef<int> > ambiguousIndexValuesList = new Array<ArrayRef<int>>();
    ArrayRef<int> ambiguousIndexesList = new Array<int>();
    for (int row = 0; row < detectionResult->getBarcodeRowCount(); row++) {
        for (int column = 0; column < detectionResult->getBarcodeColumnCount();
                column++) {
            ArrayRef<int> values = barcodeMatrix[row][column + 1]->getValue();
            int codewordIndex = row * detectionResult->getBarcodeColumnCount()
                    + column;
            if (values->size() == 0) {
                erasures->values().push_back(codewordIndex);
            } else if (values->size() == 1) {
                codewords[codewordIndex] = values[0];
            } else {
                ambiguousIndexesList->values().push_back(codewordIndex);
                ambiguousIndexValuesList->values().push_back(values);
            }
        }
    }
    return createDecoderResultFromAmbiguousValues(
            detectionResult->getBarcodeECLevel(), codewords,
            erasures, ambiguousIndexesList,
            ambiguousIndexValuesList);
}

Ref<DecoderResult> PDF417ScanningDecoder::createDecoderResultFromAmbiguousValues(
        int ecLevel, ArrayRef<int> codewords, ArrayRef<int> erasureArray,
        ArrayRef<int> ambiguousIndexes,
        ArrayRef<ArrayRef<int> > ambiguousIndexValues) {
    ArrayRef<int> ambiguousIndexCount(ambiguousIndexes->size());
    int tries = 100;
    while (tries-- > 0) {
        for (int i = 0; i < ambiguousIndexCount->size(); i++) {
            codewords[ambiguousIndexes[i]] =
                    ambiguousIndexValues[i][ambiguousIndexCount[i]];
        }
        try {
            return decodeCodewords(codewords, ecLevel, erasureArray);
        } catch (ChecksumException ignored) {
            //
        }

        if (ambiguousIndexCount->size() == 0) {
            throw ChecksumException();
        }
        for (int i = 0; i < ambiguousIndexCount->size(); i++) {
            if (ambiguousIndexCount[i] < ambiguousIndexValues[i]->size() - 1) {
                ambiguousIndexCount[i]++;
                break;
            } else {
                ambiguousIndexCount[i] = 0;
                if (i == ambiguousIndexCount->size() - 1) {
                    throw ChecksumException();
                }
            }
        }
    }
    throw ChecksumException();
}

ArrayRef<ArrayRef<Ref<BarcodeValue> > > PDF417ScanningDecoder::createBarcodeMatrix(
        Ref<DetectionResult> detectionResult) {
    ArrayRef<ArrayRef<Ref<BarcodeValue> > > barcodeMatrix(
            detectionResult->getBarcodeRowCount());
    // new BarcodeValue[detectionResult->getBarcodeRowCount()][detectionResult->getBarcodeColumnCount() + 2];
    for (int row = 0; row < barcodeMatrix->size(); row++) {
        barcodeMatrix[row] = ArrayRef<Ref<BarcodeValue> >(
                detectionResult->getBarcodeColumnCount() + 2);
        for (int column = 0; column < barcodeMatrix[row]->size(); column++) {
            barcodeMatrix[row][column] = new BarcodeValue();
        }
    }
    int column = 0;
    for (Ref<DetectionResultColumn> detectionResultColumn : detectionResult->getDetectionResultColumns()->values()) {
        if (detectionResultColumn != NULL) {
            for (Ref<Codeword> codeword : detectionResultColumn->getCodewords()->values()) {
                if (codeword != NULL) {
                    int rowNumber = codeword->getRowNumber();
                    if (rowNumber >= 0) {
                        if (rowNumber >= barcodeMatrix->size()) {
                            // We have more rows than the barcode metadata allows for, ignore them.
                            continue;
                        }
                        barcodeMatrix[rowNumber][column]->setValue(
                                codeword->getValue());
                    }
                }
            }
        }
        column++;
    }
    return barcodeMatrix;
}

bool PDF417ScanningDecoder::isValidBarcodeColumn(
        Ref<DetectionResult> detectionResult, int barcodeColumn) {
    return barcodeColumn >= 0
            && barcodeColumn <= detectionResult->getBarcodeColumnCount() + 1;
}

int PDF417ScanningDecoder::getStartColumn(
        Ref<DetectionResult> detectionResult, int barcodeColumn, int imageRow,
        bool leftToRight) {
    int offset = leftToRight ? 1 : -1;
    Ref<Codeword> codeword(NULL);
    if (isValidBarcodeColumn(detectionResult, barcodeColumn - offset)) {
        codeword = detectionResult->getDetectionResultColumn(
                barcodeColumn - offset)->getCodeword(imageRow);
    }
    if (codeword != NULL) {
        return leftToRight ? codeword->getEndX() : codeword->getStartX();
    }
    codeword =
            detectionResult->getDetectionResultColumn(barcodeColumn)->getCodewordNearby(
                    imageRow);
    if (codeword != NULL) {
        return leftToRight ? codeword->getStartX() : codeword->getEndX();
    }
    if (isValidBarcodeColumn(detectionResult, barcodeColumn - offset)) {
        codeword = detectionResult->getDetectionResultColumn(
                barcodeColumn - offset)->getCodewordNearby(imageRow);
    }
    if (codeword != NULL) {
        return leftToRight ? codeword->getEndX() : codeword->getStartX();
    }
    int skippedColumns = 0;
    while (isValidBarcodeColumn(detectionResult, barcodeColumn - offset)) {
        barcodeColumn -= offset;
        for (Ref<Codeword> previousRowCodeword : detectionResult->getDetectionResultColumn(
                barcodeColumn)->getCodewords()->values()) {
            if (previousRowCodeword != NULL) {
                return (leftToRight ?
                        previousRowCodeword->getEndX() :
                        previousRowCodeword->getStartX())
                        + offset * skippedColumns
                                * (previousRowCodeword->getEndX()
                                        - previousRowCodeword->getStartX());
            }
        }
        skippedColumns++;
    }
    return leftToRight ?
            detectionResult->getBoundingBox()->getMinX() :
            detectionResult->getBoundingBox()->getMaxX();
}

Ref<Codeword> PDF417ScanningDecoder::detectCodeword(Ref<BitMatrix> image,
        int minColumn, int maxColumn, bool leftToRight, int startColumn,
        int imageRow, int minCodewordWidth, int maxCodewordWidth) {
    startColumn = adjustCodewordStartColumn(image, minColumn, maxColumn,
            leftToRight, startColumn, imageRow);
    // we usually know fairly exact now how long a codeword is. We should provide minimum and maximum expected length
    // and try to adjust the read pixels, e.g. remove single pixel errors or try to cut off exceeding pixels.
    // min and maxCodewordWidth should not be used as they are calculated for the whole barcode an can be inaccurate
    // for the current position
    ArrayRef<int> moduleBitCount = getModuleBitCount(image, minColumn,
            maxColumn, leftToRight, startColumn, imageRow);
    if (moduleBitCount == NULL) {
        return Ref<Codeword>(NULL);
    }
    int endColumn;
    int codewordBitCount = MathUtils::sum(moduleBitCount);
    if (leftToRight) {
        endColumn = startColumn + codewordBitCount;
    } else {
        for (int i = 0; i < moduleBitCount->size() / 2; i++) {
            int tmpCount = moduleBitCount[i];
            moduleBitCount[i] = moduleBitCount[moduleBitCount->size() - 1 - i];
            moduleBitCount[moduleBitCount->size() - 1 - i] = tmpCount;
        }
        endColumn = startColumn;
        startColumn = endColumn - codewordBitCount;
    }
    // TODO implement check for width and correction of black and white bars
    // use start (and maybe stop pattern) to determine if blackbars are wider than white bars. If so, adjust.
    // should probably done only for codewords with a lot more than 17 bits.
    // The following fixes 10-1.png, which has wide black bars and small white bars
    //    for (int i = 0; i < moduleBitCount.length; i++) {
    //      if (i % 2 == 0) {
    //        moduleBitCount[i]--;
    //      } else {
    //        moduleBitCount[i]++;
    //      }
    //    }
    // We could also use the width of surrounding codewords for more accurate results, but this seems
    // sufficient for now
    if (!checkCodewordSkew(codewordBitCount, minCodewordWidth,
            maxCodewordWidth)) {
        // We could try to use the startX and endX position of the codeword in the same column in the previous row,
        // create the bit count from it and normalize it to 8. This would help with single pixel errors.
        return Ref<Codeword>(NULL);
    }
    int decodedValue = PDF417CodewordDecoder::getDecodedValue(moduleBitCount);
    int codeword = BitMatrixParser::getCodeword(decodedValue, NULL);
    if (codeword == -1) {
        return Ref<Codeword>(NULL);
    }
    return Ref<Codeword>(
            new Codeword(startColumn, endColumn,
                    getCodewordBucketNumber(decodedValue), codeword));
}

ArrayRef<int> PDF417ScanningDecoder::getModuleBitCount(
        Ref<BitMatrix> image, int minColumn, int maxColumn, bool leftToRight,
        int startColumn, int imageRow) {
    int imageColumn = startColumn;
    ArrayRef<int> moduleBitCount(8);
    int moduleNumber = 0;
    int increment = leftToRight ? 1 : -1;
    bool previousPixelValue = leftToRight;
    while ((leftToRight ? imageColumn < maxColumn : imageColumn >= minColumn)
            && moduleNumber < moduleBitCount->size()) {
        if (image->get(imageColumn, imageRow) == previousPixelValue) {
            moduleBitCount[moduleNumber]++;
            imageColumn += increment;
        } else {
            moduleNumber++;
            previousPixelValue = !previousPixelValue;
        }
    }
    if (moduleNumber == moduleBitCount->size()
            || ((imageColumn == (leftToRight ? maxColumn : minColumn))
                    && moduleNumber == moduleBitCount->size() - 1)) {
        return moduleBitCount;
    }
    return ArrayRef<int>((Array<int>*) NULL);
}

int PDF417ScanningDecoder::getNumberOfECCodeWords(int barcodeECLevel) {
    return 2 << barcodeECLevel;
}

int PDF417ScanningDecoder::adjustCodewordStartColumn(
        Ref<BitMatrix> image, int minColumn, int maxColumn, bool leftToRight,
        int codewordStartColumn, int imageRow) {
    int correctedStartColumn = codewordStartColumn;
    int increment = leftToRight ? -1 : 1;
    // there should be no black pixels before the start column. If there are, then we need to start earlier.
    for (int i = 0; i < 2; i++) {
        while ((leftToRight ?
                correctedStartColumn >= minColumn :
                correctedStartColumn < maxColumn)
                && leftToRight == image->get(correctedStartColumn, imageRow)) {
            if (abs(codewordStartColumn - correctedStartColumn)
                    > CODEWORD_SKEW_SIZE) {
                return codewordStartColumn;
            }
            correctedStartColumn += increment;
        }
        increment = -increment;
        leftToRight = !leftToRight;
    }
    return correctedStartColumn;
}

bool PDF417ScanningDecoder::checkCodewordSkew(int codewordSize,
        int minCodewordWidth, int maxCodewordWidth) {
    return minCodewordWidth - CODEWORD_SKEW_SIZE <= codewordSize
            && codewordSize <= maxCodewordWidth + CODEWORD_SKEW_SIZE;
}

Ref<DecoderResult> PDF417ScanningDecoder::decodeCodewords(
        ArrayRef<int> codewords, int ecLevel, ArrayRef<int> erasures) {
    if (codewords->size() == 0) {
        throw FormatException();
    }
    int numECCodewords = 1 << (ecLevel + 1);
    int correctedErrorsCount = correctErrors(codewords, erasures,
            numECCodewords);
    verifyCodewordCount(codewords, numECCodewords);
    // Decode the codewords
    Ref<DecoderResult> decoderResult = DecodedBitStreamParser::decode(
            codewords);
    return decoderResult;
}

int PDF417ScanningDecoder::correctErrors(ArrayRef<int> codewords,
        ArrayRef<int> erasures, int numECCodewords) {
    if (erasures != NULL && erasures->size() > numECCodewords / 2 + MAX_ERRORS
            || numECCodewords < 0 || numECCodewords > MAX_EC_CODEWORDS) {
        // Too many errors or EC Codewords is corrupted
        throw ChecksumException();
    }
    errorCorrection->decode(codewords, numECCodewords, erasures);
    return 0;
}

void PDF417ScanningDecoder::verifyCodewordCount(ArrayRef<int> codewords,
        int numECCodewords) {
    if (codewords->size() < 4) {
        // Codeword array size should be at least 4 allowing for
        // Count CW, At least one Data CW, Error Correction CW, Error Correction CW
        throw FormatException();
    }
    // The first codeword, the Symbol Length Descriptor, shall always encode the total number of data
    // codewords in the symbol, including the Symbol Length Descriptor itself, data codewords and pad
    // codewords, but excluding the number of error correction codewords.
    int numberOfCodewords = codewords[0];
    if (numberOfCodewords > codewords->size()) {
        throw FormatException();
    }
    if (numberOfCodewords == 0) {
        // Reset to the length of the array - 8 (Allow for at least level 3 Error Correction (8 Error Codewords)
        if (numECCodewords < codewords->size()) {
            codewords[0] = codewords->size() - numECCodewords;
        } else {
            throw FormatException();
        }
    }
}

ArrayRef<int> PDF417ScanningDecoder::getBitCountForCodeword(
        int codeword) {
    ArrayRef<int> result(8);
    int previousValue = 0;
    int i = result->size() - 1;
    while (true) {
        if ((codeword & 0x1) != previousValue) {
            previousValue = codeword & 0x1;
            i--;
            if (i < 0) {
                break;
            }
        }
        result[i]++;
        codeword >>= 1;
    }
    return result;
}

int PDF417ScanningDecoder::getCodewordBucketNumber(
        ArrayRef<int> moduleBitCount) {
    return (moduleBitCount[0] - moduleBitCount[2] + moduleBitCount[4]
            - moduleBitCount[6] + 9) % 9;
}

int PDF417ScanningDecoder::getCodewordBucketNumber(int codeword) {
    return getCodewordBucketNumber(getBitCountForCodeword(codeword));
}

Ref<String> PDF417ScanningDecoder::toString(
        ArrayRef<ArrayRef<BarcodeValue> > barcodeMatrix) {
    return Ref<String>(new String("Hello"));
}


} /* namespace pdf417 */
} /* namespace zxing */
