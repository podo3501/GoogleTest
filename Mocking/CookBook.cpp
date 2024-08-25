#include "CookBook.h"

//�Ϻη� Uninteresting Call �߻���Ų��. visual studio�� gui��� ���װ� �ִ�. ��¿� '�׽�Ʈ' ���� ����.
//EXPECT_CALL�� �޼ҵ带 �������� �ʰ� ȣ���� ��쿡�� uninteresting call ��� ���
//���߿� �޼ҵ尡 �߰��Ǿ� �װ��� �׽�Ʈ�� ���з� ����� �ֱ� ������ ���߿� ������� �Լ��� ���� '���'�� �ʿ��ϴ�
TEST(WarningUninterestingCall, Test)
{
	MockFoo mock_foo;
	EXPECT_CALL(mock_foo, DoThis());

	mock_foo.DoThis();
	mock_foo.Add(1); //uninteresting call �߻�
}

//�׽�Ʈ�� EXPECT_CALL�� ����ϰ� NiceMock�� StrictMock�� ������ �������� ����� ��.
//NickMock�� ����ϸ� Uninteresting Call�� �߻����� �ʴ´�.
TEST(UsingNiceMock, Test)
{
	testing::NiceMock<MockFoo> niceMock_foo;
	EXPECT_CALL(niceMock_foo, DoThis());

	niceMock_foo.DoThis();
	niceMock_foo.Add(1); 
}

//StrictMock�� ����ϸ� Uninteresting Call�� ���з� �����Ѵ�.
TEST(StrictMock, Test)
{
	testing::StrictMock<MockFoo> strictMock_foo;
	EXPECT_CALL(strictMock_foo, DoThis());

	strictMock_foo.DoThis();
	//strictMock_foo.Add(1);	//�ּ��� Ǯ�� Test�� ������.
}