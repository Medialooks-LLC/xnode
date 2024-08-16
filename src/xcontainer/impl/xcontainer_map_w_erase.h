#pragma once

#include "xcontainer_map.h"

#include <cassert>
#include <functional>
#include <map>
#include <optional>
#include <utility>
#include <variant>

namespace xsdk::impl {

class XContainerMapWithErase: public XContainerMap {

    size_t erased_values_ = 0;

    static bool IsErasedValue_(const MappedType& _value)
    {
        // Check timestamp
        return _value.IsEmpty() && !_value.TimeIsAbsent();
    }

    static MappedType ErasedValue_()
    {
        // For have timestamp
        return MappedType::EmptyWithTime();
    }

public:
    XContainerMapWithErase()                                  = default;
    XContainerMapWithErase(XContainerMapWithErase&&) noexcept = default;
    XContainerMapWithErase(const XContainerMapWithErase&)     = default;

public:
    // 2Think: Check erased values ?
    virtual bool Empty() const override;

    virtual size_t Size() const override;

    // Method: return 'false' if empty or key not found
    // Callback: return 'true' for stop enumeration
    virtual bool ForPatch(std::function<bool(const KeyType&, const MappedType&)>&& _pf_on_item,
                           const std::optional<KeyType>& _from_key = std::nullopt) const override;

    // Method: return 'false' if empty or key not found
    // Callback: return 'true' for stop enumeration
    virtual bool ForEach(std::function<bool(const KeyType&, const MappedType&)>&& _pf_on_item,
                          const std::optional<KeyType>& _from_key = std::nullopt) const override;

    // Return 'false' if empty or key not found
    virtual bool ForEach( // NOLINT(readability-function-cognitive-complexity)
        std::function<OnEachRes(const KeyType&, MappedType&)>&& _pf_on_each,
        const std::optional<KeyType>&                           _from_key,
        const OnChangePF&                                       _pf_on_change) override;

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
    OnChangePF DetectErase_(const OnChangePF& _pf_on_change);
};

} // namespace xsdk::impl