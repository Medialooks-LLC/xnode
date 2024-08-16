#pragma once

#include "../xcontainer_match.h"
#include "xkey/xkey.h"

#include <cassert>

namespace xsdk::impl {

// Base impl class
class XContainerMatchBase: public IContainerMatch {

    std::unique_ptr<IContainer> container_p_;

public:
    XContainerMatchBase(std::unique_ptr<IContainer>&& _container_p) : container_p_(std::move(_container_p))
    {
        assert(container_p_);
    }

public:
    virtual IContainer::KeyType ContainerKey(const XKey& _key, bool _sequntial_index) const override
    {
        return ContainerKey_(_key);
    }

    virtual XKey NodeKey(const IContainer::KeyType& _key) const override { return NodeKey_(_key); }

    virtual IContainer* ContainerGet() override { return container_p_.get(); }

    virtual const IContainer* ContainerGet() const override { return container_p_.get(); }

private:
    static XKey NodeKey_(const IContainer::KeyType& _key);

    static IContainer::KeyType ContainerKey_(const XKey& _key);
};

// Map mathching
class XContainerMatchMap final: public XContainerMatchBase {
public:
    using XContainerMatchBase::XContainerMatchBase;

public:
    virtual IContainer::KeyType ContainerKey(const XKey& _key, bool _sequntial_index) const override;
};

// Array mathching (copy of XContainerMatchBase)
class XContainerMatchArray final: public XContainerMatchBase {
public:
    using XContainerMatchBase::XContainerMatchBase;
};

} // namespace xsdk::impl
