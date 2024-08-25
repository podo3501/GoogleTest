#pragma once

class GlobalEnv : public ::testing::Environment
{
public:
	GlobalEnv();
	~GlobalEnv();

	void SetUp();
	void TearDown();
};
