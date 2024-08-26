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

	//�Ϻη� Uninteresting Call �߻���Ų��. visual studio�� gui��� ���װ� �ִ�. ��¿� '�׽�Ʈ' ���� ����.
	//EXPECT_CALL�� �޼ҵ带 �������� �ʰ� ȣ���� ��쿡�� uninteresting call ��� ���
	//���߿� �޼ҵ尡 �߰��Ǿ� �װ��� �׽�Ʈ�� ���з� ����� �ֱ� ������ ���߿� ������� �Լ��� ���� '���'�� �ʿ��ϴ�
	TEST(WarningUninterestingCall, Test)
	{
		MockFoo mock_foo;
		EXPECT_CALL(mock_foo, DoThis());

		mock_foo.DoThis();
		mock_foo.Add(1); //uninteresting call �߻�
	}

	//�׽�Ʈ�� EXPECT_CALL�� ����ϰ� NiceMock�� StrictMock�� ������ �������� ����� ��.
	//NickMock�� ����ϸ� Uninteresting Call�� �߻����� �ʴ´�.
	TEST(NiceAndStrict, Nice)
	{
		testing::NiceMock<MockFoo> niceMock_foo;
		EXPECT_CALL(niceMock_foo, DoThis());

		niceMock_foo.DoThis();
		niceMock_foo.Add(1);
	}

	//StrictMock�� ����ϸ� Uninteresting Call�� ���з� �����Ѵ�.
	TEST(NiceAndStrict, Strict)
	{
		testing::StrictMock<MockFoo> strictMock_foo;
		EXPECT_CALL(strictMock_foo, DoThis());

		strictMock_foo.DoThis();
		//strictMock_foo.Add(1);	//�ּ��� Ǯ�� Test�� ������.
	}

	//////////////////////////////////////////////////////////////////////
	//������ �ڵ带 ���� �ʰ� �������̽��� �ܼ�ȭ
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
	//Fake Ŭ�������� ȣ�� ����
	//Mock�� Fake�� ���� �� ���ۿ� ���� ��Ȳ�� ��� ������ ��Ÿ���� ��찡 �����ϴ�.
	// �Ƹ��� ���� ��ȣ �ۿ��� ������� �׽�Ʈ�ϴ� ����� �ͼ����� ���� ���Դϴ�.
	// �Ǵ� �������̽��� �ʹ� ���� ������ �ְ� �װ��� �����ؾ� �� ���� �ֽ��ϴ�.
	// �׷��� �������� ����� �������� ���ʽÿ�.�ڵ带 �����丵 �� �� �߰� �ܰ�θ� ����ϴ� ���� �����ϴ�.

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
		aoo.DelegateToFake();	//�����ڸ� Ȱ��ȭ

		//�ʿ��� ��� ON_CALL (foo, ...)�� ���⿡.

		//Action�� �������� �������� �⺻ Action�� ������ �ǹ��մϴ�. 
		//���ϴ� ��� ON_CALL() �Ǵ� EXPECT_CALL() ������ .WillOnce() �Ǵ� .WillRepeatedly() �� ����Ͽ� �⺻ Action�� �������� �� �ֽ��ϴ�.
		EXPECT_CALL(aoo, DoThis(5));
		EXPECT_CALL(aoo, DoThat(_, _));

		EXPECT_EQ('+', aoo.DoThis(5));
		int n = 0;
		aoo.DoThat("Hi", &n);
		EXPECT_EQ(2, n);
	}

	//////////////////////////////////////////////////////////////////////
	//Real Ŭ�������� ȣ�� ����

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
	//�θ�(Parent) Ŭ�������� ȣ�� ����
	// �̻������δ� �������̽��� ���� �ڵ带 �ۼ��ؾ��ϸ� ��� �޼ҵ�� ���� ���� �Լ� �����մϴ�.
	// �׷��� �����δ� ���� ���� �Լ��� �ƴ� ���� �޼ҵ�(��, �̹� ������ �����ϴ� �޼ҵ�)�� ����ȭ�ؾ��մϴ�

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

		//Concrete()�� Mock�ϰ� ���� ������(Times�� WillOnce�� �Ҽ� ���� ���� ȣ�⸸ �ϰ� ������)
		MockCoo  mock2;
		ON_CALL(mock2, Concrete(_)).WillByDefault(Invoke(&mock2, &MockCoo::CooConcrete));
		EXPECT_EQ(mock2.Concrete(nullptr), 1);
	}
}