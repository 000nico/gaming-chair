// dx.cpp
#include "direct_x.h"
#include <d3dcompiler.h>
#include <cstdint>
#include <cmath>
#include "../gui/imgui/imgui.h"

bool DX_Init(DX& dx, HWND hwnd, uint32_t w, uint32_t h)
{
    dx.width = w; dx.height = h;

    DXGI_SWAP_CHAIN_DESC scd       = {};
    scd.BufferCount                = 3;
    scd.BufferDesc.Width           = w;
    scd.BufferDesc.Height          = h;
    scd.BufferDesc.Format          = DXGI_FORMAT_B8G8R8A8_UNORM;
    scd.BufferUsage                = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow               = hwnd;
    scd.SampleDesc.Count           = 1;
    scd.Windowed                   = TRUE;
    scd.SwapEffect                 = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;  // mejor para overlay
    scd.Flags                      = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    UINT flags = 0;
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL fl;
    if (FAILED(D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
        nullptr, 0, D3D11_SDK_VERSION, &scd,
        dx.swapChain.GetAddressOf(), dx.device.GetAddressOf(),
        &fl, dx.ctx.GetAddressOf())))
        return false;

    ComPtr<ID3D11Texture2D> bb;
    dx.swapChain->GetBuffer(0, IID_PPV_ARGS(bb.GetAddressOf()));
    dx.device->CreateRenderTargetView(bb.Get(), nullptr, dx.rtv.GetAddressOf());

    return true;
}

// ── Shaders ───────────────────────────────────────────────────────────────

static const char* SRC_QUAD_VS = R"(
    struct OUT { float4 pos : SV_POSITION; float2 uv : TEXCOORD; };
    OUT main(uint id : SV_VertexID) {
        float2 p[6] = { float2(-1,-1),float2(-1,1),float2(1,1), float2(-1,-1),float2(1,1),float2(1,-1) };
        float2 u[6] = { float2(0,1),  float2(0,0), float2(1,0), float2(0,1),  float2(1,0),float2(1,1)  };
        OUT o; o.pos = float4(p[id],0,1); o.uv = u[id]; return o;
    }
)";

static const char* SRC_QUAD_PS = R"(
    Texture2D    tex : register(t0);
    SamplerState smp : register(s0);
    struct OUT { float4 pos : SV_POSITION; float2 uv : TEXCOORD; };
    float4 main(OUT v) : SV_TARGET { return tex.Sample(smp, v.uv); }
)";

static const char* SRC_LINE_VS = R"(
    struct V { float2 pos : POSITION; float4 col : COLOR; };
    struct O { float4 pos : SV_POSITION; float4 col : COLOR; };
    O main(V v) { O o; o.pos = float4(v.pos, 0, 1); o.col = v.col; return o; }
)";

static const char* SRC_LINE_PS = R"(
    struct O { float4 pos : SV_POSITION; float4 col : COLOR; };
    float4 main(O v) : SV_TARGET { return v.col; }
)";

// ── Line batch state ──────────────────────────────────────────────────────

struct LineVertex {
    float x, y;
    float r, g, b, a;
};

static constexpr int MAX_LINE_VERTS = 8192;
static LineVertex s_lineVerts[MAX_LINE_VERTS];
static int        s_lineVertCount = 0;

static ComPtr<ID3D11VertexShader> s_lineVS;
static ComPtr<ID3D11PixelShader>  s_linePS;
static ComPtr<ID3D11InputLayout>  s_lineIL;
static ComPtr<ID3D11Buffer>       s_lineVB;
static ComPtr<ID3D11BlendState>   s_lineBS;
static bool                       s_linePipelineReady = false;

static void InitLinePipeline(DX& dx)
{
    if (s_linePipelineReady) return;

    ComPtr<ID3DBlob> vsBlob, psBlob, err;
    D3DCompile(SRC_LINE_VS, strlen(SRC_LINE_VS), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, vsBlob.GetAddressOf(), err.GetAddressOf());
    D3DCompile(SRC_LINE_PS, strlen(SRC_LINE_PS), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, psBlob.GetAddressOf(), err.GetAddressOf());
    dx.device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, s_lineVS.GetAddressOf());
    dx.device->CreatePixelShader (psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, s_linePS.GetAddressOf());

    D3D11_INPUT_ELEMENT_DESC ied[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    dx.device->CreateInputLayout(ied, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), s_lineIL.GetAddressOf());

    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth      = sizeof(LineVertex) * MAX_LINE_VERTS;
    bd.Usage          = D3D11_USAGE_DYNAMIC;
    bd.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    dx.device->CreateBuffer(&bd, nullptr, s_lineVB.GetAddressOf());

    D3D11_BLEND_DESC bld = {};
    bld.RenderTarget[0].BlendEnable           = TRUE;
    bld.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
    bld.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
    bld.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
    bld.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
    bld.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
    bld.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
    bld.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    dx.device->CreateBlendState(&bld, s_lineBS.GetAddressOf());

    s_linePipelineReady = true;
}

static void FlushLines(DX& dx)
{
    if (s_lineVertCount == 0) return;

    D3D11_MAPPED_SUBRESOURCE ms;
    dx.ctx->Map(s_lineVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
    memcpy(ms.pData, s_lineVerts, sizeof(LineVertex) * s_lineVertCount);
    dx.ctx->Unmap(s_lineVB.Get(), 0);

    UINT stride = sizeof(LineVertex), offset = 0;
    dx.ctx->IASetVertexBuffers(0, 1, s_lineVB.GetAddressOf(), &stride, &offset);
    dx.ctx->IASetInputLayout(s_lineIL.Get());
    dx.ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    dx.ctx->VSSetShader(s_lineVS.Get(), nullptr, 0);
    dx.ctx->PSSetShader(s_linePS.Get(), nullptr, 0);
    dx.ctx->OMSetBlendState(s_lineBS.Get(), nullptr, 0xffffffff);
    dx.ctx->Draw(s_lineVertCount, 0);

    s_lineVertCount = 0;
}

// ── Public API ────────────────────────────────────────────────────────────

void DX_DrawText(float x, float y, const char* text, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    dl->AddText(ImVec2(x, y), IM_COL32(r, g, b, a), text);
}

void DX_DrawCircle(float x, float y, float radius, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    dl->AddCircle(ImVec2(x, y), radius, IM_COL32(r, g, b, a));
}

void DX_DrawTexture(DX& dx, ID3D11Texture2D* tex)
{
    static ComPtr<ID3D11VertexShader>       vs;
    static ComPtr<ID3D11PixelShader>        ps;
    static ComPtr<ID3D11SamplerState>       sampler;
    static ComPtr<ID3D11ShaderResourceView> srv;
    static ID3D11Texture2D*                 lastTex = nullptr;

    if (!vs) {
        ComPtr<ID3DBlob> vsBlob, psBlob, err;
        D3DCompile(SRC_QUAD_VS, strlen(SRC_QUAD_VS), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, vsBlob.GetAddressOf(), err.GetAddressOf());
        D3DCompile(SRC_QUAD_PS, strlen(SRC_QUAD_PS), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, psBlob.GetAddressOf(), err.GetAddressOf());
        dx.device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, vs.GetAddressOf());
        dx.device->CreatePixelShader (psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, ps.GetAddressOf());

        D3D11_SAMPLER_DESC sd = {};
        sd.Filter   = D3D11_FILTER_MIN_MAG_MIP_POINT;
        sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        dx.device->CreateSamplerState(&sd, sampler.GetAddressOf());
    }

    if (tex != lastTex) {
        srv.Reset();
        D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
        srvd.Format              = DXGI_FORMAT_B8G8R8A8_UNORM;
        srvd.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvd.Texture2D.MipLevels = 1;
        dx.device->CreateShaderResourceView(tex, &srvd, srv.GetAddressOf());
        lastTex = tex;
    }

    dx.ctx->VSSetShader(vs.Get(), nullptr, 0);
    dx.ctx->PSSetShader(ps.Get(), nullptr, 0);
    dx.ctx->IASetInputLayout(nullptr);
    dx.ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    dx.ctx->PSSetShaderResources(0, 1, srv.GetAddressOf());
    dx.ctx->PSSetSamplers(0, 1, sampler.GetAddressOf());
    dx.ctx->Draw(6, 0);
}

bool DX_InitDuplication(DX& dx, uint32_t monitorIndex)
{
    dx.copyTex.Reset();

    ComPtr<IDXGIDevice>  dxgiDev;
    ComPtr<IDXGIAdapter> adapter;
    ComPtr<IDXGIOutput>  output;
    ComPtr<IDXGIOutput1> output1;

    dx.device->QueryInterface(IID_PPV_ARGS(dxgiDev.GetAddressOf()));
    dxgiDev->GetAdapter(adapter.GetAddressOf());

    if (FAILED(adapter->EnumOutputs(monitorIndex, output.GetAddressOf())))
        return false;

    output.As(&output1);

    if (FAILED(output1->DuplicateOutput(dx.device.Get(), dx.duplication.GetAddressOf())))
        return false;

    DXGI_OUTDUPL_DESC desc = {};
    dx.duplication->GetDesc(&desc);

    D3D11_TEXTURE2D_DESC td = {};
    td.Width              = desc.ModeDesc.Width;
    td.Height             = desc.ModeDesc.Height;
    td.Format             = DXGI_FORMAT_B8G8R8A8_UNORM;
    td.ArraySize          = 1;
    td.MipLevels          = 1;
    td.SampleDesc.Count   = 1;
    td.Usage              = D3D11_USAGE_DEFAULT;
    td.BindFlags          = D3D11_BIND_SHADER_RESOURCE;

    if (FAILED(dx.device->CreateTexture2D(&td, nullptr, dx.copyTex.GetAddressOf())))
        return false;

    return true;
}

void DX_BeginFrame(DX& dx)
{
    float clear[4] = { 0, 0, 0, 1 };
    dx.ctx->ClearRenderTargetView(dx.rtv.Get(), clear);

    D3D11_VIEWPORT vp = {};
    vp.Width    = (float)dx.width;
    vp.Height   = (float)dx.height;
    vp.MaxDepth = 1.0f;

    dx.ctx->RSSetViewports(1, &vp);
    dx.ctx->OMSetRenderTargets(1, dx.rtv.GetAddressOf(), nullptr);

    // inicializar pipeline de lineas si hace falta
    InitLinePipeline(dx);
    s_lineVertCount = 0;
}

void DX_EndFrame(DX& dx)
{
    // flush todas las lineas pendientes en un solo draw call
    FlushLines(dx);

    dx.swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
}

void DX_Shutdown(DX& dx)
{
    if (dx.ctx) dx.ctx->ClearState();
}

void DX_DrawLine(DX& dx, float x1, float y1, float x2, float y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a, float thickness)
{
    if (s_lineVertCount + 6 > MAX_LINE_VERTS)
        FlushLines(dx);

    auto toNDC_X = [&](float px) { return (px / dx.width)  * 2.0f - 1.0f; };
    auto toNDC_Y = [&](float py) { return 1.0f - (py / dx.height) * 2.0f; };

    float dx_ = x2 - x1, dy_ = y2 - y1;
    float len = sqrtf(dx_ * dx_ + dy_ * dy_);
    if (len < 0.0001f) return;

    float nx = (-dy_ / len) * (thickness * 0.5f);
    float ny = ( dx_ / len) * (thickness * 0.5f);

    float fr = r / 255.0f, fg = g / 255.0f, fb = b / 255.0f, fa = a / 255.0f;

    LineVertex v[6] = {
        { toNDC_X(x1 + nx), toNDC_Y(y1 + ny), fr, fg, fb, fa },
        { toNDC_X(x1 - nx), toNDC_Y(y1 - ny), fr, fg, fb, fa },
        { toNDC_X(x2 - nx), toNDC_Y(y2 - ny), fr, fg, fb, fa },
        { toNDC_X(x1 + nx), toNDC_Y(y1 + ny), fr, fg, fb, fa },
        { toNDC_X(x2 - nx), toNDC_Y(y2 - ny), fr, fg, fb, fa },
        { toNDC_X(x2 + nx), toNDC_Y(y2 + ny), fr, fg, fb, fa },
    };

    memcpy(s_lineVerts + s_lineVertCount, v, sizeof(v));
    s_lineVertCount += 6;
}

void DX_DrawRect(DX& dx, float x, float y, float w, float h, uint8_t r, uint8_t g, uint8_t b, uint8_t a, float thickness)
{
    DX_DrawLine(dx, x,     y,     x + w, y,     r, g, b, a, thickness);
    DX_DrawLine(dx, x,     y + h, x + w, y + h, r, g, b, a, thickness);
    DX_DrawLine(dx, x,     y,     x,     y + h, r, g, b, a, thickness);
    DX_DrawLine(dx, x + w, y,     x + w, y + h, r, g, b, a, thickness);
}

void DX_DrawArrow(DX& dx, float x1, float y1, float x2, float y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    float dx_ = x2 - x1, dy_ = y2 - y1;
    float len = sqrtf(dx_ * dx_ + dy_ * dy_);
    if(len < 0.0001f) return;

    float nx = dx_ / len, ny = dy_ / len;
    float px = -ny, py = nx;

    float arrowSize  = 10.0f;
    float arrowWidth = 0.9f;
    float border     = 2.0f;

    ImVec2 tipB   = { x1 + nx * (arrowSize + border), y1 + ny * (arrowSize + border) };
    ImVec2 leftB  = { x1 - nx * (arrowSize + border) + px * (arrowSize + border) * arrowWidth, y1 - ny * (arrowSize + border) + py * (arrowSize + border) * arrowWidth };
    ImVec2 rightB = { x1 - nx * (arrowSize + border) - px * (arrowSize + border) * arrowWidth, y1 - ny * (arrowSize + border) - py * (arrowSize + border) * arrowWidth };
    ImGui::GetBackgroundDrawList()->AddTriangleFilled(tipB, leftB, rightB, IM_COL32(0, 0, 0, a));

    ImVec2 tip   = { x1 + nx * arrowSize, y1 + ny * arrowSize };
    ImVec2 left  = { x1 - nx * arrowSize + px * arrowSize * arrowWidth, y1 - ny * arrowSize + py * arrowSize * arrowWidth };
    ImVec2 right = { x1 - nx * arrowSize - px * arrowSize * arrowWidth, y1 - ny * arrowSize - py * arrowSize * arrowWidth };
    ImGui::GetBackgroundDrawList()->AddTriangleFilled(tip, left, right, IM_COL32(r, g, b, a));
}