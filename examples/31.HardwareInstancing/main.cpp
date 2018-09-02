#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
#endif

#include <irrlicht.h>
#include <driverChoice.h>

using namespace irr;

class CInstanceShaderCallback : public video::IShaderConstantSetCallBack
{
public:
	CInstanceShaderCallback(scene::ISceneManager* smgr)
		: smgr(smgr),
		firstUpdate(true), viewId(-1), projectionId(-1), worldId(-1)
	{
	}

	void OnSetConstants(video::IMaterialRendererServices* services, s32 userData)
	{
		if (firstUpdate)
		{
			viewId = services->getVertexShaderConstantID("viewMatrix");
			projectionId = services->getVertexShaderConstantID("projectionMatrix");
			worldId = services->getVertexShaderConstantID("worldMatrix");

			firstUpdate = false;
		}

		core::matrix4 view = services->getVideoDriver()->getTransform(video::ETS_VIEW);
		core::matrix4 projection = services->getVideoDriver()->getTransform(video::ETS_PROJECTION);
		core::matrix4 world = services->getVideoDriver()->getTransform(video::ETS_WORLD);

		services->setVertexShaderConstant(worldId, world.pointer(), 16);
		services->setVertexShaderConstant(viewId, view.pointer(), 16); 
		services->setVertexShaderConstant(projectionId, projection.pointer(), 16);		
	}

private:
	scene::ISceneManager* smgr;

	bool firstUpdate;

	s32 viewId;
	s32 projectionId;
	s32 worldId;
};

class MyEventReceiver : public IEventReceiver
{
public:

	MyEventReceiver(scene::ISceneNode* node, gui::IGUIElement* text) :
		node(node), text(text), enableCulling(true), showDebug(false), showSphereMesh(true)
	{
	}

	bool OnEvent(const SEvent& event)
	{
		// check if user presses the key 'W' or 'D'
		if (event.EventType == irr::EET_KEY_INPUT_EVENT && !event.KeyInput.PressedDown)
		{
			switch (event.KeyInput.Key)
			{
			case irr::KEY_KEY_W: // switch wire frame mode
				node->setMaterialFlag(video::EMF_WIREFRAME,
					!node->getMaterial(0).Wireframe);
				node->setMaterialFlag(video::EMF_POINTCLOUD, false);
				updateText();
				return true;
			case irr::KEY_KEY_P: // switch wire frame mode
				node->setMaterialFlag(video::EMF_POINTCLOUD,
					!node->getMaterial(0).PointCloud);
				node->setMaterialFlag(video::EMF_WIREFRAME, false);
				updateText();
				return true;
			case irr::KEY_KEY_X: // toggle debug information
				showDebug = !showDebug;
				node->setDebugDataVisible(showDebug ? scene::EDS_BBOX_ALL : scene::EDS_OFF);
				updateText();
				return true;
			case irr::KEY_KEY_C: // toggle culling mode
				enableCulling = !enableCulling;
				node->setAutomaticCulling(enableCulling ? scene::EAC_BOX : scene::EAC_OFF);
				updateText();
				return true;
			case irr::KEY_KEY_M: // toggle mesh
			{
				scene::IMesh* mesh = NULL;
				if (showSphereMesh)
					mesh = node->getSceneManager()->getGeometryCreator()->createCubeMesh(core::vector3df(1.f));
				else
					mesh = node->getSceneManager()->getGeometryCreator()->createSphereMesh(0.5f);

				showSphereMesh = !showSphereMesh;

				((scene::IInstancedMeshSceneNode*)node)->setMesh(mesh);

				mesh->drop();

				return true;
			}
			default:
				break;
			}
		}

		return false;
	}

private:

	void updateText()
	{
		wchar_t buff[255];
		swprintf(buff, L"Press 'W' to change wireframe mode (%s)\nPress 'X' to toggle debug mode (%s)\nPress 'C' to toggle culling mode (%s)\nPress 'M' to toogle mesh",
			node->getMaterial(0).Wireframe ? L"ON" : L"OFF",
			showDebug ? L"ON" : L"OFF",
			enableCulling ? L"ON" : L"OFF");
		text->setText(buff);
	}

	scene::ISceneNode* node;
	gui::IGUIElement* text;

	bool showDebug;
	bool enableCulling;
	bool showSphereMesh;
};

int main()
{
	video::E_DRIVER_TYPE driverType = driverChoiceConsole();
	if (driverType == video::EDT_COUNT)
		return 1;

	bool animate = false;
	char i;

	printf("Please press 'y' if you want each instance to be animated.\n");
	std::cin >> i;
	if (i == 'y')
	{
		animate = true;
	}
	// create device and exit if creation failed

	IrrlichtDevice * device = createDevice(driverType, core::dimension2d<u32>(800, 600), 32);

	if (device == 0)
		return 1; // could not create selected driver.

	/* The creation was successful, now we set the event receiver and
	store pointers to the driver and to the gui environment. */


	device->setWindowCaption(L"Irrlicht Engine - Hardware Instancing Demo");

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();
	gui::IGUIEnvironment* env = device->getGUIEnvironment();

	// add irrlicht logo
	env->addImage(driver->getTexture("../../media/irrlichtlogo2.png"),
		core::position2d<s32>(10, 10));

	//set other font
	env->getSkin()->setFont(env->getFont("../../media/fontlucida.png"));

	// add some help text
	gui::IGUIElement* text = env->addStaticText(
		L"Press 'W' to change wireframe mode (OFF)\nPress 'X' to toggle debug mode (OFF)\nPress 'C' to toggle culling mode (ON)\nPress 'M' to toogle mesh",
		core::rect<s32>(10, 421, 275, 495), true, true, 0, -1, true);

	CInstanceShaderCallback* callback = new CInstanceShaderCallback(smgr);

	video::IGPUProgrammingServices* service = smgr->getVideoDriver()->getGPUProgrammingServices();

	s32 newMaterialType = -1;

	switch (driverType)
	{
	case video::EDT_OPENGL:
		newMaterialType = service->addHighLevelShaderMaterialFromFiles("../../media/shaders/instancing.vert", "", video::EVST_VS_2_0,
			"../../media/shaders/instancing.frag", "", video::EPST_PS_2_0, callback, video::EMT_SOLID);
		break;
	case video::EDT_DIRECT3D9:
	case video::EDT_DIRECT3D11:
		newMaterialType = service->addHighLevelShaderMaterialFromFiles("../../media/InstancingFVF.hlsl", "vsmain", video::EVST_VS_3_0,
			"../../media/InstancingFVF.hlsl", "psmain", video::EPST_PS_3_0, callback, video::EMT_SOLID);
		break;
	default:
		printf("Only DirectX 9/11 and OpenGL are supported.\n");
		break;
	}

	callback->drop();

	scene::IMesh* mesh = smgr->getGeometryCreator()->createSphereMesh(0.5f);
	
	scene::IInstancedMeshSceneNode* inode = smgr->addInstancedMeshSceneNode(mesh);
	inode->setAutomaticCulling(scene::EAC_FRUSTUM_BOX);

	// with that the material setting is IMesh independent 
	// otherwise if the mesh changes we have to set the material flags again
	inode->setReadOnlyMaterials(true);

	inode->getMaterial(0).Lighting = false;
	inode->getMaterial(0).setTexture(0, smgr->getVideoDriver()->getTexture("../../media/earth.jpg"));
	inode->getMaterial(0).MaterialType = (video::E_MATERIAL_TYPE)newMaterialType;

	mesh->drop();

	for (u32 x = 0; x < 10; ++x)
	{
		for (u32 y = 0; y < 10; ++y)
		{
			for (u32 z = 0; z < 2; ++z)
			{
				scene::ISceneNode* empty = inode->addInstance(core::vector3df((f32)x, (f32)y, (f32)z), core::vector3df((f32)(rand() % 360), (f32)(rand() % 360), (f32)(rand() % 360)), core::vector3df((f32)(rand() % 10 + 1)));

				if (animate)
				{
					scene::ISceneNodeAnimator* anim = smgr->createFlyCircleAnimator(empty->getPosition(), (f32)(rand() % 200), rand()*0.0000001f);

					if (anim)
					{
						empty->addAnimator(anim);
						anim->drop();
					}
				}
			}
		}
	}

	scene::ICameraSceneNode* cam = smgr->addCameraSceneNodeFPS(NULL, 50.f, 0.1f);
	cam->setFarValue(3000);

	device->getCursorControl()->setVisible(false);

	// create event receiver
	MyEventReceiver receiver(inode, text);
	device->setEventReceiver(&receiver);

	s32 lastFPS = -1;
	while (device->run())
	{
		if (device->isWindowActive())
		{
			driver->beginScene(true, true, video::SColor(255, 122, 122, 122));

			smgr->drawAll();
			env->drawAll();

			driver->endScene();
			
			int fps = driver->getFPS();

			if (lastFPS != fps)
			{
				core::stringw str = L"fps: ";
				str += fps;
				str += ", poly: ";
				str += driver->getPrimitiveCountDrawn();
				str += ", instance count: ";
				str += inode->getInstanceCount();
				device->setWindowCaption(str.c_str());
				lastFPS = fps;
			}
		}
	}

	device->drop();

	return 0;
}