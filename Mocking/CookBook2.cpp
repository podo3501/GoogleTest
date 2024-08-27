#include "CookBook2.h"
#include <memory>
#include <algorithm>
#include <vector>
#include <ranges>
#include <array>
#include <map>

using ::testing::_;	//� ���� �������
using ::testing::Eq; //Equal(==)
using ::testing::Ne;	//Not Equal (!=)
using ::testing::Lt;	//Little(<) �۰�
using ::testing::Le;	//Little or Equal(<=) �۰�
using ::testing::Gt;	//Greater (>)
using ::testing::Ge;	//Greater or Equal (>=)
using ::testing::Not; //Not (!)
using ::testing::AllOf;	//������ ��
using ::testing::HasSubstr;	//�κ������� �� ���ڿ��� ����
using ::testing::NotNull;	//Not nullptr
using ::testing::Return;	//���ϰ�
using ::testing::SafeMatcherCast;
using ::testing::ReturnRef;	//���۷����� ����
using ::testing::Const;	//Const�� ����ȯ

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
	//�μ��� ���� �����ϰ� ��ġ
	TEST(Matcher, Simple)
	{
		MockFoo mock;
		EXPECT_CALL(mock, DoThis(Ge(5))).WillOnce(Return('a'));	//Greater Equal(>= 5)
		EXPECT_CALL(mock, DoThat(_, NotNull()));	//ù��° �μ��� ��� �Է� �������. �ι�° �μ��� �׻� nullptr�� �ƴ�

		EXPECT_EQ(mock.DoThis(5), 'a');
		int address = 0;
		mock.DoThat("test", &address);

		EXPECT_CALL(mock, DoThis(AllOf(Gt(5), Ne(10))));	//�μ��� �ݵ�� >5 && != 10
		EXPECT_CALL(mock, DoThat(Not(HasSubstr("blah")), nullptr)); //ù ��° �μ��� �κ� ���ڿ��� "blah"�� �������� ����

		mock.DoThis(7);
		mock.DoThat("ablae", nullptr);
	}

	TEST(Matcher, TypeCasting)
	{
		MockFoo mock;
		std::unique_ptr<Base> m = std::make_unique<Derived>();
		EXPECT_CALL(mock, DoThisPtr(SafeMatcherCast<Derived*>(m.get())));	//��������� �����ϰ� ��ȯ�����ش�.
		mock.DoThisPtr(static_cast<Derived*>(m.get()));
	}

	//////////////////////////////////////////////////////////////////////
	//Const�� �����ε� �� ���
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

	using ::testing::An;	//int���̸� �ƹ��ų�
	using ::testing::Matcher;
	using ::testing::TypedEq;

	//////////////////////////////////////////////////////////////////////
	//�μ��� ������ ���� ���°� �ٸ��� �����ε� �� �Լ��� ��ȣ���� ���ַ��� matcher�� ��Ȯ�� ���¸� �����ؾ��Ѵ�.
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
	//�μ��� ���� �ٸ� Action �����ϱ�
	TEST(Matcher, PerformingDifferentActions)
	{
		MockFoo foo;
		//�μ��� ������ ó���� ũ�� ��� ���� ��ü������ ������ �����ϸ� �ٸ� Action�� �����ų�� �ִ�.
		EXPECT_CALL(foo, DoThis(_)).WillRepeatedly(Return('b'));	//�⺻��
		EXPECT_CALL(foo, DoThis(Lt(5))).WillRepeatedly(Return('a'));	//�� ��ü��
		
		EXPECT_EQ(foo.DoThis(4), 'a');
		EXPECT_EQ(foo.DoThis(6), 'b');
	}

	using ::testing::Args;
	TEST(Matcher, MultipleArguments)
	{
		MockFoo foo;
		//With() ���� ���� Matcher<tr1::tuple<A1, ..., An>> ������ matcher�� �Ǿ�� ��
		EXPECT_CALL(foo, InRange(Ne(0), _)).With(Lt());	//�μ�1�� 2���� �۾ƾ���(Little<)
		foo.InRange(1, 2);

		EXPECT_CALL(foo, Blah(_, _, _)).With(AllOf(Args<0, 1>(Lt()), Args<1, 2>(Lt())));	//x < y < z
		foo.Blah(1, 2, 3);
	}

	//////////////////////////////////////////////////////////////////////
	//�����Լ��ν� Matcher Ȱ��. ���� ����� STL�ε� �ۼ� �� �� ������, ���ŷӴ�.
	using ::testing::Matches;
	TEST(Matcher, UsingMatchersAsPredicates)
	{
		std::vector<int> v1{ 1, 7, 12, 16 };
		auto count1 = std::ranges::count_if(v1, Matches(Ge(10)));
		EXPECT_EQ(count1, 2);	//10���� ū ������ ������ 2���̴�.

		std::vector<int> v2{ -1, 101, 50, 7 };
		auto matches = Matches(AllOf(Ge(0), Le(100), Ne(50)));	
		auto count2 = std::ranges::count_if(v2, matches);
		EXPECT_EQ(count2, 1);	//0�̻� 10���� 50�� ������ ���ڴ� 1���̴�.
	}

	using ::testing::MatchesRegex;
	using ::testing::StartsWith;
	//////////////////////////////////////////////////////////////////////
	//��ǿ��� Matcher ����ϱ�. Matcher�� _THAT���� ����� �� �ִ�.
	TEST(Matcher, Assertions)
	{
		auto Foo = []() { return "Hello, world!"; };
		auto Bar = []() { return std::string("Line 347"); };
		auto Baz = []() { return 8; };

		EXPECT_THAT(Foo(), StartsWith("Hello"));	//Hello�� ���۵Ǵ� ���ڿ�
		EXPECT_THAT(Bar(), MatchesRegex("Line \\d+"));	//����ǥ����. Line���� �����ؼ� �����ε� 1ȸ �̻� �ݺ��� ��Ÿ��. �� 2�ڸ� �̻� ����
		ASSERT_THAT(Baz(), AllOf(Ge(5), Le(10)));	//5�̻� 10 ����
	}

	using ::testing::Truly;
	//////////////////////////////////////////////////////////////////////
	//�����Լ��� Matcher�� Ȱ��. ���� �Լ��� Truely() �Լ��� ���������ν� ����
	TEST(Matcher, UsingPredicatesAsMatchers)
	{
		MockFoo foo;
		auto IsEven = [](int n)->int { return (n % 2) == 0 ? 1 : 0; };
		EXPECT_CALL(foo, DoThis(Truly(IsEven)));	//¦���� ȣ���� ���� ����
		foo.DoThis(2);
	}

	//////////////////////////////////////////////////////////////////////
	//������ �� ���� �μ��� ��ġ�ϱ�
	//�μ��� �⺻������ ����� ����. Bar(const Bar&) = delete; �ϰ�쿡�� ���� �߻�
	using ::testing::ByRef;
	TEST(Matcher, AreNotCopyable)
	{
		MockFoo mock;
		Bar bar0{ 0 }, bar1{ 1 };
		//EXPECT_CALL(mock, FuncByRef(bar0));	//error C2280: ������ �Լ��� �����Ϸ��� �մϴ�.

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
	//��ü�� ��� Ȯ���ϱ�. 
	// Field�� public�� �ɹ������� ����. Property�� �μ��� ������ �ʴ� 'const' �Լ��� ����
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
	//������ �μ��� ����Ű�� �� Ȯ���ϱ�. 
	//�����Ͱ� ����Ű�� ���� �Ӽ��� Ȯ��. ��ø�Ͽ� ��밡�� ex) Pointee(Pointee(Lt (3)))
	TEST(Validating, PointerArgument)
	{
		MockBoo boo;
		EXPECT_CALL(boo, Bar(Pointee(Ge(3))));	//�������� ���� 3�̻��� �μ��� ȣ���� ���� ����

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
	//Ŀ���� Matcher Class �����
	//bar() + baz() ������� Ȯ���� ���� custom matcher class
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
	
	using ::testing::ElementsAre;	//������ �°� ��� ���� Ȯ��
	using ::testing::UnorderedElementsAre;	//������ ������� ��� ���� Ȯ��
	using ::testing::ElementsAreArray;	//10�� �̻��� ��쿡�� Array�� ���
	using ::testing::UnorderedElementsAreArray;
	using ::testing::Pair;

	//////////////////////////////////////////////////////////////////////
	//�����̳� ��Ī�ϱ�
	//Tips
	//ElementAre* ()�� STL�� ���ǵ��� ���� ��쿡�� STL �ݺ��� ������ �����մϴ�(��, const_iterator ������ ������ begin() / end()�� �����մϴ�), size() �� �����ϴ� ��� �����̳ʿ� ���� �۵��մϴ�.������ ���� ������ ������ �� ���� �ۼ����� ���� �����̳� ������ ���ؼ��� �۵��մϴ�.
	//��ø ��(������) �����̳ʿ� ��ġ��Ű�� ���� ElementAre* ()�� ��ø�Ͽ� ����� �� �ֽ��ϴ�.
	//�����̳ʸ� ������ �ƴ� �����ͷ� �����Ϸ��� Pointee(ElementsAre* (...))��� ����˴ϴ�.
	//ElementsAre* ()�� ��� ��� ������ �߿��� �����Դϴ�.����� ������ ���ǵ��� ���� �����̳�(�� : hash_map)�����̸� ������� ���ʽÿ�.
	TEST(MatchingContainers, Test)
	{
		MockDoo mock;
		//ElementsAre ()�� 0���� 10������ �μ��� ���ϵ��� �����ε��
		EXPECT_CALL(mock, SetList(ElementsAre(1, Gt(0), _, 5)));	//���Ұ� 4���̰�, ������ �°� 1, 0�̻�, �ƹ��ų�, 5 �� �� �ִ�.
		mock.SetList(std::vector<int>{1, 2, 3, 5});

		EXPECT_CALL(mock, SetList(UnorderedElementsAre(1, Gt(0), _, 5)));	//���Ұ� 4���̰�, ������ ������� 1, 0�̻�, �ƹ��ų�, 5 �� �� �ִ�.		
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
	//Matcher �����ϱ�. Matcher�� ���� ī��Ʈ ����� �����ͷ� ����. 
	TEST(Matcher, Sharing)
	{
		//Matcher<int> in_range = AllOf(Gt(5), Le(10));
		auto in_range = AllOf(Gt(5), Le(10));
		EXPECT_THAT(7, in_range);
		EXPECT_THAT(8, in_range);
	}
}