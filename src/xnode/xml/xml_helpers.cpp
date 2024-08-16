#include "xml_helpers.h"

#include <assert.h>
#include <thread>

#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/util/XMLString.hpp"

namespace XC = XERCES_CPP_NAMESPACE;

namespace xsdk::impl {

XmlPlatformManager::XmlPlatformManager() {}

XmlPlatformManager::~XmlPlatformManager()
{
    if (was_init_before_)
        XC::XMLPlatformUtils::Terminate();
}

bool XmlPlatformManager::Init()
{
    std::lock_guard lock(mutex_);
    if (!was_init_before_) {
        try {
            XC::XMLPlatformUtils::Initialize();
            was_init_before_ = true;
        }
        catch (const XC::XMLException&) {
            return false;
        }
    }
    return was_init_before_;
}

XmlPlatformManager& XmlPlatformManager::GetInstance()
{
    static XmlPlatformManager s_instance;
    return s_instance;
}

} // namespace xsdk::impl