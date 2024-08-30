#include "CookBook4.h"
#include <functional>

namespace CookBook4
{
	using ::testing::_;
	using ::testing::InSequence;	//������(Sequence�� �κм���)
	using ::testing::Return;
	using ::testing::Invoke;
	using ::testing::SaveArg;
	using ::testing::ReturnPointee;
	using ::testing::DefaultValue;	//���ϰ��� ���ؼ� �⺻���� �����Ѵ�.
	using ::testing::AnyNumber;
	using ::testing::Gt;
	using ::testing::InvokeArgument;	//ex) InvokeArgument;<0>(1) ù��° �μ��� 1�� ����.
	using ::testing::DoAll;
	using ::testing::IgnoreResult;
	using ::testing::WithArgs;	//�μ��� �ʿ��� �͸� �����ϱ�.
	using ::testing::Unused;	//�μ��� � Ÿ������ �������.
	using ::testing::Action;
	using ::testing::Return;
	using ::testing::SetArgPointee;

	class Move
	{
	public:
		virtual ~Move() {}
		virtual bool IsDirty() = 0;
		virtual void Flush() = 0;
		virtual int GetPrevValue() = 0;
		virtual void UpdateValue(int n) = 0;
	};

	class MockMove : public Move
	{
	public:
		MOCK_METHOD(bool, IsDirty, (), (override));
		MOCK_METHOD(void, Flush, (), (override));

		void FlushIfDirty()
		{
			if (IsDirty())
				Flush();
		}

		MOCK_METHOD(int, GetPrevValue, (), (override));
		MOCK_METHOD(void, UpdateValue, (int n), (override));
	};

	//////////////////////////////////////////////////////////////////////
	//Mock ��ü�� ���¿� ���� �ൿ�� ����
	//InSequence�� ���� Flush�� ȣ����� ������ IsDirty�� ��� true�� ����
	TEST(Action, ChangingMockObjectBehavior)
	{
		MockMove my_mock;

		{	//�߰�ȣ�� ������ ������ InSequence�� ������ ���� EXPECT_CALL�� ������ ��ġ�� �ʰ� �ϱ� ���ؼ��̴�.
			InSequence seq;
			EXPECT_CALL(my_mock, IsDirty())
				.WillRepeatedly(Return(true));
			EXPECT_CALL(my_mock, Flush());
			EXPECT_CALL(my_mock, IsDirty())
				.WillRepeatedly(Return(false));
		}

		EXPECT_TRUE(my_mock.IsDirty());
		my_mock.FlushIfDirty();
		EXPECT_FALSE(my_mock.IsDirty());

		int previous_value = 0;
		EXPECT_CALL(my_mock, GetPrevValue)
			.WillRepeatedly(ReturnPointee(&previous_value));	//���۷����� �ƴ� ���� ���簡 �Ǿ ���� ������ �ʴ´�.
		EXPECT_CALL(my_mock, UpdateValue)
			.WillRepeatedly(SaveArg<0>(&previous_value));

		my_mock.UpdateValue(7);
		EXPECT_EQ(my_mock.GetPrevValue(), 7);
	}

	//////////////////////////////////////////////////////////////////////
	// Ư�� ��ȯ ������ ���� �⺻�� ����
	TEST(Action, DefaultValue)
	{
		Bar default_bar(1);
		// ����Ʈ�� Bar �� ���ϰ��� �����մϴ�. 
		DefaultValue<Bar>::Set(default_bar);

		MockFoo foo;
		// �⺻ ��ȯ ���� �����Ƿ� ���⼭ Action�� ������ �ʿ䰡 �����ϴ�. 
		EXPECT_CALL(foo, CalculateBar());

		EXPECT_EQ(foo.CalculateBar(), default_bar);  // �̰��� default_bar�� ��ȯ�ؾ��մϴ�. 
		// �⺻ ��ȯ ���� �����մϴ�. 
		DefaultValue<Bar>::Clear();
	}

	//////////////////////////////////////////////////////////////////////
	// Mock �޼ҵ��� �⺻ Action ���� 
	TEST(Action, DefaultAction)
	{
		MockFoo foo;

		//������ ������ ��ü���� ��Ȳ�� �ۼ�
		ON_CALL(foo, Sign(_))
			.WillByDefault(Return(-1));
		ON_CALL(foo, Sign(0))
			.WillByDefault(Return(0));
		ON_CALL(foo, Sign(Gt(0)))
			.WillByDefault(Return(1));

		EXPECT_CALL(foo, Sign(_))
			.Times(AnyNumber());

		EXPECT_EQ(foo.Sign(5), 1);
		EXPECT_EQ(foo.Sign(-9), -1);
		EXPECT_EQ(foo.Sign(0), 0);
	}

	int CalculateSum(int x, int y) { return x + y; }
	int Sum3(int x, int y, int z) { return x + y + z; }

	class Helper2
	{
	public:
		bool ComplexJob(int x)
		{
			if (x == 10) return true;
			return false;
		}
	};
	
	//////////////////////////////////////////////////////////////////////
	// Functions / Methods / Functors / Lambdas�� Action���� ����ϱ�
	TEST(Action, UsingFunctionsMethodsFunctorsLambdas)
	{
		MockFoo foo;
		Helper2 helper;
		EXPECT_CALL(foo, Sum(_, _))
			.WillOnce(&CalculateSum);
		EXPECT_CALL(foo, ComplexJob(_))
			.WillOnce(Invoke(&helper, &Helper2::ComplexJob))
			.WillOnce([] { return true; })
			.WillRepeatedly([](int x) { return x > 0; });

		EXPECT_EQ(foo.Sum(5, 6), 11);         // Invokes CalculateSum(5, 6).
		EXPECT_EQ(foo.ComplexJob(10), true);    // Invokes helper.ComplexJob(10).
		EXPECT_EQ(foo.ComplexJob(-1), true);    // Invokes [] { return true; }
		EXPECT_EQ(foo.ComplexJob(-5), false);    // Invokes [](int x) { return x > 0; }
	}

	//////////////////////////////////////////////////////////////////////
	//�μ��� �Լ� �����͸� ���� ���(callable, Listener, callback ��� ���� �Ҹ�)
	TEST(Action, ArgumentOfTheFunction)
	{
		MockFoo foo;
		EXPECT_CALL(foo, DoThis(_, _))
			.WillOnce(InvokeArgument<1>(5));  // (*fp)(5 )�� �����մϴ�. ���⼭ fp�� DoThis ()�� �� ��° �μ���.
		
		std::function<bool(int)> test = [](int x)->bool { 
			return x >= 0; };
		foo.DoThis(3, test);

		Helper helper;
		EXPECT_CALL(foo, ArgFuncRef(_))
			.WillOnce(InvokeArgument<0>(5, std::ref(helper)));

		std::function<bool(int, const Helper&)> refTest = [](int x, const Helper&)->bool { return true; };
		foo.ArgFuncRef(refTest);

		//5.0�� string("Hi") ���� �Ͻ����̸� EXPECT_CALL () ���� ���� �� ���ÿ� ��ȿȭ�ȴٴ� ���� ����
		EXPECT_CALL(foo, DoThat(_))
			.WillOnce(InvokeArgument<0>(5.0, std::string("Hi")));
		
		std::function<bool(const double&, const std::string&)> refTest2 = [](const double&, const std::string&)->bool {
			return true; };
		foo.DoThat(refTest2);
	}

	int Process(const MyData& data) { return 1; }
	std::string DoSomething() { return "doSometing"; }

	//////////////////////////////////////////////////////////////////////
	//Action ��� �����ϱ�. ���ϰ��� �����ϰ� ������ ���
	TEST(Action, IgnoringReturn)
	{
		MockFoo foo;
		EXPECT_CALL(foo, Abc(_))
			// .WillOnce(Invoke(Process));	//��ȯ ���� void ���� ������ Process()�� int�� ��ȯ�ϹǷ� ������ �ȵ�
			.WillOnce(IgnoreResult(Invoke(Process)));
		MyData data;
		foo.Abc(data);

		EXPECT_CALL(foo, Xyz())
			.WillOnce(DoAll(IgnoreResult(Invoke(DoSomething)),	// Ignores the string DoSomething() returns.				
				Return(true)));
		EXPECT_TRUE(foo.Xyz());
	}

	bool IsVisibleInQuadrant1(bool visible, int x, int y) 
	{
		return visible && x >= 0 && y >= 0;
	}

	using ::testing::WithoutArgs;	//�μ��� ������ ������ ���
	using ::testing::WithArg;	//Action�� �ϳ��� �μ��� ���� �� �����. WithArg<N>
	using ::testing::InvokeWithoutArgs;	//WithoutArgs(Invoke(...))
	//////////////////////////////////////////////////////////////////////
	// Argument�� �ʿ��� �͸� �����ϱ�
	// Tips
	// WithArgs���� �η��� ���Ǵ� ���� Action�� �ݵ�� Invoke() �� �ʿ�� ����
	// �ʿ��� ��� WithArgs <2, 3, 3, 5>(...)�� ���� �μ� ����� �μ��� �ݺ������� ����� �� ����
	// 	���� ��� WithArgs <3, 2, 1>(...)�� ���� �μ��� ������ ������ �� ����
	// ������ �μ��� ������ Action�� ���� �����ϰ� ��ġ�� �ʿ�� ����
	// ���� ���, Mock �Լ��� �� ��° �μ��� int�̰� my_action�� double�̾ �������
	TEST(Action, SelectingArguments)
	{
		MockFoo foo;
		EXPECT_CALL(foo, SoManyArguments)
			.WillOnce(WithArgs<0, 2, 3>(Invoke(IsVisibleInQuadrant1)));  // No need to define your own adaptor.
		
		EXPECT_FALSE(foo.SoManyArguments(false, "", 1, 1, {}, 0, 9, 0)); //0, 2, 3�� �μ��� ����ؼ� false�� ����
	}

	double DistanceToOrigin(Unused, double x, double y) 
	{
		return sqrt(x * x + y * y);
	}
	//////////////////////////////////////////////////////////////////////
	// Argument �����ϱ�. �μ� �κп� Unused�� ������ � ������ �μ��� �͵� �������.
	// Unused�� ������ SelectingArguments �̰ɷ� ������ �ɷ����� �ϱ� ����.
	TEST(Action, IgnoringArguments)
	{
		MockFoo foo;

		EXPECT_CALL(foo, DistanceWithLabel("abc", _, _))
			.WillOnce(Invoke(DistanceToOrigin));
		EXPECT_CALL(foo, DistanceWithIndex(5, _, _))
			.WillOnce(Invoke(DistanceToOrigin));

		EXPECT_NEAR(foo.DistanceWithLabel("abc", 2, 3), 3.605, 0.005);
		EXPECT_NEAR(foo.DistanceWithIndex(5, 1, 2), 2.236, 0.005);
	}

	int IncrementCounter(int init)
	{
		static int index = 0;
		index++;
		return init + index;
	}

	//////////////////////////////////////////////////////////////////////
	// Action ����. 
	TEST(Action, Sharing)
	{
		MockFoo foo;

		//Action�� ����
		Action<bool(int*)> set_flag = DoAll(SetArgPointee<0>(5),
			Return(true));
		EXPECT_CALL(foo, ActionSharing_1).WillOnce(set_flag);
		EXPECT_CALL(foo, ActionSharing_2).WillOnce(set_flag);

		int a1{}, a2{};
		EXPECT_TRUE(foo.ActionSharing_1(&a1));
		EXPECT_TRUE(foo.ActionSharing_2(&a2));

		//������ �ϰ� �Ǹ� IncrementCounter�� �ѹ��ۿ� �Ҹ��� �ʾƼ� result1, 2, 3���� �����ϰ� ���´�.
		Action<int()> increment = Return(IncrementCounter(0));
		EXPECT_CALL(foo, ActionSharingCounter_1())
			.WillRepeatedly(increment);
		EXPECT_CALL(foo, ActionSharingCounter_2())
			.WillRepeatedly(increment);

		int result1 = foo.ActionSharingCounter_1();
		int result2 = foo.ActionSharingCounter_1();
		int result3 = foo.ActionSharingCounter_2();
	}
} 