#include "Introduction.h"
#include <ranges>

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

namespace Introduction
{
	using ::testing::AtLeast;
	using ::testing::Return;
	using ::testing::_;
	using ::testing::Ge;
	using ::testing::InSequence;
	TEST_F(FixtureUsingMock, PaainterTest)
	{
		EXPECT_CALL(m_turtle, PenDown())
			.Times(AtLeast(1));		//��� 1���� ȣ���� �ؾ� �Ѵ�.

		Painter  painter(&m_turtle);

		EXPECT_TRUE(painter.DrawCircle(0, 0, 10));		//DrawCircle �Լ� �ȿ��� PenDown�Լ��� ȣ���
	}

	TEST_F(FixtureUsingMock, Expectation)
	{
		EXPECT_CALL(m_turtle, GetX())	//EXPECT_CALL�� �׽�Ʈ �ۼ��� '����' �Ѵ�. '����' �ϴ� ���̱� ������.
			.Times(4)	//4�� call�� �ؾ� �Ѵ�.
			.WillOnce(Return(100))	//ù��°�� 100�� ����
			.WillOnce(Return(150))	//�ι�°�� 150�� ����
			.WillRepeatedly(Return(200));	//��� 200�� ����

		EXPECT_EQ(m_turtle.GetX(), 100);
		EXPECT_EQ(m_turtle.GetX(), 150);
		EXPECT_EQ(m_turtle.GetX(), 200);
		EXPECT_EQ(m_turtle.GetX(), 200);

		//Times�� �����ȴٸ� WillOnce�� WillRepeatedly�� ���� cardinality(��� ȣ��-Times(n))�� ������
		EXPECT_CALL(m_turtle, GetY());		//�Ѵ� ���ٸ� Times(1)
		m_turtle.GetY();

		EXPECT_CALL(m_turtle, GetY())	//WillOnce�� n�� �ִٸ� Times(n)
			.WillOnce(Return(100))
			.WillOnce(Return(200));
		EXPECT_EQ(m_turtle.GetY(), 100);
		EXPECT_EQ(m_turtle.GetY(), 200);

		EXPECT_CALL(m_turtle, GetY())	//WillOnce�� n�� WillRepeatedly�� �ϳ� ������ Times(Atleast(n)) ��� n��
			.WillOnce(Return(200))
			.WillOnce(Return(300))
			.WillRepeatedly(Return(400));
		EXPECT_EQ(m_turtle.GetY(), 200);
		EXPECT_EQ(m_turtle.GetY(), 300);
		EXPECT_EQ(m_turtle.GetY(), 400);

		// turtle�� 100��ŭ ������ �� ���� ����(�׽�Ʈ �ڵ带 '�����ؼ� ����' �ۼ��Ѵ�)
		EXPECT_CALL(m_turtle, Forward(100));
		m_turtle.Forward(100);

		// turtle�� ������ ������ ��(_)��ŭ ������ ����
		EXPECT_CALL(m_turtle, Forward(_));
		m_turtle.Forward(17);

		// turtle�� Ge(Greater Equal) ��, 100 �̻�ŭ �� ���� ����
		EXPECT_CALL(m_turtle, Forward(Ge(100)));
		m_turtle.Forward(105);

		//�ΰ��� EXPECT_CALL�� �ִ� ��쿡�� �ؿ������� �´��� Ȯ���ϱ� ������ ���ʿ��� ���� �Ϲ����� ������ �ϰ�,
		//������ ������ ��ü���� ��Ȳ�� �ۼ���
		EXPECT_CALL(m_turtle, Forward(_));	//���� ������� Times(1)
		EXPECT_CALL(m_turtle, Forward(10))	//10�� ������ 2��
			.Times(2);

		m_turtle.Forward(10);	//ok
		m_turtle.Forward(10);	//ok
		m_turtle.Forward(20);	// Forward(_)���忡�� �ƹ� �μ��� �����ϴϱ� ok
	}

	TEST_F(FixtureUsingMock, InSequence)
	{
		auto Foo = [](Turtle& turtle) {
			turtle.PenDown();
			turtle.Forward(100);
			turtle.Forward(250);
			turtle.PenUp(); };

		{
			//�Լ��� ȣ�� ������ �ٲ�� ������ �������ش�.
			InSequence dummy;

			EXPECT_CALL(m_turtle, PenDown());
			EXPECT_CALL(m_turtle, Forward(_)).Times(AtLeast(1));
			EXPECT_CALL(m_turtle, PenUp());
		}
		Foo(m_turtle);
	}

	TEST_F(FixtureUsingMock, StickyExpectiation)
	{
		{
			InSequence s;

			//�Լ��� ���� �پ��ϰ� ���ϵȴٰ� �Ѵٸ� ó���� Return(30), �״����ٿ� Return(20) �̷��� �ؾ��ϴµ�
			//���������� �ʴ�. �׷������� InSequence �� ����ؼ� ������ �ٷ� ���߾� �ټ� �ִ�.
			for (auto i : std::views::iota(1, 4))
			{
				//RetiresOnSaturation �׸��� ���ٸ� �������� WillOnce(...)->Times(1)�� ���� �ʱ� ������ false�� ��ȯ
				//��, ������ �����Ѵٸ� Retires �Ǿ� �� �׸��� �򰡵��� ����. �׷��� ������ �ٸ� WillOnceüũ�� �Ҽ� ����
				EXPECT_CALL(m_turtle, GetX())
					.WillOnce(Return(10 * i))
					.RetiresOnSaturation();
			}
		}
		EXPECT_EQ(m_turtle.GetX(), 10);
		EXPECT_EQ(m_turtle.GetX(), 20);
		EXPECT_EQ(m_turtle.GetX(), 30);
	}
}
