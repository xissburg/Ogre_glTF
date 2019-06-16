file(GLOB MorphAnimations_Sources ./*.cpp ./*.hpp ./include/*.hpp)

add_executable(Ogre_glTF_MorphAnimations WIN32 MACOSX_BUNDLE ${MorphAnimations_Sources})

target_include_directories( Ogre_glTF_MorphAnimations PUBLIC
	#Ogre and the physics based high level material system
	${OGRE_INCLUDE_DIRS}
	${OGRE_HlmsPbs_INCLUDE_DIRS}
	${OGRE_INCLUDE_DIR}/Hlms/Common
)

target_link_libraries(Ogre_glTF_MorphAnimations
	${OGRE_LIBRARIES}
	${OGRE_HlmsPbs_LIBRARIES}
	Ogre_glTF
)

if (Ogre_glTF_STATIC AND UNIX AND NOT APPLE)
	target_link_libraries(Ogre_glTF_MorphAnimations
		${OGRE_RenderSystem_GL3Plus_LIBRARIES}
        dl
        pthread
        Xt
        X11
        Xaw
        Xrandr
	)
endif ()

set_target_properties(Ogre_glTF_MorphAnimations PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})