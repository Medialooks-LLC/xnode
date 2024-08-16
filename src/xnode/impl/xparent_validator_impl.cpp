#include "xparent_validator_impl.h"

#include <cassert>

namespace xsdk::impl {

std::pair<IContainer::KeyType, IContainer::MappedType> XParentValidatorArray::FindDuplicates(
    IContainer*            _container_p,
    IContainer::MappedType _value_check) const
{
    assert(_container_p);

    IContainer::KeyType    key_existed;
    IContainer::MappedType val_existed;
    _container_p->ForEach([&](const auto& key, const auto& val) {
        if (val == _value_check) {
            key_existed = key;
            val_existed = val;
            return true;
        }
        return false;
    });

    return {key_existed, val_existed};
}

IContainer::KeyType XParentValidatorArray::RemoveDuplicates(IContainer*            _container_p,
                                                            IContainer::MappedType _value_remove,
                                                            IContainer::KeyType    _key_keep) const
{
    assert(_container_p);

    IContainer::KeyType key_removed;
    _container_p->ForEach([&](const auto& key, const auto& val) {
        if (key != _key_keep && val == _value_remove) {
            key_removed = key;
            return OnEachRes::EraseStop;
        }
        return OnEachRes::Next;
    });

    return key_removed;
}

} // namespace xsdk::impl