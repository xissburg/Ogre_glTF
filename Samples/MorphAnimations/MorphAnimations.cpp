#include "SamplesCommon.h"

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

	auto smgr = root->createSceneManager(Ogre::ST_GENERIC, 2, Ogre::INSTANCING_CULLING_THREADED);
	smgr->showBoundingBoxes(true);
	smgr->setDisplaySceneNodes(true);
	auto camera = smgr->createCamera("cam");

	//Setup rendering pipeline
	auto compositor			   = root->getCompositorManager2();
	const char workspaceName[] = "workspace0";
	compositor->createBasicWorkspaceDef(workspaceName, Ogre::ColourValue { 0.2f, 0.3f, 0.4f });
	auto workspace = compositor->addWorkspace(smgr, window, camera, workspaceName, true);

	DeclareHlmsLibrary("./Media/");

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("../Media/gltfFiles.zip", "Zip");
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(true);

	Ogre::SceneNode* blobNode = nullptr;
	Ogre::SceneNode* springNode = nullptr;

	//Initialize the library
	auto gltf = std::make_unique<Ogre_glTF::glTFLoader>();

	try
	{
		auto loader = gltf->loadFromFileSystem("../Media/blob.glb");
		blobNode = loader.getFirstSceneNode(smgr);

		auto springLoader = gltf->loadFromFileSystem("../Media/spring.glb");
		springNode = springLoader.getFirstSceneNode(smgr);
	}
	catch(std::exception& e)
	{
		Ogre::LogManager::getSingleton().logMessage(e.what());
		return -1;
	}

	springNode->translate(1, 0, 0);

	camera->setNearClipDistance(0.001f);
	camera->setFarClipDistance(100);
	camera->setPosition(2.5f, 0.5f, 2.5f);
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

	auto last = root->getTimer()->getMilliseconds();
	auto now  = last;
    auto accumulator = 0.f;

	while(!window->isClosed())
	{
		now = root->getTimer()->getMilliseconds();
		accumulator += float(now - last) / 1000.0f;
		last = now;

		auto item = static_cast<Ogre::Item*>(blobNode->getAttachedObject(0));
        auto subItem = item->getSubItem(0);
        for( int i = 0; i < subItem->getNumPoses(); ++i )
        {
            subItem->setPoseWeight(i, Ogre::Math::Sin(accumulator * (1 + i * 0.1) * 3 + i) * 0.27 );
        }

		item = static_cast<Ogre::Item*>(springNode->getAttachedObject(0));
		subItem = item->getSubItem(0);
		subItem->setPoseWeight("Compressed", (Ogre::Math::Sin(accumulator * 1.4) + 1) / 2);

		root->renderOneFrame();
		Ogre::WindowEventUtilities::messagePump();
	}

	return 0;
}
