project (TelharTactical)
cmake_minimum_required(VERSION 3.14)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set (CMAKE_CXX_STANDARD 17)
set_property(GLOBAL PROPERTY USE_FOLDERS ON) # VS filter folders
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

#GameLib
set(GAMELIB_INCLUDE
	lib/gamelib/include/game/layer.h
	lib/gamelib/include/game/map.h
	lib/gamelib/include/game/tile.h
	lib/gamelib/include/math/vector2.h
	)
	
set(GAMELIB_SRC
	lib/gamelib/src/game/map.cpp
	)
	
add_library(GAMELIB STATIC ${GAMELIB_INCLUDE} ${GAMELIB_SRC})
	
target_include_directories(GAMELIB PUBLIC "${PROJECT_SOURCE_DIR}/lib/gamelib/include")
target_include_directories(GAMELIB PRIVATE "${PROJECT_SOURCE_DIR}/lib/gamelib/src")
source_group(TREE "${PROJECT_SOURCE_DIR}/lib/gamelib" FILES ${GAMELIB_INCLUDE} ${GAMELIB_SRC})

#SDL2
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

#fmt
find_package(fmt REQUIRED)

#AppLib
set(APPLIB_INCLUDE
	lib/applib/include/sdl/macro.h
	lib/applib/include/sdl/resource.h
	lib/applib/include/sdl/texture.h
	lib/applib/include/serial/config.h
	lib/applib/include/serial/error.h
	lib/applib/include/serial/tiled.h
	)
	
set(APPLIB_SRC
	lib/applib/src/sdl/resource.cpp
	lib/applib/src/sdl/texture.cpp
	lib/applib/src/serial/config.cpp
	lib/applib/src/serial/tiled.cpp
	)
	
add_library(APPLIB STATIC ${APPLIB_INCLUDE} ${APPLIB_SRC})

target_include_directories(APPLIB PUBLIC "${PROJECT_SOURCE_DIR}/lib/applib/include")
target_include_directories(APPLIB PUBLIC "${PROJECT_SOURCE_DIR}/ext/tl/include")
target_include_directories(APPLIB PRIVATE "${PROJECT_SOURCE_DIR}/lib/applib/src")
target_include_directories(APPLIB PRIVATE "${PROJECT_SOURCE_DIR}/ext/nlohmann/include")
target_include_directories(APPLIB PRIVATE ${SDL2_IMAGE_INCLUDE_DIRS})

target_link_libraries(APPLIB PUBLIC GAMELIB)
target_link_libraries(APPLIB PRIVATE SDL2::SDL2)
target_link_libraries(APPLIB PRIVATE ${SDL2_IMAGE_LIBRARIES})
target_link_libraries(APPLIB PRIVATE fmt::fmt fmt::fmt-header-only)

source_group(TREE "${PROJECT_SOURCE_DIR}/lib/applib" FILES ${APPLIB_INCLUDE} ${APPLIB_SRC})

#Main
set(MAIN_SRC
	src/main.cpp
	src/command_args.h
	src/command_args.cpp
	src/config_args.h
	src/config_args.cpp
	src/game_data.h
	src/game_data.cpp
	src/algorithm_extra.h
	)
	
add_executable(Main ${MAIN_SRC})

target_link_libraries(Main APPLIB)
target_link_libraries(Main SDL2::SDL2main)

target_include_directories(Main PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_include_directories(Main PRIVATE "${PROJECT_SOURCE_DIR}/ext/gsl/include")
target_include_directories(Main PRIVATE "${PROJECT_SOURCE_DIR}/ext/fmt/include")

source_group(TREE "${PROJECT_SOURCE_DIR}" FILES ${MAIN_SRC})

#Tests
set(APPTEST_SRC
	test/src/main.cpp
	test/src/serial/config.cpp
	test/src/serial/tiled.cpp
	test/src/serial/test_tiled_map.h
	)
	
add_executable(AppTest ${APPTEST_SRC})

target_include_directories(AppTest PRIVATE "${PROJECT_SOURCE_DIR}/test/ext/include")
target_include_directories(AppTest PRIVATE "${PROJECT_SOURCE_DIR}/test/src")

target_link_libraries(AppTest APPLIB)
target_link_libraries(AppTest SDL2::SDL2main)
