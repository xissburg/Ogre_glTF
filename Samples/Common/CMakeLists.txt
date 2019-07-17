set(SAMPLES_COMMON_HEADER_FILES SamplesCommon.h)
set(SAMPLES_COMMON_SOURCE_FILES SamplesCommon.cpp)

if (NOT Ogre_glTF_TinyGLTF_implementation)
    set(SAMPLES_COMMON_HEADER_FILES ${SAMPLES_COMMON_HEADER_FILES} ${CMAKE_SOURCE_DIR}/thirdParty/tinygltf/tiny_gltf.h)
endif ()