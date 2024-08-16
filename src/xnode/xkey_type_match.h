#pragma once

#include "xkey/xkey.h"
#include "xnode_interfaces.h"

namespace xsdk {

// 2Think ? temp place !!!
class XKeyToNodeType {
public:
    static std::optional<INode::NodeType> Match(const XKey& _key)
    {
        switch (_key.Type()) {
            case XKey::KeyType::Empty:
                return std::nullopt;

            case XKey::KeyType::Index:
                return INode::NodeType::Array;

            case XKey::KeyType::String:
                return INode::NodeType::Map;
        }

        assert(!"Wrong XKey Type");
        return std::nullopt;
    }
};

} // namespace xsdk
