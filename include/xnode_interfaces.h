#pragma once

#include "xconstant.h"
#include "xkey/xkey.h"
#include "xvalue/xvalue_rt.h"
#include "xbase.h"

#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace xsdk {

class INode: public IObject {
    // 2think: about using ?
    // using key_vec        = std::vector<XKey>;
    // using value_vec      = std::vector<XValue>;
    // using key_value_vec  = std::vector<std::pair<XKey, XValue>>;
    // using key_node_vec   = std::vector<std::pair<XKey, INode::SPtr>>;
    // using key_node_c_vec = std::vector<std::pair<XKey, INode::SPtrC>>;\

public:
    // Declate shared, unique, weak pts (for override PtrBase<IObject> in base class)
    USING_PTRS(INode)

    /**
     * @brief   Defines an enumeration class 'NodeType' with two possible values: 'Map' and 'Array'
     * @details This enumeration class is used to define different types of nodes in a data structure.
     */
    enum class NodeType { Map = 1, Array = 2 };

    /**
     * @brief   Enum class representing different types of callback reasons.
     *
     * @details This enum class defines three types of callback reasons: @ref Changes, @ref ChangesNoDiscard,
     * and @ref Rollback. These reasons determine what triggered the callback. See usage example @ref OnChangePF.
     *
     * @see CallbackReason::Changes
     * @see CallbackReason::ChangesNoDiscard
     * @see CallbackReason::Rollback
     * @see @ref callback_usage_example.cpp "Callback usage example"
     */
    enum class CallbackReason {
        /**
         * @brief Callback reason indicating that changes have been made but should not be discarded.
         *
         * This reason is sent when the changes made to the object are still pending and can be discarded or committed.
         */
        Changes,
        /**
         * @brief Callback reason indicating that changes have been made.
         *
         * This reason is sent when the changes made to the object have been accepted and committed.
         */
        ChangesNoDiscard,
        /**
         * @brief Callback reason indicating that a rollback has occurred.
         *
         * This reason is sent when an operation that resulted in changes has been rolled back, undoing those changes.
         */
        Rollback };

    /**
     * @brief Defines a function type alias for an OnChange callback function.
     *
     * This callback function takes the following arguments:
     * @tparam CallbackReason   The reason for the callback being invoked. @See CallbackReason.
     * @tparam SPtrC            A shared pointer to the node that triggered the change.
     * @tparam XKey             The key of element that was modified.
     * @tparam XValueRT         The previous value of the element.
     * @tparam XValueRT         The new value of the element.
     *
     * The function should return one of the following:
     * - If the function allows the change to proceed, return `true`.
     * - If the function prohibits the change, return `false`.
     * - If the callback needs to be removed, return `std::nullopt`.
     * @note Return value doesn't matter if CallbackReason was set to ChangesNoDiscard or Rollback.
     * @see @ref callback_usage_example.cpp "Callback usage example"
     */
    using OnChangePF = std::function<
        std::optional<bool>(CallbackReason, const INode::SPtrC&, const XKey&, const XValueRT&, const XValueRT&)>;
    /**
     * @example{lineno} callback_usage_example.cpp
     * This is an example of how to use the OnChange callbacks.
     */

public:
    //-------------------------------------------------------------------------------
    // INode specific methods
    /**
     * @brief  Returns the type of the node based on the type of the container it is associated with.
     * @return The type of the node.
     * @see    NodeType
     */
    virtual NodeType Type() const = 0;

    ///@name Parent methods
    ///@{
    /**
     * @brief  Get the constant parent node.
     * @return INode::SPtrC to the parent node, or nullptr if
     *         the node has no parent.
     * @note   This method provides the same functionality as ParentGet(), but
     *         returns a constant reference.
     */
    virtual INode::SPtrC ParentGet() const = 0;
    /**
     * @brief  Get the parent node.
     * @return INode::SPtr to the parent node, or nullptr if the
     *         node has no parent.
     */
    virtual INode::SPtr  ParentGet()       = 0;
    /**
     * @brief                       Sets a new parent node for the current node.
     * @param _parent               An optional pointer to a parent node. If nullptr, this function detach the current parent.
     * @param _name_for_new_parent  An optional name to give to the current node in the new parent node.
     * @return                      A pair containing two items.
     *                               - The first value is a bool indicating if the operation was successful or not.
     *                               - The second value is the previous parent node or a nullptr if no parent node existed before.
     * @note:                       Name chnaged ONLY if parent update (2Think!)
     */
    virtual std::pair<bool, INode::SPtr> ParentSet(
        INode::SPtr                     _parent_p,
        std::optional<std::string_view> _name_for_new_parent = std::nullopt) = 0;
    /**
     * @brief  Detaches this node from its current parent and returns the previous parent node.
     * @return INode::SPtr to the parent node, or nullptr if the
     *         node has no parent.
     */
    virtual INode::SPtr ParentDetach()                                       = 0;
    ///@}

    ///@name Node name methods
    ///@{
    /**
     * @brief  Function to get the name of this node.
     * @return The name of the node.
     */
    virtual std::string                  NameGet() const                                          = 0;
    /**
     * @brief                Function to set the name of this node.
     * @param _name_set      The new name of the node.
     * @param _update_parent Optional parameter to determine whether to update the parent node name. Default is
     * false.
     * @return               A pair containing two items.
     *                        - The first value is a bool indicating if the operation was successful or not.
     *                        - The second value is the old name if the name was changed.
     */
    virtual std::pair<bool, std::string> NameSet(std::string_view _name_set, bool _update_parent) = 0;
    /**
     * @brief             Function to check if the name of the object matches the given name.
     * @param _name_check The name to check against.
     * @return            \c true if the name matches, \c false otherwise.
     */
    virtual bool                         IsName(std::string_view _name_check) const               = 0;
    ///@}

    ///@name OnChange callback methods
    ///@{
    //-------------------------------------------------------------------------------
    // Callbacks, return uid for subsiqent remove this cb
    /**
     * @brief               Add a callback function to be called on any change of a node.
     * @param _pf_on_change A callback function to be called on any change of a node.
     * @param _id           An unique identifier for the callback. If the identifier is 0 new unique identifier
     *                      will be generated. If the identifier already exists it will be used.
     * @return              An unique identifier for the callback.
     * @see @ref callback_usage_example.cpp "Callback usage example"
     */
    virtual uint64_t OnChangeAdd(OnChangePF&& _pf_on_change, uint64_t _id = 0) const = 0;
    /**
     * @brief     Remove a callback function by identifier.
     * @param _id An identifier for the callback.
     * @return    \c true if the callback function was found and removed, \c false if it wasn't.
     * @see @ref callback_usage_example.cpp "Callback usage example"
     */
    virtual bool     OnChangeRemove(uint64_t _id) const                              = 0;
    /**
     * @brief  Reset all the callbacks.
     * @return The number of callbacks removed.
     * @see @ref callback_usage_example.cpp "Callback usage example"
     */
    virtual size_t   OnChangeReset() const                                           = 0;
    ///@}

    //-------------------------------------------------------------------------------
    ///@name IContainer direct related methods
    ///@{
    /**
     * @brief                        Check if a given key is valid.
     * @param _map_access_by_index   If \c true, the key is given as index to the key/value map.
     * @param _key                   The key to check.
     * @return                       \c true if the key is valid, else \c false.
     */
    virtual bool     IsKeyValid(bool _map_access_by_index, const XKey& _key) const = 0;
    /**
     * @brief   Erases all elements from the container.
     * @details This method iterates over the nodes in the container and detach all nodes
     * that have an associated with that one. It then clears the container of other elements.
     */
    virtual void     Clear()                                                       = 0;
    /**
     * @brief Return the number of elements in the container.
     */
    virtual size_t   Size() const                                                  = 0;
    /**
     * @brief Checks if the container has no elements or, in other words, if the node has no children.
     */
    virtual bool     Empty() const                                                 = 0;

    /**
     * @brief       Find the value associated with the given key in this node.
     * @param _key  The key.
     * @return      The XValueRT (value with timestamp) associated with the given key, or an empty value.
     * @note        This method provides the same functionality as At(const XKey& _key), but
     *       returns a constant reference.
     */
    virtual XValueRT At(const XKey& _key) const                                    = 0;
    /**
     * @brief       Find the value associated with the given key in this node.
     * @param _key  The key.
     * @return      The XValueRT (value with timestamp) associated with the given key, or an empty value.
     */
    virtual XValueRT At(const XKey& _key)                                          = 0;

    /**
    * @brief                Iterate over the node items and apply the specified function on each item.
    * @param _pf_on_item    Function that will be applied to each item in the container.
    * @param   _from_key    The starting key in the container. If set to the empty key (default),
    *                       the function will be called on all items in the container.
    * @returns              false if container is empty or _from_key not found
    * @note                 Do not allow for change to nodes.
    * @note                 Also iterate through removed nodes too.
    */
    virtual bool ForPatch(std::function<bool(const XKey&, const XValueRT&)>&& _pf_on_item,
                          const XKey&                                         _from_key = XKey()) const = 0;

    // Method for take, Erase, change items via callback
    /**
     * @brief               Iterate over the node items and apply the specified function on each item.
     * @param  _pf_on_item  Callback function that receives node and key as arguments.
     * @param  _from_key    Key from which to begin iteration. If set to the empty key (default),
     *                      the function will be called on all items in the container.
     * @return              false if container is empty or _from_key not found
     * @note                The function manages changes in the nodes connected to the keys
     *                      and handles their removal or addition accordingly.
     */
    virtual bool ForEach(std::function<OnEachRes(const XKey&, XValueRT&)>&& _pf_on_item,
                         const XKey&                                        _from_key = XKey()) = 0;

    // Return {success, previous value}
    /**
     * @brief       Set the value of the given key.
     * @param _key  The key of the child element.
     * @param _val  The value of the child element.
     *
     * @return      A pair containing two items.
     *               - The first value is a bool indicating if the operation was successful or not.
     *               - The second value is the old value that was overwritten, if any.
     */
    virtual std::pair<bool, XValueRT> Set(const XKey& _key, XValue&& _val) = 0;

    /**
     * @brief Represents the result of an insertion operation.
     *
     * This structure holds the result of an insertion operation. If the insertion
     * was successful, the 'succeeded' flag is set to \c true, the index of the inserted
     * element is stored in the 'inserted_at' field and the 'existed' field is empty.
     * If the insertion failed, the 'succeeded' flag is set to false and the 'existed'
     * field contains the value that was present at the insertion position, if any.
     */
    struct InsertRes {
        /**
         * @brief Indicates whether the insertion was successful.
         */
        bool     succeeded = false;
        /**
         * @brief The key of the inserted element.
         */
        XKey     inserted_at;
        /**
         * @brief The value that was present at the insertion position, if any.
         */
        XValueRT existed; // empty if succeeded or canceled via cb (?)
    };
    /**
     * @brief       Performs an insertion into the node's container.
     *
     * @param _key  The key to be inserted.
     * @param _val  The value to be inserted.
     * @return      The result of the insertion operation. See result structure here: @ref InsertRes.
     */
    virtual InsertRes Insert(const XKey& _key, XValue&& _val) = 0;

    /**
     * @brief       Erases the element with the given key from the container and returns the value associated with it.
     *
     * @param _key  The key of the element to be erased.
     * @return      The value associated with the erased element (xo_empty if not found).
     */
    virtual XValueRT Erase(const XKey& _key) = 0;

    // todo: implement in helpers via ForEach
    // virtual std::optional<XKey> find_value(const XValue& ValueAt_) const = 0;
    // virtual std::optional<XKey> erase_value(const XValue& ValueAt_) = 0;

    /**
     * @brief       Performs a key change in node container from key \p _from to key \p _to.
     * @param _from The key to change from.
     * @param _to   The key to change to.
     * @return      \c true if change was successful, \c false otherwise.
     * @warning     This function assumes that both keys \p _from and \p _to are valid and belong to the same type.
     */
    virtual bool KeyChange(const XKey& _from, const XKey& _to) = 0;
    ///@}

    //-------------------------------------------------------------------------------
    ///@name IContainer atomic modification methods
    /// Return resulting value
    ///@{
    /**
     * @brief               Appends the given string to the value associated with the given key in the container.
     * @param _key          The key identifying the container element to be modified.
     * @param _append_str   The string to be appended to the current value.
     * @return              The updated XValueRT for the key.
     * @note                This method uses a read-write lock to ensure thread safety.
     */
    virtual XValueRT Append(const XKey& _key, std::string_view _append_str)    = 0;
    /**
     * @brief                 Increment the value of the container for the given key with the provided increment_val.
     * @param _key            The key for the container.
     * @param _increment_val  The value to increment the container's value by.
     * @return                The new value of the container after the incrementation.
     */
    virtual XValueRT Increment(const XKey& _key, const XValue& _increment_val) = 0;
    /**
     * @brief               Performs an atomic compare-and-exchange operation on a container.
     * @param _key          The key to check for the value to be exchanged.
     * @param _expected     The value expected to be in the container before the exchange.
     * @param _exchange_to  The new value to exchange, moved into the container if the conditions are met.
     * @return              A pair containing two items.
     *                       - The first value is a bool indicating if the operation was successful or not.
     *                       - The second value is the old value from the container.
     */
    virtual std::pair<bool, XValueRT> CompareExchange(const XKey&   _key,
                                                      const XValue& _expected,
                                                      XValue&&      _exchange_to) = 0;
    ///@}

    //-------------------------------------------------------------------------------
    ///@name IContainer bulk/helpers methods
    ///@{
    // 2Think: better/distinct names (?) e.g. bulk_at/bulk_collect

    // Return vector of {key, value} of taken elements byf specified keys
    /**
     * @brief Bulk retrieves values for given keys without a custom callback function.
     * @details The BulkGet method without a custom callback function retrieves values for multiple keys at once, returning them
     * as a vector of pairs: {key, value}.
     * @param _keys Vector of keys to retrieve values for.
     * @return A vector of pairs: {key, value} for using them read only.
     */
    virtual std::vector<std::pair<XKey, XValueRT>> BulkGet(const std::vector<XKey>& _keys) const = 0;
    /**
     * @brief Bulk retrieves values for given keys without a custom callback function.
     * @details The BulkGet method without a custom callback function retrieves values for multiple keys at once,
     * returning them as a vector of pairs: {key, value}.
     * @param _keys Vector of keys to retrieve values for.
     * @return A vector of pairs: {key, value}.
     */
    virtual std::vector<std::pair<XKey, XValueRT>> BulkGet(const std::vector<XKey>& _keys)       = 0;

    // Return vector of {key, value} of taken elements, take or not elements decided by callback res,
    // - default(empty) callback is OnCopyRes::Take
    /**
     * @brief Bulk retrieves all values in the container.
     * @details Return a vector of {key, value} of taken elements, the result of callback decides whether or not to take items.
     * If no callback has been set, all elements are get. @see OnCopyRes::Take
     * @param _pf_on_item Callback function to process each item.
     * @param _key_begin Beginning of the range of keys, or an empty optional if retrieving all keys.
     * @return A vector of pairs: {key, value} for using them read only.
     */
    virtual std::vector<std::pair<XKey, XValueRT>> BulkGetAll(
        std::function<OnCopyRes(const XKey&, const XValueRT&)>&& _pf_on_item = nullptr,
        const XKey&                                              _key_begin  = XKey()) const = 0;
    /**
     * @brief Bulk retrieves all values in the container.
     * @details Return a vector of {key, value} of taken elements, the result of callback decides whether or not to take
     * items. If no callback has been set, all elements are get. @see OnCopyRes::Take
     * @param _pf_on_item Callback function to process each item.
     * @param _key_begin Beginning of the range of keys, or an empty optional if retrieving all keys.
     * @return A vector of pairs: {key, value}.
     */
    virtual std::vector<std::pair<XKey, XValueRT>> BulkGetAll(
        std::function<OnCopyRes(const XKey&, const XValueRT&)>&& _pf_on_item = nullptr,
        const XKey&                                              _key_begin  = XKey()) = 0;

    /**
     * @brief Sets multiple new children for the node in one call.
     * @param[in,out] _values List of new children with their corresponding keys. Successfully set elements are removed from the vector of values.
     * @return Number of successfully set children.
     */
    virtual size_t BulkSet(std::vector<std::pair<XKey, XValue>>&& _values) = 0;

    /**
     * @brief Inserts multiple key-value pairs into the container.
     * @param[in,out] _values A vector of key-value pairs to be inserted. Successfully inserted elements are removed from the vector of values.
     * @return The number of elements successfully inserted.
     */
    virtual size_t BulkInsert(std::vector<std::pair<XKey, XValue>>&& _values) = 0;

    /**
     * @brief Inserts multiple new values into the array node.
     * @param _insert_pos The position to insert the nodes.
     * @param[in,out] _values The vector of new values to be inserted. Successfully inserted elements are removed from the vector of values.
     * @return A std::pair containing the number of values that were successfully inserted and
     * the key of the last inserted value.
     * @note The method is acceptable only for a node of array type.
     */
    virtual std::pair<size_t, XKey> BulkInsert(XKey _insert_pos, std::vector<XValue>&& _values) = 0;

    /**
     * @brief Bulk erases the elements associated with the given keys from the node container.
     * @param[_keys] A vector of keys to be erased from the node container.
     * @return A vector containing pairs of node key and the corresponding extracted value.
     */
    virtual std::vector<std::pair<XKey, XValueRT>> BulkErase(const std::vector<XKey>& _keys) = 0;
    ///@}
};

} // namespace xsdk