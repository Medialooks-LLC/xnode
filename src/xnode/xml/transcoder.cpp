#include "transcoder.h"

#include <iostream>
#include <memory>
#include <vector>

namespace XC = XERCES_CPP_NAMESPACE;

namespace xsdk::impl {

Transcoder::Transcoder()
    : utf8_transcoder_ {XC::XMLPlatformUtils::fgTransService->makeNewTranscoderFor("UTF-8", fail_reason_, 16)}
{
}

std::unique_ptr<std::vector<XMLCh>> Transcoder::ToXmlChars(const char* const _chars) const
{
    auto      len        = strlen(_chars);
    std::vector<XMLCh> unicode(len + 1);
    auto      char_sizes = std::make_unique<unsigned char[]>(len);
    XMLSize_t processed;
    auto      res_len = utf8_transcoder_
                       ->transcodeFrom((const XMLByte*)_chars, len, unicode.data(), len, processed, char_sizes.get());
    assert(processed == len);
    assert(XC::XMLTransService::Codes::Ok == fail_reason_);
    unicode[res_len] = 0;
    unicode.resize(res_len + 1);
    return std::make_unique < std::vector<XMLCh>>(unicode);
}

std::string Transcoder::ToString(const XMLCh* const _chars)
{
    auto      len     = XC::XMLString::stringLen(_chars);
    auto      max_len = len * max_utf8_symbol_size_;
    std::string utf8;
    utf8.resize(max_len);
    XMLSize_t processed;
    auto      res_len = utf8_transcoder_->transcodeTo(_chars,
                                                 len,
                                                 reinterpret_cast<XMLByte*>(utf8.data()),
                                                 max_len,
                                                 processed,
                                                 XC::XMLTranscoder::UnRep_Throw);
    assert(processed == len);
    assert(XC::XMLTransService::Codes::Ok == fail_reason_);
    utf8.resize(res_len);
    return utf8;
}

} // namespace xsdk::impl