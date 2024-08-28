#pragma once
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <string>
#include <map>
#include <vector>

namespace CookBook3
{
	class Bar 
	{
	public:
		Bar(int n) : m_n{ n } {}
		bool operator==(const Bar& rhs) const
		{
			if (m_n != rhs.m_n) return false;
			return true;
		}

	private:
		int m_n;
	};

	class Foo
	{
	public:
		virtual ~Foo() {};
		virtual void SetBar(int n) = 0;
		virtual Bar& GetBar() = 0;
		virtual int GetValue() = 0;
		virtual bool IsBar(int n) = 0;
		virtual void SendValues(int n, const std::vector<int>& list1, const std::map<std::string, int>& list2) = 0;
	};

	class MockFoo : public Foo
	{
	public:
		MOCK_METHOD(void, SetBar, (int n), (override));
		MOCK_METHOD(void, A, ());
		MOCK_METHOD(void, D, ());
		MOCK_METHOD(Bar&, GetBar, (), (override));
		MOCK_METHOD(int, GetValue, (), (override));
		MOCK_METHOD(bool, IsBar, (int n), (override));
		//map�� std::string, int ���� ,�� ���� �Ľ̿� ������ �Ͼ�°� ���� �׷��� ��ȣ()�� �����־� �ذ��Ѵ�.
		MOCK_METHOD(void, SendValues, (int n, const std::vector<int>& list1, (const std::map<std::string, int>& list2)), (override));
	};
}