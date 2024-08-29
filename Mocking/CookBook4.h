#pragma once
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <functional>
#include <string>
#include <map>

namespace CookBook4
{
	class Bar
	{
	public:
		Bar(int n) : m_n{ n } {}
		bool operator==(const Bar& rhs) const { return (m_n == rhs.m_n); }

	private:
		int m_n;
	};

	class Helper
	{};

	class MyData
	{};
	
	class Foo
	{
	public:
		virtual ~Foo() {}
		virtual Bar CalculateBar() = 0;
		virtual int Sign(int n) = 0;

		virtual int Sum(int x, int y) = 0;
		virtual bool ComplexJob(int x) = 0;
		virtual bool DoThis(int n, std::function<bool(int)> callback) = 0;
		virtual bool ArgFuncRef(std::function<bool(int, const Helper&)> callback) = 0;
		virtual bool DoThat(std::function<bool(const double& x, const std::string& s)> callback) = 0;

		virtual void Abc(const MyData& data) = 0;
		virtual bool Xyz() = 0;

		virtual double DistanceWithLabel(const std::string& label, double x, double y) = 0;
		virtual double DistanceWithIndex(int index, double x, double y) = 0;

		virtual bool ActionSharing_1(int* ref) = 0;
		virtual bool ActionSharing_2(int* ref) = 0;

		virtual int ActionSharingCounter_1() = 0;
		virtual int ActionSharingCounter_2() = 0;
	};

	class MockFoo : public Foo 
	{
	public:
		MOCK_METHOD(Bar, CalculateBar, (), (override));
		MOCK_METHOD(int, Sign, (int n), (override));

		MOCK_METHOD(int, Sum, (int x, int y), (override));
		MOCK_METHOD(bool, ComplexJob, (int x), (override));

		MOCK_METHOD(bool, DoThis, (int n, std::function<bool(int)> callback), (override));
		MOCK_METHOD(bool, ArgFuncRef, (std::function<bool(int, const Helper&)> callback), (override));
		MOCK_METHOD(bool, DoThat, (std::function<bool(const double& x, const std::string& s)> callback), (override));

		MOCK_METHOD(void, Abc, (const MyData& data), (override));
		MOCK_METHOD(bool, Xyz, (), (override));

		MOCK_METHOD(bool, SoManyArguments, (bool visible, const std::string& name, int x, int y,
				(const std::map<int, int>&), double weight, double min_weight, double max_wight));

		MOCK_METHOD(double, DistanceWithLabel, (const std::string& label, double x, double y), (override));
		MOCK_METHOD(double, DistanceWithIndex, (int index, double x, double y), (override));

		MOCK_METHOD(bool, ActionSharing_1, (int* ref), (override));
		MOCK_METHOD(bool, ActionSharing_2, (int* ref), (override));

		MOCK_METHOD(int, ActionSharingCounter_1, (), (override));
		MOCK_METHOD(int, ActionSharingCounter_2, (), (override));
	};
}