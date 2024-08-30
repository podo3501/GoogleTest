#pragma once
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace CookBook5
{
	class Foo
	{
	public:
		virtual ~Foo() = default;

		virtual int DoThis() = 0;
		virtual bool DoThat(const char* str) = 0;
	};

	class Bar
	{
	public:
		virtual ~Bar() = default;

		virtual void A() = 0;
		virtual void B() = 0;
	};

	class MockFoo : public Foo 
	{
	public:
		// The constructor and destructor are declared, but not defined, here.
		MockFoo();
		~MockFoo() override;// { Die(); }는 cpp에 정의됨. 컴파일 속도를 위해.

		MOCK_METHOD(int, DoThis, (), (override));
		MOCK_METHOD(bool, DoThat, (const char* str), (override));
		MOCK_METHOD(void, Die, ());
	};

	class MockBar : public Bar
	{
	public:
		MockBar();
		~MockBar() override;

		MOCK_METHOD(void, A, (), (override));
		MOCK_METHOD(void, B, (), (override));
	};
}

