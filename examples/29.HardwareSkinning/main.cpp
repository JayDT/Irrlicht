/** Example 029 HardwareSkinning

This example shows how to create custom vertex format and use them
for hardware skinning.
*/
#include <irrlicht.h>
#include <iostream>
#include <string.h>
#include "driverChoice.h"

using namespace irr;

#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
#endif

// Callback for skinning shader.

class HardwareSkinningCallBack : public video::IShaderConstantSetCallBack
{
public:
	HardwareSkinningCallBack(scene::ISkinnedMesh* mesh) : Mesh(mesh), JointMatrix(0), WorldViewProjID(-1), BoneID(-1), TextureID(-1), FirstUpdate(true)
	{
		if (Mesh)
		{
			Mesh->grab();

			JointMatrix = new f32[Mesh->getJointCount() * 16];
		}
	}

	~HardwareSkinningCallBack()
	{
		if (Mesh)
		{
			Mesh->drop();

			delete[] JointMatrix;
		}
	}

	virtual void OnSetConstants(video::IMaterialRendererServices* services,
			s32 userData)
	{
		if (!Mesh)
			return;

		video::IVideoDriver* driver = services->getVideoDriver();

		if (FirstUpdate)
		{
			WorldViewProjID = services->getVertexShaderConstantID("uMVP");
			BoneID = services->getVertexShaderConstantID("uBone");

			if (driver->getDriverType() == video::EDT_OPENGL)
				TextureID = services->getPixelShaderConstantID("uTexture");

			FirstUpdate = false;
		}

		core::matrix4 worldViewProj;
		worldViewProj = driver->getTransform(video::ETS_PROJECTION);
		worldViewProj *= driver->getTransform(video::ETS_VIEW);
		worldViewProj *= driver->getTransform(video::ETS_WORLD);
		services->setVertexShaderConstant(WorldViewProjID, worldViewProj.pointer(), 16);

		u32 inc = 0;

		const u32 jointCount = Mesh->getJointCount();

		// Copy bone matrices (it shouldn't be copy in each frame, some kind of time delay should exist).

		for (u32 i = 0; i < jointCount; ++i)
		{
			const scene::ISkinnedMesh::SJoint* joint = Mesh->getAllJoints()[i];

			core::matrix4 matrix(core::matrix4::EM4CONST_NOTHING);
			matrix.setbyproduct(joint->GlobalAnimatedMatrix, joint->GlobalInversedMatrix);

			memcpy(JointMatrix + inc, matrix.pointer(), sizeof(f32) * 16);

			inc += 16;				
		}

		services->setVertexShaderConstant(BoneID, JointMatrix, jointCount * 16);

		if (driver->getDriverType() == video::EDT_OPENGL)
		{
			s32 TextureLayerID = 0;
			services->setPixelShaderConstant(TextureID, &TextureLayerID, 1);
		}
	}

private:
	scene::ISkinnedMesh* Mesh; // From this mesh we'll read bone matrices.
	f32* JointMatrix;

	s32 WorldViewProjID;
	s32 BoneID;
	s32 TextureID;

	bool FirstUpdate;
};

// New vertex format designed for hardware skinning (up to 4 indices and weights per vertex).

struct SSkinningVertex
{
	SSkinningVertex()
	{
		for (u32 i = 0; i < 4; ++i)
		{
			BlendWeight[i] = 0.f;
			BlendIndex[i] = 0.f;
		}
	}

	SSkinningVertex(const SSkinningVertex& other)
	{
		Pos = other.Pos;
		Normal = other.Normal;
		TCoords = other.TCoords;

		memcpy(BlendWeight, other.BlendWeight, sizeof(f32) * 4);
		memcpy(BlendIndex, other.BlendIndex, sizeof(f32) * 4);
	}

	SSkinningVertex& operator=(const SSkinningVertex& other)
	{
		Pos = other.Pos;
		Normal = other.Normal;
		TCoords = other.TCoords;

		memcpy(BlendWeight, other.BlendWeight, sizeof(f32) * 4);
		memcpy(BlendIndex, other.BlendIndex, sizeof(f32) * 4);

		return *this;
	}

	bool operator==(const SSkinningVertex& other) const
	{
		
		return ((Pos == other.Pos) && (Normal == other.Normal) && (TCoords == other.TCoords) &&
				memcmp(&BlendWeight, other.BlendWeight, sizeof(f32) * 4) == 0 &&
				memcmp(&BlendIndex, other.BlendIndex, sizeof(f32) * 4) == 0);
	}

	core::vector3df Pos;
	core::vector3df Normal;
	core::vector2d<f32> TCoords;
	f32 BlendWeight[4];
	f32 BlendIndex[4];
};

int main()
{
	bool UseHardwareSkinning = false;

	printf("Please press 'y' if you want to enable hardware skinning.\n");

	char c = ' ';
	std::cin >> c;
	
	if (c == 'y' || c == 'Y')
		UseHardwareSkinning = true;

	// ask user for driver.

	video::E_DRIVER_TYPE driverType = driverChoiceConsole();

	if (driverType == video::EDT_COUNT)
		return 1;

	// check if OpenGL or Direct3D9 driver was selected.

	if (UseHardwareSkinning && driverType != video::EDT_OPENGL && driverType != video::EDT_DIRECT3D9)
	{
		printf("Only OpenGL and Direct3D9 drivers support hardware skinning.\n");
		return 1;
	}

	// create device.

	IrrlichtDevice* device = createDevice(driverType, core::dimension2d<u32>(640, 480));

	if (device == 0)
		return 1;

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();
	gui::IGUIEnvironment* gui = device->getGUIEnvironment();

	// check if hight levels shaders are supported.

	if (UseHardwareSkinning && !driver->queryFeature(video::EVDF_HLSL) &&
		!driver->queryFeature(video::EVDF_ARB_GLSL))
	{
		printf("High level shaders support is require for hardware skinning.\n");
		device->drop();
		return 1;
	}

	// load mesh.

	scene::IAnimatedMesh* mesh = smgr->getMesh("../../media/dwarf.x");

	if (!mesh)
	{
		device->drop();
		return 1;
	}

	scene::ISkinnedMesh* skinMesh = (scene::ISkinnedMesh*)mesh; // x, b3d and ms3d are skinned meshes.

	// create hardware skinning shader.

	s32 SkinningMaterial = -1;

	if (UseHardwareSkinning)
	{
		video::IGPUProgrammingServices* gpu = driver->getGPUProgrammingServices();

		HardwareSkinningCallBack* mc = new HardwareSkinningCallBack(skinMesh);

		core::stringc vertexShader = "../../media/skinning";
		core::stringc fragmentShader = vertexShader;

		if (driverType == video::EDT_OPENGL)
		{
			vertexShader += "_v.glsl";
			fragmentShader += "_f.glsl";
		}
		else // EDT_DIRECT3D9
		{
			vertexShader += ".hlsl";
			fragmentShader += ".hlsl";
		}

		SkinningMaterial = gpu->addHighLevelShaderMaterialFromFiles(
					vertexShader, "vertexMain", video::EVST_VS_2_0,
					fragmentShader, "pixelMain", video::EPST_PS_2_0,
					mc, video::EMT_SOLID);

		mc->drop();

		// create new vertex format.

		video::IVertexDescriptor* vertexDescriptor = driver->addVertexDescriptor("Skinning");
	
		vertexDescriptor->addAttribute("inPosition", 3, video::EVAS_POSITION, video::EVAT_FLOAT, 0);
		vertexDescriptor->addAttribute("inNormal", 3, video::EVAS_NORMAL, video::EVAT_FLOAT, 0);
		vertexDescriptor->addAttribute("inTexCoord0", 2, video::EVAS_TEXCOORD0, video::EVAT_FLOAT, 0);
		vertexDescriptor->addAttribute("inBlendWeight", 4, video::EVAS_BLEND_WEIGHTS, video::EVAT_FLOAT, 0);
		vertexDescriptor->addAttribute("inBlendIndex", 4, video::EVAS_BLEND_INDICES, video::EVAT_FLOAT, 0);

		// convert vertices.

		for (u32 i = 0; i < skinMesh->getMeshBufferCount(); ++i)
			smgr->getMeshManipulator()->convertVertices<SSkinningVertex>(skinMesh->getMeshBuffer(i), vertexDescriptor, false);

		skinMesh->setHardwareSkinning(true);

		// fill blending data.

		for (u32 i = 0; i < skinMesh->getJointCount(); ++i)
		{
			for (u32 j = 0; j < skinMesh->getAllJoints()[i]->Weights.size(); ++j)
			{
				SSkinningVertex* Vertices = static_cast<SSkinningVertex*>(mesh->getMeshBuffer(skinMesh->getAllJoints()[i]->Weights[j].buffer_id)->getVertexBuffer(0)->getVertices());

				s32 id = -1;

				for (s32 k = 0; k < 4; ++k)
				{
					if (Vertices[skinMesh->getAllJoints()[i]->Weights[j].vertex_id].BlendWeight[k] == 0.f)
					{
						id = k;
						break;
					}
				}

				if (id >= 0)
				{
					Vertices[skinMesh->getAllJoints()[i]->Weights[j].vertex_id].BlendWeight[id] = skinMesh->getAllJoints()[i]->Weights[j].strength;
					Vertices[skinMesh->getAllJoints()[i]->Weights[j].vertex_id].BlendIndex[id] = float(i);
				}
			}
		}
	}

	// create 16 nodes.

	scene::IAnimatedMeshSceneNode* node = 0;

	for (u32 i = 0; i < 4; ++i)
		for (u32 j = 0; j < 4; ++j)
		{
			node = smgr->addAnimatedMeshSceneNode(skinMesh);
			node->setAnimationSpeed(f32(3 + i * 10 + j * 2));
			node->setPosition(core::vector3df(-105.f + 70.f * i, 0.f, -105.f + 70.f * j));
			node->setMaterialFlag(video::EMF_LIGHTING, false);

			if (UseHardwareSkinning)
				node->setMaterialType((video::E_MATERIAL_TYPE)SkinningMaterial);
		}

	// set the best hardware mapping flag.

	for (u32 i = 0; i < skinMesh->getMeshBufferCount(); ++i)
	{
		if (UseHardwareSkinning)
			skinMesh->getMeshBuffer(i)->setHardwareMappingHint(scene::EHM_STATIC);
		else
		{
			skinMesh->getMeshBuffer(i)->setHardwareMappingHint(scene::EHM_STREAM, scene::EBT_VERTEX);
			skinMesh->getMeshBuffer(i)->setHardwareMappingHint(scene::EHM_STATIC, scene::EBT_INDEX);
		}	
	}

	// others.

	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);

	smgr->addSkyBoxSceneNode(
		driver->getTexture("../../media/irrlicht2_up.jpg"),
		driver->getTexture("../../media/irrlicht2_dn.jpg"),
		driver->getTexture("../../media/irrlicht2_lf.jpg"),
		driver->getTexture("../../media/irrlicht2_rt.jpg"),
		driver->getTexture("../../media/irrlicht2_ft.jpg"),
		driver->getTexture("../../media/irrlicht2_bk.jpg"));

	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);

	scene::ICameraSceneNode* cam = smgr->addCameraSceneNode();
	cam->setPosition(core::vector3df(0.f, 120.f, -235.f));
	cam->setTarget(core::vector3df(0.f, -1.3f, -0.52f));

	int lastFPS = -1;

	while(device->run())
	if (device->isWindowActive())
	{
		driver->beginScene(true, true, video::SColor(255,0,0,0));
		smgr->drawAll();
		driver->endScene();

		int fps = driver->getFPS();

		if (lastFPS != fps)
		{
			core::stringw str = L"Irrlicht Engine - Hardware skinning example [";
			str += driver->getName();
			str += "] FPS:";
			str += fps;

			device->setWindowCaption(str.c_str());
			lastFPS = fps;
		}
	}

	device->drop();

	return 0;
}

