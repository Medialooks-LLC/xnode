#pragma once

#include "xconstant.h" // For OnEachRes
#include "xvalue/xvalue_rt.h"

#include "xbase.h"

#include <functional>
#include <optional>
#include <utility>
#include <variant>

namespace xsdk {

class IContainer: public xbase::PtrBase<IContainer> {

public:
    
    // For std::map etc. partial interoperability (e.g. for tests)
    using KeyType    = std::variant<std::monostate, size_t, std::string>;
    using MappedType = XValueRT;

    // If return 'false' changing prohibited
    using OnChangePF = std::function<bool(const KeyType&, const MappedType&, const MappedType&)>;

    enum class ContainerType { Map = 1, Array = 2 };

public:
    virtual ~IContainer() = default;

public:
    virtual ContainerType Type() const = 0;
    // Return
    virtual bool   IsKeyValid(const KeyType& _key) const = 0;
    virtual size_t Size() const                            = 0;
    virtual bool   Empty() const                           = 0;

    // Return nullopt if not found
    virtual std::optional<MappedType> At(const KeyType& _key) const = 0;

    // Method: return 'false' if empty or key not found
    // Callback: return 'true' for stop enumeration
    virtual bool ForPatch(std::function<bool(const KeyType&, const MappedType&)>&& _pf_on_item,
                           const std::optional<KeyType>&                            _from_key = std::nullopt) const
    {
        return ForEach(std::move(_pf_on_item), _from_key);
    }
    // Method: return 'false' if empty or key not found
    // Callback: return 'true' for stop enumeration
    virtual bool ForEach(std::function<bool(const KeyType&, const MappedType&)>&& _pf_on_item,
                          const std::optional<KeyType>&                            _from_key = std::nullopt) const = 0;
    // Return 'false' if empty or key not found
    // Callback: return: next, Erase, erase_stop, stop (see OnEachRes in xdefines.h)
    //           item could be modified in callback, but modification should be approved via 'on_change' cb
    virtual bool ForEach(std::function<OnEachRes(const KeyType&, MappedType&)>&& _pf_on_item,
                          const std::optional<KeyType>&                           _from_key     = std::nullopt,
                          const OnChangePF&                                       _pf_on_change = nullptr) = 0;
    // Return {succcess, previous value}
    virtual std::pair<bool, MappedType> Set(const KeyType&    _key,
                                            MappedType&&      _val,
                                            const OnChangePF& _pf_on_change = nullptr) = 0;
    // Return {succcess, previous value}
    virtual std::pair<bool, MappedType> Set(const KeyType&    _key,
                                            const MappedType& _val,
                                            const OnChangePF& _pf_on_change = nullptr)
    {
        return Set(_key, MappedType(_val), _pf_on_change);
    }

    struct EmplaceRes {
        bool       succeeded = false;
        KeyType    inserted_at;
        MappedType existed; // empty if succeeded or canceled via cb (?)
    };
    virtual EmplaceRes Emplace(const KeyType& _key, MappedType&& _val, const OnChangePF& _pf_on_change = nullptr) = 0;
    // Return {succcess, key, existed value (if failed to Insert into map)}
    virtual EmplaceRes Emplace(const KeyType& _key, const MappedType& _val, const OnChangePF& _pf_on_change = nullptr)
    {
        return Emplace(_key, MappedType(_val), _pf_on_change);
    }
    // Return 'nullopt' if key not found or failed callback
    virtual std::optional<MappedType> Erase(const KeyType& _key, const OnChangePF& _pf_on_change = nullptr) = 0;
    // Remove all items
    virtual void Clear() = 0;
};

} // namespace xsdk