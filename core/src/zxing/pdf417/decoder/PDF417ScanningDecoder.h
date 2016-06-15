/*
 * PDF417ScanningDecoder.h
 *
 *  Created on: Jun 13, 2016
 *      Author: binhnk
 */

#ifndef CORE_SRC_ZXING_PDF417_DECODER_PDF417SCANNINGDECODER_H_
#define CORE_SRC_ZXING_PDF417_DECODER_PDF417SCANNINGDECODER_H_

#include <zxing/ResultPoint.h>
#include <zxing/common/BitMatrix.h>
#include <zxing/common/DecoderResult.h>
#include <zxing/NotFoundException.h>
#include <zxing/ChecksumException.h>

#include <zxing/common/detector/MathUtils.h>
#include <zxing/pdf417/decoder/ec/ErrorCorrection.h>
#include <zxing/pdf417/decoder/BarcodeValue.h>
#include <zxing/pdf417/decoder/DetectionResult.h>
#include <zxing/pdf417/decoder/DecodedBitStreamParser.h>
#include <zxing/pdf417/decoder/BitMatrixParser.h>
#include <zxing/pdf417/decoder/PDF417CodewordDecoder.h>
#include <zxing/pdf417/decoder/DetectionResultRowIndicatorColumn.h>

using zxing::common::detector::MathUtils;
using zxing::pdf417::decoder::BitMatrixParser;
using zxing::pdf417::decoder::ec::ErrorCorrection;

namespace zxing {
namespace pdf417 {

class PDF417ScanningDecoder {
    static const int CODEWORD_SKEW_SIZE;
    static const int MAX_ERRORS;
    static const int MAX_EC_CODEWORDS;

    static const Ref<ErrorCorrection> errorCorrection;

    static Ref<DetectionResult> merge(
            Ref<DetectionResultRowIndicatorColumn> leftRowIndicatorColumn,
            Ref<DetectionResultRowIndicatorColumn> rightRowIndicatorColumn);

    static Ref<BoundingBox> adjustBoundingBox(
            Ref<DetectionResultRowIndicatorColumn> rowIndicatorColumn);

    static int getMax(ArrayRef<int> values);

    static Ref<BarcodeMetadata> getBarcodeMetadata(
            Ref<DetectionResultRowIndicatorColumn> leftRowIndicatorColumn,
            Ref<DetectionResultRowIndicatorColumn> rightRowIndicatorColumn);

    static Ref<DetectionResultRowIndicatorColumn> getRowIndicatorColumn(
            Ref<BitMatrix> image, Ref<BoundingBox> boundingBox,
            Ref<ResultPoint> startPoint, bool leftToRight, int minCodewordWidth,
            int maxCodewordWidth);

    static void adjustCodewordCount(Ref<DetectionResult> detectionResult,
            ArrayRef<ArrayRef<Ref<BarcodeValue> > > barcodeMatrix);

    static Ref<DecoderResult> createDecoderResult(
            Ref<DetectionResult> detectionResult);

    /**
     * This method deals with the fact, that the decoding process doesn't always yield a single most likely value. The
     * current error correction implementation doesn't deal with erasures very well, so it's better to provide a value
     * for these ambiguous codewords instead of treating it as an erasure. The problem is that we don't know which of
     * the ambiguous values to choose. We try decode using the first value, and if that fails, we use another of the
     * ambiguous values and try to decode again. This usually only happens on very hard to read and decode barcodes,
     * so decoding the normal barcodes is not affected by this.
     *
     * @param erasureArray contains the indexes of erasures
     * @param ambiguousIndexes array with the indexes that have more than one most likely value
     * @param ambiguousIndexValues two dimensional array that contains the ambiguous values. The first dimension must
     * be the same length as the ambiguousIndexes array
     */
    static Ref<DecoderResult> createDecoderResultFromAmbiguousValues(int ecLevel,
            ArrayRef<int> codewords, ArrayRef<int> erasureArray,
            ArrayRef<int> ambiguousIndexes,
            ArrayRef<ArrayRef<int> > ambiguousIndexValues);

    static ArrayRef<ArrayRef<Ref<BarcodeValue> > > createBarcodeMatrix(
            Ref<DetectionResult> detectionResult);

    static bool isValidBarcodeColumn(Ref<DetectionResult> detectionResult,
            int barcodeColumn);

    static int getStartColumn(Ref<DetectionResult> detectionResult, int barcodeColumn,
            int imageRow, bool leftToRight);

    static Ref<Codeword> detectCodeword(Ref<BitMatrix> image, int minColumn,
            int maxColumn, bool leftToRight, int startColumn, int imageRow,
            int minCodewordWidth, int maxCodewordWidth);

    static ArrayRef<int> getModuleBitCount(Ref<BitMatrix> image, int minColumn,
            int maxColumn, bool leftToRight, int startColumn, int imageRow);

    static int getNumberOfECCodeWords(int barcodeECLevel);

    static int adjustCodewordStartColumn(Ref<BitMatrix> image, int minColumn,
            int maxColumn, bool leftToRight, int codewordStartColumn,
            int imageRow);

    static bool checkCodewordSkew(int codewordSize, int minCodewordWidth,
            int maxCodewordWidth);

    static Ref<DecoderResult> decodeCodewords(ArrayRef<int> codewords, int ecLevel,
            ArrayRef<int> erasures);

    /**
     * <p>Given data and error-correction codewords received, possibly corrupted by errors, attempts to
     * correct the errors in-place.</p>
     *
     * @param codewords   data and error correction codewords
     * @param erasures positions of any known erasures
     * @param numECCodewords number of error correction codewords that are available in codewords
     * @throws ChecksumException if error correction fails
     */
    static int correctErrors(ArrayRef<int> codewords, ArrayRef<int> erasures,
            int numECCodewords);

    /**
     * Verify that all is OK with the codeword array.
     */
    static void verifyCodewordCount(ArrayRef<int> codewords, int numECCodewords);

    static ArrayRef<int> getBitCountForCodeword(int codeword);

    static int getCodewordBucketNumber(int codeword);

    static int getCodewordBucketNumber(ArrayRef<int> moduleBitCount);

    Ref<String> toString(ArrayRef<ArrayRef<BarcodeValue> > barcodeMatrix);
public:
    PDF417ScanningDecoder();
    virtual ~PDF417ScanningDecoder();

    // TODO don't pass in minCodewordWidth and maxCodewordWidth, pass in barcode columns for start and stop pattern
    // columns. That way width can be deducted from the pattern column.
    // This approach also allows to detect more details about the barcode, e.g. if a bar type (white or black) is wider
    // than it should be. This can happen if the scanner used a bad blackpoint.
    static Ref<DecoderResult> decode(Ref<BitMatrix> image,
            Ref<ResultPoint> imageTopLeft, Ref<ResultPoint> imageBottomLeft,
            Ref<ResultPoint> imageTopRight, Ref<ResultPoint> imageBottomRight,
            int minCodewordWidth, int maxCodewordWidth);

};

} /* namespace pdf417 */
} /* namespace zxing */

#endif /* CORE_SRC_ZXING_PDF417_DECODER_PDF417SCANNINGDECODER_H_ */
