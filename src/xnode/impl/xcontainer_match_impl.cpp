#include "xcontainer_match_impl.h"

namespace xsdk::impl {

XKey XContainerMatchBase::NodeKey_(const IContainer::KeyType& _key)
{
    return std::visit([](const auto& _elem) -> auto { return XKey(_elem); }, _key);
}

IContainer::KeyType XContainerMatchBase::ContainerKey_(const XKey& _key)
{
    return std::visit(
        [](const auto& elem) -> auto {
            // expicit conversion for std::string_view -> string
            if constexpr (std::is_same_v<std::decay_t<decltype(elem)>, std::string_view>)
                return IContainer::KeyType(std::string(elem));
            else
                return IContainer::KeyType(elem);
        },
        _key);
}

IContainer::KeyType XContainerMatchMap::ContainerKey(const XKey& _key, bool _sequntial_index) const
{
    auto* container_p = ContainerGet();
    assert(container_p);
    // Check for map access by index
    // (index key, index access enabled, container is not indexed by nature)
    if (_sequntial_index && _key.Type() == XKey::KeyType::Index) {

        size_t index = _key.IndexGet().value();
        if (index == kIdxLast && !container_p->Empty())
            index = container_p->Size() - 1;
        else if (index >= container_p->Size())
            return {};

        IContainer::KeyType key_res = {};
        container_p->ForEach([&index, &key_res](const IContainer::KeyType& key, const IContainer::MappedType&) {
            if (index == 0) {
                key_res = key;
                return true;
            }

            --index;
            return false;
        });

        return key_res;
    }

    return XContainerMatchBase::ContainerKey(_key, _sequntial_index);
}

} // namespace xsdk::impl