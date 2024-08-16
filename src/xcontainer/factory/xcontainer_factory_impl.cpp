#include "xcontainer_factory_impl.h"

#include "../impl/xcontainer_array.h"
#include "../impl/xcontainer_map.h"
#include "../impl/xcontainer_map_w_erase.h"

#include "../xcontainer.h"

namespace xsdk {

// IContainerFactory* IContainerFactory::default_factory()
IContainerFactory* XContainerFactoryGet()
{
    static std::shared_ptr<IContainerFactory> static_factory_sp = impl::XContainerFactory::create();
    assert(static_factory_sp);
    return static_factory_sp.get();
}

namespace impl {

std::shared_ptr<IContainerFactory> XContainerFactory::create()
{
    return std::shared_ptr<IContainerFactory> {new XContainerFactory()};
}

/*virtual*/ std::unique_ptr<IContainer> XContainerFactory::ContainerCreate(IContainer::ContainerType _type,
                                                                           bool                      _erase_detection)
{
    if (_type == IContainer::ContainerType::Array) {
        assert(!_erase_detection);
        return std::make_unique<XContainerArray>();
    }

    assert(_type == IContainer::ContainerType::Map);
    if (_erase_detection)
        return std::make_unique<XContainerMapWithErase>();

    return std::make_unique<XContainerMap>();
}

} // namespace impl

} // namespace xsdk