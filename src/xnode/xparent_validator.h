#pragma once

#include "../xcontainer/xcontainer.h"

namespace xsdk {

class IParentValidator {
public:
    virtual ~IParentValidator() = default;

    virtual std::pair<IContainer::KeyType, IContainer::MappedType> FindDuplicates(
        IContainer*            _container_p,
        IContainer::MappedType _value_check) const = 0;

    virtual IContainer::KeyType RemoveDuplicates(IContainer*            _container_p,
                                                 IContainer::MappedType _value_remove,
                                                 IContainer::KeyType    _key_keep) const = 0;
};

} // namespace xsdk
