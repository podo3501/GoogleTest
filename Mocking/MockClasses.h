#pragma once
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class Gadget;

class Foo1
{
public:
	virtual bool Transform(Gadget* g) = 0;

protected:
	virtual void Resume();

private:
	virtual int GetTimeOut();
};

class  MockFoo1 : public  Foo1
{
public:
	MOCK_METHOD1(Transform, bool(Gadget* g));

	// 기본 클래스의 메소드가 protected 와 private 에서도 
	// 이하는 반드시 public 으로 해 주세요. 
	MOCK_METHOD0(Resume, void());
	MOCK_METHOD0(GetTimeOut, int());
};

/////////////////////////////////////////////////////////////////////////

class Bar;
using Element = int;

class Foo2 
{
public:
	// Foo2를 상속하기 때문에 이것은 반드시 virtual. 
	virtual ~Foo2();

	// 인수의 유형과 수를 변경하여 오버로드됩니다. 
	virtual int Add(Element x);
	virtual int Add(int times, Element x);

	// 객체의 const 없이 오버로드됩니다. 
	virtual Bar& GetBar();
	virtual const Bar& GetBar() const;
};

class MockFoo2 : public Foo2 
{
public:
	MOCK_METHOD1(Add, int(Element x));
	MOCK_METHOD2(Add, int(int times, Element x));

	MOCK_METHOD0(GetBar, Bar& ());
	MOCK_CONST_METHOD0(GetBar, const Bar& ());
};

/////////////////////////////////////////////////////////////////////////
//template클래스를 Mocking 하는 법(뒤에 _T를 붙인다)

template<typename Elem>
class StackInterface
{
public:
	virtual ~StackInterface();

	virtual int GetSize() const = 0;
	virtual void Push(const Elem& x) = 0;
};

template<typename Elem>
class MockStack : public StackInterface<Elem>
{
public:
	MOCK_CONST_METHOD0_T(GetSize, int());
	MOCK_METHOD1_T(Push, void(const Elem& x));
};

/////////////////////////////////////////////////////////////////////////
//ConcretePacketStream과 MockPacketStream은 다른 클래스이기 때문에 작성자가 '알고' 있어야 한다.
//컴파일 타임에 두 클래스가 관련이 있다는 걸 알게 하기 위해서는 template를 써서 작성하는 방법이 있다.
class Packet;

class ConcretePacketStream
{
public:
	void AppendPacket(Packet* new_packet);
	const Packet* GetPacket(std::size_t packet_number);
	std::size_t NumberOfPackets() const;
};

class MockPacketStream
{
public:
	MOCK_CONST_METHOD1(GetPacket, const Packet* (std::size_t packet_number));
	MOCK_CONST_METHOD0(NumberOfPackets, std::size_t());
};

/////////////////////////////////////////////////////////////////////////
//자유 함수(c++에서 제공하는 함수)를 Mock하는 방법
//include 어딘가에 OpenFile이라는 함수가 있다고 가정하자.
bool OpenFile(const char* path, const char* mode)
{
	return true;
}

class FileInterface
{
public:
	virtual bool Open(const char* path, const char* mode) = 0;
};

class File : public FileInterface
{
public:
	virtual bool Open(const char* path, const char* mode)
	{
		return OpenFile(path, mode);
	}
};