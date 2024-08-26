#include "CookBook2.h"
#include <memory>

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
}