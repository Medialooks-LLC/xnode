#pragma once

#include "xnode_symbols.h"
#include "xnode_interfaces.h"

#include <functional>

namespace xsdk::xnode {

using OnNodeSubtreeChangePF = std::function<void(INode::CallbackReason _reason,
                                                 const INode::SPtrC&   _node,
                                                 const XKey&           _key,
                                                 const XValueRT&       _prev_value,
                                                 const XValueRT&       _new_value)>;

class XNODE_API INodeSubtreeSubscription: public xbase::PtrBase<INodeSubtreeSubscription> {
public:
    virtual ~INodeSubtreeSubscription() = default;

    /**
     * @brief Removes all subtree callbacks owned by this subscription.
     */
    virtual void Stop() = 0;
    /**
     * @brief Returns true while at least one node in the subtree is watched.
     */
    virtual bool Active() const = 0;
};

struct NodeSubtreeSubscriptionParams {
    /**
     * @brief Root node to watch.
     */
    INode::SPtr node;
    /**
     * @brief Callback invoked for visible changes in the watched mutable subtree.
     * @details The callback is called synchronously from INode callbacks. It must not modify the same node that is
     *          currently dispatching the callback because that can recursively lock the node container.
     *
     * Known limitations inherited from current INode callbacks:
     * - INode::Clear() and internal parent detachment do not emit OnChange callbacks.
     * - Const child nodes inserted with NodeConstInsert() are not watched by this mutable-subtree subscription.
     * - Concurrent changes inside a subtree while it is being detached can still be reported until Stop().
     */
    OnNodeSubtreeChangePF on_change;
    /**
     * @brief If true, subscribes to current child nodes recursively before returning from factory.
     */
    bool subscribe_existing_nodes = true;
};

XNODE_API INodeSubtreeSubscription::SPtr CreateNodeSubtreeSubscription(NodeSubtreeSubscriptionParams&& _params);

} // namespace xsdk::xnode
