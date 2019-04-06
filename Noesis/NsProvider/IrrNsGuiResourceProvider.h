#ifndef __GUI_IRRRESOURCEPROVIDER_H__
#define __GUI_IRRRESOURCEPROVIDER_H__

#include <NsGui/XamlProvider.h>
#include <NsGui/CachedFontProvider.h>
#include <NsProvider/FileTextureProvider.h>

namespace NoesisApp
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// A provider for XAMLs embeded in memory
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class EmbeddedXamlProvider : public Noesis::XamlProvider
    {
    public:
        EmbeddedXamlProvider(XamlProvider* fallback = 0);

    private:
        /// From XamlProvider
        //@{
        Noesis::Ptr<Noesis::Stream> LoadXaml(const char* uri) override;
        //@}

    private:

        Noesis::Ptr<XamlProvider> mFallback;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    ///  A provider for fonts embeded in memory
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class EmbeddedFontProvider : public Noesis::CachedFontProvider
    {
    public:
        EmbeddedFontProvider(FontProvider* fallback = 0);

    private:
        /// From FontProvider
        //@{
        Noesis::FontSource MatchFont(const char* baseUri, const char* familyName,
            Noesis::FontWeight weight, Noesis::FontStretch stretch, Noesis::FontStyle style) override;
        bool FamilyExists(const char* baseUri, const char* familyName) override;
        //@}

        /// From CachedFontProvider
        //@{
        void ScanFolder(const char* folder) override;
        Noesis::Ptr<Noesis::Stream> OpenFont(const char* folder, const char* filename) const override;
        //@}

    private:

        Noesis::Ptr<FontProvider> mFallback;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// A provider for textures embeded in memory
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class EmbeddedTextureProvider : public NoesisApp::FileTextureProvider
    {
    public:
        EmbeddedTextureProvider(TextureProvider* fallback = 0);

    protected:
        /// From TextureProvider
        //@{
        Noesis::TextureInfo GetTextureInfo(const char* uri) override;
        Noesis::Ptr<Noesis::Texture> LoadTexture(const char* uri, Noesis::RenderDevice* device) override;

		// Inherited via FileTextureProvider
		virtual Noesis::Ptr<Noesis::Stream> OpenStream(const char* uri) const override;
    private:

        Noesis::Ptr<TextureProvider> mFallback;

	};
}

#endif
