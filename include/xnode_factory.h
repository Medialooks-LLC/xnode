#pragma once

#include "xnode_interfaces.h"

#include <memory>
#include <string>
#include <cassert>

namespace xsdk {

/**
 * @class INodeFactory
 * @brief Interface for creating INode objects
 *
 * INodeFactory is an interface for creating INode objects.
 * Derived classes must implement the NodeCreate method to create specific types of INode objects.
 */
class INodeFactory {

public:
    /**
     * @brief Destructor
     *
     * A virtual destructor that allows you to call the correct destructor of a derived class.
     */
    virtual ~INodeFactory() = default;

    /**
     * @brief Create a new INode object
     *
     * @param _type NodeType specifying the type of INode to create
     * @param _name Optional name for the new INode object
     * @param _uid Optional unique identifier for the new INode object
     *
     * @return A shared pointer to the newly created INode object
     */
    virtual INode::SPtr NodeCreate(INode::NodeType _type, std::string_view _name = {}, uint64_t _uid = 0) = 0;
};

/**
 * @brief Factory function for obtaining an INodeFactory instance
 *
 * @return A pointer to the INodeFactory instance
 */
INodeFactory* XNodeFactoryGet();

} // namespace xsdk