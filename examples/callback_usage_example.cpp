// Create a node
auto spMap = node_call::Create(INode::NodeType::Map);

// Function of printing the count of calls and their reasons
INode::OnChangePF printer = [](INode::CallbackReason _cbr,
                               const INode::SPtrC&   _node,
                               const XKey&           _key,
                               const XValueRT&       _from,
                               const XValueRT&       _to) {
    std::string reason = "unknown";
    switch (_cbr)
    {
        case INode::CallbackReason::Changes:
            reason = "Changes";
            break;
        case INode::CallbackReason::ChangesNoDiscard:
            reason = "Changes No Discard";
            break;
        case INode::CallbackReason::Rollback:
            reason = "Rollback";
            break;
        default:
            break;
    }
    static int count_calls {0};
    if (_key.StringGet() == "num_clients") {
        count_calls++;
        std::cout << count_calls << " reason: " << reason << std::endl;
    }
    return true;
};
// Function that checks that the number of clients should not be greater than 10
INode::OnChangePF limit_num_clients = [](INode::CallbackReason _cbr,
                          const INode::SPtrC&   _node,
                          const XKey&           _key,
                          const XValueRT&       _from,
                          const XValueRT&       _to) {
    if (_key.StringGet() == "num_clients") {
        if (_to.Int64() > 10) {
            std::cout << "Reached the maximum number of clients" << std::endl;
            return false;
        }
    }
    return true;
};
// Add OnChange callbacks
spMap->OnChangeAdd(std::move(printer));
spMap->OnChangeAdd(std::move(limit_num_clients));

spMap->Set("num_clients", 0);
// Output: 1 reason: Changes
spMap->Set("num_clients", 1);
// Output: 2 reason: Changes

// Try setting "num_clients" to a value greater than 10
spMap->Set("num_clients", 11);
// Output: 3 reason: Changes
// Output: Reached the maximum number of clients
// Output: 4 reason: Rollback

// The value in the "num_clients" element will be 1.
