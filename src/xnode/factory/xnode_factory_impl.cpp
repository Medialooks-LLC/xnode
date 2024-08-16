#include "xnode_factory_impl.h"

#include "../impl/xcontainer_match_impl.h"
#include "../impl/xnode_impl.h"
#include "../impl/xparent_validator_impl.h"

#include "../../xcontainer/xcontainer_factory.h"

namespace xsdk {

// INodeFactory* INodeFactory::default_factory()
INodeFactory* XNodeFactoryGet()
{
    // 2Think: is thread safe ?
    static std::shared_ptr<INodeFactory> static_factory_sp = impl::XNodeFactory::create();
    assert(static_factory_sp);
    return static_factory_sp.get();
}

namespace impl {

std::shared_ptr<INodeFactory> XNodeFactory::create() { return std::shared_ptr<INodeFactory> {new XNodeFactory()}; }

/*virtual*/ INode::SPtr XNodeFactory::NodeCreate(INode::NodeType _type, std::string_view _name, uint64_t _uid)
{
    IContainer::ContainerType containter_type = _type == INode::NodeType::Array ? IContainer::ContainerType::Array :
                                                                                  IContainer::ContainerType::Map;

    // Create container
    auto container_p = XContainerFactoryGet()->ContainerCreate(containter_type, _type == INode::NodeType::Map);
    assert(container_p);
    if (!container_p)
        return nullptr;

    // Create container match
    std::unique_ptr<IContainerMatch>  container_match;
    std::unique_ptr<IParentValidator> parent_validator;
    if (containter_type == IContainer::ContainerType::Array) {
        container_match  = std::make_unique<XContainerMatchArray>(std::move(container_p));
        parent_validator = std::make_unique<XParentValidatorArray>();
    }
    else {
        assert(containter_type == IContainer::ContainerType::Map && _type == INode::NodeType::Map);
        container_match  = std::make_unique<XContainerMatchMap>(std::move(container_p));
        parent_validator = std::make_unique<XParentValidatorMap>();
    }

    auto node = XNode::Create(std::move(container_match), std::move(parent_validator), _uid, _name);
    assert(node);
    return node;
}

} // namespace impl
} // namespace xsdk