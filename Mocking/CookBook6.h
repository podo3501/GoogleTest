#pragma once
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <string>

namespace CookBook6
{
	class Foo
	{
	public:
		virtual ~Foo() = default;

		virtual int DoThis(bool flag, int n) = 0;
		virtual std::string DoThat(int x, const char* str1, const char* str2) = 0;
	};

	class MockFoo : public Foo
	{
	public:
		virtual ~MockFoo() = default;
		MOCK_METHOD(void, Bar, (int*));
		MOCK_METHOD(void, Cardinality, (int n));
		MOCK_METHOD(int, Multiply, (int n));
		MOCK_METHOD(double, DistanceTo, (double x, double y));
		MOCK_METHOD(void, ActionTemplateTest, (int a, int b, int* n));
		MOCK_METHOD(int, Baz, (int* n));

		MOCK_METHOD(int, DoThis, (bool flag, int n), (override));
		MOCK_METHOD(std::string, DoThat, (int x, const char* str1, const char* str2), (override));
	};
}