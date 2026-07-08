#pragma once

#include "xnode_symbols.h"
#include "xnode_interfaces.h"

#include "xbase/xscheduler.h"

#include <functional>
#include <system_error>

namespace xsdk::xnode {

class XNODE_API INodeAutoSave: public xbase::PtrBase<INodeAutoSave> {
public:
    using SavePF = std::function<std::error_code(const INode::SPtrC& _node)>;

    virtual ~INodeAutoSave() = default;

    /**
     * @brief Synchronously saves pending changes and clears the dirty state.
     * @details Does nothing when there are no pending changes or after Stop() has been called.
     *          If a scheduled save is already running, waits for it before returning.
     *          Must not be called from SavePF.
     */
    virtual void Flush() = 0;
    /**
     * @brief Stops watching the subtree and optionally saves pending changes.
     * @param _flush_pending If true, pending changes are saved before the object becomes inactive.
     * @details Stop(true) waits for an already running scheduled save. The destructor calls Stop(true).
     *          Must not be called from SavePF.
     */
    virtual void Stop(bool _flush_pending) = 0;
    /**
     * @brief Returns true while the auto-save object accepts subtree changes.
     */
    virtual bool Active() const = 0;
    /**
     * @brief Last SavePF error.
     * @note Failed saves are not retried automatically. The owner should inspect this value and call Flush()
     *       after fixing the underlying problem or changing the tree again.
     */
    virtual std::error_code LastError() const = 0;
};

struct NodeAutoSaveParams {
    /**
     * @brief Root node to watch.
     */
    INode::SPtr node;
    /**
     * @brief Save callback called by the scheduler worker or by Flush()/Stop(true).
     * @details Calls are serialized by INodeAutoSave. The callback must not call Flush() or Stop() on the same
     *          auto-save object.
     */
    INodeAutoSave::SavePF save;
    /**
     * @brief Delay after the latest visible subtree change before saving.
     */
    xbase::Time64 debounce = 100 * time64::kMsec;
    /**
     * @brief Maximum delay after the first unsaved visible subtree change.
     */
    xbase::Time64 max_interval = 500 * time64::kMsec;
    /**
     * @brief Scheduler used for delayed saves. If null, the default scheduler is used.
     */
    xbase::IScheduler* scheduler_p = nullptr;
    /**
     * @brief Optional worker for scheduled saves.
     */
    xbase::IWorker::SPtr worker;
};

XNODE_API INodeAutoSave::SPtr CreateNodeAutoSave(NodeAutoSaveParams&& _params);

} // namespace xsdk::xnode
