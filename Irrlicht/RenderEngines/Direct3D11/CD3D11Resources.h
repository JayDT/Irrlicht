#ifndef __C_D3D11_RESOURCE_H_INCLUDED__
#define __C_D3D11_RESOURCE_H_INCLUDED__

#include "IReferenceCounted.h"
#include <wrl.h>

namespace Msw = Microsoft::WRL;

namespace irr
{
    namespace video
    {
        class CD3D11Driver;

        struct D3D11DeviceResource
        {
            D3D11DeviceResource(CD3D11Driver* driver);

            virtual ~D3D11DeviceResource();

            // Notify the renderers that device resources need to be released.
            // This ensures all references to the existing swap chain are released so that a new one can be created.
            virtual void OnDeviceLost(CD3D11Driver* device) = 0;

            // Notify the renderers that resources can now be created again.
            virtual void OnDeviceRestored(CD3D11Driver* device) = 0;

            // Mark the GPU resource destroyed on context destruction.
            //virtual void OnDeviceDestroy(CD3D11Driver* device) = 0;

            CD3D11Driver* GetDriver() const { return Driver; }

        protected:

            CD3D11Driver* Driver;
        };
    }
}

#endif //!__C_D3D11_RESOURCE_H_INCLUDED__