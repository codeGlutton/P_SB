#pragma once

class GlobalQueue
{
public:
	GlobalQueue();
	~GlobalQueue();

	void										PushJobQ(JobQueueRef jobQueue);
	JobQueueRef									PopFromJobQ();

	void										PushProdConsQ(ProducerConsumerQueueRef prodConsQueue);
	ProducerConsumerQueueRef					PopFromProdConsQ();

private:
	class LockQueue<JobQueueRef>				_jobQueue;
	class LockQueue<ProducerConsumerQueueRef>	_prodConsQueue;
};

