#pragma once

#include "CoreMinimal.h"

template <typename InElementType, typename InPriorityType, typename InOrderType>
struct TPriorityQueueNode
{
    InElementType Element;
    InPriorityType Priority;

    TPriorityQueueNode()
    {
    }

    TPriorityQueueNode(const InElementType& InElement,const InPriorityType& InPriority)
    {
        Element = InElement;
        Priority = InPriority;
    }

    TPriorityQueueNode(InElementType&& InElement, InPriorityType&& InPriority)
    {
        Element = InElement;
        Priority = InPriority;
    }

    bool operator<(const TPriorityQueueNode<InElementType, InPriorityType, InOrderType> Other) const
    {
        InOrderType OrderCheck;
        return OrderCheck(Priority, Other.Priority);
    }
};

template <typename InElementType, typename InPriorityType, typename InOrderType = TLess<InPriorityType>>
class TPriorityQueue
{
    using NodeType = TPriorityQueueNode<InElementType, InPriorityType, InOrderType>;

public:
	TPriorityQueue()
	{
		_Array.Heapify();
	}

public:
    const InElementType& Peek()
    {
        return _Array.HeapTop().Element;
    }

    InElementType Pop()
    {
        return PopNode().Element;
    }

    NodeType PopNode()
    {
        NodeType Node;
        _Array.HeapPop(Node);
        return Node;
    }

    void Push(const InElementType& Element, const InPriorityType& Priority)
    {
        _Array.HeapPush(NodeType(Element, Priority));
    }

    void Push(InElementType&& Element, InPriorityType&& Priority)
    {
        _Array.HeapPush(NodeType(MoveTempIfPossible(Element), MoveTempIfPossible(Priority)));
    }

    void PushNode(const NodeType& Priority)
    {
        _Array.HeapPush(Priority);
    }

    void PushNode(NodeType&& Priority)
    {
        _Array.HeapPush(MoveTempIfPossible(Priority));
    }

    bool IsEmpty() const
    {
        return _Array.Num() == 0;
    }

private:
    TArray<NodeType> _Array;
};