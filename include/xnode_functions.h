#pragma once

#include "xkey/xpath.h"
#include "xnode_interfaces.h"

#include <memory>
#include <optional>
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
[[nodiscard]] INode::SPtr Create(INode::NodeType _type, std::string_view _name = {}, uint64_t _uid = 0);

/**
 * @brief Check existed node for specified type, if not suitable, creates an XNode of the given type
 *
 * @param _try_node Node for check
 * @param _type Node type to create
 * @param _name Optional name for the node
 *
 * @return std::shared_ptr to the newly created XNode
 */
[[nodiscard]] INode::SPtr CreateOrUse(const INode::SPtr& _try_node, INode::NodeType _type, std::string_view _name = {});
/**
 * @brief Check existed value for node wuth specified type, if not suitable, creates an XNode of the given type
 *
 * @param _try_node Node for check
 * @param _type Node type to create
 * @param _name Optional name for the node
 *
 * @return std::shared_ptr to the newly created XNode
 */
[[nodiscard]] INode::SPtr CreateOrUse(const XValue& _try_value, INode::NodeType _type, std::string_view _name = {});
/**
 * @brief Check existed node for specified type, if not suitable, creates an XNode of the given type, if so, clone
 * existed node
 *
 * @param _try_node Node for check
 * @param _type Node type to create
 * @param _name Optional name for the node
 * @param _uid Optional unique identifier for the node
 *
 * @return std::shared_ptr to the newly created XNode
 */
[[nodiscard]] INode::SPtr CreateOrClone(const INode::SPtrC& _try_node,
                                        const bool          _clone_nodes,
                                        INode::NodeType     _type,
                                        std::string_view    _name = {});
/**
 * @brief Creates an XNode array
 *
 * @param _values Vector of values to insert into the array
 * @param _name Optional name for the node
 * @param _uid Optional unique identifier for the node
 *
 * @return std::shared_ptr to the newly created array XNode
 */
[[nodiscard]] INode::SPtr CreateArray(std::vector<XValue>&& _values = {},
                                      std::string_view      _name   = {},
                                      uint64_t              _uid    = 0);
/**
 * @brief Creates an XNode map
 *
 * @param _values Vector of key-value pairs to insert into the map
 * @param _name Optional name for the node
 * @param _uid Optional unique identifier for the node
 *
 * @return std::shared_ptr to the newly created map XNode
 */
[[nodiscard]] INode::SPtr CreateMap(std::vector<std::pair<XKey, XValue>>&& _values = {},
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
[[nodiscard]] INode::SPtr CreateComplex(std::vector<std::pair<XPath, XValue>>&& _values,
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
[[nodiscard]] XNodeType NodeTypeGet(const XValue& _val);

/**
 * @brief Clones an xnode with a callback function for cloning items.
 * @param _value_with_node The INode containing the value to be cloned.
 * @param _clone_nodes     Determines whether to recursively clone all nodes within the given value.
 * @param _pf_on_item      A callback function that will be called for each item before it's cloned.
 *                             The callback function can decide to skip the cloning some items.
 * @param _cloned_name     Optional name for the clones node, if not specfied, source node name used
 * @param _cloned_uid      Optional unique identifier for the node
 * @return A pointer to the cloned INode or nullptr in case of failure.
 */
[[nodiscard]] INode::SPtr Clone(
    XValue&&                                                                     _value_with_node,
    bool                                                                         _clone_nodes,
    const std::function<OnCopyRes(const INode::SPtrC&, const XKey&, XValueRT&)>& _pf_on_item  = {},
    std::optional<std::string_view>                                              _cloned_name = {},
    uint64_t                                                                     _cloned_uid  = 0);

/**
 * @brief Return the size of node or 0 for nullptr
 */
inline size_t NodeSize(const INode* _node_this) { return _node_this ? _node_this->Size() : 0; }

/**
 * @brief Return the type of node (Array or Map) or std::nullopt if not node
 */
std::optional<INode::NodeType> NodeType(const XValue& _value);

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
[[nodiscard]] INode::SPtr NodeGetByKey(INode* const                         _node_this_p,
                                       const XKey&                          _key,
                                       const std::optional<INode::NodeType> _node_type       = std::nullopt,
                                       const bool                           _convert_to_type = false);
/**
 * @brief Retrieves a node by its key.
 * @param _node_this The node to search in.
 * @param _node_key The key of the node to retrieve.
 * @return A pointer to the constant node if it exists, otherwise null.
 */
[[nodiscard]] INode::SPtrC NodeConstGetByKey(const INode* const _node_this_p, const XKey& _key);

/**
 * @brief Get a node path to specified root .
 * @param _node_this The node to search in.
 * @param _root_p The last node.
 * @return A pair of node path and root node.
 */
[[nodiscard]] std::pair<XPath, INode::SPtrC> NodePath(const INode* _node_this_p, const INode* _root_p = nullptr);

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

/**
 * @brief Flags controlling the behavior of node copying and value assignment operations.
 *
 * These flags determine how a destination node is handled during copy or merge operations,
 * such as whether to create a new node, override existing values, or both.
 *
 * @note This enum is designed to be used as a bitmask. Combine flags using the bitwise OR operator (`|`).
 */
enum class CopyToFlags { kNone, kOverride = 1, kCreateNew = 2, kCreateOverrideAll = kOverride | kCreateNew };

XENUM_OPS32(CopyToFlags)
/**
 * @brief Performs a flexible deep-copy or merge operation of multiple key-value pairs into a destination node.
 *
 * This function applies a series of (XPath, XValue) assignments to a destination node, optionally creating
 * or overriding the destination node itself based on the provided flags. It supports both insertion and
 * overriding of values within the destination node, and can handle cases where the destination is initially
 * empty or points to a const node.
 *
 * @param _values A vector of key-value pairs, where each key is an XPath specifying the target location
 *                within the destination node, and each value is the data to be placed at that location.
 *                The vector is moved into the function to avoid unnecessary copying.
 * @param _dest_value The destination value, which may or may not already contain a valid node. If it does not,
 *                    the behavior depends on the copy flags (e.g., node creation may be attempted).
 * @param _copy_flags A bitmask of CopyToFlags that controls the behavior of the copy operation, such as
 *                    whether to create a new node, override existing values, or rename the destination node.
 * @param _default_new_node_name The name to assign to a newly created destination node if one must be
 *                               instantiated. If empty and a new node is needed, a default name may be used.
 *
 * @return A std::pair containing:
 *         - The resulting destination node (either the original, a cloned version, or a newly created one).
 *         - The number of successfully applied assignments from @_values.
 *
 * @note If kCreateNew is not set in @_copy_flags and @_dest_value does not contain a valid node,
 *       the function returns an empty pair ({}, 0).
 * @note If kOverride is not set and a target path already exists in the destination node,
 *       the corresponding value will not be overwritten.
 * @note If the destination value points to a const node and node creation is allowed, a mutable clone
 *       of the const node is created and used as the destination.
 */
std::pair<INode::SPtr, size_t> ComplexCopyTo(std::vector<std::pair<XPath, XValue>>&& _values,
                                             XValue&&                                _dest_value,
                                             const CopyToFlags                       _copy_flags,
                                             const std::string_view                  _default_new_node_name = {});

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
INode::SPtr NodeGet(const INode::SPtr&                   _node_this,
                    XPath&&                              _path,
                    const std::optional<INode::NodeType> _node_type       = std::nullopt,
                    const bool                           _convert_to_type = false);

/**
 * @brief Recursively retrieves an INode instance using the given XPath and optional node type.
 * @param _node_this INode instance to start traversing from.
 * @param _path XPath to traverse to the target node.
 * @param _node_type Optional node type to convert the target node to.
 * @param _convert_to_type Optional flag indicating if the target node should be converted to the given node type.
 * @return An INode::SPtr instance to the target node.
 */
INode::SPtr NodeGet(INode* const                         _node_this_p,
                    XPath&&                              _path,
                    const std::optional<INode::NodeType> _node_type       = std::nullopt,
                    const bool                           _convert_to_type = false);

/**
 * @brief Recursively retrieves an INode instance using the given XPath and optional node type.
 * @param _node_value XValue with INode instance to start traversing from.
 * @param _path XPath to traverse to the target node.
 * @param _node_type Optional node type to convert the target node to.
 * @param _convert_to_type Optional flag indicating if the target node should be converted to the given node type.
 * @return An INode::SPtr instance to the target node.
 */
INode::SPtr NodeGetV(const XValue&                        _node_value,
                     XPath&&                              _path            = {},
                     const std::optional<INode::NodeType> _node_type       = std::nullopt,
                     const bool                           _convert_to_type = false);
/**
 * @brief Const version of NodeGet function for reading purposes only.
 * @param _node_this INode const instance to start traversing from.
 * @param _path XPath to traverse to the target node.
 * @return An INode::SPtrC instance to the target node.
 */
[[nodiscard]] INode::SPtrC NodeConstGet(const INode::SPtrC& _node_this, XPath&& _path);
/**
 * @brief Const version of NodeGet function for reading purposes only.
 * @param _node_this INode const instance to start traversing from.
 * @param _path XPath to traverse to the target node.
 * @return An INode::SPtrC instance to the target node.
 */
[[nodiscard]] INode::SPtrC NodeConstGet(const INode* const _node_this_p, XPath&& _path);
/**
 * @brief Const version of NodeGet function for reading purposes only.
 * @param _node_value XValue with INode const instance to start traversing from.
 * @param _path XPath to traverse to the target node.
 * @return An INode::SPtrC instance to the target node.
 */
[[nodiscard]] INode::SPtrC NodeConstGetV(const XValue& _node_value, XPath&& _path);

/**
 * @brief Merge content of two nodes, if one is empty -> return non empty one,
 * if two is non empty, the resulting content is PatchApply(_to, _from)
 * @param _from first INode const instance to for merge.
 * @param _to second INode const instance to for merge.
 * @return An INode::SPtrC merged instance of two nodes
 */
[[nodiscard]] INode::SPtrC NodesMerge(const INode::SPtrC& _from, const INode::SPtrC& _to);

/**
 * @brief Patch content of two values:
 * - if both is nodes -> the same as PatchApply(_dest, _patch) (as in NodesMerge)
 * - if _patch is ValueType::kNull -> the resulting is empty value
 * - returned _patch value

 * @param _to destination XValue const instance for merge.
 * @param _to second XValue const instance for merge.
 * @param _modify_dest_node if is true, than PatchApply() called on dest node,
 *        if false -> on cloned node (_dest not modified)
 * @return An patched XValue
 */
[[nodiscard]] XValue ValuePatch(const XValue& _patch, const XValue& _dest, const bool _modify_dest_node);

/**
 * @brief Retrieves the size of map/array at the specified XPath or std::nullopt if the path does not exist or dest is
 * not map/array, if path empty -> Size() for _node_this
 * @param _node_this INode instance to start traversing from.
 * @param _path XPath to traverse to the target node
 * @return Size of map/array at the specified XPath or std::nullopt if the path does not exist or dest is
 * not map/array
 */
[[nodiscard]] std::optional<size_t> NodeSize(const INode::SPtrC& _node_this, XPath&& _path = {});

/**
 * @brief Recursively iterates over all key-value entries in a node tree, invoking a user-provided callback for each.
 *
 * This function performs a depth-first traversal of the node hierarchy starting from the given node.
 * For each key-value pair encountered, it constructs the full XPath to that item and calls the provided
 * callback function `_on_each_item`. If the value is itself a node, the function recurses into it.
 *
 * @param _node_value      A shared pointer to the current node to iterate over. If null, the function returns 0.
 * @param _on_each_item   A callback function that is invoked for each key-value pair in the tree.
 *                        It receives the full XPath to the item and a reference to its value.
 *                        If the callback returns `false`, iteration stops for that branch;
 *                        if it returns `true`, the current item is counted and traversal continues.
 * @param _node_path      The base XPath prefix representing the path to the current node.
 *                        This path is extended with each key during traversal.
 *
 * @return The total number of items for which the callback returned `true`.
 *
 * @note The traversal is recursive and supports nested node structures of arbitrary depth.
 * @note The `_on_each_item` callback must be non-throwing to ensure predictable behavior.
 */
size_t NodeIterate(const XValue&                                             _node_value,
                   const std::function<bool(const XPath&, const XValueRT&)>& _on_each_item,
                   const XPath&                                              _node_path = {});

/**
 * @brief Recursively iterates over all key-value entries in a node tree, and return only elements modified after
 * specified time.
 *
 * @param _value      A shared pointer to the value (also with node) to iterate over. If null, the function returns 0.
 * @param _after_timestamp   Ignore elements chnaged before or equal specified timestamps
 * @return The modified values (null mean removed) with max (most recent) modification time or empty (if no changes)
 *
 * @note The traversal is recursive and supports nested node structures .
 */
XValueRT ChangesAfterTime(const XValueRT& _value, const xbase::Time64 _after_timestamp, const bool _unwrap_const_nodes);

/**
 * @brief Retrieves the XValueRT at the specified XPath or null if the path does not exist.
 * @param _node_this INode instance to start traversing from.
 * @param _path XPath to traverse to the target node.
 * @return An XValueRT instance containing the target node's value or null if the path does not exist.
 */
[[nodiscard]] XValueRT At(const INode::SPtr& _node_this, XPath&& _path);
/**
 * @brief Retrieves the XValueRT at the specified XPath or null if the path does not exist.
 * @param _node_this INode const instance to start traversing from.
 * @param _path XPath to traverse to the target node.
 * @return An XValueRT instance containing the target node's value or null if the path does not exist.
 */
[[nodiscard]] XValueRT At(const INode::SPtrC& _node_this, XPath&& _path);
/**
 * @brief Try retrieve the XValueRT at the specified XPath from vector of nodes
 * - try each node one after one, while not found desired value.
 * @param _check_nodes vector of INode const instance to start traversing from.
 * @param _path XPath to traverse to the target node.
 * @return An XValueRT instance containing the target node's value or null if the path does not exist.
 */
[[nodiscard]] XValueRT At(const std::vector<INode::SPtrC>& _check_nodes, const XPath& _path);
/**
 * @brief Retrieves the XValueRT at the specified XPath or null if the path does not exist.
 * @param _node_value XValue with INode const instance to start traversing from.
 * @param _path XPath to traverse to the target node.
 * @return An XValueRT instance containing the target node's value or null if the path does not exist.
 */
[[nodiscard]] XValueRT At(const XValue& _node_value, XPath&& _path);
/**
 * @brief Sets the value of a node at the specified XPath.
 * @param _node_this INode instance to start traversing from.
 * @param _path XPath to traverse to the target node.
 * @param _val Value to set the target node to.
 * @return A pair of a boolean indicating success and the value of the target element before setting it.
 */
std::pair<bool, XValueRT> Set(const INode::SPtr& _node_this, XPath&& _path, XValue&& _val);

/**
 * @brief Optional sets the value of a node at the specified XPath.
 * @param _node_this INode instance to start traversing from.
 * @param _path XPath to traverse to the target node.
 * @param _val Value to set the target node to.
 * @return A pair of a boolean indicating success and the value of the target element before setting it.
 */
template <class T>
std::pair<bool, XValueRT> OptionalSet(const INode::SPtr& _node_this, XPath&& _path, const std::optional<T> _val)
{
    if (!_val.has_value())
        return {false, xnode::At(_node_this, std::move(_path))};

    return xnode::Set(_node_this, std::move(_path), _val.value());
}

/**
 * @brief Sets the value of a node at the specified XPath.
 * @param _node_value XValue with INode instance to set.
 * @param _path XPath to traverse to the target node.
 * @param _val Value to set the target node to.
 * @return A success flag and the value of the target element before setting it.
 */
std::pair<bool, XValueRT> NodeSet(XValueRT& _node_value, XPath&& _path, XValue&& _val);
/**
 * @brief Optioanl sets the value of a node at the specified XPath.
 * @param _node_value XValue with INode instance to set
 * @param _path XPath to traverse to the target node.
 * @param _val Value to set the target node to.
 * @return A success flag and the value of the target element before setting it.
 */
template <class T>
std::pair<bool, XValueRT> NodeOptionalSet(XValueRT& _node_value, XPath&& _path, const std::optional<T> _val)
{
    if (!_val.has_value())
        return {false, xnode::At(_node_value, std::move(_path))};

    return xnode::NodeSet(_node_value, std::move(_path), _val.value());
}

/**
 * @brief Create new node with base values and appended new value
 * @param _node_base INode instance with base values, can be null, for array type is ignored
 * @param _values new values added to existed node the target node.
 * @param _overwrite overwite original values flag
 * @return New node with orignial and appended values
 */
[[nodiscard]] INode::SPtr NodeCombine(const INode::SPtrC&                    _node_base,
                                      std::vector<std::pair<XKey, XValue>>&& _values,
                                      bool                                   _overwrite);
/**
 * @brief Inserts a new value into the specified XPath.
 * @param _node_this INode instance to start traversing from.
 * @param _path XPath to traverse to the target node.
 * @param _val Value to be inserted.
 * @returns The result of the insertion operation.
 */
INode::InsertRes Insert(const INode::SPtr& _node_this, XPath&& _path, XValue&& _val);
/**
 * @brief Erases a value from the specified XPath.
 * @param _node_this INode instance to start traversing from.
 * @param _path XPath to traverse to the target node.
 * @returns The erased value.
 */
XValueRT Erase(const INode::SPtr& _node_this, XPath&& _path);
/**
 * @brief Increases the value at the specified XPath by the specified value.
 * @param _node_this INode instance to start traversing from.
 * @param _path The XPath representing the element to be incremented.
 * @param _val The value to be added to the element.
 * @returns The updated value of the node.
 */
XValueRT Increment(const INode::SPtr& _node_this, XPath&& _path, XValue&& _val);

// If specified attribute exist -> convert to array
/**
 * @brief Emplaces a new value into an existing array node defined by XPath.
 * @details The function navigates through the given XPath and emplaces the new value into the existing array
 * node at the end. If target element is not an array node, it will be converted to an array node with two elements -
 * the previous value and the new value.
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
 * @note If element value is not an array or map node, it will be returned as a vector with one value.
 * @param _node_this INode instance to start traversing from.
 * @param _path The XPath to navigate through.
 * @param _only_for_type The return values only from Array or Map
 * @return A vector containing all values of the element by the specified XPath.
 */
[[nodiscard]] std::vector<XValueRT> ValuesList(const XValue&                        _target_value,
                                               XPath&&                              _path,
                                               const std::optional<INode::NodeType> _req_node_type = {});

/**
 * @brief Retrieves the values as a vector of element values of wanted types by the specified XPath.
 * @note If element value is not an array or map node, it will be returned as a vector with one value.
 * @param _node_this INode instance to start traversing from.
 * @param _path The XPath to navigate through.
 * @param _only_for_type The return values only from Array or Map
 * @return A vector containing all typed values of the element by the specified XPath.
 */
template <typename TValueType>
[[nodiscard]] std::vector<TValueType> TypedValuesList(const XValue&                        _target_value,
                                                      XPath&&                              _path,
                                                      const std::optional<INode::NodeType> _req_node_type = {})
{
    auto node_or_val = At(_target_value, std::move(_path));
    if (!node_or_val)
        return {};

    std::vector<TValueType> result;
    auto                    node = node_or_val.QueryPtrC<INode>();
    if (node && _req_node_type.value_or(node->Type()) == node->Type()) {
        node->BulkGetAll([&](const auto& key, const auto& val) {
            auto opt = val.template OptionalGet<TValueType>();
            if (opt.has_value())
                result.push_back(opt.value());
            return OnCopyRes::Skip;
        });
    }
    else if (!node && !_req_node_type.has_value()) {
        auto opt = node_or_val.template OptionalGet<TValueType>();
        if (opt.has_value())
            result.push_back(opt.value());
    }

    return result;
}

/**
 * @brief Retrieves the values as a vector of element values of specified enum by the specified XPath.
 * @note If element value is not an array or map node, it will be returned as a vector with one value.
 * @param _node_this INode instance to start traversing from.
 * @param _path The XPath to navigate through.
 * @param _for_error The enum value putted into vector if conversion to desired enum failed.
 * @param _only_for_type The return values only from Array or Map
 * @return A vector containing all typed values of the element by the specified XPath.
 */
template <typename TEnumType>
[[nodiscard]] std::vector<TEnumType> EnumValuesList(const XValue&                        _target_value,
                                                    XPath&&                              _path,
                                                    const std::optional<TEnumType>       _for_error     = {},
                                                    const std::optional<INode::NodeType> _req_node_type = {})
{
    auto node_or_val = At(_target_value, std::move(_path));
    if (!node_or_val)
        return {};

    std::vector<TEnumType> result;
    auto                   node = node_or_val.QueryPtrC<INode>();
    if (node && _req_node_type.value_or(node->Type()) == node->Type()) {
        node->BulkGetAll([&](const auto& key, const auto& val) {
            auto opt = val.template EnumGet<TEnumType>();
            if (opt.has_value())
                result.push_back(opt.value());
            else if (_for_error.has_value())
                result.push_back(_for_error.value());
            return OnCopyRes::Skip;
        });
    }
    else if (!node && !_req_node_type.has_value()) {
        auto opt = node_or_val.template EnumGet<TEnumType>();
        if (opt.has_value())
            result.push_back(opt.value());
        else if (_for_error.has_value())
            result.push_back(_for_error.value());
    }

    return result;
}

/**
 * @brief Retrieves the nodes at the given XPath.
 * @details The function navigates through the given XPath and returns all nodes at the path.
 * @param _node_this INode instance to start traversing from.
 * @param _include_const A flag indicating whether to include constant nodes in the result or not.
 * @param _path The XPath to navigate through.
 * @return A vector containing all nodes at the XPath.
 */
[[nodiscard]] std::vector<std::pair<XKey, XValueRT>> NodesList(const INode::SPtr& _node_this,
                                                               bool    _include_const, //??? may be set default value?
                                                               XPath&& _path = XPath());
/**
 * @brief Retrieves the constant nodes at the given XPath.
 * @param _node_this INode const instance to start traversing from.
 * @param _path XPath to traverse to the target node.
 * @return A vector containing all constant nodes at the XPath.
 */
[[nodiscard]] std::vector<std::pair<XKey, XValueRT>> NodesConstList(const INode::SPtrC& _node_this,
                                                                    XPath&&             _path = XPath());

/**
 * @brief Retrieves the nodes at the given XPath.
 * @details The function navigates through the given XPath and returns all nodes at the path.
 * @param _node_this INode instance to start traversing from.
 * @param _path The XPath to navigate through.
 * @param _child_nodes_unwrap Optionally 'unwrap' nodes like  { "name" : { node } } into { node } with corresponding
 * name. Applies to array nodes only.
 * @param _on_node_pf optional callback: return true for include into list, false for skip, std::nullopt for stop
 * @return A vector containing all nodes at the XPath.
 */
std::vector<INode::SPtr> ChildNodesGet(
    const INode::SPtr&                                                            _node_this,
    XPath&&                                                                       _path               = XPath(),
    const bool                                                                    _child_nodes_unwrap = false,
    std::function<std::optional<bool>(const INode::SPtr& _node, size_t _taken)>&& _on_node_pf         = {});

/**
 * @brief Retrieves the nodes at the given XPath.
 * @details The function navigates through the given XPath and returns all nodes at the path.
 * @param _node_this INode instance to start traversing from.
 * @param _include_const A flag indicating whether to include constant nodes in the result or not.
 * @param _path The XPath to navigate through.
 * @param _child_nodes_unwrap Optionally 'unwrap' nodes like  { "name" : { node } } into { node } with name
 * @param _on_node_pf optional callback: return true for include into list, false for skip, std::nullopt for stop
 * @return A vector containing all nodes at the XPath.
 */
std::vector<INode::SPtrC> ChildNodesConstGet(
    const INode::SPtrC&                                                            _node_this,
    XPath&&                                                                        _path               = XPath(),
    const bool                                                                     _child_nodes_unwrap = false,
    std::function<std::optional<bool>(const INode::SPtrC& _node, size_t _taken)>&& _on_node_pf         = {});

/**
 * @brief Wrap node into map: {"name" : { node }}
 * (used for keed nodes names e.g. in arrays)
 * @param _node_val The original node to be wrapped
 * @return A pair containing the wrapped value and a boolean indicating if the wrapping was successful.
 * @note If the wrapping fails, the original node is returned.
 */
[[nodiscard]] std::pair<XValue, bool> NodeWrap(XValue&& _node_val);

/**
 * @brief Unwrap node from map {"name" : { node }}
 * (used for keed nodes names e.g. in arrays)
 * @param _node_val The wrapped node to be unwrapped
 * @return A pair containing the unwrapped value and a boolean indicating if the unwrapping was successful.
 * @note If the unwrapping fails, the original wrapped node is returned.
 */
[[nodiscard]] std::pair<XValue, bool> NodeUnwrap(XValue&& _node_val);

/**
 * @brief Insert wrapped node into array {"name" : { node }}
 * (used for keed nodes names in array)
 * @param _node_array target INode instance
 * @param _val Value to set the target node to (e.g. node for wrap)
 * @param _insert_at index in array (default is add to the end of array)
 * @return The result of the insertion operation. See result structure here: @ref InsertRes.
 */
INode::InsertRes ArrayInsertWrapped(const INode::SPtr& _node_array, XValue&& _val, const size_t _insert_at = kIdxEnd);

/**
 * @brief Wrap nodes with names into [ { "name_of_node", { ...node... } }, ... ]
 * @param _array_node target INode instance.
 * @return Updated node or orignal one if no chnages
 */
XValue ArrayNodesWrap(XValue&& _array_val);

/**
 * @brief Unwrap nodes like [ { "name_of_node", { ...node... } } ...] into plain nodes
 * [ { ...node... } ]
 * @param _array_node target INode instance.
 * @return Updated node or orignal one if no chnages
 */
XValue ArrayNodesUnwrap(XValue&& _array_val);

/**
 * @brief Convert an XKey to XValue.
 * @param _key The XKey to convert.
 * @return The XValue corresponding to _key.
 * @note After conversion, the value will be of type size_t or string, or will be empty.
 */
[[nodiscard]] XValue ValueFromKey(const XKey& _key);
/**
 * @brief Convert an XValue to XKey.
 * @param _value The XValue to convert.
 * @return The XKey corresponding to _value or empty if the conversion fails.
 */
[[nodiscard]] XKey KeyFromValue(const XValue& _value);

/// @brief Retrieves a shared_ptr of a TObject from this XValue.
/// @tparam TObject The object type.
template <typename TObject>
[[nodiscard]] std::shared_ptr<TObject> ObjectAt(const INode::SPtr&              _node_this,
                                                XPath&&                         _path,
                                                const std::shared_ptr<TObject>& _default = {})
{
    return xnode::At(_node_this, std::move(_path)).QueryPtr(_default);
}

/// @brief Retrieves a shared_ptr of a const TObject from this XValue.
/// @tparam TObject The object type.
template <typename TObject>
[[nodiscard]] std::shared_ptr<const TObject> ConstObjectAt(const INode::SPtrC&                   _node_this,
                                                           XPath&&                               _path,
                                                           const std::shared_ptr<const TObject>& _default = {})
{
    auto value           = xnode::At(_node_this, std::move(_path));
    auto check_non_const = value.QueryPtr<TObject>();
    if (check_non_const)
        return check_non_const;

    return value.QueryPtrC(_default);
}

template <typename TValue>
std::optional<TValue> LoadValue(const INode*                _node_p,
                                const XKey&                 _key,
                                TValue*                     _out_p,
                                const std::optional<TValue> _ignore_value = {})
{
    // No reason for use LoadXXX w/o _out_p - use At(_key).OptionalGet<TValue>() instead
    assert(_out_p);

    if (!_node_p)
        return std::nullopt;

    auto val = _node_p->At(_key).template OptionalGet<TValue>();
    if (!val.has_value())
        return std::nullopt;

    if (_ignore_value.has_value() && val.value() == _ignore_value.value())
        return std::nullopt;

    if (_out_p)
        *_out_p = val.value();

    return val;
}

template <typename TValue>
std::optional<TValue> LoadOptional(const INode* _node_p, const XKey& _key, std::optional<TValue>* _out_p)
{
    // No reason for use LoadXXX w/o _out_p - use At(_key).OptionalGet<TValue>() instead
    assert(_out_p);

    if (!_node_p)
        return std::nullopt;

    auto val = _node_p->At(_key).template OptionalGet<TValue>();
    if (!val.has_value())
        return std::nullopt;

    if (_out_p)
        *_out_p = val.value();

    return val;
}

template <typename TEnum>
std::optional<TEnum> LoadEnum(const INode* _node_p, const XKey& _key, TEnum* _out_p)
{
    // No reason for use LoadXXX w/o _out_p - use At(_key).EnumGet<TEnum>() instead
    assert(_out_p);

    if (!_node_p)
        return std::nullopt;

    auto val = _node_p->At(_key).template EnumGet<TEnum>();
    if (!val.has_value())
        return std::nullopt;

    if (_out_p)
        *_out_p = val.value();

    return val;
}

} // namespace xsdk::xnode

// Utility functions, move to separate file ?
namespace xsdk::xnode::utility {
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

} // namespace xsdk::xnode::utility