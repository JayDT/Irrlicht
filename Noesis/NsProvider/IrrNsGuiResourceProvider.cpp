#include "IrrNsGuiResourceProvider.h"
#include "System/Resource/ResourceManager.h"
#include <System/Uri.h>
#include <NsCore/MemoryStream.h>
#include <NsCore/StringUtils.h>
#include <NsRender/Texture.h>

NoesisApp::EmbeddedXamlProvider::EmbeddedXamlProvider(XamlProvider* fallback)
{
}

Noesis::Ptr<Noesis::Stream> NoesisApp::EmbeddedXamlProvider::LoadXaml(const char* uri)
{
	System::URI uriAddress;
	uriAddress.Parse(uri, false);

	// Two case: 1) noesis wrap header, 2) raw os file location (never use)
	if (uriAddress.GetDescriptor().m_scheme.empty())
		uriAddress.Parse(System::String::format("pack://application:,,,/%s", uri), false);

	if (uriAddress.GetDescriptor().m_scheme == "pack")
	{
		System::Resource::ResourceObject* resource = System::Resource::ResourceManager::Instance()->FindResource(uriAddress);
		if (resource)
			return *new Noesis::MemoryStream(resource->ToBytes().data(), resource->ToBytes().size());
	}
	//else if (uriAddress.GetDescriptor().m_scheme == "file")
	//{
	//
	//}
	return Noesis::Ptr<Noesis::Stream>();
}

NoesisApp::EmbeddedFontProvider::EmbeddedFontProvider(FontProvider* fallback)
{
}

Noesis::FontSource NoesisApp::EmbeddedFontProvider::MatchFont(const char* baseUri, const char* familyName, Noesis::FontWeight weight, Noesis::FontStretch stretch, Noesis::FontStyle style)
{
	Noesis::FontSource match = Noesis::CachedFontProvider::MatchFont(baseUri, familyName, weight, stretch, style);
	if (match.file == 0 && mFallback != 0)
	{
		match = mFallback->MatchFont(baseUri, familyName, weight, stretch, style);
	}

	return match;
}

bool NoesisApp::EmbeddedFontProvider::FamilyExists(const char* baseUri, const char* familyName)
{
	bool exists = Noesis::CachedFontProvider::FamilyExists(baseUri, familyName);
	if (!exists && mFallback != 0)
	{
		exists = mFallback->FamilyExists(baseUri, familyName);
	}

	return exists;
}

void NoesisApp::EmbeddedFontProvider::ScanFolder(const char* folder)
{
	System::URI uriAddress(folder, false);
	if (uriAddress.GetDescriptor().m_scheme == "pack")
	{
		auto filelist = System::Resource::ResourceManager::Instance()->GetFileList(System::String::format("%s.*\\.ttf", uriAddress.GetDescriptor().m_path.LocalPath.c_str()), uriAddress.GetDescriptor().m_path.Assembly);

		for (const auto& file : filelist)
		{
			size_t forwardslash = file.find_last_of('/');
			if (forwardslash != std::string::npos)
			{
				std::string plainFileName = file.substr(forwardslash + 1);
				RegisterFont(folder, plainFileName.c_str());
			}
		}
	}
}

Noesis::Ptr<Noesis::Stream> NoesisApp::EmbeddedFontProvider::OpenFont(const char* folder, const char* filename) const
{
	char buffer[1024];
	snprintf(buffer, sizeof(buffer), "%s/%s", folder, filename);

	System::URI uriAddress(buffer, false);
	if (uriAddress.GetDescriptor().m_scheme == "pack")
	{
		System::Resource::ResourceObject* resource = System::Resource::ResourceManager::Instance()->FindResource(uriAddress);
		if (resource)
			return *new Noesis::MemoryStream(resource->ToBytes().data(), resource->ToBytes().size());
	}
	return Noesis::Ptr<Noesis::Stream>();
}

NoesisApp::EmbeddedTextureProvider::EmbeddedTextureProvider(TextureProvider* fallback)
{
}

Noesis::TextureInfo NoesisApp::EmbeddedTextureProvider::GetTextureInfo(const char* uri)
{
	Noesis::TextureInfo info = FileTextureProvider::GetTextureInfo(uri);
	if (info.width == 0 && info.height == 0 && mFallback != 0)
	{
		info = mFallback->GetTextureInfo(uri);
	}

	return info;
}

Noesis::Ptr<Noesis::Texture> NoesisApp::EmbeddedTextureProvider::LoadTexture(const char* uri, Noesis::RenderDevice* device)
{
	Noesis::Ptr<Noesis::Texture> texture = FileTextureProvider::LoadTexture(uri, device);
	if (texture == 0 && mFallback != 0)
	{
		texture = mFallback->LoadTexture(uri, device);
	}
	
	return texture;
}

Noesis::Ptr<Noesis::Stream> NoesisApp::EmbeddedTextureProvider::OpenStream(const char* uri) const
{
	System::URI uriAddress(uri, false);
	if (uriAddress.GetDescriptor().m_scheme == "pack")
	{
		System::Resource::ResourceObject* resource = System::Resource::ResourceManager::Instance()->FindResource(uriAddress);
		if (resource)
			return *new Noesis::MemoryStream(resource->ToBytes().data(), resource->ToBytes().size());
	}
	return Noesis::Ptr<Noesis::Stream>();
}
