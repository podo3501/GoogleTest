# gMock (Google Mock) 테스트 개요
gMock은 Google Test를 기반으로 한 C++용 Mock 프레임워크로, 실제 객체 대신 가짜(Mock) 객체를 생성해 외부 의존성을 제어하며 테스트할 수 있습니다.  
하지만 gMock에 대한 한글 번역 자료나 실제 예제 코드가 부족해 학습과 적용에 어려움이 존재합니다.  
이에 본 프로젝트는 직접 프로젝트를 구성하고 검증을 진행하여, 학습 및 교육용 자료로도 활용할 수 있는 형태로 제작되었습니다.  
일반 단위 테스트와 Mock 테스트를 모두 지원하며, TDD(Test-Driven Development) 환경에서 매우 유용하게 활용됩니다.

---

## 일반 단위 테스트 (Unit Test)
- 함수의 출력값이나 내부 상태 변화를 검증합니다.
- 입력값에 대한 기대 결과를 확인하며, 순수 함수나 내부 로직 중심 코드에 적합합니다.
- 테스트 실패 시, 예상한 결과와 실제 결과가 다를 때 발생합니다.

---

## gMock 테스트 (Mock Test)
- 외부 객체와의 상호작용을 검증합니다.
- 특정 메서드가 호출되었는지, 몇 번 호출되었는지, 어떤 인자를 받았는지를 판단합니다.
- 로깅, 네트워크 요청, DB 저장 등 실제 실행을 피하면서 행동 기반 테스트가 가능합니다.
- 테스트 실패 시, 기대한 행동이 발생하지 않으면 실패합니다.

---

## 일반 테스트 vs gMock 테스트

| 구분 | 초점 | 검증 기준 |
|------|------|-----------|
| 일반 테스트 | 출력값, 내부 상태 | 예상한 결과와 실제 결과 비교 |
| gMock 테스트 | 외부 객체 호출, 상호작용 | 특정 메서드 호출 여부, 호출 횟수, 인자 확인 |

**핵심 요약:**  
- 일반 테스트: “무엇을 리턴했는가?”  
- gMock 테스트: “무엇을 어떻게 호출했는가?”  
- 두 가지 테스트를 병행하면 코드 안정성과 외부 의존성 검증을 동시에 달성할 수 있습니다.

---

## 참고 자료
- [gMock for Dummies](https://google.github.io/googletest/gmock_for_dummies.html)  
- [gMock Cookbook](https://google.github.io/googletest/gmock_cook_book.html)

## Google Mock 사용 방법 간단정리

- **gtest**: 함수/클래스의 **결과(상태/출력)** 를 검증  
- **gmock**: 의존성과의 **협력 계약(콜라보레이션 컨트랙트)** 을 검증

### gmock이 상호작용을 검증하는 방법
1) **호출 대상**: 어떤 인터페이스/메서드가 호출되어야 하는가  
2) **호출 횟수**: 정확히 몇 번(또는 최소/최대 몇 번) 호출되는가  
3) **인자 유효성**: 인자의 값/범위/패턴/상태가 맞는가 (`Eq`, `Ge`, `StrNe`, 커스텀 매처 등)  
4) **호출 순서**: 호출 간 **순서 제약**을 만족하는가 (`InSequence`, `After`)  
5) **반환/에러 경로**: 협력자가 무엇을 반환/던지는지에 따라 호출 측이 적절히 반응하는가 (`WillOnce(Return/Throw)`)  
6) **부수효과**: 콜백/출력 파라미터/상태 변경이 의도대로 발생하는가 (`DoAll`, `SetArgPointee`, `Invoke`)  
7) **금지 호출**: 호출하면 **안 되는** 동작이 없는가 (`Times(0)`, `StrictMock`)  

> 한 줄 요약: gmock은 *“무엇을, 몇 번, 어떤 값으로, 어떤 순서로, 무엇을 반환/변경하며, 하면 안 되는 건 하지 않았는가”* 를 계약으로 고정하고 검증합니다.

### 최소 예시
```cpp
struct IRepository {
    virtual ~IRepository() = default;
    virtual bool Save(int id, const std::string& name) = 0;
    virtual void Update(int id, const std::string& name) = 0;
};

class MockRepository : public IRepository {
public:
    MOCK_METHOD(bool, Save, (int, const std::string&), (override));
    MOCK_METHOD(void, Update, (int, const std::string&), (override));
};

class UserService {
    IRepository& repo_;
public:
    explicit UserService(IRepository& r) : repo_(r) {}
    bool Register(int id, const std::string& name) {
        if (id <= 0 || name.empty()) return false;
        return repo_.Save(id, name);
    }
};

TEST(UserServiceTest, Register_CallsSaveOnceWithValidArgs_NoUpdate) {
    StrictMock<MockRepository> repo;
    UserService svc{repo};

    ::testing::InSequence seq; // 순서 제약이 필요할 때 사용

    EXPECT_CALL(repo, Save(::testing::Ge(1), ::testing::StrNe("")))
        .Times(1)
        .WillOnce(::testing::Return(true));

    EXPECT_CALL(repo, Update(::testing::_, ::testing::_))
        .Times(0); // 금지 호출

    EXPECT_TRUE(svc.Register(42, "Alice"));
}

