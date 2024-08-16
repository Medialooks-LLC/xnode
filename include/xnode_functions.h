#pragma once

#include "xkey/xpath.h"
#include "xnode_interfaces.h"

#include <memory>
#include <string_view>

namespace xsdk::xnode {

/**
 * @brief Creates an XNode of the given type
 *
 * @param _type Node type to create
 * @param _name Optional name for the node
 * @param _uid Optional unique identifier for the node
 *
 * @return std::shared_ptr to the newly created XNode
 */
INode::SPtr Create(INode::NodeType _type, std::string_view _name = {}, uint64_t _uid = 0);
/**
 * @brief Creates an XNode array
 *
 * @param _values Vector of values to insert into the array
 * @param _name Optional name for the node
 * @param _uid Optional unique identifier for the node
 *
 * @return std::shared_ptr to the newly created array XNode
 */
INode::SPtr CreateArray(std::vector<XValue>&& _values = {}, std::string_view _name = {}, uint64_t _uid = 0);
/**
 * @brief Creates an XNode map
 *
 * @param _values Vector of key-value pairs to insert into the map
 * @param _name Optional name for the node
 * @param _uid Optional unique identifier for the node
 *
 * @return std::shared_ptr to the newly created map XNode
 */
INode::SPtr CreateMap(std::vector<std::pair<XKey, XValue>>&& _values = {},
                      std::string_view                       _name   = {},
                      uint64_t                               _uid    = 0);
/**
 * @brief Creates a complex XNode (map and/or array)
 *
 * @param _values Vector of path-value pairs or values to insert into the node. @see XPath
 * @param _name Optional name for the node
 * @param _uid Optional unique identifier for the node
 *
 * @return std::shared_ptr to the newly created complex XNode
 */
INode::SPtr CreateComplex(std::vector<std::pair<XPath, XValue>>&& _values,
                          std::string_view                        _name = {},
                          uint64_t                                _uid  = 0);

/**
 * @brief Enum class representing different types of nodes.
  */
enum class XNodeType {
    not_node,   ///< Node is not a valid XNode.
    map,        ///< Node represents a map.
    const_map,  ///< Node represents a constant map.
    array,      ///< Node represents an array.
    const_array ///< Node represents a constant array.
};
/**
 * @brief Returns the type of a given value.
 * @param _val The value to check.
 * @return The type of the value. @see XNodeType.
 */
XNodeType NodeTypeGet(const XValue& _val);

/**
 * @brief Clones an xnode with a callback function for cloning items.
 * @param _value_with_node The INode containing the value to be cloned.
 * @param _clone_nodes     Determines whether to recursively clone all nodes within the given value.
 * @param _pf_on_item      A callback function that will be called for each item before it's cloned.
 *                             The callback function can decide to skip the cloning some items.
 * @param _cloned_name     Optional name for the node
 * @param _cloned_uid      Optional unique identifier for the node
 * @return A pointer to the cloned INode or nullptr in case of failure.
 */
INode::SPtr Clone(XValue&&                                                                     _value_with_node,
                  bool                                                                         _clone_nodes,
                  const std::function<OnCopyRes(const INode::SPtrC&, const XKey&, XValueRT&)>& _pf_on_item  = nullptr,
                  std::string_view                                                             _cloned_name = {},
                  uint64_t                                                                     _cloned_uid  = 0);

/**
 * @brief Inserts a new node into an existing node.
 * @param _node_this The existing node.
 * @param _node_insert The node to be inserted.
 * @param _replace_node If true, replace an existing node with the same key.
 * @param _node_key The key of the new node.
 * @return The result of the insertion operation.
 */
INode::InsertRes NodeInsert(const INode::SPtr& _node_this,
                            const INode::SPtr& _node_insert,
                            bool               _replace_node,
                            const XKey&        _node_key = {});
/**
 * @brief Inserts a new constant node into an existing node.
 * @param _node_this The existing node.
 * @param _node_insert The node to be inserted.
 * @param _replace_node If true, replace an existing node with the same key.
 * @param _node_key The key of the new node.
 * @return The result of the insertion operation.
 */
INode::InsertRes NodeConstInsert(const INode::SPtr&  _node_this,
                                 const INode::SPtrC& _node_insert,
                                 bool                _replace_node,
                                 const XKey&         _node_key = {});
/**
 * @brief Retrieves a node by its key.
 * @param _node_this The node to search in.
 * @param _node_key The key of the node to retrieve.
 * @return A pointer to the node if it exists, otherwise null.
 */
INode::SPtr      NodeGetByKey(const INode::SPtr&             _node_this,
                              const XKey&                    _key,
                              std::optional<INode::NodeType> _node_type       = std::nullopt,
                              bool                           _convert_to_type = false);
/**
 * @brief Retrieves a node by its key.
 * @param _node_this The node to search in.
 * @param _node_key The key of the node to retrieve.
 * @return A pointer to the constant node if it exists, otherwise null.
 */
INode::SPtrC     NodeConstGetByKey(const INode::SPtrC& _node_this, const XKey& _key);

/**
 * @brief Compares two nodes based on their content and structure.
 * @param _node_left The left node to be compared.
 * @param _node_right The right node to be compared.
 * @param _nodes_unwrap Whether to unwrap nested nodes during comparison.
 * @param _pf_on_different A function to handle elements with different keys and values.
 *
 * @return The comparison result. A value less than zero if _node_left comes before _node_right.
 * A value greater than zero if _node_left comes after _node_right. Zero if both nodes are equal.
 */
int32_t Compare(const INode::SPtrC& _node_left,
                const INode::SPtrC& _node_right,
                bool                _nodes_unwrap,
                const std::function<bool(const INode::SPtrC&, const XKey&, const XValueRT&, const XValueRT&)>&
                    _pf_on_different = nullptr);

/**
 * @brief Applies patches to nodes by adding, updating, or removing elements.
 * @param _node The node to be patched.
 * @param _patch The patch to be applied to the node.
 * @return The number of added and erased elements in the node.
 */
std::pair<size_t, size_t> PatchApply(const INode::SPtr& _target, const INode::SPtrC& _patch);

/**
 * @brief Copies data from a source node to a destination node.
 * @details This method copies data from a source node to a destination node
 *           recursively. It calls the BulkGetAll method of the source node
 *           to retrieve all the keys and values, and for each key-value pair,
 *           it checks if there is a corresponding node in the destination node,
 *           and if so, it recursively copies the data to the destination node.
 * @param _source The source node to copy data from.
 * @param _dest The destination node to copy data to.
 * @param _override If true, overrides existing nodes in the destination
 *                      node with the same key.
 * @param _nodes_as_refs If true, sets the destination node's value for the
 *                           given key to be the same node from the source
 *                           instead of creating a clone of it.
 * @param _depth The depth of the recursion.
 * @return The number of nodes copied.
 */
size_t CopyTo(const INode::SPtrC& _source,
              const INode::SPtr&  _dest,
              bool                _override,
              bool                _nodes_as_refs = false,
              const size_t        _depth         = -1);

// XPath (heierachic modes support)
// todo: compare performance with simple keys
/**
 * @brief Recursively retrieves an INode instance using the given XPath and optional node type.
 * @param _node_this INode instance to start traversing from.
 * @param _path XPath to traverse to the target node.
 * @param _node_type Optional node type to convert the target node to.
 * @param _convert_to_type Optional flag indicating if the target node should be converted to the given node type.
 * @return An INode::SPtr instance to the target node.
 */
INode::SPtr  NodeGet(const INode::SPtr&             _node_this,
                     XPath&&                        _path,
                     std::optional<INode::NodeType> _node_type       = std::nullopt,
                     bool                           _convert_to_type = false);
/**
 * @brief Const version of NodeGet function for reading purposes only.
 * @param _node_this INode const instance to start traversing from.
 * @param _path XPath to traverse to the target node.
 * @return An INode::SPtrC instance to the target node.
 */
INode::SPtrC NodeConstGet(const INode::SPtrC& _node_this, XPath&& _path);

/**
 * @brief Retrieves the XValueRT at the specified XPath or null if the path does not exist.
 * @param _node_this INode instance to start traversing from.
 * @param _path XPath to traverse to the target node.
 * @return An XValueRT instance containing the target node's value or null if the path does not exist.
 */
XValueRT                  At(const INode::SPtr& _node_this, XPath&& _path);
/**
 * @brief Retrieves the XValueRT at the specified XPath or null if the path does not exist.
 * @param _node_this INode const instance to start traversing from.
 * @param _path XPath to traverse to the target node.
 * @return An XValueRT instance containing the target node's value or null if the path does not exist.
 */
XValueRT                  At(const INode::SPtrC& _node_this, XPath&& _path);
/**
 * @brief Sets the value of a node at the specified XPath.
 * @param _node_this INode instance to start traversing from.
 * @param _path XPath to traverse to the target node.
 * @param _val Value to set the target node to.
 * @return A pair of a boolean indicating success and the value of the target element before setting it.
 */
std::pair<bool, XValueRT> Set(const INode::SPtr& _node_this, XPath&& _path, XValue&& _val);
/**
 * @brief Inserts a new value into the specified XPath.
 * @param _node_this INode instance to start traversing from.
 * @param _path XPath to traverse to the target node.
 * @param _val Value to be inserted.
 * @returns The result of the insertion operation.
 */
INode::InsertRes          Insert(const INode::SPtr& _node_this, XPath&& _path, XValue&& _val);
/**
 * @brief Erases a value from the specified XPath.
 * @param _node_this INode instance to start traversing from.
 * @param _path XPath to traverse to the target node.
 * @returns The erased value.
 */
XValueRT                  Erase(const INode::SPtr& _node_this, XPath&& _path);
/**
 * @brief Increases the value at the specified XPath by the specified value.
 * @param _node_this INode instance to start traversing from.
 * @param _path The XPath representing the element to be incremented.
 * @param _val The value to be added to the element.
 * @returns The updated value of the node.
 */
XValueRT                  Increment(const INode::SPtr& _node_this, XPath&& _path, XValue&& _val);

// If specified attribute exist -> convert to array
/**
 * @brief Emplaces a new value into an existing array node defined by XPath.
 * @details The function navigates through the given XPath and emplaces the new value into the existing array
 * node at the end. If target element is not an array node, it will be converted to an array node with two elements - the previous value and the new value.
 * @param _node_this INode instance to start traversing from.
 * @param _array_path The XPath to navigate to the array node.
 * @param _val The new value to be emplaced.
 * @return The number of elements in the array node after the emplacement.
 */
size_t EmplaceToArray(const INode::SPtr& _node_this, XPath&& _array_path, XValue&& _val);

// If dest is attribute -> return value, if dest is array -> return array value
// For make equal: "a" : [123] and "a" : 123
/**
 * @brief Retrieves the values as a vector of element value by the specified XPath.
 * @note If element value is not an array node, it will be returned as a vector with one value. 
 * @param _node_this INode instance to start traversing from.
 * @param _path The XPath to navigate through.
 * @return A vector containing all values of the element by the specified XPath.
 */
std::vector<XValueRT> ValuesList(const INode::SPtrC& _node_this, XPath&& _path);

/**
 * @brief Retrieves the nodes at the given XPath.
 * @details The function navigates through the given XPath and returns all nodes at the path.
 * @param _node_this INode instance to start traversing from.
 * @param _include_const A flag indicating whether to include constant nodes in the result or not.
 * @param _path The XPath to navigate through.
 * @return A vector containing all nodes at the XPath.
 */
std::vector<std::pair<XKey, XValueRT>> NodesList(const INode::SPtr& _node_this,
                                                 bool               _include_const, //??? may be set default value?
                                                 XPath&&            _path = XPath());
/**
 * @brief Retrieves the constant nodes at the given XPath.
 * @param _node_this INode const instance to start traversing from.
 * @param _path XPath to traverse to the target node.
 * @return A vector containing all constant nodes at the XPath.
 */
std::vector<std::pair<XKey, XValueRT>> NodesConstList(const INode::SPtrC& _node_this, XPath&& _path = XPath());

// Move to utility
// Return map of {child, parent} nodes with improper parents
/**
 * @brief Checks for improperly connected parents in the nodes tree.
 * @details This function recursively checks for improperly connected parents of a given node and stores them in the
 * _improper_map. If a cycle is detected, the function skips that node and continues checking the remaining nodes.
 * @param _root The root node of the subtree to check.
 * @param _include_const If true, also check const nodes.
 * @param _improper_map A map to store nodes with improper parents.
 * @return The updated _improper_map containing the nodes with improper parents.
 */
std::map<XValueRT, XValueRT> ParentsCheck(
    const XValue&                  _root,
    bool                           _include_const,
    std::map<XValueRT, XValueRT>&& _improper_map = std::map<XValueRT, XValueRT>());

/**
 * @brief Fixes the connections of nodes with improper parents.
 * @param _fix_map A map containing the nodes with improper parents.
 * @return The updated map with the fixed connections.
 */
std::map<XValueRT, XValueRT> ParentsFix(const std::map<XValueRT, XValueRT>& _fix_map);

} // namespace xsdk::xnode