#include "GlobalVariables.h"
#include "Resources.h"

#include "WinInit.h"
#include "D3DInit.h"

#include "Timer.h"

bool LoadResource()
{
	XMFLOAT3* l_particleData = new XMFLOAT3[particleCount];
	XMFLOAT3* l_particlePositionData = new XMFLOAT3[particleCount];

	SecureZeroMemory(l_particleData, sizeof(XMFLOAT3) * particleCount);
	SecureZeroMemory(l_particlePositionData, sizeof(XMFLOAT3) * particleCount);

	for (int i = 0; i < particleCount; ++i)
	{
		int x = i - particleCount / 2;
		srand(x);
		l_particleData[i] = XMFLOAT3(((float)(rand() % 100) - 50) / 100.f, ((float)(rand() % 100) - 50) / 100.f, ((float)(rand() % 100) - 50) / 100.f);
		XMVECTOR tmp = XMVectorSet(l_particleData[i].x, l_particleData[i].y, l_particleData[i].z, 0.f);
		tmp = XMVector3Normalize(tmp);
		l_particleData[i].x = XMVectorGetX(tmp) * 1.f;
		l_particleData[i].y = XMVectorGetY(tmp) * 1.f;
		l_particleData[i].z = XMVectorGetZ(tmp) * 1.f;
	}

	D3D11_BUFFER_DESC bufferDesc;
	SecureZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(XMFLOAT3) * particleCount;
	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	bufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA particleData;
	SecureZeroMemory(&particleData, sizeof(D3D11_SUBRESOURCE_DATA));
	particleData.pSysMem = l_particleData;

	g_hr = g_d3dDevice->CreateBuffer(&bufferDesc, &particleData, &g_d3dParticleBuffer);
	delete[] l_particleData;
	if (FAILED(g_hr))
	{
		delete[] l_particlePositionData;
		return false;
	}

	SecureZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(XMFLOAT3) * particleCount;
	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D10_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;

	SecureZeroMemory(&particleData, sizeof(D3D11_SUBRESOURCE_DATA));
	particleData.pSysMem = l_particlePositionData;

	g_hr = g_d3dDevice->CreateBuffer(&bufferDesc, &particleData, &g_d3dVertexBuffer);
	delete[] l_particlePositionData;
	if (FAILED(g_hr))
		return false;

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	SecureZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = 0;
	uavDesc.Buffer.NumElements = particleCount * 3;

	g_hr = g_d3dDevice->CreateUnorderedAccessView(g_d3dParticleBuffer, &uavDesc, &g_d3dParticleView);
	if (FAILED(g_hr)) return false;

	SecureZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = 0;
	uavDesc.Buffer.NumElements = particleCount * 3;

	g_hr = g_d3dDevice->CreateUnorderedAccessView(g_d3dVertexBuffer, &uavDesc, &g_d3dVertexView);
	if (FAILED(g_hr)) return false;

	return true;
}

bool CreateBuffers()
{
	XMMATRIX wvpMatrix = 
		XMMatrixTranslation(0.f, 0.f, 2.f) * 
		XMMatrixPerspectiveFovLH(XMConvertToRadians(45.f), static_cast<float>(g_screenW) / static_cast<float>(g_screenH), 0.1f, 10.f);

	D3D11_BUFFER_DESC constantBufferDesc;
	SecureZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.ByteWidth = sizeof(XMMATRIX);
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

	D3D11_SUBRESOURCE_DATA sData;
	SecureZeroMemory(&sData, sizeof(D3D11_SUBRESOURCE_DATA));
	sData.pSysMem = &wvpMatrix;

	g_hr = g_d3dDevice->CreateBuffer(&constantBufferDesc, &sData, &g_d3dConstantBufferOnce);
	if (FAILED(g_hr)) return false;

	SecureZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.ByteWidth = sizeof(ConstBufferPerFrame);
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;

	g_hr = g_d3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &g_d3dConstantBufferPerFrame);
	if (FAILED(g_hr)) return false;

	return true;
}

bool LoadShaders()
{
	UINT shaderFlags = 0;

#if DEBUG || _DEBUG
	shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* shaderBlob;

	g_hr = D3DCompileFromFile(
		L"../../Shaders/Shaders.hlsl", NULL, NULL, "CS", "cs_5_0",
		shaderFlags, 0, &shaderBlob, nullptr);
	if (FAILED(g_hr))
		return false;
	g_hr = g_d3dDevice->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &g_d3dComputeShader);
	if (FAILED(g_hr)) return false;
	SafeRelease(shaderBlob);

	g_hr = D3DCompileFromFile(
		L"../../Shaders/Shaders.hlsl", NULL, NULL, "VS", "vs_5_0",
		shaderFlags, 0, &shaderBlob, nullptr);
	if (FAILED(g_hr))
		return false;
	g_hr = g_d3dDevice->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &g_d3dVertexShader);
	if (FAILED(g_hr)) return false;

	D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	g_hr = g_d3dDevice->CreateInputLayout(vertexLayoutDesc, _countof(vertexLayoutDesc), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &g_d3dInputLayout);
	if (FAILED(g_hr)) return false;
	SafeRelease(shaderBlob);

	g_hr = D3DCompileFromFile(
		L"../../Shaders/Shaders.hlsl", NULL, NULL, "GS", "gs_5_0",
		shaderFlags, 0, &shaderBlob, nullptr);
	if (FAILED(g_hr))
		return false;
	g_hr = g_d3dDevice->CreateGeometryShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &g_d3dGeometryShader);
	if (FAILED(g_hr)) return false;
	SafeRelease(shaderBlob);

	g_hr = D3DCompileFromFile(
		L"../../Shaders/Shaders.hlsl", NULL, NULL, "PS", "ps_5_0",
		shaderFlags, 0, &shaderBlob, nullptr);
	if (FAILED(g_hr))
		return false;
	g_hr = g_d3dDevice->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &g_d3dPixelShader);
	if (FAILED(g_hr)) return false;
	SafeRelease(shaderBlob);

	return true;
}

void PreRendering()
{
	g_d3dDeviceContext->GSSetConstantBuffers(0, 1, &g_d3dConstantBufferOnce);
	g_d3dDeviceContext->CSSetConstantBuffers(1, 1, &g_d3dConstantBufferPerFrame);

	g_d3dDeviceContext->RSSetState(g_d3dRasterizerState);
	g_d3dDeviceContext->RSSetViewports(1, &g_d3dViewport);

	g_d3dDeviceContext->CSSetUnorderedAccessViews(0, 1, &g_d3dParticleView, nullptr);

	g_d3dDeviceContext->IASetInputLayout(g_d3dInputLayout);
	g_d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	g_d3dDeviceContext->CSSetShader(g_d3dComputeShader, nullptr, 0);
	g_d3dDeviceContext->VSSetShader(g_d3dVertexShader, nullptr, 0);
	g_d3dDeviceContext->GSSetShader(g_d3dGeometryShader, nullptr, 0);
	g_d3dDeviceContext->PSSetShader(g_d3dPixelShader, nullptr, 0);

	g_d3dDeviceContext->OMSetRenderTargets(1, &g_d3dBackBuffer, g_d3dDepthStencilView);
	g_d3dDeviceContext->OMSetDepthStencilState(g_d3dDepthStencilState, 1);
}

void Draw()
{
	g_d3dDeviceContext->ClearRenderTargetView(g_d3dBackBuffer, Colors::Black);
	g_d3dDeviceContext->ClearDepthStencilView(g_d3dDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	ConstBufferPerFrame cb;
	cb.acceleration = XMFLOAT3(sinf(Timer::GetTimeFromBeginning() * 3.f) * 0.4f, cosf(Timer::GetTimeFromBeginning() * 3.f) * 0.4f, 0.f);
	cb.deltaTime = Timer::GetDeltaTime();

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	SecureZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	g_d3dDeviceContext->Map(g_d3dConstantBufferPerFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &cb, sizeof(ConstBufferPerFrame));
	g_d3dDeviceContext->Unmap(g_d3dConstantBufferPerFrame, 0);

	const UINT vertexStride = sizeof(XMFLOAT3);
	const UINT offset = 0;
	static ID3D11Buffer* l_nullBuffer = nullptr;
	static ID3D11UnorderedAccessView* l_nullUAV = nullptr;

	g_d3dDeviceContext->IASetVertexBuffers(0, 1, &l_nullBuffer, &vertexStride, &offset);
	g_d3dDeviceContext->CSSetUnorderedAccessViews(1, 1, &g_d3dVertexView, nullptr);
	g_d3dDeviceContext->Dispatch(250000, 1, 1);

	g_d3dDeviceContext->CSSetUnorderedAccessViews(1, 1, &l_nullUAV, nullptr);
	g_d3dDeviceContext->IASetVertexBuffers(0, 1, &g_d3dVertexBuffer, &vertexStride, &offset);

	g_d3dDeviceContext->Draw(particleCount, 0);
	g_d3dSwapChain->Present(0, 0);
}

void Clear()
{
	SafeRelease(g_d3dDevice);
	SafeRelease(g_d3dDeviceContext);
	SafeRelease(g_d3dSwapChain);
	SafeRelease(g_d3dBackBuffer);
	SafeRelease(g_d3dDepthStencilBuffer);
	SafeRelease(g_d3dDepthStencilView);
	SafeRelease(g_d3dDepthStencilState);
	SafeRelease(g_d3dRasterizerState);
	SafeRelease(g_d3dVertexBuffer);
	SafeRelease(g_d3dParticleBuffer);
	SafeRelease(g_d3dConstantBufferOnce);
	SafeRelease(g_d3dConstantBufferPerFrame);
	SafeRelease(g_d3dVertexView);
	SafeRelease(g_d3dParticleView);
	SafeRelease(g_d3dComputeShader);
	SafeRelease(g_d3dVertexShader);
	SafeRelease(g_d3dGeometryShader);
	SafeRelease(g_d3dPixelShader);
	SafeRelease(g_d3dInputLayout);
}

int Run()
{
	MSG msg = { 0 };
	PreRendering();
	while (msg.message != WM_QUIT)
	{
		Timer::Tick();
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Draw();
	}
	Clear();
	return (int)msg.wParam;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nShowCmd)
{
	if (!InitWindowsApp(hInstance) ||
		!InitDirectX())
		return 0;

	if (!LoadResource() ||
		!CreateBuffers() ||
		!LoadShaders())
		return 0;
	Timer::Setup();
	return Run();
}