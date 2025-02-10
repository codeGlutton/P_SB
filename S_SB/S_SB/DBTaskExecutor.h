#pragma once

/*************************
	DBPlayerTaskExecutor
**************************/

// Player 엔티티 DB 요청을 받아 처리하는 ProdConsQueue
class DBPlayerTaskExecutor : public ProducerConsumerQueue
{
public:
	/* thread-unsafe (prodConsQ로 보호) */

	void GetSelectablePlayers(GameSessionRef gameSession, int32 dbId);
};

/*************************
	DBAchvTaskExecutor
**************************/

// Achievement 엔티티 DB 요청을 받아 처리하는 ProdConsQueue
class DBAchvTaskExecutor : public ProducerConsumerQueue
{
public:
	/* thread-unsafe (prodConsQ로 보호) */
};