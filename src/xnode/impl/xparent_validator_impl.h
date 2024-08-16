#pragma once

#include "../xparent_validator.h"

namespace xsdk::impl {

class XParentValidatorBase: public IParentValidator {
public:
    virtual std::pair<IContainer::KeyType, IContainer::MappedType> FindDuplicates(
        IContainer*            _container_p,
        IContainer::MappedType _value_check) const override
    {
        return {};
    }

    virtual IContainer::KeyType RemoveDuplicates(IContainer*            _container_p,
                                                 IContainer::MappedType _value_remove,
                                                 IContainer::KeyType    _key_keep) const override
    {
        return {};
    }
};

class XParentValidatorMap: public XParentValidatorBase {};

class XParentValidatorArray: public XParentValidatorBase {
public:
    virtual std::pair<IContainer::KeyType, IContainer::MappedType> FindDuplicates(
        IContainer*            _container_p,
        IContainer::MappedType _value_check) const override;

    virtual IContainer::KeyType RemoveDuplicates(IContainer*            _container_p,
                                                 IContainer::MappedType _value_remove,
                                                 IContainer::KeyType    _key_keep) const override;
};

} // namespace xsdk::impl