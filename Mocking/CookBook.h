#pragma once
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace CookBook
{
	class Bar;
	using Element = int;

	class Foo
	{
	public:
		virtual ~Foo() {};
		virtual int Add(Element x) = 0;
		virtual int Add(int times, Element x) = 0;

		virtual Bar& GetBar() = 0;
		virtual const Bar& GetBar() const = 0;

		virtual void DoThis() = 0;
	};

	class MockFoo : public Foo
	{
	public:
		MOCK_METHOD1(Add, int(Element x));
		MOCK_METHOD2(Add, int(int times, Element x));

		MOCK_METHOD0(GetBar, Bar& ());
		MOCK_CONST_METHOD0(GetBar, const Bar& ());

		MOCK_METHOD0(DoThis, void());
	};
}