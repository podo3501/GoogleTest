#include "CookBook4.h"
#include <functional>

namespace CookBook4
{
	using ::testing::_;
	using ::testing::InSequence;	//순차적(Sequence는 부분순차)
	using ::testing::Return;
	using ::testing::Invoke;
	using ::testing::SaveArg;
	using ::testing::ReturnPointee;
	using ::testing::DefaultValue;	//리턴값에 대해서 기본값을 설정한다.
	using ::testing::AnyNumber;
	using ::testing::Gt;
	using ::testing::InvokeArgument;	//ex) InvokeArgument;<0>(1) 첫번째 인수에 1을 넣음.
	using ::testing::DoAll;
	using ::testing::IgnoreResult;
	using ::testing::WithArgs;	//인수를 필요한 것만 선택하기.
	using ::testing::Unused;	//인수가 어떤 타입인지 상관안함.
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
	//Mock 객체의 상태에 따라 행동을 변경
	//InSequence에 따라서 Flush가 호출되지 않으면 IsDirty는 계속 true를 리턴
	TEST(Action, ChangingMockObjectBehavior)
	{
		MockMove my_mock;

		{	//중괄호로 묶어준 이유는 InSequence가 다음에 오는 EXPECT_CALL에 영향을 미치지 않게 하기 위해서이다.
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
			.WillRepeatedly(ReturnPointee(&previous_value));	//레퍼런스가 아닐 경우는 복사가 되어서 값이 변하지 않는다.
		EXPECT_CALL(my_mock, UpdateValue)
			.WillRepeatedly(SaveArg<0>(&previous_value));

		my_mock.UpdateValue(7);
		EXPECT_EQ(my_mock.GetPrevValue(), 7);
	}

	//////////////////////////////////////////////////////////////////////
	// 특정 반환 유형에 대한 기본값 설정
	TEST(Action, DefaultValue)
	{
		Bar default_bar(1);
		// 디폴트의 Bar 형 리턴값을 설정합니다. 
		DefaultValue<Bar>::Set(default_bar);

		MockFoo foo;
		// 기본 반환 값이 있으므로 여기서 Action을 지정할 필요가 없습니다. 
		EXPECT_CALL(foo, CalculateBar());

		EXPECT_EQ(foo.CalculateBar(), default_bar);  // 이것은 default_bar를 반환해야합니다. 
		// 기본 반환 값을 제거합니다. 
		DefaultValue<Bar>::Clear();
	}

	//////////////////////////////////////////////////////////////////////
	// Mock 메소드의 기본 Action 설정 
	TEST(Action, DefaultAction)
	{
		MockFoo foo;

		//밑으로 갈수록 구체적인 상황을 작성
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
	// Functions / Methods / Functors / Lambdas를 Action으로 사용하기
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
	//인수로 함수 포인터를 받을 경우(callable, Listener, callback 등등 으로 불림)
	TEST(Action, ArgumentOfTheFunction)
	{
		MockFoo foo;
		EXPECT_CALL(foo, DoThis(_, _))
			.WillOnce(InvokeArgument<1>(5));  // (*fp)(5 )를 실행합니다. 여기서 fp는 DoThis ()의 두 번째 인수임.
		
		std::function<bool(int)> test = [](int x)->bool { 
			return x >= 0; };
		foo.DoThis(3, test);

		Helper helper;
		EXPECT_CALL(foo, ArgFuncRef(_))
			.WillOnce(InvokeArgument<0>(5, std::ref(helper)));

		std::function<bool(int, const Helper&)> refTest = [](int x, const Helper&)->bool { return true; };
		foo.ArgFuncRef(refTest);

		//5.0과 string("Hi") 값은 일시적이며 EXPECT_CALL () 문이 끝날 때 동시에 무효화된다는 점에 유의
		EXPECT_CALL(foo, DoThat(_))
			.WillOnce(InvokeArgument<0>(5.0, std::string("Hi")));
		
		std::function<bool(const double&, const std::string&)> refTest2 = [](const double&, const std::string&)->bool {
			return true; };
		foo.DoThat(refTest2);
	}

	int Process(const MyData& data) { return 1; }
	std::string DoSomething() { return "doSometing"; }

	//////////////////////////////////////////////////////////////////////
	//Action 결과 무시하기. 리턴값을 무시하고 싶을때 사용
	TEST(Action, IgnoringReturn)
	{
		MockFoo foo;
		EXPECT_CALL(foo, Abc(_))
			// .WillOnce(Invoke(Process));	//반환 값이 void 여야 하지만 Process()는 int를 반환하므로 컴파일 안됨
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

	using ::testing::WithoutArgs;	//인수를 취하지 않을때 사용
	using ::testing::WithArg;	//Action이 하나의 인수를 취할 때 사용함. WithArg<N>
	using ::testing::InvokeWithoutArgs;	//WithoutArgs(Invoke(...))
	//////////////////////////////////////////////////////////////////////
	// Argument를 필요한 것만 선택하기
	// Tips
	// WithArgs같은 부류는 사용되는 내부 Action은 반드시 Invoke() 될 필요는 없음
	// 필요한 경우 WithArgs <2, 3, 3, 5>(...)와 같이 인수 목록의 인수를 반복적으로 사용할 수 있음
	// 	예를 들어 WithArgs <3, 2, 1>(...)과 같이 인수의 순서를 변경할 수 있음
	// 선택한 인수의 유형이 Action의 형과 엄격하게 일치할 필요는 없음
	// 예를 들어, Mock 함수의 네 번째 인수가 int이고 my_action이 double이어도 상관없음
	TEST(Action, SelectingArguments)
	{
		MockFoo foo;
		EXPECT_CALL(foo, SoManyArguments)
			.WillOnce(WithArgs<0, 2, 3>(Invoke(IsVisibleInQuadrant1)));  // No need to define your own adaptor.
		
		EXPECT_FALSE(foo.SoManyArguments(false, "", 1, 1, {}, 0, 9, 0)); //0, 2, 3번 인수로 계산해서 false를 리턴
	}

	double DistanceToOrigin(Unused, double x, double y) 
	{
		return sqrt(x * x + y * y);
	}
	//////////////////////////////////////////////////////////////////////
	// Argument 무시하기. 인수 부분에 Unused가 있으면 어떤 형태의 인수가 와도 상관없다.
	// Unused가 없으면 SelectingArguments 이걸로 번번히 걸러내야 하기 때문.
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
	// Action 공유. 
	TEST(Action, Sharing)
	{
		MockFoo foo;

		//Action을 공유
		Action<bool(int*)> set_flag = DoAll(SetArgPointee<0>(5),
			Return(true));
		EXPECT_CALL(foo, ActionSharing_1).WillOnce(set_flag);
		EXPECT_CALL(foo, ActionSharing_2).WillOnce(set_flag);

		int a1{}, a2{};
		EXPECT_TRUE(foo.ActionSharing_1(&a1));
		EXPECT_TRUE(foo.ActionSharing_2(&a2));

		//공유를 하게 되면 IncrementCounter는 한번밖에 불리지 않아서 result1, 2, 3값이 동일하게 나온다.
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