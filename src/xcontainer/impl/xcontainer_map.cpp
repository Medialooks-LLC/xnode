#include "xcontainer_map.h"

#include <cassert>

namespace xsdk::impl {

bool XContainerMap::IsKeyValid(const KeyType& _key) const { return !KeyToString_(_key).empty(); }

bool XContainerMap::Empty() const { return values_map_.empty(); }

size_t XContainerMap::Size() const { return values_map_.size(); }

// Return 'false' if empty or key not found
bool XContainerMap::ForEach(std::function<bool(const KeyType&, const MappedType&)>&& _pf_on_item,
                            const std::optional<KeyType>&                            _from_key) const
{
    auto it = values_map_.begin();
    if (_from_key.has_value())
        it = MapFind_(_from_key.value());

    if (it == values_map_.end())
        return false;

    while (_pf_on_item && it != values_map_.end()) {
        if (_pf_on_item(StringToKey_(it->first), it->second))
            break;

        ++it;
    }

    return true;
}

// Return 'false' if empty or key not found
bool XContainerMap::ForEach(std::function<OnEachRes(const KeyType&, MappedType&)>&& _pf_on_each,
                            const std::optional<KeyType>&                           _from_key,
                            const OnChangePF&                                       _pf_on_change)
{
    auto it = values_map_.begin();
    if (_from_key.has_value())
        it = MapFind_(_from_key.value()).first;

    if (it == values_map_.end())
        return false;

    if (_pf_on_each) {
        while (it != values_map_.end()) {
            MappedType val = it->second; // For detect chnaging
            auto       res = _pf_on_each(StringToKey_(it->first), val);
            if (res == OnEachRes::Erase || res == OnEachRes::EraseStop) {
                if (!_pf_on_change || _pf_on_change(StringToKey_(it->first), it->second, MappedType())) {
                    assert(val == it->second);
                    it = values_map_.erase(it);
                }
                else {
                    ++it;
                }
            }
            else {
                if (val != it->second && (!_pf_on_change || _pf_on_change(StringToKey_(it->first), it->second, val))) {

                    it->second = val;
                }

                ++it;
            }

            if (res == OnEachRes::Stop || res == OnEachRes::EraseStop)
                break;
        }
    }

    return true;
}

std::optional<IContainer::MappedType> XContainerMap::At(const KeyType& _key) const
{
    auto it = MapFind_(_key);
    if (it == values_map_.end())
        return std::nullopt;

    return it->second;
}

std::pair<bool, IContainer::MappedType> XContainerMap::Set(const KeyType&    _key,
                                                           const MappedType& _val,
                                                           const OnChangePF& _pf_on_change)
{
    auto [it, key] = MapFind_(_key);
    if (it == values_map_.end() && key.empty())
        return {false, MappedType()}; // 2think about res

    if (ValueAt_(it) == _val)
        return {false, ValueAt_(it)}; // 2think about res

    if (_pf_on_change && !_pf_on_change(_key, ValueAt_(it), _val))
        return {false, ValueAt_(it)}; // 2think about res

    if (it == values_map_.end()) {
        values_map_.emplace(key, _val);
        return {true, MappedType()};
    }

    return {true, std::exchange(it->second, _val)};
}

std::pair<bool, IContainer::MappedType> XContainerMap::Set(const KeyType&    _key,
                                                           MappedType&&      _val,
                                                           const OnChangePF& _pf_on_change)
{
    auto [it, key] = MapFind_(_key);
    if (it == values_map_.end() && key.empty())
        return {false, MappedType()}; // 2think about res

    if (ValueAt_(it) == _val)
        return {false, ValueAt_(it)}; // 2think about res

    if (_pf_on_change && !_pf_on_change(_key, ValueAt_(it), _val))
        return {false, ValueAt_(it)}; // 2think about res

    if (it == values_map_.end()) {
        values_map_.emplace(key, _val);
        return {true, MappedType()};
    }

    return {true, std::exchange(it->second, std::move(_val))};
}

IContainer::EmplaceRes XContainerMap::Emplace(const KeyType&    _key,
                                              const MappedType& _val,
                                              const OnChangePF& _pf_on_change)
{
    auto [it, key] = MapFind_(_key);
    if (key.empty() || it != values_map_.end())
        return {false, _key, ValueAt_(it)}; // Add result description

    if (_pf_on_change && !_pf_on_change(_key, ValueAt_(it), _val))
        return {false, _key, MappedType()}; // Add result description

    values_map_.emplace(key, _val);
    return {true, StringToKey_(key), MappedType() /*_val*/};
}

IContainer::EmplaceRes XContainerMap::Emplace(const KeyType& _key, MappedType&& _val, const OnChangePF& _pf_on_change)
{
    auto [it, key] = MapFind_(_key);
    if (key.empty() || it != values_map_.end())
        return {false, _key, ValueAt_(it)}; // Add result description

    if (_pf_on_change && !_pf_on_change(_key, ValueAt_(it), _val))
        return {false, _key, MappedType()}; // Add result description

    it = values_map_.emplace(key, std::move(_val)).first;
    return {true, StringToKey_(key), MappedType() /*it->second*/};
}

std::optional<IContainer::MappedType> XContainerMap::Erase(const KeyType& _key, const OnChangePF& _pf_on_change)
{
    auto [it, key] = MapFind_(_key);
    if (it == values_map_.end())
        return std::nullopt;

    if (_pf_on_change && !_pf_on_change(_key, it->second, MappedType()))
        return std::nullopt; // 2think about res

    auto nh = values_map_.extract(it);
    return nh.mapped();
}

void XContainerMap::Clear() { values_map_.clear(); }

inline const IContainer::MappedType& XContainerMap::ValueAt_(
    const typename std::map<std::string, MappedType>::const_iterator& _it) const
{
    static const MappedType empty;
    return _it != values_map_.end() ? _it->second : empty;
}

} // namespace xsdk::impl