#pragma once

#include "Common.h"
#include "CommonMath.h"
#include "CommonRendering.h"

static const int g_screenW = 1280;
static const int g_screenH = 720;

HWND g_mainWin;
HRESULT g_hr;

ID3D11Device*					g_d3dDevice			= nullptr;
ID3D11DeviceContext*			g_d3dDeviceContext	= nullptr;

IDXGISwapChain*					g_d3dSwapChain		= nullptr;
ID3D11RenderTargetView*			g_d3dBackBuffer		= nullptr;

ID3D11Texture2D*				g_d3dDepthStencilBuffer		= nullptr;
ID3D11DepthStencilView*			g_d3dDepthStencilView		= nullptr;
ID3D11DepthStencilState*		g_d3dDepthStencilState		= nullptr;

ID3D11RasterizerState*			g_d3dRasterizerState	= nullptr;
D3D11_VIEWPORT					g_d3dViewport;

ID3D11Buffer*					g_d3dVertexBuffer			= nullptr;
ID3D11Buffer*					g_d3dParticleBuffer			= nullptr;
ID3D11Buffer*					g_d3dConstantBufferOnce		= nullptr;
ID3D11Buffer*					g_d3dConstantBufferPerFrame = nullptr;

ID3D11UnorderedAccessView*		g_d3dVertexView			= nullptr;
ID3D11UnorderedAccessView*		g_d3dParticleView		= nullptr;

ID3D11ComputeShader*			g_d3dComputeShader		= nullptr;
ID3D11VertexShader*				g_d3dVertexShader		= nullptr;
ID3D11GeometryShader*			g_d3dGeometryShader		= nullptr;
ID3D11PixelShader*				g_d3dPixelShader		= nullptr;

ID3D11InputLayout*				g_d3dInputLayout		= nullptr;


