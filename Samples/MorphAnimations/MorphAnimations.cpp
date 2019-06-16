#include <Ogre.h>
//To create workspace definitions and workspaces
#include <Compositor/OgreCompositorManager2.h>
//To use the hlms
#include <Hlms/Pbs/OgreHlmsPbs.h>
#include <OgreHlms.h>
//To load Hlms
#include <OgreArchive.h>
//To use objects
#include <OgreItem.h>
#include <OgreMesh2.h>
#include <OgreSubMesh2.h>
#include <Hlms/Pbs/OgreHlmsPbsDatablock.h>
//To play animations
#include <Animation/OgreSkeletonAnimation.h>
//To use smart pointers
#include <memory>

//The library we are trying out in this program
#include <Ogre_glTF.hpp>

#ifdef _DEBUG
const char GL_RENDER_PLUGIN[] = "RenderSystem_GL3Plus_d";
#else
const char GL_RENDER_PLUGIN[] = "RenderSystem_GL3Plus";
#endif

#ifdef _WIN32
#ifdef _DEBUG
const char D3D11_RENDER_PLUGIN[] = "RenderSystem_Direct3D11_d";
#else
const char D3D11_RENDER_PLUGIN[] = "RenderSystem_Direct3D11";
#endif
#endif

#define Ogre_glTF_STATIC
#ifdef Ogre_glTF_STATIC
#include <RenderSystems/GL3Plus/OgreGL3PlusPlugin.h>
#endif

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#define main() WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
#define main() main(int argc, char* argv[])
#endif

void declareHlmsLibrary(Ogre::String dataFolder)
{
	//Make sure the string we got is a valid path
	if(dataFolder.empty())
		dataFolder = "./";
	else if(dataFolder[dataFolder.size() - 1] != '/')
		dataFolder += '/';

	//For retrieval of the paths to the different folders needed
	Ogre::String dataFolderPath;
	Ogre::StringVector libraryFoldersPaths;

#ifdef USE_UNLIT //We are loading materials based on physics
	//Get the path to all the subdirectories used by HlmsUnlit
	Ogre::HlmsUnlit::getDefaultPaths(dataFolderPath, libraryFoldersPaths);

	//Create the Ogre::Archive objects needed
	Ogre::Archive* archiveUnlit = Ogre::ArchiveManager::getSingletonPtr()->load(dataFolder + dataFolderPath, "FileSystem", true);
	Ogre::ArchiveVec archiveUnlitLibraryFolders;
	for(const auto& libraryFolderPath : libraryFoldersPaths)
	{
		Ogre::Archive* archiveLibrary = Ogre::ArchiveManager::getSingletonPtr()->load(dataFolder + libraryFolderPath, "FileSystem", true);
		archiveUnlitLibraryFolders.push_back(archiveLibrary);
	}

	//Create and register the unlit Hlms
	Ogre::HlmsUnlit* hlmsUnlit = OGRE_NEW Ogre::HlmsUnlit(archiveUnlit, &archiveUnlitLibraryFolders);
	Ogre::Root::getSingleton().getHlmsManager()->registerHlms(hlmsUnlit);
	hlmsUnlit->setDebugOutputPath(false, false);
#endif

	//Do the same for HlmsPbs:
	Ogre::HlmsPbs::getDefaultPaths(dataFolderPath, libraryFoldersPaths);
	Ogre::Archive* archivePbs = Ogre::ArchiveManager::getSingletonPtr()->load(dataFolder + dataFolderPath, "FileSystem", true);

	//Get the library archive(s)
	Ogre::ArchiveVec archivePbsLibraryFolders;
	for(const auto& libraryFolderPath : libraryFoldersPaths)
	{
		Ogre::Archive* archiveLibrary = Ogre::ArchiveManager::getSingletonPtr()->load(dataFolder + libraryFolderPath, "FileSystem", true);
		archivePbsLibraryFolders.push_back(archiveLibrary);
	}

	//Create and register
	Ogre::HlmsPbs* hlmsPbs = OGRE_NEW Ogre::HlmsPbs(archivePbs, &archivePbsLibraryFolders);
	Ogre::Root::getSingleton().getHlmsManager()->registerHlms(hlmsPbs);
	hlmsPbs->setDebugOutputPath(false, false);
}

int main()
{
	//Init Ogre
	auto root = std::make_unique<Ogre::Root>();
	Ogre::LogManager::getSingleton().setLogDetail(Ogre::LoggingLevel::LL_BOREME);

#ifdef Ogre_glTF_STATIC
#if __linux__
    Ogre::String renderSystemName("GL");
    auto glPlugin = std::unique_ptr<Ogre::GL3PlusPlugin>(OGRE_NEW Ogre::GL3PlusPlugin());
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

	declareHlmsLibrary("./Media/");

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
		subItem->setPoseWeight(0, (Ogre::Math::Sin(accumulator * 1.4) + 1) / 2);

		root->renderOneFrame();
		Ogre::WindowEventUtilities::messagePump();
	}

	return 0;
}
