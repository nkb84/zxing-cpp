/*
 * PDF417CodewordDecoder.cpp
 *
 *  Created on: Jun 13, 2016
 *      Author: binhnk
 */

#include <zxing/pdf417/decoder/PDF417CodewordDecoder.h>
#include <zxing/common/detector/MathUtils.h>

#include <cfloat>
using zxing::common::detector::MathUtils;

namespace zxing {
namespace pdf417 {

vector<vector<float>> PDF417CodewordDecoder::RATIOS_TABLE = vector<vector<float>>(BitMatrixParser::SYMBOL_TABLE_LENGTH, vector<float>(BitMatrixParser::BARS_IN_MODULE));
bool PDF417CodewordDecoder::tableInit = false;

PDF417CodewordDecoder::PDF417CodewordDecoder() {
}

PDF417CodewordDecoder::~PDF417CodewordDecoder() {
}

void PDF417CodewordDecoder::initRatiosTable() {
    if (PDF417CodewordDecoder::tableInit) {
        return;
    }

    // Pre-computes the symbol ratio table.
    for (int i = 0; i < BitMatrixParser::SYMBOL_TABLE_LENGTH; i++) {
        int currentSymbol = BitMatrixParser::SYMBOL_TABLE[i];
        int currentBit = currentSymbol & 0x1;
        for (int j = 0; j < BitMatrixParser::BARS_IN_MODULE; j++) {
            float size = 0.0f;
            while ((currentSymbol & 0x1) == currentBit) {
                size += 1.0f;
                currentSymbol >>= 1;
            }
            currentBit = currentSymbol & 0x1;
            RATIOS_TABLE[i][BitMatrixParser::BARS_IN_MODULE - j - 1] = size
                    / BitMatrixParser::MODULES_IN_CODEWORD;
        }
    }

    PDF417CodewordDecoder::tableInit = true;
}

int PDF417CodewordDecoder::getDecodedValue(ArrayRef<int> moduleBitCount) {
    int decodedValue = getDecodedCodewordValue(sampleBitCounts(moduleBitCount));
    if (decodedValue != -1) {
        return decodedValue;
    }
    return getClosestDecodedValue(moduleBitCount);
}

ArrayRef<int> PDF417CodewordDecoder::sampleBitCounts(ArrayRef<int> moduleBitCount) {
    float bitCountSum = MathUtils::sum(moduleBitCount);
    ArrayRef<int> result(BitMatrixParser::BARS_IN_MODULE);
    int bitCountIndex = 0;
    int sumPreviousBits = 0;
    for (int i = 0; i < BitMatrixParser::MODULES_IN_CODEWORD; i++) {
        float sampleIndex =
        bitCountSum / (2 * BitMatrixParser::MODULES_IN_CODEWORD) +
        (i * bitCountSum) / BitMatrixParser::MODULES_IN_CODEWORD;
        if (sumPreviousBits + moduleBitCount[bitCountIndex] <= sampleIndex) {
            sumPreviousBits += moduleBitCount[bitCountIndex];
            bitCountIndex++;
        }
        result[bitCountIndex]++;
    }
    return result;
}

int PDF417CodewordDecoder::getDecodedCodewordValue(ArrayRef<int> moduleBitCount) {
    int decodedValue = getBitValue(moduleBitCount);
    return BitMatrixParser::getCodeword(decodedValue) == -1 ? -1 : decodedValue;
}

int PDF417CodewordDecoder::getBitValue(ArrayRef<int> moduleBitCount) {
    long result = 0;
    for (int i = 0; i < moduleBitCount->size(); i++) {
        for (int bit = 0; bit < moduleBitCount[i]; bit++) {
            result = (result << 1) | (i % 2 == 0 ? 1 : 0);
        }
    }
    return (int) result;
}

int PDF417CodewordDecoder::getClosestDecodedValue(ArrayRef<int> moduleBitCount) {
    initRatiosTable();
    int bitCountSum = MathUtils::sum(moduleBitCount);
    vector<float> bitCountRatios(BitMatrixParser::BARS_IN_MODULE);
    for (int i = 0; i < bitCountRatios.size(); i++) {
        bitCountRatios[i] = moduleBitCount[i] / (float) bitCountSum;
    }
    float bestMatchError = FLT_MAX;
    int bestMatch = -1;
    for (int j = 0; j < RATIOS_TABLE.size(); j++) {
        float error = 0.0f;
        vector<float> ratioTableRow = RATIOS_TABLE[j];
        for (int k = 0; k < BitMatrixParser::BARS_IN_MODULE; k++) {
            float diff = ratioTableRow[k] - bitCountRatios[k];
            error += diff * diff;
            if (error >= bestMatchError) {
                break;
            }
        }
        if (error < bestMatchError) {
            bestMatchError = error;
            bestMatch = BitMatrixParser::SYMBOL_TABLE[j];
        }
    }
    return bestMatch;
}

} /* namespace pdf417 */
} /* namespace zxing */
