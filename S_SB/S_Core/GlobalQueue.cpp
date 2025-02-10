#include "pch.h"
#include "GlobalQueue.h"
#include "LockQueue.h"

GlobalQueue::GlobalQueue()
{
}

GlobalQueue::~GlobalQueue()
{
}

void GlobalQueue::PushJobQ(JobQueueRef jobQueue)
{
	_jobQueue.Push(jobQueue);
}

JobQueueRef GlobalQueue::PopFromJobQ()
{
	return _jobQueue.Pop();
}

void GlobalQueue::PushProdConsQ(ProducerConsumerQueueRef prodConsQueue)
{
	_prodConsQueue.Push(prodConsQueue);
}

ProducerConsumerQueueRef GlobalQueue::PopFromProdConsQ()
{
	return _prodConsQueue.Pop();
}
