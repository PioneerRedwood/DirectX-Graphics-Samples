#include <cmath>
#include <cstring>
#include <d3dcompiler.h>

#include "GameCore.h"
#include "BufferManager.h"
#include "Camera.h"
#include "CommandContext.h"
#include "GraphicsCommon.h"
#include "PipelineState.h"
#include "RootSignature.h"

using namespace GameCore;
using namespace Graphics;
using namespace Math;

namespace
{
    constexpr float kOrbitSpeed = 0.35f;

    using Microsoft::WRL::ComPtr;

    using D3DCompileFunc = HRESULT(WINAPI*)(LPCVOID, SIZE_T, LPCSTR, const D3D_SHADER_MACRO*, ID3DInclude*, LPCSTR, LPCSTR, UINT, UINT, ID3DBlob**, ID3DBlob**);

    D3DCompileFunc GetD3DCompile()
    {
        static D3DCompileFunc s_D3DCompile = []() -> D3DCompileFunc
        {
            HMODULE compiler = LoadLibraryW(L"d3dcompiler_47.dll");
            if (compiler == nullptr)
            {
                compiler = LoadLibraryW(L"d3dcompiler_43.dll");
            }
            ASSERT(compiler != nullptr, "Failed to load d3dcompiler DLL.");
            auto proc = reinterpret_cast<D3DCompileFunc>(GetProcAddress(compiler, "D3DCompile"));
            ASSERT(proc != nullptr, "Failed to locate D3DCompile.");
            return proc;
        }();
        return s_D3DCompile;
    }

    ComPtr<ID3DBlob> CompileShaderFromString(const char* source, const char* entryPoint, const char* profile)
    {
        const D3DCompileFunc compile = GetD3DCompile();

        UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
        compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        ComPtr<ID3DBlob> bytecode;
        ComPtr<ID3DBlob> errors;
        const HRESULT hr = compile(
            source,
            std::strlen(source),
            nullptr,
            nullptr,
            nullptr,
            entryPoint,
            profile,
            compileFlags,
            0,
            &bytecode,
            &errors
        );

        if (FAILED(hr))
        {
            if (errors)
            {
                OutputDebugStringA(static_cast<const char*>(errors->GetBufferPointer()));
            }
            ASSERT_SUCCEEDED(hr, "Shader compilation failed.");
        }

        return bytecode;
    }

    struct alignas(16) SceneConstants
    {
        Matrix4 ViewProjection;
    };

    struct Vertex
    {
        XMFLOAT3 Position;
        XMFLOAT3 Color;
    };

    struct BasicGeometryPipeline
    {
        RootSignature RootSig;
        GraphicsPSO LinePSO;
        GraphicsPSO TrianglePSO;
        bool Initialized = false;

        void Initialize(DXGI_FORMAT colorFormat, DXGI_FORMAT depthFormat)
        {
            if (Initialized)
            {
                return;
            }

            RootSig.Reset(1, 0);
            RootSig[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
            RootSig.Finalize(L"Prototype Basic RootSig", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

            static constexpr char kVertexShader[] = R"(
cbuffer SceneCB : register(b0)
{
    float4x4 g_ViewProjection;
};

struct VSInput
{
    float3 Position : POSITION;
    float3 Color : COLOR;
};

struct PSInput
{
    float4 Position : SV_Position;
    float3 Color : COLOR;
};

PSInput main(VSInput input)
{
    PSInput output;
    output.Position = mul(g_ViewProjection, float4(input.Position, 1.0));
    output.Color = input.Color;
    return output;
}
)";

            static constexpr char kPixelShader[] = R"(
struct PSInput
{
    float4 Position : SV_Position;
    float3 Color : COLOR;
};

float4 main(PSInput input) : SV_Target
{
    return float4(input.Color, 1.0);
}
)";

            const ComPtr<ID3DBlob> vs = CompileShaderFromString(kVertexShader, "main", "vs_5_0");
            const ComPtr<ID3DBlob> ps = CompileShaderFromString(kPixelShader, "main", "ps_5_0");

            static const D3D12_INPUT_ELEMENT_DESC kInputElements[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,   D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
            };

            auto configurePSO = [&](GraphicsPSO& pso, D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType)
            {
                pso.SetRootSignature(RootSig);
                pso.SetRasterizerState(Graphics::RasterizerDefault);
                pso.SetBlendState(Graphics::BlendDisable);
                pso.SetDepthStencilState(Graphics::DepthStateReadWrite);
                pso.SetSampleMask(UINT_MAX);
                pso.SetPrimitiveTopologyType(topologyType);
                pso.SetInputLayout(_countof(kInputElements), kInputElements);
                pso.SetRenderTargetFormats(1, &colorFormat, depthFormat);
                pso.SetVertexShader(vs->GetBufferPointer(), vs->GetBufferSize());
                pso.SetPixelShader(ps->GetBufferPointer(), ps->GetBufferSize());
                pso.Finalize();
            };

            configurePSO(LinePSO, D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
            configurePSO(TrianglePSO, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

            Initialized = true;
        }
    };

    BasicGeometryPipeline g_BasicGeometryPipeline;
}

class PrototypeApp final : public GameCore::IGameApp
{
public:
    void Startup() override
    {
        m_FocusPoint = Vector3(0.0f, 0.5f, 0.0f);
        m_OrbitRadius = 4.0f;

        const Vector3 eye(0.0f, 1.5f, -m_OrbitRadius);
        m_Camera.SetEyeAtUp(eye, m_FocusPoint, Vector3(0.0f, 1.0f, 0.0f));
        m_Camera.SetZRange(0.1f, 100.0f);
        m_Camera.Update();

        SyncViewport();
    }

    void Cleanup() override
    {
    }

    void Update(float deltaT) override
    {
        m_TotalTime += deltaT;
        const float orbitAngle = m_TotalTime * kOrbitSpeed;

        const Vector3 eye(
            std::sin(orbitAngle) * m_OrbitRadius,
            1.5f,
            std::cos(orbitAngle) * m_OrbitRadius
        );

        m_Camera.SetEyeAtUp(eye, m_FocusPoint, Vector3(0.0f, 1.0f, 0.0f));
        m_Camera.Update();
    }

    void RenderScene() override
    {
        SyncViewport();

        GraphicsContext& gfxContext = GraphicsContext::Begin(L"Prototype Scene");
        gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
        gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        gfxContext.ClearColor(g_SceneColorBuffer);
        gfxContext.ClearDepth(g_SceneDepthBuffer);
        gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV(), g_SceneDepthBuffer.GetDSV());
        gfxContext.SetViewportAndScissor(m_MainViewport, m_MainScissor);

        g_BasicGeometryPipeline.Initialize(g_SceneColorBuffer.GetFormat(), g_SceneDepthBuffer.GetFormat());

        SceneConstants constants{};
        constants.ViewProjection = m_Camera.GetViewProjMatrix();

        gfxContext.SetRootSignature(g_BasicGeometryPipeline.RootSig);
        gfxContext.SetDynamicConstantBufferView(0, sizeof(constants), &constants);

        alignas(16) Vertex axisVertices[] =
        {
            { { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
            { { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
            { { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
            { { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
            { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
            { { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } }
        };

        gfxContext.SetPipelineState(g_BasicGeometryPipeline.LinePSO);
        gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
        gfxContext.SetDynamicVB(0, _countof(axisVertices), sizeof(Vertex), axisVertices);
        gfxContext.Draw(_countof(axisVertices));

        alignas(16) Vertex triangleVertices[] =
        {
            { { 0.0f, 0.5f, 0.0f }, { 1.0f, 0.8f, 0.2f } },
            { { 0.5f, -0.5f, 0.0f }, { 0.2f, 0.6f, 1.0f } },
            { { -0.5f, -0.5f, 0.0f }, { 0.8f, 0.2f, 1.0f } }
        };

        gfxContext.SetPipelineState(g_BasicGeometryPipeline.TrianglePSO);
        gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        gfxContext.SetDynamicVB(0, _countof(triangleVertices), sizeof(Vertex), triangleVertices);
        gfxContext.SetDynamicConstantBufferView(0, sizeof(constants), &constants);
        gfxContext.Draw(_countof(triangleVertices));

        gfxContext.Finish();
    }

private:
    void SyncViewport()
    {
        m_MainViewport.TopLeftX = 0.0f;
        m_MainViewport.TopLeftY = 0.0f;
        m_MainViewport.Width = static_cast<float>(g_SceneColorBuffer.GetWidth());
        m_MainViewport.Height = static_cast<float>(g_SceneColorBuffer.GetHeight());
        m_MainViewport.MinDepth = 0.0f;
        m_MainViewport.MaxDepth = 1.0f;

        m_MainScissor.left = 0;
        m_MainScissor.top = 0;
        m_MainScissor.right = static_cast<LONG>(g_SceneColorBuffer.GetWidth());
        m_MainScissor.bottom = static_cast<LONG>(g_SceneColorBuffer.GetHeight());
    }

    Camera m_Camera;
    Vector3 m_FocusPoint;
    float m_OrbitRadius = 0.0f;
    float m_TotalTime = 0.0f;
    D3D12_VIEWPORT m_MainViewport{};
    D3D12_RECT m_MainScissor{};
};

CREATE_APPLICATION(PrototypeApp)
