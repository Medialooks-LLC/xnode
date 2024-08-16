#pragma once

#include "xnode_factory.h"

#include <memory>
#include <string>
#include <cassert>

namespace xsdk::impl {

class XNodeFactory final: public INodeFactory, public std::enable_shared_from_this<XNodeFactory> {

    XNodeFactory() = default;

public:
    static std::shared_ptr<INodeFactory> create();

public:
    virtual INode::SPtr NodeCreate(INode::NodeType _type, std::string_view _name, uint64_t _uid) override;
};

} // namespace xsdk::impl