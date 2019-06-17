#pragma once

#include <memory>
#include <Ogre.h>
#include <OgreItem.h>
#include "Ogre_glTF_DLL.hpp"

namespace Ogre_glTF
{

	//Forward declare main class
	class glTFLoader;

	///Structure that contains the information of the model that you loaded
	struct ModelInformation
	{
		///Easy method to build an item to use
		/// \param smgr pointer to the scene manager where to create the item
		/// \param sceneType tell if it will be static or dynamic
		Ogre::Item* makeItem(Ogre::SceneManager* smgr, Ogre::SceneMemoryMgrTypes sceneType = Ogre::SCENE_DYNAMIC)
		{
			auto item = smgr->createItem(mesh, sceneType);
			for(size_t i = 0; i < item->getNumSubItems(); ++i) { item->getSubItem(i)->setDatablock(pbrMaterialList[i]); }
			return item;
		}

		///Smart pointer to the loaded mesh
		Ogre::MeshPtr mesh;
		///List of materials that correspond to each of the submeshes in the model
		std::vector<Ogre::HlmsDatablock*> pbrMaterialList;

		///Local transform on the glTF node this model came from
		struct ModelTransform
		{
			Ogre::Vector3 position		 = Ogre::Vector3::ZERO;
			Ogre::Vector3 scale			 = Ogre::Vector3::UNIT_SCALE;
			Ogre::Quaternion orientation = Ogre::Quaternion::IDENTITY;

			void apply (Ogre::SceneNode* node)
			{
				node->setPosition(position);
				node->setScale(scale);
				node->setOrientation(orientation);
			}
		} transform;
	};

	///Plugin accessible interface that plugin users can use
	struct glTFLoaderInterface
	{
		///Polymorphic dtor
		virtual ~glTFLoaderInterface() = default;

		///location where to load the data
		enum class LoadFrom { FileSystem, ResourceManager };
	};

	///Class that hold the loaded content of a glTF file and that can create Ogre objects from it
	class Ogre_glTF_EXPORT loaderAdapter
	{
		friend class glTFLoader;

		///opaque content of the class
		struct impl;

		///pointer to implementation
		std::unique_ptr<impl> pimpl;

		std::string adapterName;

	public:
		///This will also initialize the "pimpl" structure
		loaderAdapter();

		///This clear the pimpl structure
		~loaderAdapter();

		///Deleted copy constructor : non copyable class
		loaderAdapter(const loaderAdapter&) = delete;

		///Deleted assignment constructor : non copyable class
		loaderAdapter& operator=(const loaderAdapter&) = delete;

		///Return one of the datablocks loaded from the gltf file.
		///\param index of the datablocks used. In a multi-material file, it should be one different per primitive (submesh).
		Ogre::HlmsDatablock* getDatablock(size_t index = 0) const;

		///Return the number of datablock stored
		size_t getDatablockCount();

		///Construct an item for this object
		/// \param smgr pointer to the scene manager where we are creating the item
		Ogre::SceneNode* getFirstSceneNode(Ogre::SceneManager* smgr) const;

		Ogre::SceneNode* getSceneNode(size_t index, Ogre::SceneNode* parentSceneNode, Ogre::SceneManager* smgr) const;

		void createTagPoints(int boneIndex, Ogre::SkeletonInstance* skeletonInstance, Ogre::SceneManager* smgr) const;

		///Move constructor : object is movable
		/// \param other object to move
		loaderAdapter(loaderAdapter&& other) noexcept;

		///Move assignment operator
		loaderAdapter& operator=(loaderAdapter&& other) noexcept;

		///Return the current state of the adapter
		bool isOk() const;

		///Return the last error generated by the underlying glTF loading library
		std::string getLastError() const;
	};

	///Class that is responsible for initializing the library with the loader, and giving out
	class Ogre_glTF_EXPORT glTFLoader final : public glTFLoaderInterface
	{
		///object that acutally communicate with the underlying glTF loading library
		struct glTFLoaderImpl;

		///Opaque pointer that handle the underlying glTF loading library (pimpl)
		std::unique_ptr<glTFLoaderImpl> loaderImpl;

	public:
		///Initialize the library by creating this object.
		glTFLoader();

		///Move constructor
		/// \param other object to move
		glTFLoader(glTFLoader&& other) noexcept;

		///Move assign operator
		glTFLoader& operator=(glTFLoader&& other) noexcept;

		///Deinitialize the library at this object destruction
		~glTFLoader();

		///Load a glTF text or binary file. Give you an adapter to use this file with Ogre
		/// \param path String containing the path to a file to load (either .glTF or .glc)
		loaderAdapter loadFromFileSystem(const std::string& path) const;

		///Load a GLB from Ogre's resource manager
		loaderAdapter loadGlbResource(const std::string& name) const;

		///Deleted copy constructor
		glTFLoader(const glTFLoader&) = delete;

		///Deleted assignment operator
		glTFLoader& operator=(const glTFLoader&) = delete;
	};

	class Exception : public Ogre::Exception
	{
	public:
		Exception(int number, const std::string& desc, const std::string& src) : Ogre::Exception(number, "gltf exception: " + desc, "gltf: " + src) {}
	};

	class FileIOError : public Exception
	{
	public:
		FileIOError(const std::string& message = "", const std::string& src = "") : Exception(ERR_FILE_NOT_FOUND, "couldn't open file: " + message, src) {}
	};

	class RootNotInitializedYet : public Exception
	{
	public:
		RootNotInitializedYet(const std::string& message = "", const std::string& src = "") : Exception(ERR_INVALID_CALL, message, src) {}
	};

	class LoadingError : public Exception
	{
	public:
		LoadingError(const std::string& message, const std::string& src = "") : Exception(ERR_INTERNAL_ERROR, "while loading: " + message, src) {}
	};

	class InitError : public Exception
	{
	public:
		InitError(const std::string& message, const std::string& src = "") : Exception(ERR_INVALID_STATE, message, src) {}
	};
}

//To facilitate the use of the library:
#include "Ogre_glTF_OgreResource.hpp"
#include "Ogre_glTF_OgrePlugin.hpp"
