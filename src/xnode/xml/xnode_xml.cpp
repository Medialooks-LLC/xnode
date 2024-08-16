#include "xnode_xml.h"

#include "sax_handler.h"
#include "transcoder.h"
#include "xml_creator.h"
#include "xml_helpers.h"
#include "xnode_interfaces.h"
#include "xnode_factory.h"
#include "xnode_functions.h"

#include <atomic>
#include <iostream>

#include "xercesc/dom/DOMLSSerializer.hpp"
#include "xercesc/framework/MemBufFormatTarget.hpp"
#include "xercesc/framework/MemBufInputSource.hpp"
#include "xercesc/sax2/SAX2XMLReader.hpp"
#include "xercesc/sax2/XMLReaderFactory.hpp"
#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/util/XMLString.hpp"

namespace XC = XERCES_CPP_NAMESPACE;

namespace xsdk {

bool xnode::XmlPlatformInit() { return impl::XmlPlatformManager::GetInstance().Init(); }

std::pair<INode::SPtr, size_t> xnode::FromXml(std::string_view _xml,
                                              uint64_t         _uid,
                                              std::string_view _name,
                                              std::string_view _attribute_prefix,
                                              std::string_view _value_name)
{
    if (_xml.empty())
        return {nullptr, -1};

    if (!impl::XmlPlatformManager::GetInstance().Init())
        return {nullptr, -2};

    INode::SPtr res     = nullptr;
    size_t      err_pos = 0;

    std::unique_ptr<XC::SAX2XMLReader> parser {XC::XMLReaderFactory::createXMLReader()};
    parser->setFeature(XC::XMLUni::fgSAX2CoreValidation, true);
    parser->setFeature(XC::XMLUni::fgSAX2CoreNameSpaces, true); // optional

    auto handler = std::make_unique<impl::XNodeSaxHandler>(_uid, _name, _attribute_prefix, _value_name);
    parser->setContentHandler(handler.get());
    parser->setErrorHandler(handler.get());

    try {
        XC::MemBufInputSource xml_buf((XMLByte*)_xml.data(), _xml.size() * sizeof(_xml[0]), "xml (in memory)");
        parser->parse(xml_buf);
    }
    catch (const XC::XMLException& to_catch) {
        err_pos = to_catch.getSrcLine();
    }
    catch (const XC::SAXParseException& to_catch) {
        err_pos = to_catch.getLineNumber() * 1000 + to_catch.getColumnNumber(); //??
    }

    if (err_pos == 0)
        res = handler->root.QueryPtr<INode>();

    return {res, err_pos};
}

std::string xnode::ToXml(const INode::SPtrC& _node_this,
                         xnode::OnCopyPF&    _pf_on_item,
                         xnode::XmlFormat    _xml_format,
                         std::string_view    _attribute_prefix,
                         std::string_view    _value_name,
                         size_t              _indent_char_count,
                         char                _indent_char)
{
    if (!_node_this)
        return {};

    if (!impl::XmlPlatformManager::GetInstance().Init())
        return {};

    std::string res = {};

    auto creator = std::make_unique<impl::XmlDocCreator>(_node_this, _attribute_prefix, _value_name);
    auto doc     = creator->GetDocument();

    XMLCh dom_impl_name[3];
    XC::XMLString::transcode("LS", dom_impl_name, 2);
    XC::DOMImplementationLS* impl = (XC::DOMImplementationLS*)XC::DOMImplementationRegistry::getDOMImplementation(
        dom_impl_name);
    if (impl) {
        impl::XmlUniquePtr<XC::DOMLSSerializer> serializer {impl->createLSSerializer()};
        if (_xml_format == XmlFormat::kPretty) {
            if (serializer->getDomConfig()->canSetParameter(XC::XMLUni::fgDOMWRTFormatPrettyPrint, true))
                serializer->getDomConfig()->setParameter(XC::XMLUni::fgDOMWRTFormatPrettyPrint, true);
            if (serializer->getDomConfig()->canSetParameter(XC::XMLUni::fgDOMWRTXercesPrettyPrint, false))
                serializer->getDomConfig()->setParameter(XC::XMLUni::fgDOMWRTXercesPrettyPrint, false);
        }

        auto                                mem_buffer = std::make_shared<XC::MemBufFormatTarget>();
        impl::XmlUniquePtr<XC::DOMLSOutput> output {impl->createLSOutput()};
        output->setByteStream(mem_buffer.get());
        try {
            serializer->write(doc, output.get());
            auto buffer = mem_buffer->getRawBuffer();
            auto len    = mem_buffer->getLen();
            res         = std::string((char*)buffer, len);
        }
        catch (const XC::XMLException&) {
        }
        catch (const XC::DOMException&) {
        }
    }

    return res;
}

} // namespace xsdk