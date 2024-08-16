#pragma once

#include "../xcontainer_factory.h"

#include <cassert>
#include <memory>
#include <string>

namespace xsdk::impl {

class XContainerFactory final: public IContainerFactory, public std::enable_shared_from_this<XContainerFactory> {

    XContainerFactory() = default;

public:
    static std::shared_ptr<IContainerFactory> create();

public:
    virtual std::unique_ptr<IContainer> ContainerCreate(IContainer::ContainerType _type,
                                                        bool                      _erase_detection) override;
};

} // namespace xsdk::impl