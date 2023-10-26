#pragma once

namespace DBServerProtocol
{
	// with success option?
	// with UINT64 sessionId;
	enum PACKET_ID : UINT
	{
		TEST = 0
		, BATCHED_DB_JOB
		, INPUT_TEST
		, SELECT_TEST
		, SELECT_TEST_2
		, SELECT_TEST_3
		, STRING_TEST_PROC
		, UPDATE_TEST
	};
}