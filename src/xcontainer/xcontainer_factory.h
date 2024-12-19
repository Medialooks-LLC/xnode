#pragma once

#include "xcontainer.h"

#include <map>
#include <string>

namespace xsdk {

class IContainerFactory {
public:
    // 2Think: use custom container type + type match ?
    virtual std::unique_ptr<IContainer> ContainerCreate(IContainer::ContainerType _type, bool _erase_detection) = 0;
};

IContainerFactory* XContainerFactoryGet();

} // namespace xsdk