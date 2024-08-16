#include "xcontainer_map_w_erase.h"

namespace xsdk::impl {

// 2Think: Check erased values ?
bool XContainerMapWithErase::Empty() const { return Size() == 0; }

size_t XContainerMapWithErase::Size() const
{
    assert(erased_values_ <= XContainerMap::Size());
    return XContainerMap::Size() - erased_values_;
}

// Method: return 'false' if empty or key not found
// Callback: return 'true' for stop enumeration
bool XContainerMapWithErase::ForPatch(std::function<bool(const KeyType&, const MappedType&)>&& _pf_on_item,
                                      const std::optional<KeyType>&                            _from_key) const
{
    return XContainerMap::ForEach(std::move(_pf_on_item), _from_key);
}

// Method: return 'false' if empty or key not found
// Callback: return 'true' for stop enumeration
bool XContainerMapWithErase::ForEach(std::function<bool(const KeyType&, const MappedType&)>&& _pf_on_item,
                                     const std::optional<KeyType>&                            _from_key) const
{
    return XContainerMap::ForEach(
        [&_pf_on_item](const KeyType& key, const MappedType& val) {
            if (IsErasedValue_(val))
                return false;

            if (!_pf_on_item)
                return true;

            return _pf_on_item(key, val);
        },
        _from_key);
}

// Return 'false' if empty or key not found
bool XContainerMapWithErase::ForEach(std::function<OnEachRes(const KeyType&, MappedType&)>&& _pf_on_each,
                                     const std::optional<KeyType>&                           _from_key,
                                     const OnChangePF&                                       _pf_on_change)
{
    bool found = false;
    XContainerMap::ForEach(
        [&](const KeyType& key, MappedType& val) {
            if (IsErasedValue_(val)) {
                // Check for erased lookup value
                if (!found && _from_key.has_value() && _from_key.value() == key)
                    return OnEachRes::Stop;

                return OnEachRes::Next;
            }

            found = true;

            if (!_pf_on_each)
                return OnEachRes::Stop;

            auto each_res = _pf_on_each(key, val);
            if (each_res == OnEachRes::Erase || each_res == OnEachRes::EraseStop) {
                if (!_pf_on_change || _pf_on_change(key, val, MappedType())) {
                    // For erasing -> replace via empty value
                    val = ErasedValue_();
                }

                return each_res == OnEachRes::EraseStop ? OnEachRes::Stop : OnEachRes::Next;
            }

            assert(each_res == OnEachRes::Stop || each_res == OnEachRes::Next);
            return each_res;
        },
        _from_key,
        DetectErase_(_pf_on_change));

    return found;
}

std::pair<bool, IContainer::MappedType> XContainerMapWithErase::Set(const KeyType&    _key,
                                                                    const MappedType& _val,
                                                                    const OnChangePF& _pf_on_change)
{
    return XContainerMap::Set(_key, _val, DetectErase_(_pf_on_change));
}

std::pair<bool, IContainer::MappedType> XContainerMapWithErase::Set(const KeyType&    _key,
                                                                    MappedType&&      _val,
                                                                    const OnChangePF& _pf_on_change)
{
    return XContainerMap::Set(_key, std::move(_val), DetectErase_(_pf_on_change));
}

IContainer::EmplaceRes XContainerMapWithErase::Emplace(const KeyType&    _key,
                                                       const MappedType& _val,
                                                       const OnChangePF& _pf_on_change)
{
    return XContainerMap::Emplace(_key, _val, DetectErase_(_pf_on_change));
}

IContainer::EmplaceRes XContainerMapWithErase::Emplace(const KeyType&    _key,
                                                       MappedType&&      _val,
                                                       const OnChangePF& _pf_on_change)
{
    return XContainerMap::Emplace(_key, std::move(_val), DetectErase_(_pf_on_change));
}

std::optional<IContainer::MappedType> XContainerMapWithErase::Erase(const KeyType&    _key,
                                                                    const OnChangePF& _pf_on_change)
{
    auto [success, value] = XContainerMap::Set(_key, ErasedValue_(), DetectErase_(_pf_on_change));
    if (!success)
        return std::nullopt;

    return value;
}

void XContainerMapWithErase::Clear()
{
    XContainerMap::Clear();
    erased_values_ = 0;
}

IContainer::OnChangePF XContainerMapWithErase::DetectErase_(const OnChangePF& _pf_on_change)
{
    return [&](const KeyType& key, const MappedType& from, const MappedType& to) {
        bool allowed = !_pf_on_change || _pf_on_change(key, from, to);
        if (allowed && !IsErasedValue_(from) && IsErasedValue_(to)) {
            ++erased_values_;
        }
        // VVB: orignaly check !from.TimeIsAbsent() - not remember why...
        // if (allowed && !from && !from.TimeIsAbsent() && to) {
        else if (allowed && IsErasedValue_(from) && !IsErasedValue_(to)) {
            assert(erased_values_ > 0);
            --erased_values_;
        }
        return allowed;
    };
}

} // namespace xsdk::impl