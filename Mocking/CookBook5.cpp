#include "CookBook5.h"
#include <string>
#include <memory>

namespace CookBook5
{
	using ::testing::IsNull;
	using ::testing::Return;	
	using ::testing::ByMove;	//자료형이 이동형식일때 사용
	using ::testing::_;	//아무 값이나 들어와도 상관없음
	using ::testing::NotNull;
	using ::testing::Unused;	//함수 인자에 형식 상관없음
	//VerifyAndClearExpectations heap에 있는 경우 delete가 안될시 테스트가 안된다. 그래서 강제로 검증해 본다.
	using ::testing::Mock;	

	enum class AccessLevel { kInternal, kPublic };

	class Buzz
	{
	public:
		explicit Buzz(AccessLevel access) {};
	};

	class Buzzer
	{
	public: 
		virtual ~Buzzer() = default;
		virtual std::unique_ptr<Buzz> MakeBuzz(const std::string& text) = 0;
		virtual bool ShareBuzz(std::unique_ptr<Buzz> buzz, int64_t timestamp) = 0;
	};

	class MockBuzzer : public Buzzer
	{
	public:
		MOCK_METHOD(std::unique_ptr<Buzz>, MakeBuzz, (const std::string& text), (override));
		MOCK_METHOD(bool, ShareBuzz, (std::unique_ptr<Buzz> buzz, int64_t timestamp), (override));
	};

	//////////////////////////////////////////////////////////////////////
	// 이동 전용타입 일 경우(ex) unique_ptr. ByMove를 써 줄것
	TEST(MiscRecipes, UseMoveOnlyTypes)
	{
		MockBuzzer mock_buzzer_;

		EXPECT_CALL(mock_buzzer_, MakeBuzz("hello"));
		EXPECT_THAT(mock_buzzer_.MakeBuzz("hello"), IsNull());

		//MoveOnly 일 경우는 ByMove를 써 주어야 한다.
		EXPECT_CALL(mock_buzzer_, MakeBuzz("hello"))
			.WillOnce(Return(ByMove(std::make_unique<Buzz>(AccessLevel::kInternal))));	
		EXPECT_NE(nullptr, mock_buzzer_.MakeBuzz("hello"));
		//EXPECT_NE(nullptr, mock_buzzer_.MakeBuzz("hello"));	//이렇게 할 수 없다.	
		//왜냐하면 Action에 make_unique가 먼저 실행되어서 한번밖에 실행이 안되기 때문에
		//값을 하나 밖에 가지고 있지 않다.

		EXPECT_CALL(mock_buzzer_, MakeBuzz("x"))
			.WillRepeatedly([](const std::string& text) {	//람다를 사용할 경우는 가능하다.
			return std::make_unique<Buzz>(AccessLevel::kInternal);
				});
		//람다를 사용할 경우에는 실제 MakeBuzz가 call될때 Action이 실행되어서 2번 불려지기 때문에 가능하다.
		EXPECT_NE(nullptr, mock_buzzer_.MakeBuzz("x"));
		EXPECT_NE(nullptr, mock_buzzer_.MakeBuzz("x"));

		//이동전용형식이 함수의 인자일 경우 컴파일이 안될(?)수 있다고 한다.
		EXPECT_CALL(mock_buzzer_, ShareBuzz(NotNull(), _)).WillOnce(Return(true));
		EXPECT_TRUE(mock_buzzer_.ShareBuzz(std::make_unique<Buzz>(AccessLevel::kInternal), 0));

		EXPECT_CALL(mock_buzzer_, ShareBuzz(_, _)).WillOnce([](std::unique_ptr<Buzz> buzz, Unused) {
			return buzz != nullptr; 
			});
		EXPECT_FALSE(mock_buzzer_.ShareBuzz(nullptr, 0));
	}

	// 정의는 사소해 보일 수 있지만, 함수는 실제로 모의 메서드를 구현하는 데 사용되는 멤버 변수의 
	// 생성자/소멸자를 통해 많은 작업을 수행합니다.
	MockFoo::MockFoo() = default;
	MockFoo::~MockFoo() { Die(); }

	MockBar::MockBar() = default;
	MockBar::~MockBar() = default;
	//////////////////////////////////////////////////////////////////////
	// 컴파일을 더 빠르게. 생성자와 소멸자를 생성하는 시간에 대부분을 할애
	// 선언은 h에 해 주고 정의는 cpp에 한다.
	TEST(MiscRecipes, CompilationFaster)
	{
		MockFoo foo;
	}

	class MyServer
	{
	public:
		MyServer(Foo* foo) : m_foo{ foo } {}

	private:
		Foo* m_foo;
	};

	//////////////////////////////////////////////////////////////////////
	// 강제로 검증하기. 테스트는 그 오브젝트가 파괴될때 확인하는데 파괴되지 않는다면 통과 되었다고 나온다.
	// 그 이유는 힙에 생성된 객체가 delete를 하지 않을 경우에 발생하는데 그전에 강제로 확인여부를 체크 할수있다.
	TEST(MiscRecipes, ForcingVerification)
	{
		MockFoo* const foo = new MockFoo;
		EXPECT_CALL(*foo, DoThis()).Times(2);
		foo->DoThis();
		foo->DoThis();	//한번만 호출하고 VerifyAndClearExpectations가 없다면 테스트가 통과된다.

		//foo의 소유권은 server로...
		MyServer server(foo);

		//서버의 소멸자가 foo를 삭제하는 것을 잊어버릴 경우를 대비하여 기대값을 확인
		Mock::VerifyAndClearExpectations(foo); //이 줄을 제거하면 한번만 호출 되었음에도 테스트가 통괴된다.
		delete foo;	//메모리 안새게 하기 위해서.
	}

	using ::testing::MockFunction;
	using ::testing::InSequence;

	class MyMock
	{
	public:
		MOCK_METHOD(void, Bar, (const std::string& text), (const));
	};

	void InvokeMockFunc(const MyMock& testMock, int n)
	{
		if (n == 2) return;
		testMock.Bar("a");
	}

	//////////////////////////////////////////////////////////////////////
	// 체크 포인트. 어떤 함수가 불려지는지 체크포인트를 설정해서 확인하고 싶을때 사용
	TEST(MiscRecipes, UsingCheckpoints)
	{
		MyMock mock;

		MockFunction<void(std::string check_point_name)> check;
		{
			InSequence s;	//순차적으로 불려지는지 확인

			//콜이 a, 체크포인트, 체크포인트, a로 불려져야한다.
			EXPECT_CALL(mock, Bar("a"));
			EXPECT_CALL(check, Call("1"));
			EXPECT_CALL(check, Call("2"));
			EXPECT_CALL(mock, Bar("a"));
		}

		//InvokeMockFunc(mock, 2); 이것은 mock, Bar("a")를 call 하지 않는다는 것을 확인
		InvokeMockFunc(mock, 1);
		check.Call("1");
		InvokeMockFunc(mock, 2);
		check.Call("2");
		InvokeMockFunc(mock, 3);
	}
	
	//////////////////////////////////////////////////////////////////////
	// 소멸자 함수를 Mocking하기. 소멸자 함수는 특수 함수이기 때문에 소멸자 함수에서 Die함수를 부른다.
	TEST(MiscRecipes, MockingDestructors)
	{
		//::testing::FLAGS_gmock_verbose = "info";	//실행 스택을 추적해서 문제를 찾아낼때 사용. warning이 기본값

		std::unique_ptr<MockFoo> foo = std::make_unique<MockFoo>();
		std::unique_ptr<MockBar> bar = std::make_unique<MockBar>();

		{
			InSequence s;

			// Expects *foo to die after bar->A() and before bar->B().
			EXPECT_CALL(*bar, A());
			EXPECT_CALL(*foo, Die());	//소멸자가 불릴때 Die함수도 call된다.
			EXPECT_CALL(*bar, B());
		}

		bar->A();
		foo.reset();	//unique_ptr을 강제적으로 지운다.
		bar->B();

		//::testing::FLAGS_gmock_verbose = "warning";	//실행 스택을 추적해서 문제를 찾아낼때 사용. warning이 기본값
	}
}