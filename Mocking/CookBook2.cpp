#include "CookBook2.h"
#include <memory>

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
using ::testing::ReturnRef;
using ::testing::Const;

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
	{};

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
}