#include "xnode_impl.h"

#include <deque>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace xsdk::impl {

XNode::XNode(std::unique_ptr<IContainerMatch>&&  _container_match,
             std::unique_ptr<IParentValidator>&& _parent_validator,
             uint64_t                            _uid,
             std::string_view                    _name)
    : object_uid_(_uid),
      container_match_p_(std::move(_container_match)),
      parent_validator_p_(std::move(_parent_validator))
{
    assert(parent_validator_p_.get());
    assert(container_match_p_.get());
    assert(ContainerGet_());
    if (!_name.empty())
        name_p_ = std::make_unique<std::string>(_name);

#ifdef _DEBUG
    nodes_counter_.fetch_add(1);
#endif
}

//-------------------------------------------------------------------------------
// IObject override

std::any XNode::QueryPtr(xbase::Uid _type_query)
{
    if (_type_query == xbase::TypeUid<INodePrivate>())
        return std::static_pointer_cast<INodePrivate>(shared_from_this());
    
    if (_type_query == xbase::TypeUid<INode>())
        return std::static_pointer_cast<INode>(shared_from_this());

     if (_type_query == xbase::TypeUid<IObject>())
        return std::static_pointer_cast<IObject>(shared_from_this());

    return {};
}

std::any XNode::QueryPtrC(xbase::Uid _type_query) const
{
    if (_type_query == xbase::TypeUid<const INodePrivate>())
        return std::static_pointer_cast<const INodePrivate>(shared_from_this());
   
    if (_type_query == xbase::TypeUid<const INode>())
        return std::static_pointer_cast<const INode>(shared_from_this());

    if (_type_query == xbase::TypeUid<const IObject>())
        return std::static_pointer_cast<const IObject>(shared_from_this());

    return {};
}

INode::NodeType XNode::Type() const
{
    if (ContainerGet_()->Type() == IContainer::ContainerType::Array)
        return NodeType::Array;

    assert(ContainerGet_()->Type() == IContainer::ContainerType::Map);
    return NodeType::Map;
}

INode::SPtr XNode::ParentGet()
{
    std::shared_lock lck(parent_n_name_rw_);

    return parent_wp_.lock();
}

INode::SPtrC XNode::ParentGet() const
{
    std::shared_lock lck(parent_n_name_rw_);

    return parent_wp_.lock();
}

std::pair<bool, INode::SPtr> XNode::ParentSet(INode::SPtr                     _parent,
                                              std::optional<std::string_view> _name_for_new_parent /*= std::nullopt*/)
{
    auto [success, parent_prev] = PrivateParentSet(_parent, _name_for_new_parent);
    if (!success)
        return {success, parent_prev};

    // Remove from previous parent
    auto parent_prev_private = xobject::PtrQuery<INodePrivate>(parent_prev.get());
    if (parent_prev_private)
        parent_prev_private->PrivateErase(NodeThis_());

    // Add to new parent
    auto parent_private = xobject::PtrQuery<INodePrivate>(_parent.get());
    if (parent_private) {
        if (_parent->Type() == NodeType::Map)
            parent_private->PrivateSet(NameGet(), NodeThis_());
        else
            parent_private->PrivateInsert(kIdxEnd, NodeThis_());
    }

    return {true, parent_prev};
}

INode::SPtr XNode::ParentDetach() { return ParentSet(nullptr).second; }

std::string XNode::NameGet() const
{
    std::shared_lock lck(parent_n_name_rw_);
    return name_p_ ? *name_p_ : std::string();
}

std::pair<bool, std::string> XNode::NameSet(std::string_view _name_set, bool _update_parent)
{
    std::unique_lock lck(parent_n_name_rw_);

    // check for same name
    if (_name_set == (name_p_ ? *name_p_ : ""))
        return {true, std::string(_name_set)};

    if (!name_p_) {
        if (!_name_set.empty())
            name_p_ = std::make_unique<std::string>(_name_set);

        return {true, {}};
    }

    auto parent_sp = parent_wp_.lock();
    if (parent_sp && parent_sp->Type() == NodeType::Map) {

        if (!_update_parent)
            return {false, *name_p_};

        auto name = *name_p_;
        lck.unlock();

        return {parent_sp->KeyChange(name, _name_set), name};
    }

    auto existed_name = std::exchange(*name_p_, std::string(_name_set));
    if (name_p_->empty())
        name_p_.reset();

    return {true, existed_name};
}

bool XNode::IsName(std::string_view _name_check) const
{
    std::shared_lock lck(parent_n_name_rw_);

    if (_name_check.empty())
        return name_p_ ? false : true;

    return name_p_ ? _name_check == std::string_view(*name_p_) : false;
}

bool XNode::KeyChange(const XKey& _from, const XKey& _to)
{
    if (!_from || !_to || _from.Type() != _to.Type())
        return false;

    std::unique_lock lck(container_rw_);

    auto key_to = ContainerKey_(_to, false);
    if (ContainerGet_()->At(key_to).has_value())
        return false;

    auto moved_val = ContainerGet_()->Erase(ContainerKey_(_from, true), OnChangePF_());
    if (!moved_val.has_value())
        return false;

    auto [is_set, prev] = ContainerGet_()->Set(key_to, moved_val.value(), OnChangePF_(true));
    assert(is_set && prev.IsEmpty());

    lck.unlock();

    if (_to.Type() == XKey::KeyType::String) {
        auto node_private = moved_val.value().QueryPtr<INodePrivate>();
        if (node_private)
            node_private->PrivateNameSet(_to.StringGet().value());
    }

    return true;
}

//-------------------------------------------------------------------------------
// Callbacks, return uid for subsiqent remove this cb

uint64_t XNode::OnChangeAdd(OnChangePF&& _pf_on_change, uint64_t _id /*= 0*/) const
{
    return node_callbacks_.OnChangeAdd(std::move(_pf_on_change), _id);
}
bool   XNode::OnChangeRemove(uint64_t _id) const { return node_callbacks_.OnChangeRemove(_id); }
size_t XNode::OnChangeReset() const { return node_callbacks_.OnChangeReset(); }

//-------------------------------------------------------------------------------
bool XNode::IsKeyValid(bool _map_access_by_index, const XKey& _key) const
{
    if (_map_access_by_index) {
        std::shared_lock lck(container_rw_);
        return ContainerGet_()->IsKeyValid(ContainerKey_(_key, true));
    }

    return ContainerGet_()->IsKeyValid(ContainerKey_(_key, false));
}

void XNode::Clear()
{
    std::unique_lock lck(container_rw_);

    std::vector<INode::SPtr> vec_removed_nodes;

    // Collect nodes
    ContainerGet_()->ForEach([&](const IContainer::KeyType&, const IContainer::MappedType& val) {
        auto node_remove_p = val.QueryPtr<INode>();
        if (node_remove_p)
            vec_removed_nodes.emplace_back(std::move(node_remove_p));
        return false;
    });

    ContainerGet_()->Clear();

    lck.unlock();

    for (const auto& node_remove_p : vec_removed_nodes)
        node_remove_p->ParentDetach();
}

size_t   XNode::Size() const { return ContainerGet_()->Size(); }
bool     XNode::Empty() const { return ContainerGet_()->Empty(); }
XValueRT XNode::At(const XKey& _key) const
{
    return MakeConst_(const_cast<XNode*>(this)->At(_key));
}
XValueRT XNode::At(const XKey& _key)
{
    std::shared_lock lck(container_rw_);

    return ContainerGet_()->At(ContainerKey_(_key, true)).value_or(XValueRT());
}

bool XNode::ForPatch(std::function<bool(const XKey&, const XValueRT&)>&& _pf_on_item,
                     const XKey&                                         _from_key /*= XKey()*/) const
{
    std::unique_lock lck(container_rw_);

    std::function<bool(const IContainer::KeyType&, const IContainer::MappedType&)> pf_on_item;
    if (_pf_on_item) {
        pf_on_item = [&](const IContainer::KeyType& _key, const IContainer::MappedType& _val) -> auto {
            return _pf_on_item(NodeKey_(_key), _val);
        };
    }

    return ContainerGet_()->ForPatch(
        std::move(pf_on_item),
        _from_key ? std::optional<IContainer::KeyType>(ContainerKey_(_from_key, true)) : std::nullopt);
}

// Method for take, Erase, change items via callback
bool XNode::ForEach(std::function<OnEachRes(const XKey&, XValueRT&)>&& _pf_on_item, const XKey& _from_key /*= XKey()*/)
{
    std::vector<INode::SPtr>                                 vec_removed_nodes;
    std::vector<std::pair<INode::SPtr, IContainer::KeyType>> vec_set_nodes;

    std::function<OnEachRes(const IContainer::KeyType&, IContainer::MappedType&)> pf_on_item;
    if (_pf_on_item) {
        pf_on_item = [&](const IContainer::KeyType& _key, IContainer::MappedType& _val) -> auto {
            auto prev_val = _val;
            auto key      = NodeKey_(_key);
            auto cb_res   = _pf_on_item(key, _val);

            // Detect nodes changes
            if (cb_res == OnEachRes::Erase || cb_res == OnEachRes::EraseStop) {
                auto node_remove_p = prev_val.QueryPtr<INode>();
                if (node_remove_p)
                    vec_removed_nodes.emplace_back(std::move(node_remove_p));
            }
            else if (_val != prev_val) {
                // Do not allow for chanage to nodes !!!
                auto node_set_p = _val.QueryPtr<INode>();
                if (node_set_p) {
                    vec_set_nodes.emplace_back(std::move(node_set_p), _key);
                }

                auto node_remove_p = prev_val.QueryPtr<INode>();
                if (node_remove_p)
                    vec_removed_nodes.emplace_back(std::move(node_remove_p));
            }

            return cb_res;
        };
    }

    std::unique_lock lck(container_rw_);

    auto result = ContainerGet_()->ForEach(
        std::move(pf_on_item),
        _from_key ? std::optional<IContainer::KeyType>(ContainerKey_(_from_key, true)) : std::nullopt,
        OnChangePF_());

    // Remove duplicated nodes for array
    for (const auto& [node_set_p, key] : vec_set_nodes)
        parent_validator_p_->RemoveDuplicates(ContainerGet_(), node_set_p, key);

    lck.unlock();

    for (const auto& node_remove_p : vec_removed_nodes)
        node_remove_p->ParentDetach();

    for (const auto& [node_set_p, key] : vec_set_nodes)
        SetAsChild_(node_set_p, NodeKey_(key).StringGet());

    return result;
}

std::pair<bool, XValueRT> XNode::Set(const XKey& _key, XValue&& _val)
{
    auto [is_valid, child_node] = IsValidChild_(_val);
    if (!is_valid)
        return {false, {}};

    std::unique_lock lck(container_rw_);

    auto key_set             = ContainerKey_(_key, true);
    auto [success, replaced] = ContainerGet_()->Set(key_set, std::move(_val), OnChangePF_());
    if (!success)
        return {false, replaced};

    if (child_node)
        parent_validator_p_->RemoveDuplicates(ContainerGet_(), child_node, key_set);

    lck.unlock();

    auto replaced_node = replaced.QueryPtr<INode>();
    assert(!child_node || replaced_node != child_node);
    if (replaced_node) {
        auto replaced_private = xobject::PtrQuery<INodePrivate>(replaced_node.get());
        replaced_private->PrivateParentSet(nullptr);
    }

    if (child_node)
        SetAsChild_(child_node, NodeKey_(key_set).StringGet());

    return {true, replaced};
}

INode::InsertRes XNode::Insert(const XKey& _key, XValue&& _val)
{
    auto [is_valid, child_node] = IsValidChild_(_val);
    if (!is_valid)
        return {false, {}};

    std::unique_lock lck(container_rw_);

    if (child_node) {
        auto [key_existed, val_existed] = parent_validator_p_->FindDuplicates(ContainerGet_(), child_node);
        if (!val_existed.IsEmpty())
            return {false, NodeKey_(key_existed), /*val_existed*/ XValueRT()};
    }

    auto [success, key, existed] = ContainerGet_()->Emplace(ContainerKey_(_key, false),
                                                           XValueRT(std::move(_val)),
                                                           OnChangePF_());
    if (!success)
        return {success, NodeKey_(key), existed};

    lck.unlock();

    assert(!existed.QueryPtr<INode>() || existed == child_node);

    if (child_node)
        SetAsChild_(child_node, NodeKey_(key).StringGet());

    return {true, NodeKey_(key), existed};
}

XValueRT XNode::Erase(const XKey& _key)
{
    std::unique_lock lck(container_rw_);

    auto erased_val = ContainerGet_()->Erase(ContainerKey_(_key, true), OnChangePF_()).value_or(XValueRT());

    lck.unlock();

    auto erased_node = erased_val.QueryPtr<INode>();
    if (erased_node)
        erased_node->ParentDetach();

    return erased_val;
}

XValueRT XNode::Append(const XKey& _key, std::string_view _append_str)
{
    std::unique_lock lck(container_rw_);

    XValue appended;
    if (ContainerGet_()->ForEach(
            [&](const auto& key, XValueRT& value) {
                appended = XValue(value.String() + std::string(_append_str));
                value    = appended;
                return OnEachRes::Stop;
            },
            ContainerKey_(_key, true),
            OnChangePF_())) {

        return appended;
    }

    auto [success, key_res, val] = ContainerGet_()->Emplace(ContainerKey_(_key, false), _append_str, OnChangePF_());
    assert(success);
    return success ? val : XValueRT();
}

XValueRT XNode::Increment(const XKey& _key, const XValue& _increment_val)
{
    std::unique_lock lck(container_rw_);

    XValue appended;
    if (ContainerGet_()->ForEach(
            [&](const auto& key, XValueRT& value) {
                if (_increment_val.Type() == XValue::kDouble)
                    appended = value.Double() + _increment_val.Double();
                else if (_increment_val.Type() == XValue::kUint64)
                    appended = value.Uint64() + _increment_val.Uint64();
                else
                    appended = value.Int64() + _increment_val.Int64();

                value = appended;
                return OnEachRes::Stop;
            },
            ContainerKey_(_key, true),
            OnChangePF_())) {
        return appended;
    }

    auto [success, key_res, val] = ContainerGet_()->Emplace(ContainerKey_(_key, false),
                                                           XValueRT(_increment_val),
                                                           OnChangePF_());
    assert(success);
    return success ? val : XValueRT();
}

std::pair<bool, XValueRT> XNode::CompareExchange(const XKey& _key, const XValue& _expected, XValue&& _exchange_to)
{
    std::unique_lock lck(container_rw_);

    auto node_set_p = _exchange_to.QueryPtr<INode>();

    auto key_for_exchange = ContainerKey_(_key, true);

    std::optional<XValueRT> value_other;
    bool                    found = ContainerGet_()->ForEach(
        [&](const IContainer::KeyType& key, IContainer::MappedType& value) {
            if (value == _expected && OnChangePF_()(key, value, _exchange_to))
                value = std::move(_exchange_to);
            else
                value_other.emplace(value);

            return OnEachRes::Stop;
        },
        key_for_exchange);

    // Not expected value
    if (value_other.has_value())
        return {false, value_other.value()};

    // Not found
    if (!found) {

        // Expect non-empty value
        if (!_expected.IsEmpty())
            return {false, XValueRT()};

        auto [ok, replaced] = ContainerGet_()->Set(key_for_exchange, std::move(_exchange_to), OnChangePF_());
        assert(ok && replaced.IsEmpty());
    }

    lck.unlock();

    if (node_set_p)
        SetAsChild_(node_set_p, NodeKey_(key_for_exchange).StringGet());

    return {true, _expected};
}

//---------------------------------------------------------------------------------------------
// bulk methods

std::vector<std::pair<XKey, XValueRT>> XNode::BulkGet(const std::vector<XKey>& _keys) const
{
    return BulkGet_(true, _keys);
}

std::vector<std::pair<XKey, XValueRT>> XNode::BulkGet(const std::vector<XKey>& _keys) { return BulkGet_(false, _keys); }

std::vector<std::pair<XKey, XValueRT>> XNode::BulkGetAll(
    std::function<OnCopyRes(const XKey&, const XValueRT&)>&& _pf_on_item,
    const XKey&                                              _key_begin) const
{
    return BulkGet_(true, _key_begin, std::move(_pf_on_item));
}

std::vector<std::pair<XKey, XValueRT>> XNode::BulkGetAll(
    std::function<OnCopyRes(const XKey&, const XValueRT&)>&& _pf_on_item,
    const XKey&                                              _key_begin)
{
    return BulkGet_(false, _key_begin, std::move(_pf_on_item));
}

size_t XNode::BulkSet(std::vector<std::pair<XKey, XValue>>&& _values)
{
    // Get wrong childs (out of mutex)
    std::set<XValue> invalid_childs;
    for (const auto& [key, val] : _values) {
        if (!IsValidChild_(val).first)
            invalid_childs.emplace(val);
    }

    std::unique_lock lck(container_rw_);

    std::vector<INode::SPtr>                   vec_replaced_nodes;
    std::map<INode::SPtr, IContainer::KeyType> map_set_nodes;

    size_t succeeded = 0;
    auto   it        = _values.begin();
    while (it != _values.end()) {
        // Check what child is valid (not cicled)
        if (invalid_childs.count(it->second)) {
            ++it;
            continue;
        }

        auto node_set_p = it->second.QueryPtr<INode>();
        if (node_set_p) {
            // Check duplicates in already set nodes
            auto it_dup = map_set_nodes.find(node_set_p);
            if (it_dup != map_set_nodes.end()) {
                auto res = ContainerGet_()->Erase(it_dup->second, OnChangePF_());
                if (res.has_value()) {
                    map_set_nodes.erase(it_dup);
                }
                else {
                    // Skip this node as can't remove previously setted
                    ++it;
                }
            }
        }

        // Do not call std::move(it->second) - for keep value in case of failed set
        auto key                 = ContainerKey_(it->first, true);
        auto [success, replaced] = ContainerGet_()->Set(key, it->second, OnChangePF_());
        if (!success) {
            ++it;
            continue;
        }

        auto replaced_node = replaced.QueryPtr<INode>();
        assert(!node_set_p || replaced_node != node_set_p);
        if (replaced_node)
            vec_replaced_nodes.emplace_back(std::move(replaced_node));
        if (node_set_p)
            map_set_nodes.emplace(std::move(node_set_p), std::move(key));

        it = _values.erase(it);
        ++succeeded;
    }

    // Remove duplicated nodes for array
    for (const auto& [node_set_p, key] : map_set_nodes)
        parent_validator_p_->RemoveDuplicates(ContainerGet_(), node_set_p, key);

    lck.unlock();

    for (const auto& replaced_node : vec_replaced_nodes)
        replaced_node->ParentDetach();

    for (const auto& [node_set_p, key] : map_set_nodes)
        SetAsChild_(node_set_p, NodeKey_(key).StringGet());

    return succeeded;
}

size_t XNode::BulkInsert(std::vector<std::pair<XKey, XValue>>&& _values)
{
    // Get wrong childs (out of mutex)
    std::set<XValue> invalid_childs;
    for (const auto& [key, val] : _values) {
        if (!IsValidChild_(val).first)
            invalid_childs.emplace(val);
    }

    std::unique_lock lck(container_rw_);

    std::map<INode::SPtr, IContainer::KeyType> map_inserted_nodes;

    size_t succeeded = 0;
    auto   it        = _values.begin();
    while (it != _values.end()) {
        // Check what child is valid (not cicled)
        if (invalid_childs.count(it->second)) {
            ++it;
            continue;
        }

        auto node_insert_p = it->second.QueryPtr<INode>();
        if (node_insert_p) {
            // Check duplicates in already inserted nodes
            if (map_inserted_nodes.count(node_insert_p)) {
                ++it;
                continue;
            }

            // Check duplicated nodes for array
            auto [key_dup, duplicated] = parent_validator_p_->FindDuplicates(ContainerGet_(), node_insert_p);
            if (!duplicated.IsEmpty()) {
                it->first = NodeKey_(key_dup);
                ++it;
                continue;
            }
        }

        auto [success, key, existed] = ContainerGet_()->Emplace(ContainerKey_(it->first, false),
                                                               std::move(it->second),
                                                               OnChangePF_());
        if (!success) {
            it->second = existed;
            ++it;
            continue;
        }

        if (node_insert_p)
            map_inserted_nodes.emplace(std::move(node_insert_p), std::move(key));

        it = _values.erase(it);
        ++succeeded;
    }

    lck.unlock();

    for (const auto& [node_insert_p, key] : map_inserted_nodes) {
        node_insert_p->ParentDetach();
        SetAsChild_(node_insert_p, NodeKey_(key).StringGet());
    }

    return succeeded;
}

std::pair<size_t, XKey> XNode::BulkInsert(XKey _insert_pos, std::vector<XValue>&& _values)
{
    if (_insert_pos.Type() == XKey::KeyType::String)
        return {0, _insert_pos};

    if (ContainerGet_()->Type() != IContainer::ContainerType::Array)
        return {0, XKey()};

    // Get wrong childs (out of mutex)
    std::set<XValue> invalid_childs;
    for (const auto& val : _values) {
        if (!IsValidChild_(val).first)
            invalid_childs.emplace(val);
    }

    std::unique_lock lck(container_rw_);

    std::vector<INode::SPtr> vec_inserted_nodes;

    size_t inserted   = 0;
    size_t insert_pos = _insert_pos.IndexGet().value_or(kIdxEnd);

    IContainer::EmplaceRes EmplaceRes;
    auto                   it = _values.begin();
    while (it != _values.end()) {
        // Check what child is valid (not cicled)
        if (invalid_childs.count(*it)) {
            ++it;
            continue;
        }

        // Check duplicated nodes for array
        auto node_insert_p         = it->QueryPtr<INode>();
        auto [key_dup, duplicated] = parent_validator_p_->FindDuplicates(ContainerGet_(), node_insert_p);
        if (!duplicated.IsEmpty()) {
            ++it;
            continue;
        }

        // Do not call std::move(*it) - for keep value in case of failed emplace (e.g. cb canceled)
        EmplaceRes = ContainerGet_()->Emplace(insert_pos, *it, OnChangePF_());
        if (!EmplaceRes.succeeded) {
            ++it;
            continue;
        }

        if (node_insert_p)
            vec_inserted_nodes.emplace_back(std::move(node_insert_p));

        if (insert_pos != kIdxEnd && insert_pos != kIdxLast)
            ++insert_pos;

        ++inserted;
        it = _values.erase(it);
    }

    lck.unlock();

    for (const auto& node_insert_p : vec_inserted_nodes)
        SetAsChild_(node_insert_p);

    return {inserted, NodeKey_(EmplaceRes.inserted_at)};
}

std::vector<std::pair<XKey, XValueRT>> XNode::BulkErase(const std::vector<XKey>& _keys)
{
    std::unique_lock lck(container_rw_);

    // Convert to container keys (for keep index)
    std::vector<std::pair<XKey, IContainer::KeyType>> keys;
    for (const auto& key : _keys)
        keys.emplace_back(key, ContainerKey_(key, true));

    // Remove from container
    std::vector<std::pair<XKey, XValueRT>> extracted;
    for (const auto& [node_key, container_key] : keys) {
        auto val_op = ContainerGet_()->Erase(container_key);
        if (val_op.has_value()) {
            extracted.emplace_back(node_key, val_op.value());
        }
    }

    lck.unlock();

    // Update parents
    for (const auto& [key, val] : extracted) {
        auto node_extracted_p = val.QueryPtr<INode>();
        if (node_extracted_p)
            node_extracted_p->ParentDetach();
    }

    return extracted;
}

//----------------------------------------------------------------------------------------------
// INodePrivate

std::string XNode::PrivateNameSet(std::string_view _name_set)
{
    std::unique_lock lck(parent_n_name_rw_);

    // check for same name
    if (_name_set == (name_p_ ? *name_p_ : ""))
        return std::string(_name_set);

    if (!name_p_) {
        if (!_name_set.empty())
            name_p_ = std::make_unique<std::string>(_name_set);

        return std::string();
    }

    auto existed_name = std::exchange(*name_p_, std::string(_name_set));
    if (name_p_->empty())
        name_p_.reset();

    return existed_name;
}

std::pair<bool, INode::SPtr> XNode::PrivateParentSet(const INode::SPtr&              _parent,
                                                     std::optional<std::string_view> _name_for_new_parent)
{
    if (!IsValidParent_(_parent))
        return {false, _parent};

    std::unique_lock lck(parent_n_name_rw_);

    auto parent_prev_sp = parent_wp_.lock();
    if (parent_prev_sp == _parent)
        return {false, parent_prev_sp};

    // Check for valid map key
    auto node_name = _name_for_new_parent.value_or(name_p_ ? *name_p_ : std::string_view {});
    if (_parent && _parent->Type() == NodeType::Map && node_name.empty())
        return {false, nullptr};

    // Update name
    if (_name_for_new_parent.has_value())
        name_p_ = node_name.empty() ? nullptr : std::make_unique<std::string>(node_name);

    parent_wp_ = _parent;

    return {true, parent_prev_sp};
}

XKey XNode::PrivateErase(const INode::SPtr& _node_p)
{
    assert(_node_p);

    std::unique_lock lck(container_rw_);

    auto name = _node_p->NameGet();
    if (!name.empty() && ContainerGet_()->Erase(name).has_value())
        return name;

    XKey erased_key;
    ContainerGet_()->ForEach([&](const auto& key, auto& val) {
        if (val == _node_p) {
            erased_key = NodeKey_(key);
            return OnEachRes::EraseStop;
        }
        return OnEachRes::Next;
    });

    return erased_key;
}

std::pair<bool, XValueRT> XNode::PrivateSet(const XKey& _key, XValue&& _val)
{
    std::unique_lock lck(container_rw_);

    auto node_set_p = _val.QueryPtr<INode>();

    auto key_set             = ContainerKey_(_key, true);
    auto [success, replaced] = ContainerGet_()->Set(key_set, std::move(_val), OnChangePF_());
    if (!success)
        return {success, replaced};

    parent_validator_p_->RemoveDuplicates(ContainerGet_(), node_set_p, key_set);

    return {success, replaced};
}

INode::InsertRes XNode::PrivateInsert(const XKey& _key, XValue&& _val)
{
    auto node_insert_p = _val.QueryPtr<INode>();

    std::unique_lock lck(container_rw_);

    if (node_insert_p) {
        auto [key_existed, val_existed] = parent_validator_p_->FindDuplicates(ContainerGet_(), node_insert_p);
        if (!val_existed.IsEmpty())
            return {false, NodeKey_(key_existed), /*val_existed*/ XValueRT()};
    }

    auto [success, key, existed] = ContainerGet_()->Emplace(ContainerKey_(_key, false),
                                                           XValueRT(std::move(_val)),
                                                           OnChangePF_());
    return {success, NodeKey_(key), existed};
}

//---------------------------------------------------------------------------------------------
// Private helpers

/*static*/ XValueRT XNode::MakeConst_(XValueRT&& _val) {
    if (_val.Type() == XValue::kObject)
        return XValueRT(_val.ObjectPtrC(), _val.Timestamp());

    return std::move(_val);
}

/*static*/ XValueRT XNode::MakeConst_(const XValueRT& _val)
{
    if (_val.Type() == XValue::kObject)
        return XValueRT(_val.ObjectPtrC(), _val.Timestamp());

    return _val;
}

inline IContainer::OnChangePF XNode::OnChangePF_(bool _no_discard)
{
    return [=](const IContainer::KeyType& _key, const IContainer::MappedType& _from, const IContainer::MappedType& _to)
               -> auto { return node_callbacks_.DoCallbacks(NodeThis_(), NodeKey_(_key), _from, _to, _no_discard); };
}

inline IContainer::KeyType XNode::ContainerKey_(const XKey& _key, bool _use_index) const
{
    return container_match_p_->ContainerKey(_key, _use_index);
}

inline XKey XNode::NodeKey_(const IContainer::KeyType& _key) const { return container_match_p_->NodeKey(_key); }

bool XNode::IsValidParent_(const INode::SPtrC& _node_parent) const
{
    // We can't set parent node for whom we are ancestor(grad-parent)
    auto node_check = _node_parent;
    while (node_check) {
        if (node_check == NodeThis_()) {
            assert(!"cicle child node");
            return false;
        }

        node_check = node_check->ParentGet();
        assert(node_check != _node_parent);
        if (node_check == _node_parent)
            return false; // Got cyclic parents
    }
    return true;
}

std::pair<bool, INode::SPtr> XNode::IsValidChild_(const XValue& _check) const
{
    auto node_child_c = _check.QueryPtrC<INode>();
    if (!node_child_c)
        return {true, nullptr};

    auto node_child = _check.QueryPtr<INode>();

    // We can't add child node which are ancestor(grad-parent) for us
    auto node_check = NodeThis_();
    while (node_check) {
        if (node_check == node_child_c) {
            assert(!"cicle child node");
            return {false, node_child};
        }

        node_check = node_check->ParentGet();
        assert(node_check != NodeThis_());
        if (node_check == NodeThis_())
            return {false, node_child}; // Got cyclic parents
    }
    return {true, node_child};
}

bool XNode::SetAsChild_(const INode::SPtr& _node_child, std::optional<std::string_view> _child_name)
{
    auto child_private = xobject::PtrQuery<INodePrivate>(_node_child.get());
    if (!child_private)
        return false;

    // Set new parent
    auto [success, child_parent_prev] = child_private->PrivateParentSet(NodeThis_(), _child_name);
    if (!success)
        return false;

    // Remove from previous parent
    auto child_parent_private = xobject::PtrQuery<INodePrivate>(child_parent_prev.get());
    if (child_parent_private)
        child_parent_private->PrivateErase(_node_child);

    return true;
}

std::vector<std::pair<XKey, XValueRT>> XNode::BulkGet_(bool _read_only, const std::vector<XKey>& _keys) const
{
    std::shared_lock lck(container_rw_);

    std::vector<std::pair<XKey, XValueRT>> values;
    for (const auto& key : _keys) {
        auto val_op = ContainerGet_()->At(ContainerKey_(key, true));
        if (val_op.has_value())
            values.emplace_back(key, _read_only ? MakeConst_(val_op.value()) : val_op.value());
    }

    return values;
}

std::vector<std::pair<XKey, XValueRT>> XNode::BulkGet_(
    bool                                                     _read_only,
    const XKey&                                              _key_begin,
    std::function<OnCopyRes(const XKey&, const XValueRT&)>&& _pf_on_item) const
{
    std::shared_lock lck(container_rw_);

    std::vector<std::pair<XKey, XValueRT>> values;
    ContainerGet_()->ForEach(
        [&](const auto& key, const auto& value) {
            XValueRT value_take = _read_only ? MakeConst_(value) : value;
            auto     cb_res     = _pf_on_item ? _pf_on_item(NodeKey_(key), value_take) : OnCopyRes::Take;

            if (cb_res == OnCopyRes::TakeStop || cb_res == OnCopyRes::Take)
                values.emplace_back(NodeKey_(key), value_take);

            return (cb_res == OnCopyRes::TakeStop || cb_res == OnCopyRes::Stop) ? true : false;
        },
        _key_begin ? std::optional<IContainer::KeyType>(ContainerKey_(_key_begin, true)) : std::nullopt);

    return values;
}

} // namespace xsdk::impl