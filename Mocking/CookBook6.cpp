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
	
	//TEST�Լ� �ȿ����� MATCHER�� �ۼ� �ȵ�
	//arg�� Ÿ���� �Ű� ���� �ʾƵ� ��.
	MATCHER(IsDivisibleBy7, 
		std::string(negation ? "isn't" : "is") + std::string(" divisible by 7")) 
	{
		auto remain = arg % 7;
		if (remain == 0)
			return true;

		//������� �ٿ��� ������ ����Ѵ�.
		*result_listener << "the remainder is " << remain << '\n';
		return false;
	}

	MATCHER(IsDivisibleBy7_UsingExpect, "") {
		const auto remainder = arg % 7;
		EXPECT_EQ(remainder, 0);
		return true;
	}

	//////////////////////////////////////////////////////////////////////
	//���ο� Matcher�� ����� ���
	TEST(Extending, WritingNewMatchers)
	{
		// Verifies that a value is divisible by 7 and the other is not.
		EXPECT_THAT(21, IsDivisibleBy7());
		EXPECT_THAT(24, Not(IsDivisibleBy7()));

		EXPECT_THAT(21, IsDivisibleBy7_UsingExpect());
		//27�� �־����� �޼���
		//Expected equality of these values :
		//remainder
		//	Which is : 6
		//0
	}

	//std::format�� c++20����
	MATCHER_P2(InClosedRange_1, low, hi,
		std::format("{} in range [{}, {}]", negation ? "isn't" : "is",
			PrintToString(low), PrintToString(hi))) {
		return low <= arg && arg <= hi;
	}

	MATCHER_P2(InClos_edRaNge_2, low, hi, "") {
		return low <= arg && arg <= hi;
	}

	//////////////////////////////////////////////////////////////////////
	//�Ű������� �ִ� Matcher �ۼ����
	TEST(Extending, ParameterizedMatchers)
	{
		EXPECT_THAT(4, InClosedRange_1(4, 6));
		//3�� �־��� ���
		//error: Value of : 3
		//Expected : is in range[4, 6]
		//Actual : 3 (of type int)

		EXPECT_THAT(4, InClos_edRaNge_2(4, 6));
		//3�� �־��� ���
		//error: Value of : 3
		//Expected: in clos ed ra nge 2 (4, 6)	//�빮�ڳ� _�� �°� ������ ���
		//Actual : 3 (of type int)
	}

	class DivisibleBy7Matcher
	{
	public:
		using is_gtest_matcher = void;	//�� Ŭ������ matcher���� ��Ÿ��

		bool MatchAndExplain(int n, std::ostream*) const {
			return (n % 7) == 0;
		}

		void DescribeTo(std::ostream* os) const {	//std::ostream���� ����ϸ� ����� â������ ���� ����
			*os << "is divisible by 7";
		}

		void DescribeNegationTo(std::ostream* os) const {
			*os << "is not divisible by 7";
		}
	};

	// DivisibleBy7 �Լ�: ��ó�� ��ȯ�ϴ� ���� �Լ�
	Matcher<int> DivisibleBy7() 
	{
		return DivisibleBy7Matcher();
	}

	//////////////////////////////////////////////////////////////////////
	//������(�������� �ݴ�) Matcher�ۼ�
	TEST(Extending, MonomorphicMatchers)
	{
		EXPECT_THAT(21, DivisibleBy7());
		//23�� �־����� '�����'â����.
		//error: Value of : 23
		//Expected : is divisible by 7
		//Actual : 23 (of type int)
	}

	class NotNullMatcher {
	public:
		using is_gtest_matcher = void;

		virtual ~NotNullMatcher() = default;
		//������ Matcher�� �����Ϸ��� MatchAndExplain�ɹ��Լ� ���ø����� ����� ù ��° �μ��� T�� �ϸ� ��.
		template <typename T>
		bool MatchAndExplain(T* p, std::ostream*) const {
			return p != nullptr;
		}

		// �� Matcher�� ��ġ�ϴ� ���� �Ӽ��� ����
		void DescribeTo(std::ostream* os) const { *os << "is not NULL"; }

		// �� Matcher�� ��ġ���� �ʴ� ���� �Ӽ��� ����
		void DescribeNegationTo(std::ostream* os) const { *os << "is NULL"; }
	};

	NotNullMatcher NotNull() {
		return NotNullMatcher();
	}

	//////////////////////////////////////////////////////////////////////
	//������ Matcher�ۼ�
	TEST(Extending, PolymorphicMatchers)
	{
		MockFoo foo;
		EXPECT_CALL(foo, Bar(NotNull()));

		int a{ 0 };
		foo.Bar(&a);
		//nullptr�� �־�����
		//error: Actual function call count doesn't match EXPECT_CALL(foo, Bar(NotNullMatcher()))...
		//Expected : to be called once
		//Actual : never called - unsatisfied and active
	}

	class EvenNumberCardinality : public CardinalityInterface {
	public:
		//�� Matcher�� ¦���϶��� ������ ��Ÿ��
		bool IsSatisfiedByCallCount(int call_count) const override {
			return (call_count % 2) == 0;
		}

		//ȣ��Ƚ���� Ư������ �����ߴ��� üũ
		bool IsSaturatedByCallCount(int call_count) const override {
			return false;
		}

		//����
		void DescribeTo(std::ostream* os) const {
			*os << "called even number of times";
		}
	};

	Cardinality EvenNumber() {
		return MakeCardinality(new EvenNumberCardinality);
	}

	//////////////////////////////////////////////////////////////////////
	//Ƚ���� ���ǿ� �°�.
	TEST(Extending, NewCardinalities)
	{
		MockFoo foo;
		EXPECT_CALL(foo, Cardinality(3))
			.Times(EvenNumber());

		foo.Cardinality(3);
		foo.Cardinality(3);
		//foo.Cardinality(3); //���� ȣ���������� �޼���
		//error: Actual function call count doesn't match EXPECT_CALL(foo, Cardinality(3))...
		//Expected : to be called even number of times
		//Actual : called 3 times - unsatisfied and active
	}

	//int DoSomething(bool flag, int* ptr); //�� ����鿡�� ��밡���� �͵�
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

	//ACTION �μ� �� �Ű� ���� ���� ����
	/*
		ACTION(Foo) {
			// arg0�� int�� ��ȯ �� �� �ִ��� Ȯ���Ͻʽÿ�. 
			int  n = arg0;
			// ... ���⼭ arg0 ��� n�� ����մϴ� ... 
		}

		ACTION_P(Bar, param) {
			// arg1�� ������ const char *���� Ȯ���Ͻʽÿ�. 
			::testing::StaticAssertTypeEq < const  char*, arg1_type >();

			// param�� bool�� ��ȯ �� �� �ִ��� Ȯ���Ͻʽÿ�. 
			bool  flag = param;
		}
	*/

	struct MultiplyBy 
	{
		template <typename T>
		T operator()(T arg) { return arg * multiplier; }

		int multiplier;
	};
	
	//������ ���ڰ� arg0, arg1�� �ȴ�.
	ACTION_P2(ReturnDistanceTo, x, y) {
		double dx = arg0 - x;
		double dy = arg1 - y;
		return sqrt(dx * dx + dy * dy);
	}

	//////////////////////////////////////////////////////////////////////
	//���ο� Action �ۼ��ϱ�
	TEST(Extending, NewAction)
	{
		MockFunction<int(int)> mock1;
		EXPECT_CALL(mock1, Call).WillOnce([](const int input) { return input * 7; });	//���ٷ� �ϴ� ���
		EXPECT_EQ(mock1.AsStdFunction()(2), 14);

		MockFoo foo;
		EXPECT_CALL(foo, Multiply).WillOnce(MultiplyBy{ 7 });	//��Ʈ���ķ� �ϴ� ���
		EXPECT_EQ(foo.Multiply(2), 14);

		EXPECT_CALL(foo, DistanceTo).WillOnce(ReturnDistanceTo(5.0, 26.5));	//�Լ��� �ϴ� ���
		foo.DistanceTo(2, 3);
	}

	// DuplicateArg<k, T>(output) converts the k-th argument of the mock
	// function to type T and copies it to *output.
	ACTION_TEMPLATE(DuplicateArg,
		// Note the comma between int and k:
		HAS_2_TEMPLATE_PARAMS(int, k, typename, T),	//�׼��� ����� '���ø� �Ķ������ ��'�� Ÿ���� ����
		AND_1_VALUE_PARAMS(output)) {	//�׼ǿ��� ����� '�� �Ķ����'�� ���� �̸��� ����
		*output = T(std::get<k>(args));
	}

	//////////////////////////////////////////////////////////////////////
	//���ο� Action ���ø� �ۼ��ϱ�
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
	//���ο� Monomorphic Actions �ۼ��ϱ�
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
			// i-��°(0 ����) �μ��� �������� std::get<i>(args) ����մϴ�. 
			return std::get<1>(args);
		}
	};

	PolymorphicAction<ReturnSecondArgumentAction> ReturnSecondArgument() {
		return MakePolymorphicAction(ReturnSecondArgumentAction());
	}

	//////////////////////////////////////////////////////////////////////
	//���ο� Monomorphic Actions �ۼ��ϱ�
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
	//std::Function mock �ϱ�
	TEST(Extending, MockFunction) 
	{
		MockFunction<int(std::string)> mock_function;
		//"bar"�� �μ�, 1�� ���ϰ��̴�.
		EXPECT_CALL(mock_function, Call("bar")).WillOnce(Return(1));

		EXPECT_EQ(Foo(mock_function.AsStdFunction()), 1);
	}
}