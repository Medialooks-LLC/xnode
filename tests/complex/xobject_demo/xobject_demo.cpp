#include "xobject_demo.h"

namespace xsdk {

IObject::UPtr xobject::CreateUnique() { return std::make_unique<impl::XObjectImpl>(); }
IObject::SPtr xobject::CreateShared() { return std::make_shared<impl::XObjectImpl>(); }

inline std::any impl::XObjectImpl::QueryPtr(xbase::Uid _type_query)
{
    if (_type_query == xbase::TypeUid<IObject>())
        return std::static_pointer_cast<IObject>(shared_from_this());

    return {};
}

inline std::any impl::XObjectImpl::QueryPtrC(xbase::Uid _type_query) const
{
    if (_type_query == xbase::TypeUid<const IObject>())
        return std::static_pointer_cast<const IObject>(shared_from_this());

    return {};
}

} // namespace xsdk