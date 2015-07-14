#pragma once

#include "CommonMath.h"

struct ConstBufferPerFrame
{
	XMFLOAT3 acceleration;
	float deltaTime;
};

const int particleCount = 64 * 2 * 250000;