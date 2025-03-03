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
	PlayerDataProtector() : currentPlayer(), isVerified(false), rtt(0.f), _accountId(), _players(), _state(STATE::EMPTY), _startPingTime(), _isMeasuringPing(false) {}

public:
	std::atomic<PlayerRef>					currentPlayer;
	std::atomic<bool>						isVerified;
	float									rtt;

private:
	int32									_accountId;
	xVector<PlayerRef>						_players;
	enum STATE : uint8 
	{
		EMPTY,
		READY,
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

private:
	// Connect 후 JWT 인증 절차 제한시간
	static const uint64					AUTH_TIMEOUT_MS;
};