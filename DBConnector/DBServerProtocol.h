#pragma once
#include "DBServer.h"

namespace DBServerProtocol
{
	// with success option?
	// with UINT64 sessionId;
	enum PACKET_ID : UINT
	{
		TEST = 0
		, BATCHED_DB_JOB
		, SELECT_TEST_2
	};
}
