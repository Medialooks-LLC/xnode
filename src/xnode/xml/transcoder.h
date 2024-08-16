#pragma once
#include <iostream>
#include <vector>
#include <memory>

#include "xercesc/util/TransService.hpp"

namespace XC = XERCES_CPP_NAMESPACE;

namespace xsdk::impl {

class Transcoder {

public:
    Transcoder();

    std::unique_ptr<std::vector<XMLCh>> ToXmlChars(const char* const _chars) const;

    std::string ToString(const XMLCh* const _chars);

private:
    XC::XMLTransService::Codes         fail_reason_;
    const size_t                       max_utf8_symbol_size_ = 4;
    std::unique_ptr<XC::XMLTranscoder> utf8_transcoder_;
    std::unique_ptr<XMLCh[]>           unicode_;
};

} // namespace xsdk::impl