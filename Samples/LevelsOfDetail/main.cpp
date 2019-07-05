#include "SamplesCommon.h"
#include "Ogre_glTF_common.hpp"

#include <OgreLodStrategyManager.h>

int main()
{
#ifdef Ogre_glTF_STATIC
	// Must instantiate before Root so that it'll be destroyed afterwards. 
	// Otherwise we get a crash on Ogre::Root::shutdownPlugins()
#if __linux__
    auto glPlugin = std::make_unique<Ogre::GL3PlusPlugin>();
#endif
#endif

	//Init Ogre
	auto root = std::make_unique<Ogre::Root>();
	Ogre::LogManager::getSingleton().setLogDetail(Ogre::LoggingLevel::LL_BOREME);

#ifdef Ogre_glTF_STATIC
#if __linux__
    root->installPlugin(glPlugin.get());
#endif
#else
	root->loadPlugin(GL_RENDER_PLUGIN);
#ifdef _WIN32
	root->loadPlugin(D3D11_RENDER_PLUGIN);
#endif
#endif
	root->showConfigDialog();
	root->getRenderSystem()->setConfigOption("FSAA", "16");
	root->getRenderSystem()->setConfigOption("sRGB Gamma Conversion", "Yes");
	root->initialise(false);

	//Create a window and a scene
	Ogre::NameValuePairList params;
	params["FSAA"]	= "16";
	const auto window = root->createRenderWindow("glTF test!", 800, 600, false, &params);

	auto smgr = root->createSceneManager(Ogre::ST_GENERIC, 0, Ogre::INSTANCING_CULLING_THREADED);
	smgr->showBoundingBoxes(true);
	smgr->setDisplaySceneNodes(true);
	auto camera = smgr->createCamera("cam");

	//Setup rendering pipeline
	auto compositor			   = root->getCompositorManager2();
	const char workspaceName[] = "workspace0";
	compositor->createBasicWorkspaceDef(workspaceName, Ogre::ColourValue { 0.2f, 0.3f, 0.4f });
	auto workspace = compositor->addWorkspace(smgr, window, camera, workspaceName, true);

	DeclareHlmsLibrary("./Media");

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("../Media/gltfFiles.zip", "Zip");
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation("../Media/", "FileSystem");
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(true);

	auto gltf = std::make_unique<Ogre_glTF::glTFLoader>();
	Ogre::SceneNode* objectNode = nullptr;

	try
	{
		auto adapter = gltf->loadGlbResource("sphere_lod.glb");
		objectNode = adapter.getFirstSceneNode(smgr);
	}
	catch(std::exception& e)
	{
		Ogre::LogManager::getSingleton().logMessage(e.what());
		return -1;
	}

	camera->setNearClipDistance(0.001f);
	camera->setFarClipDistance(100);
	camera->setPosition(3.5f, 0.0f, 0.0f);
	camera->lookAt({ 0, 0, 0 });
	camera->setAutoAspectRatio(true);

	auto light = smgr->createLight();
	smgr->getRootSceneNode()->createChildSceneNode()->attachObject(light);
	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection(Ogre::Vector3 { -1, -1, -0.5f });
	light->setPowerScale(5);

	light = smgr->createLight();
	smgr->getRootSceneNode()->createChildSceneNode()->attachObject(light);
	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection(Ogre::Vector3 { +1, +1, +0.5f });
	light->setPowerScale(5);

    //Ogre::LodStrategyManager::getSingleton().setDefaultStrategy("screen_ratio_pixel_count");

	auto last = root->getTimer()->getMilliseconds();
	auto now  = last;
	Ogre::Real accumulator = 0;

	while(!window->isClosed())
	{
		now = root->getTimer()->getMilliseconds();
		accumulator += (now - last) / 1000.0f;
		last = now;

        objectNode->setPosition(Ogre::Math::Sin(accumulator) * 2.f, 0, 0);

		root->renderOneFrame();
		Ogre::WindowEventUtilities::messagePump();
	}

	return 0;
}