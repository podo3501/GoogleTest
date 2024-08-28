#include "CookBook3.h"
#include <memory>

namespace CookBook3
{
	using ::testing::_;
	using ::testing::InSequence;	//순차적으로
	using ::testing::Sequence;	//부분적인 순차
	using ::testing::AnyNumber;
	using ::testing::ReturnRef;	//리턴값을 참조로 반환
	using ::testing::ReturnPointee; //일반적인 리턴값을 반환할때
	using ::testing::DoAll;	//여러개의 Action을 실행하도록 가능하게 해 준다.
	using ::testing::Return;	//리턴
	using ::testing::SaveArg;	//Argument를 저장한다. ex) SaveArg<0>(&a) 첫번째 Argument를 a에 저장
	using ::testing::ElementsAre;
	using ::testing::UnorderedElementsAre;
	using ::testing::Pair;
	using ::testing::SetArgPointee;	//Argument Pointer에 값을 넣는다.
	using ::testing::SetArrayArgument;	//Array Argument 에 값을 넣는다.
	using ::testing::NotNull;

	class FooFactory
	{
	public:
		MOCK_METHOD(std::unique_ptr<Foo>, MakeFoo, (const std::string&, int));
	};

	std::unique_ptr<Foo> BuildFooForTest(const std::string&, int) { return nullptr; }

	//////////////////////////////////////////////////////////////////////
	//상관없는 인수 무시하기. 인수를 생략한다.
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
	//부분적 순서대로 호출을 기대
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
		//A->B, C(순서에 상관없음)->D 즉, ABCD, ACBD, ACDB
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
	//Expectation 비활성화를 제어하기
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

		//AnyNumber인데도 불구하고 # 2 또는 # 3과 일치하면 즉시 # 1이 비활성됨
		log1.Log(WARNING, 7, "File too large.");
		log1.Log(WARNING, 2, "Data set is empty.");
		log1.Log(WARNING, 4, "User not found.");
		//log1.Log(WARNING, 6, "File too large.");	//Failed

		MockLog log2;
		EXPECT_CALL(log2, Log(WARNING, _, _));                   // #4 
		EXPECT_CALL(log2, Log(WARNING, _, "File too large."));   // #5
		log2.Log(WARNING, 7, "File too large.");
		//log2.Log(WARNING, 7, "File too large.");	//Failed. #5가 한번만 될 것임을 예상 했는데 2번호출
		log2.Log(WARNING, 4, "Unexpected");

		MockLog log3;
		EXPECT_CALL(log3, Log(WARNING, _, _));                   // #6
		EXPECT_CALL(log3, Log(WARNING, _, "File too large."))	//#7
			.RetiresOnSaturation();
		log3.Log(WARNING, 7, "File too large.");	//#7 일치. 일치 하자마자 Retire 되어서 즉시 비활성화됨
		log3.Log(WARNING, 7, "File too large.");	//#6 일치. ok. 
		
	}
	
	//////////////////////////////////////////////////////////////////////
	// 반환 유형이 참조일 경우 ReturnRef를 사용
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
	// 반환 유형이 살아있는(Action이 실행된 시점의 값) 값을 반환할때
	// 반환 유형이 참조일 경우에는 ReturnRef를 사용하면 되지만 int 같은 경우에는 ReturnPointee
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
	// 복수개의 Action을 실행. 
	// SaveArg<0>은 첫번째 <1>은 두번째 Argument를 뜻함.
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
	// 복잡한 인수 확인
	// 인수가 많아서 복잡할때에는 SaveArg로 저장해서 비교하는 방법을 사용한다.
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
	// Side Effects 활용법(부작용 아님)
	// Side Effect란? 실행 중에 어떤 객체를 접근해서 변화가 일어나는 행위(라이브러리 I / O, 객체 변경 등)
	// ex) x = 3 + 4; 위의 표현식은 1개의 side effect가 있음
	// y = x++; 위의 표현식은 총 2개의 side effect가 있음
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

		//실행하게 되면 SetArrayArgument<0>(values, values + 3)에 의해 { 1, 2, 3, 10, 12 }로 값이 바뀐다(Side Effect)
		int real[5] = { 7, 8, 9, 10, 12 };	
		mutator.MutateArray(real, 5);
		EXPECT_THAT(real, ElementsAre(1, 2, 3, 10, 12));
		
		//이것은 인수가 출력 반복자 인 경우에도 작동함.
		std::vector<std::string> names = { "George", "John", "Thomas" };
		EXPECT_CALL(mutator, GetNames(_))
			.WillOnce(SetArrayArgument<0>(names.begin(), names.end()));

		std::vector<std::string> outName;
		mutator.GetNames(std::back_insert_iterator(outName));
		EXPECT_THAT(outName, ElementsAre("George", "John", "Thomas"));
	}
}