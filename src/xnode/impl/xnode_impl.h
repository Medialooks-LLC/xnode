#pragma once

#include "../xcontainer_match.h"
#include "../xparent_validator.h"

#include "xnode_callbacks.h"

#include "xnode_interfaces.h"

#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace xsdk::impl {

// Private methods for set w/o affect on childs/parents relations
class INodePrivate {
public:
    virtual ~INodePrivate() = default;

    // Do not change keys in parent's map
    virtual std::string PrivateNameSet(std::string_view _name_set) = 0;

    // Do not add node to new parent child list and remove from previous parent childs
    virtual std::pair<bool, INode::SPtr> PrivateParentSet(
        const INode::SPtr&              _parent,
        std::optional<std::string_view> _name_for_new_parent = std::nullopt) = 0;

    // Do not change erased node parent
    virtual XKey PrivateErase(const INode::SPtr& _node_p) = 0;

    // Do not change parent of added and replaced node (if _val is node)
    virtual std::pair<bool, XValueRT> PrivateSet(const XKey& _key, XValue&& _val) = 0;

    // Do not change parent of inserted node (if _val is node)
    virtual INode::InsertRes PrivateInsert(const XKey& _key, XValue&& _val) = 0;
};

class XNode final: public INode, public INodePrivate, public std::enable_shared_from_this<XNode> {

    const uint64_t                          object_uid_;
    mutable std::shared_mutex               container_rw_;
    const std::unique_ptr<IContainerMatch>  container_match_p_;
    const std::unique_ptr<IParentValidator> parent_validator_p_;

    mutable std::shared_mutex    parent_n_name_rw_;
    std::weak_ptr<INode>         parent_wp_;
    std::unique_ptr<std::string> name_p_; // for reduce footprint (?)

    mutable XNodeCallbacks node_callbacks_;
#ifdef _DEBUG
    inline static std::atomic<int64_t> nodes_counter_;
#endif

    XNode(std::unique_ptr<IContainerMatch>&&  _container_match,
          std::unique_ptr<IParentValidator>&& _parent_validator,
          uint64_t                            _uid,
          std::string_view                    _name);
public:
    static std::shared_ptr<XNode> Create(std::unique_ptr<IContainerMatch>&&  _container_match,
                                               std::unique_ptr<IParentValidator>&& _parent_validator,
                                               uint64_t                            _uid,
                                               std::string_view                    _name)
    {
        return std::shared_ptr<XNode> {new XNode(std::move(_container_match), std::move(_parent_validator), _uid, _name)};
    }

#ifdef _DEBUG
    virtual ~XNode() { nodes_counter_.fetch_sub(1); }

    static int64_t counter() { return nodes_counter_.load(); }
#endif

    //-------------------------------------------------------------------------------
    // IObject override
    virtual uint64_t ObjectUid() const override { return object_uid_; }
    virtual std::any QueryPtr(xbase::Uid _type_query) override;
    virtual std::any QueryPtrC(xbase::Uid _type_query) const override;

    //-------------------------------------------------------------------------------
    // INode specific methods

    virtual NodeType Type() const override;

    // Parents
    virtual INode::SPtr                  ParentGet() override;
    virtual INode::SPtrC                 ParentGet() const override;
    virtual std::pair<bool, INode::SPtr> ParentSet(
        INode::SPtr                     _parent_p,
        std::optional<std::string_view> _name_for_new_parent = std::nullopt) override;
    virtual INode::SPtr ParentDetach() override;

    // Names
    virtual std::string                  NameGet() const override;
    virtual std::pair<bool, std::string> NameSet(std::string_view _name_set, bool _update_parent) override;
    virtual bool                         IsName(std::string_view _name_check) const override;

    // Replace key by another one
    virtual bool KeyChange(const XKey& _from, const XKey& _to) override;

    //-------------------------------------------------------------------------------
    // Callbacks, return uid for subsiqent remove this cb
    virtual uint64_t OnChangeAdd(OnChangePF&& _pf_on_change, uint64_t _id /*= 0*/) const override;
    virtual bool     OnChangeRemove(uint64_t _id) const override;
    virtual size_t   OnChangeReset() const override;

    //-------------------------------------------------------------------------------
    // IContainer direct related methods
    virtual bool     IsKeyValid(bool _map_access_by_index, const XKey& _key) const override;
    virtual void     Clear() override;
    virtual size_t   Size() const override;
    virtual bool     Empty() const override;
    virtual XValueRT At(const XKey& _key) const override;
    virtual XValueRT At(const XKey& _key) override;

    // Return all items include erased
    virtual bool ForPatch(std::function<bool(const XKey&, const XValueRT&)>&& _pf_on_item,
                          const XKey&                                         _from_key) const override;

    // Method for take, Erase, change items via callback
    virtual bool ForEach(std::function<OnEachRes(const XKey&, XValueRT&)>&& _pf_on_item,
                         const XKey&                                        _from_key) override;

    // Return {success, current value}
    virtual std::pair<bool, XValueRT> Set(const XKey& _key, XValue&& _val) override;

    // Return {succcess, key, existed value (if failed to Insert into map)}
    virtual InsertRes Insert(const XKey& _key, XValue&& _val) override;

    // Return {success, current value (failed to emplace)}
    virtual XValueRT Erase(const XKey& _key) override;

    //-------------------------------------------------------------------------------
    // IContainer atomic modification methods
    // Return resulting value
    virtual XValueRT Append(const XKey& _key, std::string_view _append_str) override;
    virtual XValueRT Increment(const XKey& _key, const XValue& _increment_val) override;
    // Return {success, previus value}  // 2Discuss
    virtual std::pair<bool, XValueRT> CompareExchange(const XKey&   _key,
                                                      const XValue& _expected,
                                                      XValue&&      _exchange_to) override;
    //-------------------------------------------------------------------------------
    // IContainer bulk/helpers methods

    virtual std::vector<std::pair<XKey, XValueRT>> BulkGet(const std::vector<XKey>& _keys) const override;
    virtual std::vector<std::pair<XKey, XValueRT>> BulkGet(const std::vector<XKey>& _keys) override;
    virtual std::vector<std::pair<XKey, XValueRT>> BulkGetAll(
        std::function<OnCopyRes(const XKey&, const XValueRT&)>&& _pf_on_item,
        const XKey&                                              _key_begin) const override;
    virtual std::vector<std::pair<XKey, XValueRT>> BulkGetAll(
        std::function<OnCopyRes(const XKey&, const XValueRT&)>&& _pf_on_item,
        const XKey&                                              _key_begin) override;

    // Return {number of succeeded, vector of failed {key, mapped} }
    // virtual std::pair<size_t, key_value_vec> BulkSet(const key_value_vec& _values) override { return {};}
    // Return number of succeeded (removed from _values), failed keeped in _values
    virtual size_t BulkSet(std::vector<std::pair<XKey, XValue>>&& _values) override;
    // Return {number of succeeded, vector of failed emplace() result (usually current mapped value)}
    // virtual std::pair<size_t, key_value_vec> BulkInsert(const key_value_vec& _values) override { return {};}
    // Return number of succeeded (removed from _values), for fail - emplace() result (usually current) put into _values
    virtual size_t BulkInsert(std::vector<std::pair<XKey, XValue>>&& _values) override;

    // For arrays only
    virtual std::pair<size_t, XKey> BulkInsert(XKey _insert_pos, std::vector<XValue>&& _values) override;

    // Return vector of extracted values
    virtual std::vector<std::pair<XKey, XValueRT>> BulkErase(const std::vector<XKey>& _keys) override;

    //----------------------------------------------------------------------------------------------
    // INodePrivate

    virtual std::string PrivateNameSet(std::string_view _name_set) override;

    virtual std::pair<bool, INode::SPtr> PrivateParentSet(
        const INode::SPtr&              _parent,
        std::optional<std::string_view> _name_for_new_parent = std::nullopt) override;

    virtual XKey PrivateErase(const INode::SPtr& _node_p) override;

    virtual std::pair<bool, XValueRT> PrivateSet(const XKey& _key, XValue&& _val) override;

    virtual InsertRes PrivateInsert(const XKey& _key, XValue&& _val) override;

private:
    // Const conversions
    static XValueRT MakeConst_(XValueRT&& _val);
    static XValueRT MakeConst_(const XValueRT& _val);

    // Basic helpers
    INode::SPtr       NodeThis_() { return std::static_pointer_cast<INode>(shared_from_this()); }
    INode::SPtrC      NodeThis_() const { return std::static_pointer_cast<const INode>(shared_from_this()); }
    IContainer*       ContainerGet_() { return container_match_p_->ContainerGet(); }
    const IContainer* ContainerGet_() const { return container_match_p_->ContainerGet(); }

    // Callback helper
    IContainer::OnChangePF OnChangePF_(bool _no_discard = false);

    // Key conversions
    IContainer::KeyType ContainerKey_(const XKey& _key, bool _use_index) const;
    XKey                NodeKey_(const IContainer::KeyType& _key) const;

    // Parent's check
    bool                         IsValidParent_(const INode::SPtrC& _node_parent) const;
    std::pair<bool, INode::SPtr> IsValidChild_(const XValue& _check) const;
    bool SetAsChild_(const INode::SPtr& _node_child, std::optional<std::string_view> _child_name = std::nullopt);

    // Bulk helpers
    std::vector<std::pair<XKey, XValueRT>> BulkGet_(bool _read_only, const std::vector<XKey>& _keys) const;
    std::vector<std::pair<XKey, XValueRT>> BulkGet_(
        bool                                                     _read_only,
        const XKey&                                              _key_begin,
        std::function<OnCopyRes(const XKey&, const XValueRT&)>&& _pf_on_item) const;
};

} // namespace xsdk::impl