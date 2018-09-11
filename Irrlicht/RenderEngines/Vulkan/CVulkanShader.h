#ifndef __C_VULKAN_SHADER_H_INCLUDED__
#define __C_VULKAN_SHADER_H_INCLUDED__

#include "RenderEngines/General/CIrrVertexDeclaration.h"
#include "RenderEngines/General/CIrrShader.h"
#include "CVulkanUtility.h"
#include "CVulkanVertexDeclaration.h"
#include "CVulkanResources.h"

namespace glslang
{
    class TType;
    class TIntermediate;
}

namespace irr
{
    namespace video
    {
        class CVulkanGLSLang;
        class CVulkanHardwareBuffer;
        class VulkanDescriptorLayout;
        class VulkanGraphicsPipelineState;
        class VulkanGpuParams;

        class CVulkanGLSLProgram
            : public CNullShader
            , public CVulkanDeviceResource
        {
        public:

            VkShaderModule* GetShaderModule(E_SHADER_TYPES type)
            {
                switch (type)
                {
                    case E_SHADER_TYPES::EST_VERTEX_SHADER:  
                    case E_SHADER_TYPES::EST_DOMAIN_SHADER:  
                    case E_SHADER_TYPES::EST_HULL_SHADER:    
                    case E_SHADER_TYPES::EST_GEOMETRY_SHADER:
                    case E_SHADER_TYPES::EST_FRAGMENT_SHADER:
                    case E_SHADER_TYPES::EST_COMPUTE_SHADER:
                        return &mStages[type].second;
                }

                return nullptr;
            }

            bool HasShaderModule(E_SHADER_TYPES type)
            {
                switch (type)
                {
                    case E_SHADER_TYPES::EST_VERTEX_SHADER:
                    case E_SHADER_TYPES::EST_DOMAIN_SHADER:
                    case E_SHADER_TYPES::EST_HULL_SHADER:
                    case E_SHADER_TYPES::EST_GEOMETRY_SHADER:
                    case E_SHADER_TYPES::EST_FRAGMENT_SHADER:
                    case E_SHADER_TYPES::EST_COMPUTE_SHADER:
                        return mStages[type].second != VK_NULL_HANDLE ? true : false;
                }

                return false;
            }

            const char* GetShaderEntryPoint(E_SHADER_TYPES type)
            {
                switch (type)
                {
                    case E_SHADER_TYPES::EST_VERTEX_SHADER:
                    case E_SHADER_TYPES::EST_DOMAIN_SHADER:
                    case E_SHADER_TYPES::EST_HULL_SHADER:
                    case E_SHADER_TYPES::EST_GEOMETRY_SHADER:
                    case E_SHADER_TYPES::EST_FRAGMENT_SHADER:
                    case E_SHADER_TYPES::EST_COMPUTE_SHADER:
                        return mStages[type].first.c_str();
                }

                return nullptr;
            }

            VkShaderStageFlagBits GetShaderStageBit(E_SHADER_TYPES type)
            {
                switch (type)
                {
                    case E_SHADER_TYPES::EST_VERTEX_SHADER:      return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
                    case E_SHADER_TYPES::EST_DOMAIN_SHADER:      return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                    case E_SHADER_TYPES::EST_HULL_SHADER:        return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                    case E_SHADER_TYPES::EST_GEOMETRY_SHADER:    return VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;
                    case E_SHADER_TYPES::EST_FRAGMENT_SHADER:    return VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
                    case E_SHADER_TYPES::EST_COMPUTE_SHADER:     return VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
                }

                return VkShaderStageFlagBits::VK_SHADER_STAGE_ALL_GRAPHICS;
            }

            void BuildBufferDesc(irr::video::CVulkanGLSLang& compiler, E_SHADER_TYPES type);

        private:
            friend class VulkanGraphicsPipelineState;
            friend class CVulkanDriver;

            std::pair<std::string /*entry point*/, VkShaderModule> mStages[E_SHADER_TYPES::EST_HIGH_LEVEL_SHADER];
            std::vector<VkDescriptorSetLayoutBinding> mBindings;
            VulkanDescriptorLayout* mLayout = nullptr;
            VulkanGpuParams* mParams = nullptr;

        public:
            explicit CVulkanGLSLProgram(video::IVideoDriver* context, E_SHADER_LANG type = E_SHADER_LANG::ESV_GLSL_HIGH_LEVEL);

            virtual ~CVulkanGLSLProgram();

            virtual void Init() override;

            bool enumInputLayout(void*);
            bool initializeConstantBuffers(irr::video::CVulkanGLSLang& compiler, E_SHADER_TYPES shaderType);
            bool initializeUniforms(irr::video::CVulkanGLSLang& compiler, E_SHADER_TYPES shaderType);

            bool CreateShaderModul(E_SHADER_TYPES type, CVulkanDriver* device, System::IO::IFileReader* file, const char* entryPoint, const char* shaderModel);

            VulkanDescriptorLayout* getLayout();
            VulkanGpuParams* GetDefaultGpuParams();

            const std::vector<VkDescriptorSetLayoutBinding>& GetBindings() const { return mBindings; }

            // Inherited via D3D11DeviceResource
            virtual void OnDeviceLost(CVulkanDriver * device) _IRR_OVERRIDE_;
            virtual void OnDeviceRestored(CVulkanDriver * device) _IRR_OVERRIDE_;
            virtual void OnDeviceDestroy(CVulkanDriver* device) _IRR_OVERRIDE_ {}

        private:

            void ReflParseStruct(glslang::TIntermediate* shaderIntermediate, irr::video::SConstantBuffer* buffdesc, irr::video::IShaderVariable* parent, const glslang::TType* type, std::vector<irr::video::IShaderVariable*>& Variables, std::string namePrefix, u32 pParentSize);
        };
    }
}

#endif //!__C_DIRECTX11_SHADER_H_INCLUDED__