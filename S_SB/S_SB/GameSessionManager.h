#pragma once

class GameSessionManager
{
public:
	void					Add(GameSessionRef session);
	void					Remove(GameSessionRef session);
	void					Broadcast(SendBufferRef sendBuffer);

private:
	USE_LOCK;

	xSet<GameSessionRef>	_sessions;
};

extern GameSessionManager*	GSessionManager;
