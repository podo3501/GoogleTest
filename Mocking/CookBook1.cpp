#include "CookBook1.h"
#include <string>

using ::testing::_;
using ::testing::Invoke;
using ::testing::AtLeast;

namespace CookBook1
{
	class Bar;
	using Element = int;

	class Foo
	{
	public:
		virtual ~Foo() {};
		virtual int Add(Element x) = 0;
		virtual int Add(int times, Element x) = 0;

		virtual Bar& GetBar() = 0;
		virtual const Bar& GetBar() const = 0;

		virtual void DoThis() = 0;
	};

	class MockFoo : public Foo
	{
	public:
		MOCK_METHOD(int, Add, (Element x), (override));
		MOCK_METHOD(int, Add, (int times, Element x), (override));

		MOCK_METHOD(Bar&, GetBar, (), (override));
		MOCK_METHOD(const Bar&, GetBar, (), (const, override));

		MOCK_METHOD(void, DoThis, (), (override));
	};

	//일부러 Uninteresting Call 발생시킨다. visual studio의 gui경고에 버그가 있다. 출력에 '테스트' 탭을 보자.
	//EXPECT_CALL에 메소드를 지정하지 않고 호출할 경우에는 uninteresting call 경고를 출력
	//나중에 메소드가 추가되어 그것이 테스트를 실패로 만들수 있기 때문에 나중에 만들어진 함수에 대한 '경고'가 필요하다
	TEST(WarningUninterestingCall, Test)
	{
		MockFoo mock_foo;
		EXPECT_CALL(mock_foo, DoThis());

		mock_foo.DoThis();
		mock_foo.Add(1); //uninteresting call 발생
	}

	//테스트는 EXPECT_CALL로 사용하고 NiceMock과 StrictMock은 마지막 수단으로 사용할 것.
	//NickMock을 사용하면 Uninteresting Call이 발생하지 않는다.
	TEST(NiceAndStrict, Nice)
	{
		testing::NiceMock<MockFoo> niceMock_foo;
		EXPECT_CALL(niceMock_foo, DoThis());

		niceMock_foo.DoThis();
		niceMock_foo.Add(1);
	}

	//StrictMock을 사용하면 Uninteresting Call은 실패로 간주한다.
	TEST(NiceAndStrict, Strict)
	{
		testing::StrictMock<MockFoo> strictMock_foo;
		EXPECT_CALL(strictMock_foo, DoThis());

		strictMock_foo.DoThis();
		//strictMock_foo.Add(1);	//주석을 풀면 Test가 실패함.
	}

	//////////////////////////////////////////////////////////////////////
	//기존의 코드를 깨지 않고 인터페이스를 단순화
	enum LogSeverity { high, medium, low };

	class LogSink
	{
	public:
		virtual void Send(LogSeverity severity, const char* full_filename, const char* base_filename, int line,
			const struct tm* tm_time, const char* message, std::size_t message_len) = 0;
	};

	class ScopedMockLog : public LogSink
	{
	public:
		virtual void Send(LogSeverity severity, const char* full_filename, const char* base_filename, int line,
			const struct tm* tm_time, const char* message, std::size_t message_len)
		{
			Log(severity, full_filename, std::string(message, message_len));
		}

		MOCK_METHOD(void, Log, (LogSeverity severity, const std::string& file_path, const std::string& message));
	};

	//////////////////////////////////////////////////////////////////////
	//Fake 클래스에게 호출 위임
	//Mock와 Fake를 같이 쓸 수밖에 없는 상황은 어떠한 결함의 나타나는 경우가 많습니다.
	// 아마도 아직 상호 작용을 기반으로 테스트하는 방법에 익숙하지 않을 것입니다.
	// 또는 인터페이스에 너무 많은 역할이 있고 그것을 분할해야 할 수도 있습니다.
	// 그렇기 때문에이 방법을 남용하지 마십시오.코드를 리팩토링 할 때 중간 단계로만 사용하는 것이 좋습니다.

	class Aoo
	{
	public:
		virtual ~Aoo() {}
		virtual char DoThis(int n) = 0;
		virtual void DoThat(const char* s, int* p) = 0;
	};

	class FakeAoo : public Aoo
	{
	public:
		virtual char DoThis(int n) { return (n > 0) ? '+' : (n < 0) ? '-' : '0'; }
		virtual void DoThat(const char* s, int* p) { *p = static_cast<int>(strlen(s)); }
	};

	class MockAoo : public Aoo
	{
	public:
		MOCK_METHOD(char, DoThis, (int n), (override));
		MOCK_METHOD(void, DoThat, (const char* s, int* p), (override));

		void DelegateToFake()
		{
			ON_CALL(*this, DoThis(_)).WillByDefault(Invoke(&fake_, &FakeAoo::DoThis));
			ON_CALL(*this, DoThat(_, _)).WillByDefault(Invoke(&fake_, &FakeAoo::DoThat));
		}

	private:
		FakeAoo fake_;
	};

	TEST(Delegate, Fake)
	{
		MockAoo aoo;
		aoo.DelegateToFake();	//위임자를 활성화

		//필요한 경우 ON_CALL (foo, ...)을 여기에.

		//Action이 지정되지 않으면이 기본 Action이 사용됨을 의미합니다. 
		//원하는 경우 ON_CALL() 또는 EXPECT_CALL() 내에서 .WillOnce() 또는 .WillRepeatedly() 를 사용하여 기본 Action을 재정의할 수 있습니다.
		EXPECT_CALL(aoo, DoThis(5));
		EXPECT_CALL(aoo, DoThat(_, _));

		EXPECT_EQ('+', aoo.DoThis(5));
		int n = 0;
		aoo.DoThat("Hi", &n);
		EXPECT_EQ(2, n);
	}

	//////////////////////////////////////////////////////////////////////
	//Real 클래스에게 호출 위임

	class Boo
	{
	public:
		virtual ~Boo() {}
		char DoThis(int n) { return (n > 0) ? '+' : (n < 0) ? '-' : '0'; }
		void DoThat(const char* s, int* p) { *p = static_cast<int>(strlen(s)); }
	};

	class MockBoo : public Boo
	{
	public:
		MockBoo()
		{
			ON_CALL(*this, DoThis(_)).WillByDefault(Invoke(&real_, &Boo::DoThis));
			ON_CALL(*this, DoThat(_, _)).WillByDefault(Invoke(&real_, &Boo::DoThat));
		}

		MOCK_METHOD(char, DoThis, (int n));
		MOCK_METHOD(void, DoThat, (const char* s, int* p));

	private:
		Boo real_;
	};

	TEST(Delegate, Real)
	{
		MockBoo  mock;

		EXPECT_CALL(mock, DoThis(2)).Times(2);
		EXPECT_CALL(mock, DoThat(_, _)).Times(AtLeast(1));

		EXPECT_EQ('+', mock.DoThis(2));
		EXPECT_EQ('+', mock.DoThis(2));
		int n = 0;
		mock.DoThat("Hi", &n);
		EXPECT_EQ(2, n);
	}

	//////////////////////////////////////////////////////////////////////
	//부모(Parent) 클래스에게 호출 위임
	// 이상적으로는 인터페이스에 대한 코드를 작성해야하며 모든 메소드는 순수 가상 함수 여야합니다.
	// 그러나 실제로는 순수 가상 함수가 아닌 가상 메소드(즉, 이미 구현이 존재하는 메소드)를 모형화해야합니다

	class Coo
	{
	public:
		virtual ~Coo() {}
		
		virtual void Pure(int n) = 0;
		virtual int Concrete(const char* str) { return 1; }
	};

	class MockCoo : public Coo
	{
	public:
		MOCK_METHOD(void, Pure, (int n), (override));
		MOCK_METHOD(int, Concrete, (const char* str));

		int CooConcrete(const char* str) 
		{ 
			return Coo::Concrete(str); 
		}
	};

	TEST(Delegate, Parent)
	{
		MockCoo  mock1;
		EXPECT_CALL(mock1, Concrete(_)).WillOnce(Invoke(&mock1, &MockCoo::CooConcrete));
		EXPECT_EQ(mock1.Concrete(nullptr), 1);

		//Concrete()를 Mock하고 싶지 않을때(Times나 WillOnce를 할수 없고 단지 호출만 하고 싶을때)
		MockCoo  mock2;
		ON_CALL(mock2, Concrete(_)).WillByDefault(Invoke(&mock2, &MockCoo::CooConcrete));
		EXPECT_EQ(mock2.Concrete(nullptr), 1);
	}
}