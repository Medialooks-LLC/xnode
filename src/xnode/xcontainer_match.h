#pragma once

#include "../xcontainer/xcontainer.h"
#include "xkey/xkey.h"

#include <cassert>

namespace xsdk {

class IContainerMatch {
public:
    virtual ~IContainerMatch() = default;

    virtual IContainer::KeyType ContainerKey(const XKey& _key, bool _sequntial_index) const = 0;
    virtual XKey                NodeKey(const IContainer::KeyType& _key) const              = 0;
    virtual IContainer*         ContainerGet()                                              = 0;
    virtual const IContainer*   ContainerGet() const                                        = 0;
};

} // namespace xsdk
