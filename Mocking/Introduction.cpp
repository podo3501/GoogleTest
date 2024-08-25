#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <ranges>

//mock 입문
class  Turtle {
public:
	virtual  ~Turtle() {}
	virtual  void  PenUp() = 0;
	virtual  void  PenDown() = 0;
	virtual  void  Forward(int  distance) = 0;
	virtual  void  Turn(int  degrees) = 0;
	virtual  void  GoTo(int  x, int  y) = 0;
	virtual  int  GetX()  const = 0;
	virtual  int  GetY()  const = 0;
};

class MockTurtle : public Turtle {
public:
	MOCK_METHOD0(PenUp, void());
	MOCK_METHOD0(PenDown, void());
	MOCK_METHOD1(Forward, void(int distance));
	MOCK_METHOD1(Turn, void(int degrees));
	MOCK_METHOD2(GoTo, void(int x, int y));
	MOCK_CONST_METHOD0(GetX, int());
	MOCK_CONST_METHOD0(GetY, int());
};

class Painter
{
public:
	Painter(Turtle* turtle) : m_turtle{ turtle }	{}

	bool DrawCircle(int x, int y, int z)
	{
		m_turtle->PenDown();
		return true;
	}

private:
	Turtle* m_turtle;
};

using  ::testing::AtLeast;                      
TEST(PainterTest, CanDrawSomething) {
	MockTurtle  turtle;
	EXPECT_CALL(turtle, PenDown())               
		. Times(AtLeast(1));		//적어도 1번은 호출을 해야 한다.

	Painter  painter(&turtle);

	EXPECT_TRUE(painter.DrawCircle(0, 0, 10));		//DrawCircle 함수 안에서 PenDown함수가 호출됨
}

TEST(Expectation, Test)
{
	using ::testing::Return;
	using  ::testing::_;
	using  ::testing::Ge;

	MockTurtle  turtle;
	EXPECT_CALL(turtle, GetX())	//EXPECT_CALL은 테스트 작성을 '먼저' 한다. '예상' 하는 것이기 때문에.
		.Times(4)	//4번 call을 해야 한다.
		.WillOnce(Return(100))	//첫번째는 100을 리턴
		.WillOnce(Return(150))	//두번째는 150을 리턴
		.WillRepeatedly(Return(200));	//계속 200을 리턴

	EXPECT_EQ(turtle.GetX(), 100);
	EXPECT_EQ(turtle.GetX(), 150);
	EXPECT_EQ(turtle.GetX(), 200);
	EXPECT_EQ(turtle.GetX(), 200);

	//Times가 생략된다면 WillOnce와 WillRepeatedly를 보고 cardinality(몇번 호출-Times(n))를 추정함
	EXPECT_CALL(turtle, GetY());		//둘다 없다면 Times(1)
	turtle.GetY();

	EXPECT_CALL(turtle, GetY())	//WillOnce가 n개 있다면 Times(n)
		.WillOnce(Return(100))
		.WillOnce(Return(200));
	EXPECT_EQ(turtle.GetY(), 100);
	EXPECT_EQ(turtle.GetY(), 200);

	EXPECT_CALL(turtle, GetY())	//WillOnce가 n개 WillRepeatedly가 하나 있으면 Times(Atleast(n)) 적어도 n번
		.WillOnce(Return(200))
		.WillOnce(Return(300))
		.WillRepeatedly(Return(400));
	EXPECT_EQ(turtle.GetY(), 200);
	EXPECT_EQ(turtle.GetY(), 300);
	EXPECT_EQ(turtle.GetY(), 400);

	// turtle이 100만큼 앞으로 갈 것을 예상(테스트 코드를 '예상해서 먼저' 작성한다)
	EXPECT_CALL(turtle, Forward(100));
	turtle.Forward(100);

	// turtle이 앞으로 임의의 값(_)만큼 갈것을 예상
	EXPECT_CALL(turtle, Forward(_));
	turtle.Forward(17);

	// turtle이 Ge(Greater Equal) 즉, 100 이상만큼 갈 것을 예상
	EXPECT_CALL(turtle, Forward(Ge(100)));
	turtle.Forward(105);

	//두가지 EXPECT_CALL이 있는 경우에는 밑에서부터 맞는지 확인하기 때문에 위쪽에는 보다 일반적인 것으로 하고,
	//밑으로 갈수록 구체적인 상황을 작성함
	EXPECT_CALL(turtle, Forward(_));	//값에 상관없이 Times(1)
	EXPECT_CALL(turtle, Forward(10))	//10의 값으로 2번
		.Times(2);

	turtle.Forward(10);	//ok
	turtle.Forward(10);	//ok
	turtle.Forward(20);	//EXPECT_CALL(turtle, Forward(10)).Times(2); 여기서는 문제이지만 위에 EXPECT문장에서 ok
}

TEST(InSequence, Test)
{
	auto Foo = [](Turtle& turtle) {
		turtle.PenDown();
		turtle.Forward(100);
		turtle.Forward(250);
		turtle.PenUp(); };

	MockTurtle  turtle;
	{
		//함수의 호출 순서가 바뀌면 오류를 감지해준다.
		testing::InSequence dummy;

		EXPECT_CALL(turtle, PenDown());
		EXPECT_CALL(turtle, Forward(testing::_)).Times(testing::AtLeast(1));
		EXPECT_CALL(turtle, PenUp());
	}
	Foo(turtle);
}

TEST(StickyExpectiation, Test)
{
	using ::testing::InSequence;
	using ::testing::Return;

	MockTurtle  turtle;
	{
		InSequence s;

		//함수의 값이 다양하게 리턴된다고 한다면 처음에 Return(30), 그다음줄에 Return(20) 이렇게 해야하는데
		//직관적이지 않다. 그럴때에는 InSequence 를 사용해서 순서를 바로 맞추어 줄수 있다.
		for(auto i : std::views::iota(1, 4))
		{
			//RetiresOnSaturation 항목이 없다면 마지막줄 WillOnce(...)->Times(1)에 맞지 않기 때문에 false를 반환
			//즉, 조건이 만족한다면 Retires 되어 그 항목은 평가되지 않음. 그렇기 때문에 다른 WillOnce체크를 할수 있음
			EXPECT_CALL(turtle, GetX())
				.WillOnce(Return(10 * i))
				.RetiresOnSaturation();	
		}
	}
	EXPECT_EQ(turtle.GetX(), 10);
	EXPECT_EQ(turtle.GetX(), 20);
	EXPECT_EQ(turtle.GetX(), 30);
}
