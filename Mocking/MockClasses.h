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

	// �⺻ Ŭ������ �޼ҵ尡 protected �� private ������ 
	// ���ϴ� �ݵ�� public ���� �� �ּ���. 
	MOCK_METHOD0(Resume, void());
	MOCK_METHOD0(GetTimeOut, int());
};

/////////////////////////////////////////////////////////////////////////

class Bar;
using Element = int;

class Foo2 
{
public:
	// Foo2�� ����ϱ� ������ �̰��� �ݵ�� virtual. 
	virtual ~Foo2();

	// �μ��� ������ ���� �����Ͽ� �����ε�˴ϴ�. 
	virtual int Add(Element x);
	virtual int Add(int times, Element x);

	// ��ü�� const ���� �����ε�˴ϴ�. 
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
//templateŬ������ Mocking �ϴ� ��(�ڿ� _T�� ���δ�)

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
//ConcretePacketStream�� MockPacketStream�� �ٸ� Ŭ�����̱� ������ �ۼ��ڰ� '�˰�' �־�� �Ѵ�.
//������ Ÿ�ӿ� �� Ŭ������ ������ �ִٴ� �� �˰� �ϱ� ���ؼ��� template�� �Ἥ �ۼ��ϴ� ����� �ִ�.
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
//���� �Լ�(c++���� �����ϴ� �Լ�)�� Mock�ϴ� ���
//include ��򰡿� OpenFile�̶�� �Լ��� �ִٰ� ��������.
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