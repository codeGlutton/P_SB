#pragma once
#include "Session.h"
#include "Player.h"
#include <chrono>

/************************
	PlayerDataProtector
*************************/

class PlayerDataProtector
{
public:
	PlayerDataProtector() : currentPlayer(), rtt(0.f), _players(), _state(STATE::EMPTY), _startPingTime(), _isMeasuringPing(false) { }

public:
	std::atomic<PlayerRef>					currentPlayer;
	float									rtt;

private:
	xVector<PlayerRef>						_players;
	enum STATE : uint8 
	{
		EMPTY,
		LOADED,
		FULL,
		DELETED
	}										_state;

	std::chrono::steady_clock::time_point	_startPingTime;
	bool									_isMeasuringPing;

	friend class ConnectionRoom;
};

/************************
	   GameSession
*************************/

class GameSession : public PacketSession
{
public:
	GameSession() : PacketSession()
	{
		playerDataProtector = MakeXShared<PlayerDataProtector>();
	}

	~GameSession()
	{
		std::cout << "~GameSession" << std::endl;
	}

	virtual void						OnConnected() override;
	virtual void						OnDisconnected() override;
	virtual void						OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void						OnSend(int32 len) override;

public:
	PlayerDataProtectorRef				playerDataProtector;
};