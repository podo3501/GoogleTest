#include "CookBook5.h"
#include <string>
#include <memory>

namespace CookBook5
{
	using ::testing::IsNull;
	using ::testing::Return;	
	using ::testing::ByMove;	//�ڷ����� �̵������϶� ���
	using ::testing::_;	//�ƹ� ���̳� ���͵� �������
	using ::testing::NotNull;
	using ::testing::Unused;	//�Լ� ���ڿ� ���� �������
	//VerifyAndClearExpectations heap�� �ִ� ��� delete�� �ȵɽ� �׽�Ʈ�� �ȵȴ�. �׷��� ������ ������ ����.
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
	// �̵� ����Ÿ�� �� ���(ex) unique_ptr. ByMove�� �� �ٰ�
	TEST(MiscRecipes, UseMoveOnlyTypes)
	{
		MockBuzzer mock_buzzer_;

		EXPECT_CALL(mock_buzzer_, MakeBuzz("hello"));
		EXPECT_THAT(mock_buzzer_.MakeBuzz("hello"), IsNull());

		//MoveOnly �� ���� ByMove�� �� �־�� �Ѵ�.
		EXPECT_CALL(mock_buzzer_, MakeBuzz("hello"))
			.WillOnce(Return(ByMove(std::make_unique<Buzz>(AccessLevel::kInternal))));	
		EXPECT_NE(nullptr, mock_buzzer_.MakeBuzz("hello"));
		//EXPECT_NE(nullptr, mock_buzzer_.MakeBuzz("hello"));	//�̷��� �� �� ����.	
		//�ֳ��ϸ� Action�� make_unique�� ���� ����Ǿ �ѹ��ۿ� ������ �ȵǱ� ������
		//���� �ϳ� �ۿ� ������ ���� �ʴ�.

		EXPECT_CALL(mock_buzzer_, MakeBuzz("x"))
			.WillRepeatedly([](const std::string& text) {	//���ٸ� ����� ���� �����ϴ�.
			return std::make_unique<Buzz>(AccessLevel::kInternal);
				});
		//���ٸ� ����� ��쿡�� ���� MakeBuzz�� call�ɶ� Action�� ����Ǿ 2�� �ҷ����� ������ �����ϴ�.
		EXPECT_NE(nullptr, mock_buzzer_.MakeBuzz("x"));
		EXPECT_NE(nullptr, mock_buzzer_.MakeBuzz("x"));

		//�̵����������� �Լ��� ������ ��� �������� �ȵ�(?)�� �ִٰ� �Ѵ�.
		EXPECT_CALL(mock_buzzer_, ShareBuzz(NotNull(), _)).WillOnce(Return(true));
		EXPECT_TRUE(mock_buzzer_.ShareBuzz(std::make_unique<Buzz>(AccessLevel::kInternal), 0));

		EXPECT_CALL(mock_buzzer_, ShareBuzz(_, _)).WillOnce([](std::unique_ptr<Buzz> buzz, Unused) {
			return buzz != nullptr; 
			});
		EXPECT_FALSE(mock_buzzer_.ShareBuzz(nullptr, 0));
	}

	// ���Ǵ� ����� ���� �� ������, �Լ��� ������ ���� �޼��带 �����ϴ� �� ���Ǵ� ��� ������ 
	// ������/�Ҹ��ڸ� ���� ���� �۾��� �����մϴ�.
	MockFoo::MockFoo() = default;
	MockFoo::~MockFoo() { Die(); }

	MockBar::MockBar() = default;
	MockBar::~MockBar() = default;
	//////////////////////////////////////////////////////////////////////
	// �������� �� ������. �����ڿ� �Ҹ��ڸ� �����ϴ� �ð��� ��κ��� �Ҿ�
	// ������ h�� �� �ְ� ���Ǵ� cpp�� �Ѵ�.
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
	// ������ �����ϱ�. �׽�Ʈ�� �� ������Ʈ�� �ı��ɶ� Ȯ���ϴµ� �ı����� �ʴ´ٸ� ��� �Ǿ��ٰ� ���´�.
	// �� ������ ���� ������ ��ü�� delete�� ���� ���� ��쿡 �߻��ϴµ� ������ ������ Ȯ�ο��θ� üũ �Ҽ��ִ�.
	TEST(MiscRecipes, ForcingVerification)
	{
		MockFoo* const foo = new MockFoo;
		EXPECT_CALL(*foo, DoThis()).Times(2);
		foo->DoThis();
		foo->DoThis();	//�ѹ��� ȣ���ϰ� VerifyAndClearExpectations�� ���ٸ� �׽�Ʈ�� ����ȴ�.

		//foo�� �������� server��...
		MyServer server(foo);

		//������ �Ҹ��ڰ� foo�� �����ϴ� ���� �ؾ���� ��츦 ����Ͽ� ��밪�� Ȯ��
		Mock::VerifyAndClearExpectations(foo); //�� ���� �����ϸ� �ѹ��� ȣ�� �Ǿ������� �׽�Ʈ�� �뱫�ȴ�.
		delete foo;	//�޸� �Ȼ��� �ϱ� ���ؼ�.
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
	// üũ ����Ʈ. � �Լ��� �ҷ������� üũ����Ʈ�� �����ؼ� Ȯ���ϰ� ������ ���
	TEST(MiscRecipes, UsingCheckpoints)
	{
		MyMock mock;

		MockFunction<void(std::string check_point_name)> check;
		{
			InSequence s;	//���������� �ҷ������� Ȯ��

			//���� a, üũ����Ʈ, üũ����Ʈ, a�� �ҷ������Ѵ�.
			EXPECT_CALL(mock, Bar("a"));
			EXPECT_CALL(check, Call("1"));
			EXPECT_CALL(check, Call("2"));
			EXPECT_CALL(mock, Bar("a"));
		}

		//InvokeMockFunc(mock, 2); �̰��� mock, Bar("a")�� call ���� �ʴ´ٴ� ���� Ȯ��
		InvokeMockFunc(mock, 1);
		check.Call("1");
		InvokeMockFunc(mock, 2);
		check.Call("2");
		InvokeMockFunc(mock, 3);
	}
	
	//////////////////////////////////////////////////////////////////////
	// �Ҹ��� �Լ��� Mocking�ϱ�. �Ҹ��� �Լ��� Ư�� �Լ��̱� ������ �Ҹ��� �Լ����� Die�Լ��� �θ���.
	TEST(MiscRecipes, MockingDestructors)
	{
		//::testing::FLAGS_gmock_verbose = "info";	//���� ������ �����ؼ� ������ ã�Ƴ��� ���. warning�� �⺻��

		std::unique_ptr<MockFoo> foo = std::make_unique<MockFoo>();
		std::unique_ptr<MockBar> bar = std::make_unique<MockBar>();

		{
			InSequence s;

			// Expects *foo to die after bar->A() and before bar->B().
			EXPECT_CALL(*bar, A());
			EXPECT_CALL(*foo, Die());	//�Ҹ��ڰ� �Ҹ��� Die�Լ��� call�ȴ�.
			EXPECT_CALL(*bar, B());
		}

		bar->A();
		foo.reset();	//unique_ptr�� ���������� �����.
		bar->B();

		//::testing::FLAGS_gmock_verbose = "warning";	//���� ������ �����ؼ� ������ ã�Ƴ��� ���. warning�� �⺻��
	}
}