/*
 * PDF417ResultMetadata.h
 *
 *  Created on: Jun 10, 2016
 *      Author: binhnk
 */

#ifndef CORE_SRC_ZXING_PDF417_PDF417RESULTMETADATA_H_
#define CORE_SRC_ZXING_PDF417_PDF417RESULTMETADATA_H_

#include <zxing/common/Str.h>

namespace zxing {
namespace pdf417 {

class PDF417ResultMetadata : public Counted {
    int segmentIndex;
    Ref<String> fileId;
    ArrayRef<int> optionalData;
    bool lastSegment;

public:
    int getSegmentIndex() {
      return segmentIndex;
    }

    void setSegmentIndex(int segmentIndex) {
      this->segmentIndex = segmentIndex;
    }

    Ref<String> getFileId() {
      return fileId;
    }

    void setFileId(Ref<String> fileId) {
      this->fileId = fileId;
    }

    ArrayRef<int> getOptionalData() {
      return optionalData;
    }

    void setOptionalData(ArrayRef<int> optionalData) {
      this->optionalData = optionalData;
    }

    bool isLastSegment() {
      return lastSegment;
    }

    void setLastSegment(bool lastSegment) {
      this->lastSegment = lastSegment;
    }
};

} /* namespace pdf417 */
} /* namespace zxing */

#endif /* CORE_SRC_ZXING_PDF417_PDF417RESULTMETADATA_H_ */
