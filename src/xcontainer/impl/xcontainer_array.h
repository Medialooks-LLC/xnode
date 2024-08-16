#pragma once

#include "../xcontainer.h"

#include <deque>
#include <functional>
#include <optional>
#include <utility>
#include <variant>

namespace xsdk::impl {

class XContainerArray: public IContainer {

    std::deque<MappedType> values_deq_;

    static std::optional<size_t> KeyToIndex_(const KeyType& _key)
    {
        const auto* key_p = std::get_if<size_t>(&_key);
        return key_p ? std::optional<size_t>(*key_p) : std::nullopt;
    }

    static KeyType IndexToKey_(size_t _idx) { return {_idx}; }

public:
    // used for e.g. set(100) -> resize to 100 and set at 100th place
    static constexpr size_t max_size_increase = 1000;

    XContainerArray()                           = default;
    XContainerArray(XContainerArray&&) noexcept = default;
    XContainerArray(const XContainerArray&)     = default;

public:
    virtual ContainerType Type() const override { return ContainerType::Array; }

    virtual bool IsKeyValid(const KeyType& _key) const override;

    virtual bool Empty() const override;

    virtual size_t Size() const override;

    // Return 'false' if empty or key not found
    virtual bool ForEach(std::function<bool(const KeyType&, const MappedType&)>&& _pf_on_item,
                          const std::optional<KeyType>& _from_key = std::nullopt) const override;

    // Return 'false' if empty or key not found
    virtual bool ForEach( // NOLINT(readability-function-cognitive-complexity)
        std::function<OnEachRes(const KeyType&, MappedType&)>&& _pf_on_each,
        const std::optional<KeyType>&                           _from_key     = std::nullopt,
        const OnChangePF&                                       _pf_on_change = nullptr) override;

    virtual std::optional<MappedType> At(const KeyType& _key) const override;

    virtual std::pair<bool, MappedType> Set(const KeyType&    _key,
                                            const MappedType& _val,
                                            const OnChangePF& _pf_on_change) override;

    virtual std::pair<bool, MappedType> Set(const KeyType&    _key,
                                            MappedType&&      _val,
                                            const OnChangePF& _pf_on_change) override;

    virtual EmplaceRes Emplace(const KeyType& _key, const MappedType& _val, const OnChangePF& _pf_on_change) override;

    virtual EmplaceRes Emplace(const KeyType& _key, MappedType&& _val, const OnChangePF& _pf_on_change) override;

    virtual std::optional<MappedType> Erase(const KeyType& _key, const OnChangePF& _pf_on_change) override;

    virtual void Clear() override;

protected:
    const MappedType& ValueAt_(const typename std::deque<MappedType>::const_iterator& _it);

    auto DeqFind_(const KeyType& _key) const -> auto
    {
        auto idx = KeyToIndex_(_key).value_or(kIdxEnd);
        if (idx == kIdxLast) // special pos
            idx = values_deq_.size() > 1 ? values_deq_.size() - 1 : 0;
        if (idx < values_deq_.size()) {
            auto it = values_deq_.begin();
            std::advance(it, idx);
            return it;
        }

        return values_deq_.end();
    }

    auto DeqFind_(const KeyType& _key, bool _increase_size) -> auto
    {
        auto idx = KeyToIndex_(_key).value_or(kIdxEnd);
        if (idx == kIdxEnd)
            return values_deq_.end();
        if (idx == kIdxLast) // special pos
            idx = values_deq_.size() > 1 ? values_deq_.size() - 1 : 0;

        while (_increase_size && idx >= values_deq_.size() && idx < values_deq_.size() + max_size_increase)
            values_deq_.emplace_back();

        if (idx < values_deq_.size()) {
            auto it = values_deq_.begin();
            std::advance(it, idx);
            return it;
        }

        return values_deq_.end();
    }
};

} // namespace xsdk::impl