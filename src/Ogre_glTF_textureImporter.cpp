#include "Ogre_glTF_textureImporter.hpp"
#include <OgreLogManager.h>
#include <OgreTextureManager.h>
#include <OgreTexture.h>
#include <OgreImage.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreRenderTexture.h>
#include <OgreColourValue.h>

size_t Ogre_glTF_textureImporter::id{ 0 };

inline void OgreLog(const std::string& message)
{
	Ogre::LogManager::getSingleton().logMessage(message);
}

void Ogre_glTF_textureImporter::loadTexture(const tinygltf::Texture& texture)
{
	auto textureManager = Ogre::TextureManager::getSingletonPtr();
	const auto& image = model.images[texture.source];
	const auto name = "glTF_texture_" + image.name + std::to_string(id) + std::to_string(texture.source);
	OgreLog("Loading image " + name);

	const auto pixelFormat = [&]
	{
		if (image.component == 3)
			return Ogre::PF_B8G8R8;
		if (image.component == 4)
			return Ogre::PF_B8G8R8A8;

		//TODO do this properly. Right now it is guesswork

		OgreLog("unrecognized pixel format from tinygltf image");
	}();

	if (image.image.size() / image.component == image.width*image.height)
	{
		OgreLog("It looks like the image.component field and the image size does match");
	}
	else
	{
		OgreLog("I have no idea what is going on with the image format");
	}

	Ogre::Image OgreImage;
	Ogre::TexturePtr OgreTexture;

	//The OgreImage class can take ownership of the pointer to the data and automatically delete it.
	//We don't want that.
	//The rest of the funciton is not modifying the model object. We get the image as a const ref.
	//In order to keep the rest of this code const correct, and knowing that the "autoDelete" is specifically
	//set to `false`, we're casting away const on the pointer to the image data
	OgreImage.loadDynamicImage(const_cast<Ogre::uchar*>(image.image.data()),
		image.width, image.height, 1, pixelFormat, false);

	OgreTexture = textureManager->createManual(name,
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		Ogre::TextureType::TEX_TYPE_2D_ARRAY, image.width, image.height,
		1, 1, pixelFormat, Ogre::TU_DEFAULT
	);

	OgreTexture->loadImage(OgreImage);

	loadedTextures.insert({ texture.source, OgreTexture });
}

Ogre_glTF_textureImporter::Ogre_glTF_textureImporter(tinygltf::Model& input) :
	model{ input }
{
	id++;
}

void Ogre_glTF_textureImporter::loadTextures()
{
	for (const auto& texture : model.textures)
	{
		loadTexture(texture);
	}
}

Ogre::TexturePtr Ogre_glTF_textureImporter::getTexture(int glTFTextureSourceID)
{
	auto texture = loadedTextures.find(glTFTextureSourceID);
	if (texture == std::end(loadedTextures)) return { };

	return texture->second;
}

Ogre::TexturePtr Ogre_glTF_textureImporter::generateGreyScaleFromChannel(int gltfTextureSourceID, int channel)
{
	auto textureManager = Ogre::TextureManager::getSingletonPtr();
	const auto& image = model.images[gltfTextureSourceID];

	assert(channel < 4 && channel >= 0 /*, "Channel needs to be between 0 and 3"*/);
	const auto name = "glTF_texture_" + image.name + std::to_string(id) + std::to_string(gltfTextureSourceID) + "_geyscale_channel" + std::to_string(channel);

	auto texture = textureManager->getByName(name);
	if (texture) return texture;

	OgreLog("Can't find texure " + name + ". Generating it from glTF");

	assert(channel < image.component);

	//Greyscale the image by putting all channel to the same value, ignoring alpha
	std::vector<Ogre::uchar> imageData(image.image.size());
	const auto pixelCount{ imageData.size() / image.component };
	for (size_t i{ 0 }; i < pixelCount; i++)//for each pixel
	{
		//Get the channel that has the value
		Ogre::uchar grey = image.image[(i * image.component) + channel];

		//Turn pixel at this specific shade of grey
		for (size_t c{ 0 }; c < 3; c++)
			imageData[i * image.component + c] = grey;

		//If there's an alpha channel, put it to 1.0f (255)
		if (image.component > 3)
			imageData[i*image.component + 3] = 255;
	}

	const auto pixelFormat = [&]
	{
		if (image.component == 3)
			return Ogre::PF_B8G8R8;
		if (image.component == 4)
			return Ogre::PF_B8G8R8A8;

		//TODO do this properly. Right now it is guesswork

		OgreLog("unrecognized pixel format from tinygltf image");
	}();

	Ogre::Image OgreImage;
	Ogre::TexturePtr OgreTexture;

	//TODO refactor this

	//The OgreImage class can take ownership of the pointer to the data and automatically delete it.
	//We don't want that.
	//The rest of the funciton is not modifying the model object. We get the image as a const ref.
	//In order to keep the rest of this code const correct, and knowing that the "autoDelete" is specifically
	//set to `false`, we're casting away const on the pointer to the image data
	OgreImage.loadDynamicImage(imageData.data(),
		image.width, image.height, 1, pixelFormat, false);

	OgreTexture = textureManager->createManual(name,
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		Ogre::TextureType::TEX_TYPE_2D_ARRAY, image.width, image.height,
		1, 1, pixelFormat, Ogre::TU_DEFAULT
	);
	OgreTexture->loadImage(OgreImage);
	return OgreTexture;
}

Ogre::TexturePtr Ogre_glTF_textureImporter::getNormalSNORM(int gltfTextureSourceID)
{
	auto textureManager = Ogre::TextureManager::getSingletonPtr();
	const auto& image = model.images[gltfTextureSourceID];
	const auto name = "glTF_texture_" + image.name + std::to_string(id) + std::to_string(gltfTextureSourceID) + "_NormalFixed";

	auto texture = textureManager->getByName(name);
	if (texture) return texture;

	OgreLog("Can't find texure " + name + ". Generating it from glTF");

	const auto pixelFormat = [&]
	{
		if (image.component == 3)
			return Ogre::PF_R8G8B8;
		if (image.component == 4)
			return Ogre::PF_R8G8B8A8;

		//TODO do this properly. Right now it is guesswork

		OgreLog("unrecognized pixel format from tinygltf image");
	}();

	const auto pixelFormatSnorm = [&]
	{
		if (image.component == 3)
			return Ogre::PF_R8G8B8_SNORM;
		if (image.component == 4)
			return Ogre::PF_R8G8B8A8_SNORM;
	}();

	Ogre::TexturePtr OgreTexture = textureManager->createManual(name,
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		Ogre::TextureType::TEX_TYPE_2D_ARRAY, image.width, image.height,
		1, 1, pixelFormatSnorm, Ogre::TU_DEFAULT
	);

	auto pixels = OgreTexture->getBuffer()->lock(
		{ 0, 0, unsigned(image.width), unsigned(image.height) }, //PixelBox that take the whole image
		Ogre::v1::HardwareBuffer::LockOptions::HBL_NORMAL);
	//This loop convert BGR to RGB image data while also putting the value in the SNORM range [-1.0; +1.0]
	for (size_t y{ 0 }; y < image.height; y++)
		for (size_t x{ 0 }; x < image.width; x++)
			pixels.setColourAt(Ogre::ColourValue(
				2.0f * (float(image.image[image.component * (y * image.width + x) + 2]) / 255.0f) - 1.0f, //R to B
				2.0f * (float(image.image[image.component * (y * image.width + x) + 1]) / 255.0f) - 1.0f, //G to G
				2.0f * (float(image.image[image.component * (y * image.width + x) + 0]) / 255.0f) - 1.0f, //B to R
				1.0f
			), x, y, 0);

	OgreTexture->getBuffer()->unlock();

	return OgreTexture;
}