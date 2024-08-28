#include "CookBook3.h"
#include <memory>

namespace CookBook3
{
	using ::testing::_;
	using ::testing::InSequence;	//����������
	using ::testing::Sequence;	//�κ����� ����
	using ::testing::AnyNumber;
	using ::testing::ReturnRef;	//���ϰ��� ������ ��ȯ
	using ::testing::ReturnPointee; //�Ϲ����� ���ϰ��� ��ȯ�Ҷ�
	using ::testing::DoAll;	//�������� Action�� �����ϵ��� �����ϰ� �� �ش�.
	using ::testing::Return;	//����
	using ::testing::SaveArg;	//Argument�� �����Ѵ�. ex) SaveArg<0>(&a) ù��° Argument�� a�� ����
	using ::testing::ElementsAre;
	using ::testing::UnorderedElementsAre;
	using ::testing::Pair;
	using ::testing::SetArgPointee;	//Argument Pointer�� ���� �ִ´�.
	using ::testing::SetArrayArgument;	//Array Argument �� ���� �ִ´�.
	using ::testing::NotNull;

	class FooFactory
	{
	public:
		MOCK_METHOD(std::unique_ptr<Foo>, MakeFoo, (const std::string&, int));
	};

	std::unique_ptr<Foo> BuildFooForTest(const std::string&, int) { return nullptr; }

	//////////////////////////////////////////////////////////////////////
	//������� �μ� �����ϱ�. �μ��� �����Ѵ�.
	TEST(Expectations, IgnoringUninterestingArguments)
	{
		MockFoo foo;
		EXPECT_CALL(foo, SetBar).Times(2);
		foo.SetBar(1);
		foo.SetBar(2);

		FooFactory foo_factory;
		ON_CALL(foo_factory, MakeFoo).WillByDefault(&BuildFooForTest);
	}

	class MockBar
	{
	public:
		MOCK_METHOD(void, B, ());
		MOCK_METHOD(void, C, ());
	};

	//////////////////////////////////////////////////////////////////////
	//�κ��� ������� ȣ���� ���
	TEST(Expectations, PartiallyOrderedCalls)
	{
		MockFoo foo;
		MockBar bar;
		Sequence s1, s2;
		
        //           +--->B
        //            |
        //    A--- |
        //            |
        //           +--->C--->D
		//A->B, C(������ �������)->D ��, ABCD, ACBD, ACDB
		EXPECT_CALL(foo, A()).InSequence(s1, s2);
		EXPECT_CALL(bar, B()).InSequence(s1);
		EXPECT_CALL(bar, C()).InSequence(s2);
		EXPECT_CALL(foo, D()).InSequence(s2);
		foo.A();
		bar.C();
		foo.D();
		bar.B();
	}

	enum LogType
	{
		WARNING,
	};

	class MockLog
	{
	public:
		MOCK_METHOD(void, Log, (LogType, int error, const std::string strError));
	};

	//////////////////////////////////////////////////////////////////////
	//Expectation ��Ȱ��ȭ�� �����ϱ�
	TEST(Expectations, Retires)
	{
		MockLog log1;
		Sequence s1, s2;

		EXPECT_CALL(log1, Log(WARNING, _, "File too large."))      // #1
			.Times(AnyNumber())
			.InSequence(s1, s2);
		EXPECT_CALL(log1, Log(WARNING, _, "Data set is empty."))   // #2
			.InSequence(s1);
		EXPECT_CALL(log1, Log(WARNING, _, "User not found."))      // #3
			.InSequence(s2);

		//AnyNumber�ε��� �ұ��ϰ� # 2 �Ǵ� # 3�� ��ġ�ϸ� ��� # 1�� ��Ȱ����
		log1.Log(WARNING, 7, "File too large.");
		log1.Log(WARNING, 2, "Data set is empty.");
		log1.Log(WARNING, 4, "User not found.");
		//log1.Log(WARNING, 6, "File too large.");	//Failed

		MockLog log2;
		EXPECT_CALL(log2, Log(WARNING, _, _));                   // #4 
		EXPECT_CALL(log2, Log(WARNING, _, "File too large."));   // #5
		log2.Log(WARNING, 7, "File too large.");
		//log2.Log(WARNING, 7, "File too large.");	//Failed. #5�� �ѹ��� �� ������ ���� �ߴµ� 2��ȣ��
		log2.Log(WARNING, 4, "Unexpected");

		MockLog log3;
		EXPECT_CALL(log3, Log(WARNING, _, _));                   // #6
		EXPECT_CALL(log3, Log(WARNING, _, "File too large."))	//#7
			.RetiresOnSaturation();
		log3.Log(WARNING, 7, "File too large.");	//#7 ��ġ. ��ġ ���ڸ��� Retire �Ǿ ��� ��Ȱ��ȭ��
		log3.Log(WARNING, 7, "File too large.");	//#6 ��ġ. ok. 
		
	}
	
	//////////////////////////////////////////////////////////////////////
	// ��ȯ ������ ������ ��� ReturnRef�� ���
	TEST(Action, ReturnRef)
	{
		MockFoo foo;
		Bar bar(2);

		EXPECT_CALL(foo, GetBar())
			.WillOnce(ReturnRef(bar));
		
		Bar& nBar = foo.GetBar();
		EXPECT_EQ(nBar, bar);
	}

	//////////////////////////////////////////////////////////////////////
	// ��ȯ ������ ����ִ�(Action�� ����� ������ ��) ���� ��ȯ�Ҷ�
	// ��ȯ ������ ������ ��쿡�� ReturnRef�� ����ϸ� ������ int ���� ��쿡�� ReturnPointee
	TEST(Action, ReturnLiveValues)
	{
		int x = 0;
		MockFoo foo;
		EXPECT_CALL(foo, GetValue())
			.WillRepeatedly(ReturnPointee(&x));
		x = 42;

		EXPECT_EQ(foo.GetValue(), 42);
	}
	
	//////////////////////////////////////////////////////////////////////
	// �������� Action�� ����. 
	// SaveArg<0>�� ù��° <1>�� �ι�° Argument�� ����.
	TEST(Action, Combining)
	{
		MockFoo foo;
		int arg0{ 0 };
		EXPECT_CALL(foo, IsBar(_))
			.WillOnce(DoAll(SaveArg<0>(&arg0), Return(true)));

		auto result1 = foo.IsBar(2);

		EXPECT_EQ(result1, true);
		EXPECT_EQ(arg0, 2);
	}
	
	//////////////////////////////////////////////////////////////////////
	// ������ �μ� Ȯ��
	// �μ��� ���Ƽ� �����Ҷ����� SaveArg�� �����ؼ� ���ϴ� ����� ����Ѵ�.
	TEST(Action, ComplexArgument)
	{
		MockFoo foo;
		std::vector<int> actual_array;
		std::map<std::string, int> actual_map;
		EXPECT_CALL(foo, SendValues)
			.WillOnce(DoAll(SaveArg<1>(&actual_array), SaveArg<2>(&actual_map)));

		foo.SendValues(7, { 1, 4, 4, 7 }, { {"a", 1}, {"b", 2} });

		EXPECT_THAT(actual_array, ElementsAre(1, 4, 4, 7));
		EXPECT_THAT(actual_map, UnorderedElementsAre(Pair("a", 1), Pair("b", 2)));
	}

	class Mutator
	{
	public:
		virtual ~Mutator() {}
		virtual void Mutate(bool mutate, int* value) = 0;
		virtual bool MutateInt(int* value) = 0;
		virtual void MutateArray(int* values, int num_values) = 0;
		virtual void GetNames(std::back_insert_iterator<std::vector<std::string>>) = 0;
	};

	class MockMutator : public Mutator 
	{
	public:
		MOCK_METHOD(void, Mutate, (bool mutate, int* value), (override));
		MOCK_METHOD(bool, MutateInt, (int* value), (override));
		MOCK_METHOD(void, MutateArray, (int* values, int num_values), (override));
		MOCK_METHOD(void, GetNames, (std::back_insert_iterator<std::vector<std::string>>), (override));
	};

	//////////////////////////////////////////////////////////////////////
	// Side Effects Ȱ���(���ۿ� �ƴ�)
	// Side Effect��? ���� �߿� � ��ü�� �����ؼ� ��ȭ�� �Ͼ�� ����(���̺귯�� I / O, ��ü ���� ��)
	// ex) x = 3 + 4; ���� ǥ������ 1���� side effect�� ����
	// y = x++; ���� ǥ������ �� 2���� side effect�� ����
	TEST(Action, SideEffects)
	{
		MockMutator mutator;
		EXPECT_CALL(mutator, Mutate(true, _))
			.WillOnce(SetArgPointee<1>(5));

		int outValue{ 0 };
		mutator.Mutate(true, &outValue);
		EXPECT_EQ(outValue, 5);

		EXPECT_CALL(mutator, MutateInt(_))
			.WillOnce(DoAll(SetArgPointee<0>(5),
				Return(true)));

		int outMutateInt{ 0 };
		auto result = mutator.MutateInt(&outMutateInt);
		EXPECT_EQ(outMutateInt, 5);
		EXPECT_TRUE(result);

		int values[5] = { 1, 2, 3, 4, 5 };
		EXPECT_CALL(mutator, MutateArray(NotNull(), 5))
			.WillOnce(SetArrayArgument<0>(values, values + 3));

		//�����ϰ� �Ǹ� SetArrayArgument<0>(values, values + 3)�� ���� { 1, 2, 3, 10, 12 }�� ���� �ٲ��(Side Effect)
		int real[5] = { 7, 8, 9, 10, 12 };	
		mutator.MutateArray(real, 5);
		EXPECT_THAT(real, ElementsAre(1, 2, 3, 10, 12));
		
		//�̰��� �μ��� ��� �ݺ��� �� ��쿡�� �۵���.
		std::vector<std::string> names = { "George", "John", "Thomas" };
		EXPECT_CALL(mutator, GetNames(_))
			.WillOnce(SetArrayArgument<0>(names.begin(), names.end()));

		std::vector<std::string> outName;
		mutator.GetNames(std::back_insert_iterator(outName));
		EXPECT_THAT(outName, ElementsAre("George", "John", "Thomas"));
	}
}