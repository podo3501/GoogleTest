#include "gtest/gtest.h"
#include "Testsuite.h"
#include <windows.h>

GlobalEnv::GlobalEnv() = default;
GlobalEnv::~GlobalEnv() = default;

void GlobalEnv::SetUp()
{
	//메모리 릭을 잡기 위한 옵션
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
}

void GlobalEnv::TearDown()
{}

//GlobalEnv 클래스의 SetUp을 해주기 위해서.
int main(int argc, char** argv)
{
	//쓰레드가 돌아갈때 강제로 종료할 경우 프로그램이 살아있는 경우가 있어서 
	//다음 테스트에 오동작 하는 경우가 있다. 그럴때를 대비해서 프로그램이 띄워져 있는지 체크
	HANDLE Mutex;
	std::wstring progMutex{ L"Network Test" }; // 프로젝트명
	if ((Mutex = OpenMutex(MUTEX_ALL_ACCESS, false, progMutex.c_str())) == NULL)
		Mutex = CreateMutex(NULL, true, progMutex.c_str());
	else
	{
		OutputDebugString(L"\nTest 실패! 프로그램이 중복 실행 되었습니다. 자동 종료 합니다.\n\n");
		return 0;
	}

	::testing::InitGoogleTest(&argc, argv);
	::testing::AddGlobalTestEnvironment(new GlobalEnv);

	auto silent_run = false;
	if (silent_run) {
		auto& listeners = ::testing::UnitTest::GetInstance()->listeners();
		delete listeners.Release(listeners.default_result_printer());
	}

	return RUN_ALL_TESTS();
}