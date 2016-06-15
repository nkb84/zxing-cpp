/*
 * BarcodeValue.cpp
 *
 *  Created on: Jun 13, 2016
 *      Author: binhnk
 */

#include <zxing/pdf417/decoder/BarcodeValue.h>

namespace zxing {
namespace pdf417 {

void BarcodeValue::setValue(int value) {
    int confidence = 0;
    if (values.find(value) == values.end()) {
        confidence = 0;
    } else {
        confidence = values[value];
    }
    confidence++;
    values[value] = confidence;
}

ArrayRef<int> BarcodeValue::getValue() {
    Array<int>* result = new Array<int>();
    int maxConfidence = -1;
    for (map<int, int>::iterator it = values.begin(); it != values.end();
            it++) {
        if (it->second > maxConfidence) {
            maxConfidence = it->second;
            result->values().clear();
            result->values().push_back(it->first);
        } else if (it->second == maxConfidence) {
            result->values().push_back(it->first);
        }
    }
    return ArrayRef<int>(result);
}

int BarcodeValue::getConfidence(int value) {
    return values.at(value);
}

} /* namespace pdf417 */
} /* namespace zxing */
