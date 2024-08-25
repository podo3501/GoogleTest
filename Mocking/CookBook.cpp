#include "CookBook.h"

//일부러 Uninteresting Call 발생시킨다. visual studio의 gui경고에 버그가 있다. 출력에 '테스트' 탭을 보자.
//EXPECT_CALL에 메소드를 지정하지 않고 호출할 경우에는 uninteresting call 경고를 출력
//나중에 메소드가 추가되어 그것이 테스트를 실패로 만들수 있기 때문에 나중에 만들어진 함수에 대한 '경고'가 필요하다
TEST(WarningUninterestingCall, Test)
{
	MockFoo mock_foo;
	EXPECT_CALL(mock_foo, DoThis());

	mock_foo.DoThis();
	mock_foo.Add(1); //uninteresting call 발생
}

//테스트는 EXPECT_CALL로 사용하고 NiceMock과 StrictMock은 마지막 수단으로 사용할 것.
//NickMock을 사용하면 Uninteresting Call이 발생하지 않는다.
TEST(UsingNiceMock, Test)
{
	testing::NiceMock<MockFoo> niceMock_foo;
	EXPECT_CALL(niceMock_foo, DoThis());

	niceMock_foo.DoThis();
	niceMock_foo.Add(1); 
}

//StrictMock을 사용하면 Uninteresting Call은 실패로 간주한다.
TEST(StrictMock, Test)
{
	testing::StrictMock<MockFoo> strictMock_foo;
	EXPECT_CALL(strictMock_foo, DoThis());

	strictMock_foo.DoThis();
	//strictMock_foo.Add(1);	//주석을 풀면 Test가 실패함.
}