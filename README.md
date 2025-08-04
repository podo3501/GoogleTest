구글 테스트인 gMock의 예시들을 c++을 사용해 실제로 어떻게 작동하는지 테스트 한 코드.
gmock 안에는 일반 테스트와 mock 테스트 둘다 들어가 있다.
일반적으로 tdd 라고 하면 일반 테스트를 의미하며, 서버같이 다른 라이브러리나 코드에 의존적일때(ex. db) 그 것 자체를 돌릴 수 없기 때문에 mock으로 가짜 객체를 만들어서
테스트 할 수 있게끔 한다.

항목	           일반 단위 테스트 (Google Test)	                               GMock (Google Mock)
목적          함수의 **결과(출력)**를 검증	                                객체 간 **상호작용(행동)**을 검증
검증          대상 리턴값, 상태 변화, 부작용 등	                            특정 함수가 호출되었는지, 몇 번 호출되었는지 등
예상방식      EXPECT_EQ, ASSERT_TRUE 등으로 값 기반 비교	                  EXPECT_CALL로 행동 기반 기대 설정
사용시점	     테스트 대상이 외부 의존성을 직접 포함하지 않을 때	              테스트 대상이 **외부 의존성(interface)**을 가질 때
종속성처리	   실제 객체를 사용하거나 hand-written stub	                      Mock 객체로 외부 객체를 대체
결함탐지	     결과가 다를 경우 테스트 실패	                                  호출되지 않거나, 잘못 호출되면 테스트 실패
강점	       알고리즘, 내부 로직, 계산 결과 등 순수 함수 테스트에 강함	      콜백, 이벤트, 의존 객체와의 프로토콜 검증에 강함
복잡도	       상대적으로 단순	                                              설정과 제어가 많지만 복잡한 테스트 가능
대표구문	     EXPECT_EQ(func(x), expectedValue);	                          EXPECT_CALL(mock, Func(Eq(x))).Times(1);


gMock for Dummies
https://google.github.io/googletest/gmock_for_dummies.html

gMock Cookbook
https://google.github.io/googletest/gmock_cook_book.html
