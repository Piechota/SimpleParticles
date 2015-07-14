cbuffer ConstBufferOnce : register(b0)
{
	matrix wvpMatrix;
}
cbuffer ConstBufferPerFrame : register(b1)
{
	float3 acc;
	float deltaTime;
}
struct GSO
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};
static const float3 GSOffsets[] =
{
	float3(-0.001f, -0.001f, 0.00f),
	float3(-0.001f, +0.001f, 0.00f),
	float3(+0.001f, -0.001f, 0.00f),
	float3(+0.001f, +0.001f, 0.00f)
};
RWBuffer<float> particleData : register(u0);
RWBuffer<float> particlePosition : register(u1);
[numthreads(1024, 1, 1)]
void CS(int3 dispatchThreadID : SV_DISPATCHTHREADID)
{
	const int positionOffset = dispatchThreadID.x * 3;
	float3 position = float3(particlePosition[positionOffset + 0], particlePosition[positionOffset + 1], particlePosition[positionOffset + 2]);
	float3 particleVelocity = float3(particleData[positionOffset + 0], particleData[positionOffset + 1], particleData[positionOffset + 2]);
	float3 acc0 = normalize(acc - position) * .2f;

	particleVelocity += acc0 * deltaTime;
	float3 velocity = particleVelocity * deltaTime;
	position += velocity;

	float3 minus_v = float3(position.x < -0.5f ? 1.f : 0, position.y < -0.5f ? 1.f : 0, position.z < -0.5f ? 1.f : 0);
	float3 plus_v = float3(position.x > +0.5f ? 1.f : 0, position.y > +0.5f ? 1.f : 0, position.z > +0.5f ? 1.f : 0);

	position -= velocity * minus_v;
	position -= velocity * plus_v;


	particleVelocity = particleVelocity * (1.f - minus_v) + reflect(velocity, minus_v) * minus_v;
	particleVelocity -= particleVelocity * 0.05f * saturate(dot(minus_v, 1.f));

	particleVelocity = particleVelocity * (1.f - plus_v) + reflect(velocity, -plus_v) * plus_v;
	particleVelocity -= particleVelocity * 0.05f * saturate(dot(plus_v, 1.f));
	
	particleData[positionOffset + 0] = particleVelocity.x;
	particleData[positionOffset + 1] = particleVelocity.y;
	particleData[positionOffset + 2] = particleVelocity.z;

	particlePosition[positionOffset + 0] = position.x;
	particlePosition[positionOffset + 1] = position.y;
	particlePosition[positionOffset + 2] = position.z;
}

[maxvertexcount(4)]
void GS(point float3 p[1] : POSITION, inout TriangleStream<GSO> triStream)
{
	GSO OUT;

	float t = p[0].y + .5f;
	float4 color = lerp(float4(0.f, 1.f, 1.f, 1.f), float4(1.f, .0f, 1.f, 1.f), t);

	OUT.color = color;

	OUT.position = float4(p[0] + GSOffsets[0], 1.f);
	OUT.position = mul(wvpMatrix, OUT.position);
	triStream.Append(OUT);
	OUT.position = float4(p[0] + GSOffsets[1], 1.f);
	OUT.position = mul(wvpMatrix, OUT.position);
	triStream.Append(OUT);
	OUT.position = float4(p[0] + GSOffsets[2], 1.f);
	OUT.position = mul(wvpMatrix, OUT.position);
	triStream.Append(OUT);
	OUT.position = float4(p[0] + GSOffsets[3], 1.f);
	OUT.position = mul(wvpMatrix, OUT.position);
	triStream.Append(OUT);
}

float3 VS(float3 position : POSITION) : POSITION
{
	return position;
}

float4 PS(GSO IN) : SV_TARGET
{
	return IN.color;
}