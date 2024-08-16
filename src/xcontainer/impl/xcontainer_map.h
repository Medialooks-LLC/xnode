#pragma once

#include "../xcontainer.h"

#include <functional>
#include <map>
#include <optional>
#include <utility>
#include <variant>

namespace xsdk::impl {

class XContainerMap: public IContainer {

    std::map<std::string, MappedType> values_map_;

    static const std::string& KeyToString_(const KeyType& _key)
    {
        static const std::string empty;
        const auto*              str_p = std::get_if<std::string>(&_key);
        return str_p ? *str_p : empty;
    }

    static KeyType StringToKey_(const std::string& _str) { return {_str}; }

    static KeyType StringToKey_(std::string&& _str) { return {std::move(_str)}; }

public:
    XContainerMap()                         = default;
    XContainerMap(XContainerMap&&) noexcept = default;
    XContainerMap(const XContainerMap&)     = default;

public:
    virtual ContainerType Type() const override { return ContainerType::Map; }

    virtual bool IsKeyValid(const KeyType& _key) const override;

    virtual bool Empty() const override;

    virtual size_t Size() const override;

    virtual bool ForEach(std::function<bool(const KeyType&, const MappedType&)>&& _pf_on_item,
                          const std::optional<KeyType>&                            _from_key) const override;

    // Return 'false' if empty or key not found
    virtual bool ForEach( // NOLINT(readability-function-cognitive-complexity)
        std::function<OnEachRes(const KeyType&, MappedType&)>&& _pf_on_each,
        const std::optional<KeyType>&                           _from_key,
        const OnChangePF&                                       _pf_on_change) override;

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
    const MappedType& ValueAt_(const typename std::map<std::string, MappedType>::const_iterator& _it) const;

    auto MapFind_(const KeyType& _key) const -> auto
    {
        const auto& key = KeyToString_(_key);
        if (key.empty())
            return values_map_.end();

        return values_map_.find(key);
    }

    auto MapFind_(const KeyType& _key) -> auto
    {
        const auto& key = KeyToString_(_key);
        if (key.empty())
            return std::make_pair(values_map_.end(), std::string());

        return std::make_pair(values_map_.find(key), key);
    }
};

} // namespace xsdk::impl