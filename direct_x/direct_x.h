// dx.h
#pragma once
#include <d3d11.h>
#include <dxgi1_2.h>
#include <cstdint>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

struct DX {
    ComPtr<ID3D11Device>           device;
    ComPtr<ID3D11DeviceContext>    ctx;
    ComPtr<IDXGISwapChain>         swapChain;
    ComPtr<ID3D11RenderTargetView> rtv;
    ComPtr<IDXGIOutputDuplication> duplication;
    ComPtr<ID3D11Texture2D> copyTex;

    uint32_t width  = 0;
    uint32_t height = 0;
};

bool DX_Init            (DX& dx, HWND hwnd, uint32_t w, uint32_t h);
bool DX_InitDuplication (DX& dx, uint32_t monitorIndex = 0);
void DX_BeginFrame      (DX& dx);
void DX_EndFrame        (DX& dx);
void DX_Shutdown        (DX& dx);
void DX_DrawTexture(DX& dx, ID3D11Texture2D* tex);
void DX_DrawText(float x, float y, const char* text, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void DX_DrawLine(DX& dx, float x1, float y1, float x2, float y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a, float thickness = 1.0f);
void DX_DrawRect(DX& dx, float x, float y, float w, float h, uint8_t r, uint8_t g, uint8_t b, uint8_t a, float thickness = 1.0f);
void DX_DrawArrow(DX& dx, float x1, float y1, float x2, float y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void DX_BeginBatch(DX& dx);
void DX_EndBatch(DX& dx);
void DX_AddLine(DX& dx, float x1, float y1, float x2, float y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a, float thickness);
void DX_DrawCircle(float x, float y, float radius, uint8_t r, uint8_t g, uint8_t b, uint8_t a);