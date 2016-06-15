/*
 * PDF417CodewordDecoder.h
 *
 *  Created on: Jun 13, 2016
 *      Author: binhnk
 */

#ifndef CORE_SRC_ZXING_PDF417_DECODER_PDF417CODEWORDDECODER_H_
#define CORE_SRC_ZXING_PDF417_DECODER_PDF417CODEWORDDECODER_H_

#include <vector>
#include <zxing/pdf417/decoder/BitMatrixParser.h>
using namespace std;

using zxing::pdf417::decoder::BitMatrixParser;

namespace zxing {
namespace pdf417 {

class PDF417CodewordDecoder {
    static vector<vector<float>> RATIOS_TABLE;
    static bool tableInit;

    static void initRatiosTable();

public:

    static int getDecodedValue(ArrayRef<int> moduleBitCount);

    static ArrayRef<int> sampleBitCounts(ArrayRef<int> moduleBitCount);

    static int getDecodedCodewordValue(ArrayRef<int> moduleBitCount);

    static int getBitValue(ArrayRef<int> moduleBitCount);

    static int getClosestDecodedValue(ArrayRef<int> moduleBitCount);

    PDF417CodewordDecoder();
    virtual ~PDF417CodewordDecoder();
};

} /* namespace pdf417 */
} /* namespace zxing */

#endif /* CORE_SRC_ZXING_PDF417_DECODER_PDF417CODEWORDDECODER_H_ */
