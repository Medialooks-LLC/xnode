#include "xcontainer_array.h"

#include <cassert>

namespace xsdk::impl {

bool XContainerArray::IsKeyValid(const KeyType& _key) const { return KeyToIndex_(_key).has_value(); }

bool XContainerArray::Empty() const { return values_deq_.empty(); }

size_t XContainerArray::Size() const { return values_deq_.size(); }

// Return 'false' if empty or key not found
bool XContainerArray::ForEach(std::function<bool(const KeyType&, const MappedType&)>&& _pf_on_item,
                              const std::optional<KeyType>&                            _from_key) const
{
    auto it = values_deq_.begin();
    if (_from_key.has_value())
        it = DeqFind_(_from_key.value());

    if (it == values_deq_.end())
        return false;

    size_t idx = 0;
    while (_pf_on_item && it != values_deq_.end()) {
        if (_pf_on_item(IndexToKey_(idx++), *it))
            break;

        ++it;
    }

    return true;
}

// Return 'false' if empty or key not found
bool XContainerArray::ForEach(std::function<OnEachRes(const KeyType&, MappedType&)>&& _pf_on_each,
                              const std::optional<KeyType>&                           _from_key,
                              const OnChangePF&                                       _pf_on_change)
{
    auto it = values_deq_.begin();
    if (_from_key.has_value())
        it = DeqFind_(_from_key.value(), false);

    if (it == values_deq_.end())
        return false;

    if (_pf_on_each) {
        size_t idx = 0;
        while (it != values_deq_.end()) {
            MappedType val = *it; // For detect chnaging
            auto       res = _pf_on_each(KeyType(idx++), val);
            if ((res == OnEachRes::Erase || res == OnEachRes::EraseStop) &&
                (!_pf_on_change || _pf_on_change(IndexToKey_(idx), *it, MappedType()))) {
                assert(val == *it);
                it = values_deq_.erase(it);
            }
            else {
                if (val != *it && (!_pf_on_change || _pf_on_change(IndexToKey_(idx), *it, val)))
                    *it = val;

                ++it;
            }

            if (res == OnEachRes::Stop || res == OnEachRes::EraseStop)
                break;
        }
    }

    return true;
}

std::optional<IContainer::MappedType> XContainerArray::At(const KeyType& _key) const
{
    auto it = DeqFind_(_key);
    if (it == values_deq_.end())
        return std::nullopt;

    return *it;
}

std::pair<bool, IContainer::MappedType> XContainerArray::Set(const KeyType&    _key,
                                                             const MappedType& _val,
                                                             const OnChangePF& _pf_on_change)
{
    // todo: fill array to index
    auto it = DeqFind_(_key, true);
    if (it == values_deq_.end())
        return {false, MappedType()}; // 2think about res

    if (ValueAt_(it) == _val)
        return {false, *it}; // 2think about res

    if (_pf_on_change && !_pf_on_change(_key, ValueAt_(it), _val))
        return {false, ValueAt_(it)}; // 2think about res

    return {true, std::exchange(*it, _val)};
}

std::pair<bool, IContainer::MappedType> XContainerArray::Set(const KeyType&    _key,
                                                             MappedType&&      _val,
                                                             const OnChangePF& _pf_on_change)
{
    // todo: fill array to index
    auto it = DeqFind_(_key, true);
    if (it == values_deq_.end())
        return {false, MappedType()}; // 2think about res

    if (ValueAt_(it) == _val)
        return {false, *it}; // 2think about res

    if (_pf_on_change && !_pf_on_change(_key, ValueAt_(it), _val))
        return {false, ValueAt_(it)}; // 2think about res

    return {true, std::exchange(*it, std::move(_val))};
}

IContainer::EmplaceRes XContainerArray::Emplace(const KeyType&    _key,
                                                const MappedType& _val,
                                                const OnChangePF& _pf_on_change)
{
    auto it = DeqFind_(_key, false);
    if (_pf_on_change && !_pf_on_change(_key, ValueAt_(it), _val))
        return {false, KeyType(), ValueAt_(it)}; // 2think about res

    it = values_deq_.insert(it, _val);
    return {true, IndexToKey_(it - values_deq_.begin()), MappedType() /*_val*/};
}

IContainer::EmplaceRes XContainerArray::Emplace(const KeyType& _key, MappedType&& _val, const OnChangePF& _pf_on_change)
{
    auto it = DeqFind_(_key, false);
    if (_pf_on_change && !_pf_on_change(_key, ValueAt_(it), _val))
        return {false, KeyType(), ValueAt_(it)}; // 2think about res

    it = values_deq_.insert(it, std::move(_val));
    return {true, IndexToKey_(it - values_deq_.begin()), MappedType() /**it*/};
}

std::optional<IContainer::MappedType> XContainerArray::Erase(const KeyType& _key, const OnChangePF& _pf_on_change)
{
    auto it = DeqFind_(_key, false);
    if (it == values_deq_.end())
        return std::nullopt;

    if (_pf_on_change && !_pf_on_change(_key, *it, MappedType()))
        return std::nullopt; // 2think about res

    auto val = std::move(*it);
    values_deq_.erase(it);
    return val;
}

void XContainerArray::Clear() { values_deq_.clear(); }

inline const IContainer::MappedType& XContainerArray::ValueAt_(const typename std::deque<MappedType>::const_iterator& _it)
{
    static const MappedType empty;
    return _it != values_deq_.end() ? *_it : empty;
}

} // namespace xsdk::impl