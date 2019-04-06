#ifndef __C_DIRECTX11_SHADER_H_INCLUDED__
#define __C_DIRECTX11_SHADER_H_INCLUDED__

#include "RenderEngines/General/CIrrVertexDeclaration.h"
#include "RenderEngines/General/CIrrShader.h"
#include "CD3D11VertexDeclaration.h"
#include "CD3D11Resources.h"
#include <d3d11_1.h>
#include <D3DCompiler.h>

namespace irr
{
    namespace video
    {
        class CD3D11HardwareBuffer;

        class D3D11HLSLProgram
            : public CNullShader
            , protected D3D11DeviceResource
        {
        private:
            friend class CD3D11FixedFunctionMaterialRenderer;
            friend class CD3D11MaterialRenderer_ONETEXTURE_BLEND;
            friend class CD3D11Driver;

            Msw::ComPtr<ID3D11VertexShader>     m_vertexShader;
            Msw::ComPtr<ID3D11HullShader>       m_hullShader;
            Msw::ComPtr<ID3D11DomainShader>     m_domainShader;
            Msw::ComPtr<ID3D11ComputeShader>    m_computeShader;
            Msw::ComPtr<ID3D11GeometryShader>   m_geometryShader;
            Msw::ComPtr<ID3D11PixelShader>      m_pixelShader;
            Msw::ComPtr<ID3D11InputLayout>      m_inputLayout;          // Temporary for old style ui draw
            core::array<byte>       m_vertexShaderBytecode;

            Msw::ComPtr<ID3D11ShaderReflection> pReflectVertexShader;
            Msw::ComPtr<ID3D11ShaderReflection> pReflectHullShader;
            Msw::ComPtr<ID3D11ShaderReflection> pReflectDomainShader;
            Msw::ComPtr<ID3D11ShaderReflection> pReflectComputeShader;
            Msw::ComPtr<ID3D11ShaderReflection> pReflectGeometryShader;
            Msw::ComPtr<ID3D11ShaderReflection> pReflectPixelShader;

            core::array<D3D11_INPUT_ELEMENT_DESC> inputLayoutArray;

        public:
            explicit D3D11HLSLProgram(video::IVideoDriver* context);

            virtual ~D3D11HLSLProgram();

            virtual E_SHADER_LANG getShaderVersion() const { return ESV_HLSL_HIGH_LEVEL; }
            virtual void Init();
            IConstantBuffer* AddUnknownBuffer(E_SHADER_TYPES shaderType, u32 size) override;

            void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const char* shaderFilename);
            virtual bool createVertexShader(ID3D11Device* device, System::IO::IFileReader* file, const char* main, const char* shaderModel, D3D_SHADER_MACRO* Macros);
            virtual bool createGeometryShader(ID3D11Device* device, System::IO::IFileReader* file, const char* main, const char* shaderModel, D3D_SHADER_MACRO* Macros);
            virtual bool createPixelShader(ID3D11Device* device, System::IO::IFileReader* file, const char* main, const char* shaderModel, D3D_SHADER_MACRO* Macros);
            HRESULT enumInputLayout(CD3D11Driver * d3dDevice, ID3D11ShaderReflection*);
            HRESULT initializeConstantBuffers(CD3D11Driver * d3dDevice, ID3D11ShaderReflection*, E_SHADER_TYPES shaderType);

            ID3D11InputLayout*       GetInputLayout() { return m_inputLayout.Get(); }

            // Inherited via D3D11DeviceResource
            virtual void OnDeviceLost(CD3D11Driver * device) override;
            virtual void OnDeviceRestored(CD3D11Driver * device) override;

            const core::array<byte>& GetVertexShaderByteCode() const { return m_vertexShaderBytecode; }

        private:
            void ReflParseStruct(SConstantBuffer* buffdesc, irr::video::IShaderVariable* parent, ID3D11ShaderReflectionConstantBuffer * pConstBuffer, ID3D11ShaderReflectionType * pParentVariableType, std::vector<irr::video::IShaderVariable*>& Variables, std::string namePrefix, u32 pParentSize);
        };
    }
}

#endif //!__C_DIRECTX11_SHADER_H_INCLUDED__