#pragma once

namespace Protocol { struct R_SERVER_DATA; }

/*************************
	DBPlayerTaskExecutor
**************************/

// Player Redis DB 요청을 받아 처리하는 ProdConsQueue
class DBPlayerTaskExecutor : public ProducerConsumerQueue
{
public:
	DBPlayerTaskExecutor();
	virtual ~DBPlayerTaskExecutor();

	/* thread-unsafe (prodConsQ로 보호) */

	// 주기적으로 ConnectionRoom에서 밀집도를 받아 Redis DB에 현재 서버 정보 업데이트
	void							UpdateServerInfo(float dencity);

	void							GetVerifiedAccount(GameSessionRef gameSession, int32 accountId, xString tokenValue);
	void							ClearVerifiedAccount(int32 accountId);

private:
	const std::string				GetJwtPasswordStr();

private:
	Protocol::R_SERVER_DATA*		_serverInfo;
	xString							_jwtPassword;
};

/*************************
	DBAchvTaskExecutor
**************************/

// Achievement Redis  DB 요청을 받아 처리하는 ProdConsQueue
class DBAchvTaskExecutor : public ProducerConsumerQueue
{
public:
	/* thread-unsafe (prodConsQ로 보호) */
};