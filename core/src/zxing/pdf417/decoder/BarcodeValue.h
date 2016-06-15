/*
 * BarcodeValue.h
 *
 *  Created on: Jun 13, 2016
 *      Author: binhnk
 */

#ifndef CORE_SRC_ZXING_PDF417_DECODER_BARCODEVALUE_H_
#define CORE_SRC_ZXING_PDF417_DECODER_BARCODEVALUE_H_

#include <map>
#include <string>
#include <zxing/common/Array.h>
using namespace std;

namespace zxing {
namespace pdf417 {

using namespace std;

class BarcodeValue : public Counted {
    map<int, int> values;

public:
    /**
     * Add an occurrence of a value
     */
    void setValue(int value);

    /**
     * Determines the maximum occurrence of a set value and returns all values which were set with this occurrence.
     * @return an array of int, containing the values with the highest occurrence, or null, if no value was set
     */
    ArrayRef<int> getValue();

    int getConfidence(int value);
};

} /* namespace pdf417 */
} /* namespace zxing */

#endif /* CORE_SRC_ZXING_PDF417_DECODER_BARCODEVALUE_H_ */
