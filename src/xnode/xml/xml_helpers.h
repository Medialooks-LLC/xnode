#pragma once

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

namespace xsdk::impl {

template <typename TXmlObject>
struct XmlObjectDeleter {
    void operator()(TXmlObject* _p)
    {
        if (_p)
            _p->release();
    }
};

template <typename TXmlObject>
using XmlUniquePtr = std::unique_ptr<TXmlObject, XmlObjectDeleter<TXmlObject>>;

class XmlPlatformManager {
public:
    static XmlPlatformManager& GetInstance();

    bool Init();

    XmlPlatformManager(const XmlPlatformManager& _p_other)           = delete;
    XmlPlatformManager operator=(const XmlPlatformManager& _p_other) = delete;

private:
    XmlPlatformManager();
    ~XmlPlatformManager();

    std::atomic<bool> was_init_before_;
    std::mutex        mutex_;
};

} // namespace xsdk::impl