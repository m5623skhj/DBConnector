#include "PreCompile.h"

#include "GoogleTest.h"

namespace GTestHelper
{
	bool StartTest()
	{
		testing::InitGoogleTest();
		return RUN_ALL_TESTS() == 0;
	}
}