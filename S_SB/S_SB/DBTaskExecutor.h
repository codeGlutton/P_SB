#pragma once

namespace Protocol { class R_SERVER_DATA; class R_ACCOUNT_DATA; class R_PLAYER_DATA; }

/*************************
	DBEnterTaskExecutor
**************************/

// 입장 관련 Redis DB 요청을 받아 처리하는 ProdConsQueue
class DBEnterTaskExecutor : public ProducerConsumerQueue
{
public:
	DBEnterTaskExecutor();
	virtual ~DBEnterTaskExecutor();

	/* thread-unsafe (prodConsQ로 보호) */

	void							GetVerifiedAccount(GameSessionRef gameSession, int32 accountId, xString tokenValue);
	void							ClearVerifiedAccount(int32 accountId, Protocol::R_ACCOUNT_DATA accountData);

private:
	const std::string				GetJwtPasswordStr();

	// 초기 계정 정보를 가져오기
	const std::string				DownloadAccountPlayers(const int32& accountId);
	// 종료 전 최신 계정 정보를 내보내기 (MSSQL과 Redis의 업데이트 간격에 의한 불일치 방지)
	void							UploadAccountPlayers(const int32& accountId, const Protocol::R_ACCOUNT_DATA& accountData);

private:
	xString							_jwtPassword;

	/* Redis 키 값들 */

	xString							_accountKeyPrefix;
	xString							_playingAccountKey;

	// 게임 종료 시 계정 정보 TTL (웹 서버의 DB 업데이트의 주기 * 1.5)
	static const int32				EXIT_ACCOUNT_TTL_MIN;
};

/*************************
	DBUpdateTaskExecutor
**************************/

// 업데이트 관련 Redis  DB 요청을 받아 처리하는 ProdConsQueue
class DBUpdateTaskExecutor : public ProducerConsumerQueue
{
public:
	DBUpdateTaskExecutor();
	virtual ~DBUpdateTaskExecutor();

	/* thread-unsafe (prodConsQ로 보호) */

	// 주기적으로 ConnectionRoom에서 밀집도를 받아 Redis DB에 현재 서버 정보 업데이트
	void							UpdateServerInfo(float dencity);
	void							UpdatePlayer(int32 accountId, Protocol::R_PLAYER_DATA playerData);

private:
	Protocol::R_SERVER_DATA*		_serverInfo;

	/* Redis 키 값들 */

	xString							_serverKey;
	xString							_updateAccountKey;
};