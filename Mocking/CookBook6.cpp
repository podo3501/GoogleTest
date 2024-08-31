#include "CookBook6.h"
#include <string>
#include <format>

namespace CookBook6
{
	using ::testing::Not;
	using ::testing::PrintToString;
	using ::testing::Matcher;
	using ::testing::Cardinality;
	using ::testing::CardinalityInterface;
	using ::testing::MakeCardinality;
	using ::testing::MockFunction;
	using ::testing::_;
	using ::testing::Action;
	using ::testing::ActionInterface;
	using ::testing::MakeAction;
	using ::testing::MakePolymorphicAction;
	using ::testing::PolymorphicAction;
	using ::testing::Return;
	
	//TEST함수 안에서는 MATCHER가 작성 안됨
	//arg의 타입은 신경 쓰지 않아도 됨.
	MATCHER(IsDivisibleBy7, 
		std::string(negation ? "isn't" : "is") + std::string(" divisible by 7")) 
	{
		auto remain = arg % 7;
		if (remain == 0)
			return true;

		//결과값에 붙여서 설명문을 출력한다.
		*result_listener << "the remainder is " << remain << '\n';
		return false;
	}

	MATCHER(IsDivisibleBy7_UsingExpect, "") {
		const auto remainder = arg % 7;
		EXPECT_EQ(remainder, 0);
		return true;
	}

	//////////////////////////////////////////////////////////////////////
	//새로운 Matcher를 만드는 방법
	TEST(Extending, WritingNewMatchers)
	{
		// Verifies that a value is divisible by 7 and the other is not.
		EXPECT_THAT(21, IsDivisibleBy7());
		EXPECT_THAT(24, Not(IsDivisibleBy7()));

		EXPECT_THAT(21, IsDivisibleBy7_UsingExpect());
		//27을 넣었을때 메세지
		//Expected equality of these values :
		//remainder
		//	Which is : 6
		//0
	}

	//std::format은 c++20부터
	MATCHER_P2(InClosedRange_1, low, hi,
		std::format("{} in range [{}, {}]", negation ? "isn't" : "is",
			PrintToString(low), PrintToString(hi))) {
		return low <= arg && arg <= hi;
	}

	MATCHER_P2(InClos_edRaNge_2, low, hi, "") {
		return low <= arg && arg <= hi;
	}

	//////////////////////////////////////////////////////////////////////
	//매개변수가 있는 Matcher 작성방법
	TEST(Extending, ParameterizedMatchers)
	{
		EXPECT_THAT(4, InClosedRange_1(4, 6));
		//3을 넣었을 경우
		//error: Value of : 3
		//Expected : is in range[4, 6]
		//Actual : 3 (of type int)

		EXPECT_THAT(4, InClos_edRaNge_2(4, 6));
		//3을 넣었을 경우
		//error: Value of : 3
		//Expected: in clos ed ra nge 2 (4, 6)	//대문자나 _에 맞게 공간을 띄움
		//Actual : 3 (of type int)
	}

	class DivisibleBy7Matcher
	{
	public:
		using is_gtest_matcher = void;	//이 클래스가 matcher임을 나타냄

		bool MatchAndExplain(int n, std::ostream*) const {
			return (n % 7) == 0;
		}

		void DescribeTo(std::ostream* os) const {	//std::ostream으로 출력하면 디버그 창에서도 볼수 있음
			*os << "is divisible by 7";
		}

		void DescribeNegationTo(std::ostream* os) const {
			*os << "is not divisible by 7";
		}
	};

	// DivisibleBy7 함수: 매처를 반환하는 헬퍼 함수
	Matcher<int> DivisibleBy7() 
	{
		return DivisibleBy7Matcher();
	}

	//////////////////////////////////////////////////////////////////////
	//단형성(다형성과 반댓말) Matcher작성
	TEST(Extending, MonomorphicMatchers)
	{
		EXPECT_THAT(21, DivisibleBy7());
		//23을 넣었을때 '디버그'창에서.
		//error: Value of : 23
		//Expected : is divisible by 7
		//Actual : 23 (of type int)
	}

	class NotNullMatcher {
	public:
		using is_gtest_matcher = void;

		virtual ~NotNullMatcher() = default;
		//다형성 Matcher를 구현하려면 MatchAndExplain맴버함수 템플릿으로 만들고 첫 번째 인수를 T로 하면 됨.
		template <typename T>
		bool MatchAndExplain(T* p, std::ostream*) const {
			return p != nullptr;
		}

		// 이 Matcher와 일치하는 값의 속성을 설명
		void DescribeTo(std::ostream* os) const { *os << "is not NULL"; }

		// 이 Matcher와 일치하지 않는 값의 속성을 설명
		void DescribeNegationTo(std::ostream* os) const { *os << "is NULL"; }
	};

	NotNullMatcher NotNull() {
		return NotNullMatcher();
	}

	//////////////////////////////////////////////////////////////////////
	//다형성 Matcher작성
	TEST(Extending, PolymorphicMatchers)
	{
		MockFoo foo;
		EXPECT_CALL(foo, Bar(NotNull()));

		int a{ 0 };
		foo.Bar(&a);
		//nullptr을 넣었을때
		//error: Actual function call count doesn't match EXPECT_CALL(foo, Bar(NotNullMatcher()))...
		//Expected : to be called once
		//Actual : never called - unsatisfied and active
	}

	class EvenNumberCardinality : public CardinalityInterface {
	public:
		//이 Matcher는 짝수일때만 참임을 나타냄
		bool IsSatisfiedByCallCount(int call_count) const override {
			return (call_count % 2) == 0;
		}

		//호출횟수가 특정값에 도달했는지 체크
		bool IsSaturatedByCallCount(int call_count) const override {
			return false;
		}

		//설명
		void DescribeTo(std::ostream* os) const {
			*os << "called even number of times";
		}
	};

	Cardinality EvenNumber() {
		return MakeCardinality(new EvenNumberCardinality);
	}

	//////////////////////////////////////////////////////////////////////
	//횟수를 편의에 맞게.
	TEST(Extending, NewCardinalities)
	{
		MockFoo foo;
		EXPECT_CALL(foo, Cardinality(3))
			.Times(EvenNumber());

		foo.Cardinality(3);
		foo.Cardinality(3);
		//foo.Cardinality(3); //세번 호출했을때의 메세지
		//error: Actual function call count doesn't match EXPECT_CALL(foo, Cardinality(3))...
		//Expected : to be called even number of times
		//Actual : called 3 times - unsatisfied and active
	}

	//int DoSomething(bool flag, int* ptr); //이 문장들에서 사용가능한 것들
	/*
		arg0 :	the value of flag
		arg0_type :	the type bool
		arg1 :	the value of ptr
		arg1_type :	the type int*
		args	the :	tuple(flag, ptr)
		args_type :	the type std::tuple<bool, int*>
		return_type :	the type int
		function_type :	the type int(bool, int*)
	*/

	//ACTION 인수 및 매개 변수 유형 제한
	/*
		ACTION(Foo) {
			// arg0이 int로 변환 될 수 있는지 확인하십시오. 
			int  n = arg0;
			// ... 여기서 arg0 대신 n을 사용합니다 ... 
		}

		ACTION_P(Bar, param) {
			// arg1의 유형이 const char *인지 확인하십시오. 
			::testing::StaticAssertTypeEq < const  char*, arg1_type >();

			// param이 bool로 변환 될 수 있는지 확인하십시오. 
			bool  flag = param;
		}
	*/

	struct MultiplyBy 
	{
		template <typename T>
		T operator()(T arg) { return arg * multiplier; }

		int multiplier;
	};
	
	//들어오는 인자가 arg0, arg1이 된다.
	ACTION_P2(ReturnDistanceTo, x, y) {
		double dx = arg0 - x;
		double dy = arg1 - y;
		return sqrt(dx * dx + dy * dy);
	}

	//////////////////////////////////////////////////////////////////////
	//새로운 Action 작성하기
	TEST(Extending, NewAction)
	{
		MockFunction<int(int)> mock1;
		EXPECT_CALL(mock1, Call).WillOnce([](const int input) { return input * 7; });	//람다로 하는 방법
		EXPECT_EQ(mock1.AsStdFunction()(2), 14);

		MockFoo foo;
		EXPECT_CALL(foo, Multiply).WillOnce(MultiplyBy{ 7 });	//스트럭쳐로 하는 방법
		EXPECT_EQ(foo.Multiply(2), 14);

		EXPECT_CALL(foo, DistanceTo).WillOnce(ReturnDistanceTo(5.0, 26.5));	//함수로 하는 방법
		foo.DistanceTo(2, 3);
	}

	// DuplicateArg<k, T>(output) converts the k-th argument of the mock
	// function to type T and copies it to *output.
	ACTION_TEMPLATE(DuplicateArg,
		// Note the comma between int and k:
		HAS_2_TEMPLATE_PARAMS(int, k, typename, T),	//액션이 사용할 '템플릿 파라미터의 수'와 타입을 정의
		AND_1_VALUE_PARAMS(output)) {	//액션에서 사용할 '값 파라미터'의 수와 이름을 정의
		*output = T(std::get<k>(args));
	}

	//////////////////////////////////////////////////////////////////////
	//새로운 Action 템플릿 작성하기
	TEST(Extending, NewActionTemplates)
	{
		MockFoo foo;
		int n;
		EXPECT_CALL(foo, ActionTemplateTest).WillOnce(DuplicateArg<1, unsigned char>(&n));
		foo.ActionTemplateTest(1, 'a', &n);
		EXPECT_EQ(n, 97);
	}

	typedef int IncrementMethod(int*);

	class IncrementArgumentAction : public ActionInterface<IncrementMethod> {
	public:
		int Perform(const std::tuple<int*>& args) override {
			int* p = std::get<0>(args);  // Grabs the first argument.
			auto result = *p;
			*p += 1;
			return result;
		}
	};

	Action<IncrementMethod> IncrementArgument() {
		return MakeAction(new IncrementArgumentAction);
	}

	//////////////////////////////////////////////////////////////////////
	//새로운 Monomorphic Actions 작성하기
	TEST(Extending, NewMonomorphicActions)
	{
		MockFoo foo;

		EXPECT_CALL(foo, Baz(_))
			.WillOnce(IncrementArgument());

		int n = 5;
		auto result = foo.Baz(&n);
		EXPECT_EQ(result, 5);
		EXPECT_EQ(n, 6);
	}

	class ReturnSecondArgumentAction {
	public:
		template <typename Result, typename ArgumentTuple>
		Result Perform(const ArgumentTuple& args) const {
			// i-번째(0 기준) 인수를 얻으려면 std::get<i>(args) 사용합니다. 
			return std::get<1>(args);
		}
	};

	PolymorphicAction<ReturnSecondArgumentAction> ReturnSecondArgument() {
		return MakePolymorphicAction(ReturnSecondArgumentAction());
	}

	//////////////////////////////////////////////////////////////////////
	//새로운 Monomorphic Actions 작성하기
	TEST(Extending, NewPolymorphicActions)
	{
		MockFoo foo;
		EXPECT_CALL(foo, DoThis).WillOnce(ReturnSecondArgument());
		EXPECT_CALL(foo, DoThat).WillOnce(ReturnSecondArgument());

		EXPECT_EQ(foo.DoThis(true, 5), 5);
		EXPECT_EQ(foo.DoThat(1, "Hi", "Bye"), "Hi");
	}

	int Foo(std::function<int(std::string)> fun)
	{
		return fun("bar");
	};

	//////////////////////////////////////////////////////////////////////
	//std::Function mock 하기
	TEST(Extending, MockFunction) 
	{
		MockFunction<int(std::string)> mock_function;
		//"bar"가 인수, 1이 리턴값이다.
		EXPECT_CALL(mock_function, Call("bar")).WillOnce(Return(1));

		EXPECT_EQ(Foo(mock_function.AsStdFunction()), 1);
	}
}