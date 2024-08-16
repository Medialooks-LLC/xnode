#pragma once

#include "xbase/xobject.h"

#include <memory>
#include <string>
#include <cassert>

namespace xsdk::impl {

class XObjectImpl: public IObject, public std::enable_shared_from_this<XObjectImpl> {

    const uint64_t uid_;

public:
    XObjectImpl() : uid_(xbase::NextUid()) {}

    virtual uint64_t ObjectUid() const override { return uid_; };
    virtual std::any QueryPtr(xbase::Uid _type_query) override;
    virtual std::any QueryPtrC(xbase::Uid _type_query) const override;
};


} // namespace xsdk::impl