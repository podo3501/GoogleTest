#include "CookBook2.h"
#include <memory>
#include <algorithm>
#include <vector>
#include <ranges>
#include <array>
#include <map>

using ::testing::_;	//어떤 수라도 상관없음
using ::testing::Eq; //Equal(==)
using ::testing::Ne;	//Not Equal (!=)
using ::testing::Lt;	//Little(<) 작게
using ::testing::Le;	//Little or Equal(<=) 작게
using ::testing::Gt;	//Greater (>)
using ::testing::Ge;	//Greater or Equal (>=)
using ::testing::Not; //Not (!)
using ::testing::AllOf;	//모든것이 참
using ::testing::HasSubstr;	//부분적으로 이 문자열이 있음
using ::testing::NotNull;	//Not nullptr
using ::testing::Return;	//리턴값
using ::testing::SafeMatcherCast;
using ::testing::ReturnRef;	//레퍼런스로 리턴
using ::testing::Const;	//Const로 형변환

namespace CookBook2
{
	class Base 
	{ 
	public:
		virtual ~Base() {} 
	};

	class Derived : public Base
	{};

	class Bar
	{
	public:
		Bar() = default;
		Bar(int n) : m_n{ n } {}
		Bar(const Bar&) = delete;

		bool operator==(const Bar&) const { return true; }
		bool operator<(const Bar& rhs) const { return m_n < rhs.m_n; }

	private:
		int m_n = 0;
	};

	class Foo
	{
	public:
		virtual ~Foo() {};
		virtual char DoThis(int n) = 0;
		virtual void DoThat(const char* s, int* p) = 0;

		virtual void DoThisPtr(Derived* derived) = 0;

		virtual Bar& GetBar() = 0;
		virtual const Bar& GetBar() const = 0;

		virtual void InRange(int start, int end) = 0;
		virtual void Blah(int x, int y, int z) = 0;

		virtual void FuncByRef(Bar& bar) = 0;
	};

	class MockFoo : public Foo
	{
	public:
		MOCK_METHOD(char, DoThis, (int n), (override));
		MOCK_METHOD(void, DoThat, (const char* s, int* p), (override));

		MOCK_METHOD(void, DoThisPtr, (Derived* derived), (override));

		MOCK_METHOD(Bar&, GetBar, (), (override));
		MOCK_METHOD(const Bar&, GetBar, (), (const, override));

		MOCK_METHOD(void, InRange, (int start, int end), (override));
		MOCK_METHOD(void, Blah, (int x, int y, int z), (override));

		MOCK_METHOD(void, FuncByRef, (Bar& bar), (override));
	};

	//////////////////////////////////////////////////////////////////////
	//인수의 값에 엄밀하게 매치
	TEST(Matcher, Simple)
	{
		MockFoo mock;
		EXPECT_CALL(mock, DoThis(Ge(5))).WillOnce(Return('a'));	//Greater Equal(>= 5)
		EXPECT_CALL(mock, DoThat(_, NotNull()));	//첫번째 인수는 모든 입력 상관없음. 두번째 인수는 항상 nullptr이 아님

		EXPECT_EQ(mock.DoThis(5), 'a');
		int address = 0;
		mock.DoThat("test", &address);

		EXPECT_CALL(mock, DoThis(AllOf(Gt(5), Ne(10))));	//인수는 반드시 >5 && != 10
		EXPECT_CALL(mock, DoThat(Not(HasSubstr("blah")), nullptr)); //첫 번째 인수는 부분 문자열로 "blah"를 포함하지 않음

		mock.DoThis(7);
		mock.DoThat("ablae", nullptr);
	}

	TEST(Matcher, TypeCasting)
	{
		MockFoo mock;
		std::unique_ptr<Base> m = std::make_unique<Derived>();
		EXPECT_CALL(mock, DoThisPtr(SafeMatcherCast<Derived*>(m.get())));	//상속했을때 안전하게 변환시켜준다.
		mock.DoThisPtr(static_cast<Derived*>(m.get()));
	}

	//////////////////////////////////////////////////////////////////////
	//Const로 오버로드 된 경우
	TEST(Matcher, ConstOverloaded)
	{
		MockFoo mock;

		Bar bar1, bar2;
		EXPECT_CALL(mock, GetBar())         // The non-const GetBar().
			.WillOnce(ReturnRef(bar1));
		EXPECT_CALL(Const(mock), GetBar())  // The const GetBar().
			.WillOnce(ReturnRef(bar2));

		Bar& retBar1 = mock.GetBar();
		const Bar& retBar2 = Const(mock).GetBar();
	}

	class Printer
	{
	public:
		virtual ~Printer() {}
		virtual void Print(int n) = 0;
		virtual void Print(char c) = 0;
	};

	class MockPrinter : public Printer {
	public:
		MOCK_METHOD(void, Print, (int n), (override));
		MOCK_METHOD(void, Print, (char c), (override));
	};

	using ::testing::An;	//int형이면 아무거나
	using ::testing::Matcher;
	using ::testing::TypedEq;

	//////////////////////////////////////////////////////////////////////
	//인수의 개수는 같고 형태가 다르게 오버로드 된 함수의 모호성을 없애려면 matcher의 정확한 형태를 지정해야한다.
	TEST(Matcher, Overloaded)
	{
		MockPrinter printer;

		EXPECT_CALL(printer, Print(An<int>()));            // void Print(int);
		EXPECT_CALL(printer, Print(Matcher<int>(Lt(5))));  // void Print(int);
		EXPECT_CALL(printer, Print(TypedEq<char>('a')));   // void Print(char);

		printer.Print(3);
		printer.Print(6);
		printer.Print('a');
	}

	//////////////////////////////////////////////////////////////////////
	//인수에 따라 다른 Action 실행하기
	TEST(Matcher, PerformingDifferentActions)
	{
		MockFoo foo;
		//인수의 조건을 처음은 크게 잡고 점점 구체적으로 좁혀서 설정하면 다른 Action을 실행시킬수 있다.
		EXPECT_CALL(foo, DoThis(_)).WillRepeatedly(Return('b'));	//기본값
		EXPECT_CALL(foo, DoThis(Lt(5))).WillRepeatedly(Return('a'));	//더 구체적
		
		EXPECT_EQ(foo.DoThis(4), 'a');
		EXPECT_EQ(foo.DoThis(6), 'b');
	}

	using ::testing::Args;
	TEST(Matcher, MultipleArguments)
	{
		MockFoo foo;
		//With() 내부 식은 Matcher<tr1::tuple<A1, ..., An>> 형식의 matcher가 되어야 함
		EXPECT_CALL(foo, InRange(Ne(0), _)).With(Lt());	//인수1이 2보다 작아야함(Little<)
		foo.InRange(1, 2);

		EXPECT_CALL(foo, Blah(_, _, _)).With(AllOf(Args<0, 1>(Lt()), Args<1, 2>(Lt())));	//x < y < z
		foo.Blah(1, 2, 3);
	}

	//////////////////////////////////////////////////////////////////////
	//술어함수로써 Matcher 활용. 같은 기능을 STL로도 작성 할 수 있지만, 번거롭다.
	using ::testing::Matches;
	TEST(Matcher, UsingMatchersAsPredicates)
	{
		std::vector<int> v1{ 1, 7, 12, 16 };
		auto count1 = std::ranges::count_if(v1, Matches(Ge(10)));
		EXPECT_EQ(count1, 2);	//10보다 큰 숫자의 갯수는 2개이다.

		std::vector<int> v2{ -1, 101, 50, 7 };
		auto matches = Matches(AllOf(Ge(0), Le(100), Ne(50)));	
		auto count2 = std::ranges::count_if(v2, matches);
		EXPECT_EQ(count2, 1);	//0이상 10이하 50은 제외인 숫자는 1개이다.
	}

	using ::testing::MatchesRegex;
	using ::testing::StartsWith;
	//////////////////////////////////////////////////////////////////////
	//어설션에서 Matcher 사용하기. Matcher는 _THAT에서 사용할 수 있다.
	TEST(Matcher, Assertions)
	{
		auto Foo = []() { return "Hello, world!"; };
		auto Bar = []() { return std::string("Line 347"); };
		auto Baz = []() { return 8; };

		EXPECT_THAT(Foo(), StartsWith("Hello"));	//Hello로 시작되는 문자열
		EXPECT_THAT(Bar(), MatchesRegex("Line \\d+"));	//정규표현식. Line으로 시작해서 숫자인데 1회 이상 반복을 나타냄. 즉 2자리 이상 숫자
		ASSERT_THAT(Baz(), AllOf(Ge(5), Le(10)));	//5이상 10 이하
	}

	using ::testing::Truly;
	//////////////////////////////////////////////////////////////////////
	//술어함수를 Matcher로 활용. 술어 함수를 Truely() 함수로 래핑함으로써 가능
	TEST(Matcher, UsingPredicatesAsMatchers)
	{
		MockFoo foo;
		auto IsEven = [](int n)->int { return (n % 2) == 0 ? 1 : 0; };
		EXPECT_CALL(foo, DoThis(Truly(IsEven)));	//짝수로 호출할 것을 예상
		foo.DoThis(2);
	}

	//////////////////////////////////////////////////////////////////////
	//복사할 수 없는 인수와 일치하기
	//인수는 기본적으로 복사로 저장. Bar(const Bar&) = delete; 일경우에는 에러 발생
	using ::testing::ByRef;
	TEST(Matcher, AreNotCopyable)
	{
		MockFoo mock;
		Bar bar0{ 0 }, bar1{ 1 };
		//EXPECT_CALL(mock, FuncByRef(bar0));	//error C2280: 삭제된 함수를 참조하려고 합니다.

		EXPECT_CALL(mock, FuncByRef(Eq(ByRef(bar0))));
		mock.FuncByRef(bar0);

		EXPECT_CALL(mock, FuncByRef(Lt(ByRef(bar1))));
		mock.FuncByRef(bar0);
	}

	struct Aoo
	{
		Aoo(int n) : m_number{ n } {}
		int GetNumber() { return m_number; };
		std::string name() const { return "John Doe"; };
		int m_number{ 0 };
	};

	using ::testing::Field;
	using ::testing::Property;
	//////////////////////////////////////////////////////////////////////
	//객체의 멤버 확인하기. 
	// Field는 public인 맴버변수에 접근. Property는 인수를 취하지 않는 'const' 함수에 적용
	TEST(Validating, Member)
	{
		Aoo aoo(6);
		EXPECT_THAT(aoo, Field(&Aoo::m_number, Ge(5)));
		EXPECT_THAT(aoo, Property(&Aoo::name, StartsWith("John")));
	}

	class Boo
	{
	public:
		virtual ~Boo() {}
		virtual void Bar(int* data) = 0;
	};

	class MockBoo : public Boo
	{
	public:
		MOCK_METHOD(void, Bar, (int* data), (override));
	};

	using ::testing::Pointee;
	//////////////////////////////////////////////////////////////////////
	//포인터 인수의 가리키는 값 확인하기. 
	//포인터가 가리키는 값의 속성을 확인. 중첩하여 사용가능 ex) Pointee(Pointee(Lt (3)))
	TEST(Validating, PointerArgument)
	{
		MockBoo boo;
		EXPECT_CALL(boo, Bar(Pointee(Ge(3))));	//포인터의 값이 3이상인 인수로 호출할 것을 예상

		int test = 3;
		boo.Bar(&test);
	}

	class Coo
	{
	public:
		int bar() const { return 2; }
		int baz() const { return 3; }
	};

	class BarPlusBazEqMatcher {
	public:
		using is_gtest_matcher = void;

		explicit BarPlusBazEqMatcher(int expected_sum)
			: expected_sum_(expected_sum) {}

		bool MatchAndExplain(const Coo& coo,
			std::ostream* /* listener */) const {
			return (coo.bar() + coo.baz()) == expected_sum_;
		}

		void DescribeTo(std::ostream* os) const {
			*os << "bar() + baz() equals " << expected_sum_;
		}

		void DescribeNegationTo(std::ostream* os) const {
			*os << "bar() + baz() does not equal " << expected_sum_;
		}
	private:
		const int expected_sum_;
	};

	::testing::Matcher<const Coo&> BarPlusBazEq(int expected_sum) {
		return BarPlusBazEqMatcher(expected_sum);
	}

	//////////////////////////////////////////////////////////////////////
	//커스텀 Matcher Class 만들기
	//bar() + baz() 결과값을 확인차 만든 custom matcher class
	TEST(CustomMatcherClass, Test)
	{
		Coo coo;
		EXPECT_THAT(coo, BarPlusBazEq(5));
	}

	class Doo
	{
	public:
		virtual ~Doo() {}
		virtual void SetList(const std::vector<int>& numbers) = 0;
	};

	class MockDoo : public Doo
	{
	public:
		MOCK_METHOD(void, SetList, (const std::vector<int>& numbers), (override));
	};
	
	using ::testing::ElementsAre;	//순서에 맞게 어떤게 들어갈지 확인
	using ::testing::UnorderedElementsAre;	//순서에 상관없이 어떤게 들어갈지 확인
	using ::testing::ElementsAreArray;	//10개 이상일 경우에는 Array를 사용
	using ::testing::UnorderedElementsAreArray;
	using ::testing::Pair;

	//////////////////////////////////////////////////////////////////////
	//컨테이너 매칭하기
	//Tips
	//ElementAre* ()는 STL에 정의되지 않은 경우에도 STL 반복자 개념을 구현합니다(즉, const_iterator 유형을 가지며 begin() / end()를 지원합니다), size() 를 지원하는 모든 컨테이너에 대해 작동합니다.오히려 위의 패턴을 따르는 한 아직 작성되지 않은 컨테이너 유형에 대해서도 작동합니다.
	//중첩 된(다차원) 컨테이너와 일치시키기 위해 ElementAre* ()를 중첩하여 사용할 수 있습니다.
	//컨테이너를 참조가 아닌 포인터로 전달하려면 Pointee(ElementsAre* (...))라고 쓰면됩니다.
	//ElementsAre* ()의 경우 요소 순서는 중요한 문제입니다.요소의 순서가 정의되지 않은 컨테이너(예 : hash_map)에는이를 사용하지 마십시오.
	TEST(MatchingContainers, Test)
	{
		MockDoo mock;
		//ElementsAre ()는 0에서 10까지의 인수를 취하도록 오버로드됨
		EXPECT_CALL(mock, SetList(ElementsAre(1, Gt(0), _, 5)));	//원소가 4개이고, 순서에 맞게 1, 0이상, 아무거나, 5 가 들어가 있다.
		mock.SetList(std::vector<int>{1, 2, 3, 5});

		EXPECT_CALL(mock, SetList(UnorderedElementsAre(1, Gt(0), _, 5)));	//원소가 4개이고, 순서에 상관없이 1, 0이상, 아무거나, 5 가 들어가 있다.		
		mock.SetList(std::vector<int>{5, 3, 2, 1});

		const int expected_vector1[] = { 1, 5, 2, 4, 1, 2, 3, 4, 5, 6, 7 };
		EXPECT_CALL(mock, SetList(ElementsAreArray(expected_vector1)));
		mock.SetList(std::vector<int>{ 1, 5, 2, 4, 1, 2, 3, 4, 5, 6, 7 });

		Matcher<int> expected_vector2[] = { 1, Gt(2), _, 3, 1, 2, 3, 4, 5, 6, 7 };
		EXPECT_CALL(mock, SetList(ElementsAreArray(expected_vector2)));
		mock.SetList(std::vector<int>{ 1, 5, 2, 3, 1, 2, 3, 4, 5, 6, 7 });

		std::vector<int> expected_vector3{ 1, 7, 5 };
		EXPECT_CALL(mock, SetList(ElementsAreArray(expected_vector3.data(), expected_vector3.size())));
		mock.SetList(std::vector<int>{ 1, 7, 5 });

		std::map<std::string, int> m = { {"a", 1}, {"b", 2}, {"c", 3} };
		EXPECT_THAT(m, UnorderedElementsAre(Pair("a", 1), Pair("b", 2), Pair("c", 3)));
	}

	//////////////////////////////////////////////////////////////////////
	//Matcher 공유하기. Matcher는 참조 카운트 방식의 포인터로 구성. 
	TEST(Matcher, Sharing)
	{
		//Matcher<int> in_range = AllOf(Gt(5), Le(10));
		auto in_range = AllOf(Gt(5), Le(10));
		EXPECT_THAT(7, in_range);
		EXPECT_THAT(8, in_range);
	}
}