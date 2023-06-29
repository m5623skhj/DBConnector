#pragma once

namespace DBServerProtocol
{
	enum PACKET_ID
	{
		TEST = 0
		, INPUT_TEST
		, SELECT_TEST
		, SELECT_TEST_2
		, SELECT_TEST_3
		, STRING_TEST_PROC
		, UPDATE_TEST
	};

	struct test
	{
		int id3 = 0;
		WCHAR teststring[30];
	};

	struct input_test
	{
		int item;
		int item2;
	};

	struct update_test
	{
		long long _id = 0;
	};

	struct string_test_proc
	{
		WCHAR test[30];
	};

	struct select_test
	{
		long long id = 0;
	};

	struct select_test_2
	{
		long long id = 0;
	};

	struct select_test_3
	{
	};
}