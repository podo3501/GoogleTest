#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <ranges>

//mock �Թ�
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
		. Times(AtLeast(1));		//��� 1���� ȣ���� �ؾ� �Ѵ�.

	Painter  painter(&turtle);

	EXPECT_TRUE(painter.DrawCircle(0, 0, 10));		//DrawCircle �Լ� �ȿ��� PenDown�Լ��� ȣ���
}

TEST(Expectation, Test)
{
	using ::testing::Return;
	using  ::testing::_;
	using  ::testing::Ge;

	MockTurtle  turtle;
	EXPECT_CALL(turtle, GetX())	//EXPECT_CALL�� �׽�Ʈ �ۼ��� '����' �Ѵ�. '����' �ϴ� ���̱� ������.
		.Times(4)	//4�� call�� �ؾ� �Ѵ�.
		.WillOnce(Return(100))	//ù��°�� 100�� ����
		.WillOnce(Return(150))	//�ι�°�� 150�� ����
		.WillRepeatedly(Return(200));	//��� 200�� ����

	EXPECT_EQ(turtle.GetX(), 100);
	EXPECT_EQ(turtle.GetX(), 150);
	EXPECT_EQ(turtle.GetX(), 200);
	EXPECT_EQ(turtle.GetX(), 200);

	//Times�� �����ȴٸ� WillOnce�� WillRepeatedly�� ���� cardinality(��� ȣ��-Times(n))�� ������
	EXPECT_CALL(turtle, GetY());		//�Ѵ� ���ٸ� Times(1)
	turtle.GetY();

	EXPECT_CALL(turtle, GetY())	//WillOnce�� n�� �ִٸ� Times(n)
		.WillOnce(Return(100))
		.WillOnce(Return(200));
	EXPECT_EQ(turtle.GetY(), 100);
	EXPECT_EQ(turtle.GetY(), 200);

	EXPECT_CALL(turtle, GetY())	//WillOnce�� n�� WillRepeatedly�� �ϳ� ������ Times(Atleast(n)) ��� n��
		.WillOnce(Return(200))
		.WillOnce(Return(300))
		.WillRepeatedly(Return(400));
	EXPECT_EQ(turtle.GetY(), 200);
	EXPECT_EQ(turtle.GetY(), 300);
	EXPECT_EQ(turtle.GetY(), 400);

	// turtle�� 100��ŭ ������ �� ���� ����(�׽�Ʈ �ڵ带 '�����ؼ� ����' �ۼ��Ѵ�)
	EXPECT_CALL(turtle, Forward(100));
	turtle.Forward(100);

	// turtle�� ������ ������ ��(_)��ŭ ������ ����
	EXPECT_CALL(turtle, Forward(_));
	turtle.Forward(17);

	// turtle�� Ge(Greater Equal) ��, 100 �̻�ŭ �� ���� ����
	EXPECT_CALL(turtle, Forward(Ge(100)));
	turtle.Forward(105);

	//�ΰ��� EXPECT_CALL�� �ִ� ��쿡�� �ؿ������� �´��� Ȯ���ϱ� ������ ���ʿ��� ���� �Ϲ����� ������ �ϰ�,
	//������ ������ ��ü���� ��Ȳ�� �ۼ���
	EXPECT_CALL(turtle, Forward(_));	//���� ������� Times(1)
	EXPECT_CALL(turtle, Forward(10))	//10�� ������ 2��
		.Times(2);

	turtle.Forward(10);	//ok
	turtle.Forward(10);	//ok
	turtle.Forward(20);	//EXPECT_CALL(turtle, Forward(10)).Times(2); ���⼭�� ���������� ���� EXPECT���忡�� ok
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
		//�Լ��� ȣ�� ������ �ٲ�� ������ �������ش�.
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

		//�Լ��� ���� �پ��ϰ� ���ϵȴٰ� �Ѵٸ� ó���� Return(30), �״����ٿ� Return(20) �̷��� �ؾ��ϴµ�
		//���������� �ʴ�. �׷������� InSequence �� ����ؼ� ������ �ٷ� ���߾� �ټ� �ִ�.
		for(auto i : std::views::iota(1, 4))
		{
			//RetiresOnSaturation �׸��� ���ٸ� �������� WillOnce(...)->Times(1)�� ���� �ʱ� ������ false�� ��ȯ
			//��, ������ �����Ѵٸ� Retires �Ǿ� �� �׸��� �򰡵��� ����. �׷��� ������ �ٸ� WillOnceüũ�� �Ҽ� ����
			EXPECT_CALL(turtle, GetX())
				.WillOnce(Return(10 * i))
				.RetiresOnSaturation();	
		}
	}
	EXPECT_EQ(turtle.GetX(), 10);
	EXPECT_EQ(turtle.GetX(), 20);
	EXPECT_EQ(turtle.GetX(), 30);
}
