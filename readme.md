# Telhar Tactical (Working Title)
Software project with the goal of implementing a Tactical RPG in the setting of Telhar

## Scope
Ceci n'est pas un game engine. While the code aims to be as generic and reusable as possible, having a full game engine is not the goal of this project, however you define a game engine
This means no custom editor, only one target architecture (x64 Windows), and no other programs than the game
The project aims to use other libraries as much as possible wherever it makes sense. For example, the project uses SDL for handling much media requirements (rendering, windows, input, sound, image loading, etc...). However, the project should aim to minimize the use of intrusive frameworks.
	
## Modules
This section describes the various modules found in their project, including their purpose, constraints, and dependencies. Private dependencies refer to dependencies that are required for compiling, linking, and running a program using the module. Public dependencies refer to dependencies which in addition to the private requirements, also require users of the module to be able to include the headers of that dependency to use the module.
### GameLib
Collection of generic game components, used for modeling and updating the game simulation
This module has no awareness of media (graphics, sound), I/O (networking, files), or system resources (memory allocation, threading). Such things should be done externally, by the module using GameLib
This means that Game entities will have no audio or visual  components attached to them - at least not directly
#### Dependencies
Public: C++ Standard Library
### AppLib
Collection of components which handle Application logic in a generic way, without necessarily dictating how they are used
While the module is aware of media, I/O, or system resources, it should not allocate or in any way force their use internally. 
#### Dependencies
Public: C++ Standard Library, expected
Private: SDL, fmt, nlohmann JSON
### Main
The actual Application, which will result in an executable binary.
All system resources and I/O should be allocated and managed from this module only.
Configuration and command arguments should be processed and acted upon only in this module.
The application will then run a game simulation on a given map, listen to user input, update the simulation, and render it to a window.
#### Dependencies
AppLib, GameLib, C++ Standard Library, GSL, expected

### Extra Dependencies
This section describes the current "extra" requirements due indirect dependencies by third party libraries.
#### SDL_image
SDL2, libjpeg-9 (with license), libpng16-16 (with license), libtiff-5 (with license), libwebp-7 (with license), zlib1 (with license)
#### SDL_ttf
SDL2, libfreetype-6 (with license), zlib1 (with license)
		
## Design Choices
### Maps
- Maps are edited from the Tiled editor
- Maps have a dynamic size, meaning that they can be as big as their tile chunks go
- Each tile on the map has 32 per 32 pixels
### Media
- Most media goes through SDL libraries

## Terminology
This section will describe the terminology used globally in this project.
- **Chunk**: A fixed group of tiles. The tiles will be adjacent to each other spatially. Very few game logic should interact with chunks; they are mostly a way to reason with dynamically-sized maps
- **Component**: This project will use it in two different ways. In the context of software engineering, component will refer to a logical functionality within a module. In game design, this will represent game entity components, which represent a type of data an entity can contain
- **Entity**: A unique identity in the game simulation. Each simulation consists of a set of entities and an environment, and interactions between each other.
- **Map**: A simulation area. Each map contains its own unique environment, and has entities native to it. The player's experience will consist of traversing a sequence of maps. A map's environment contains a tile layout, which represents the terrain.
- **Tile**: A tile is an arbitrary atomic unit of terrain. For example, each tile might be a group of 32 per 32 pixels. While certain game logic might operate on pixel or even sub-pixel levels (or even have "analog" logic), many gameplay components will align themselves at tile boundaries
			
## Controls
- F2: Reload map
- Arrow Keys: Move the Camera
	
## Configuration Arguments
A config file named 'config.ini' can be placed in the CWD, which help parameterize the game without recompilation, in a persistent way. This file should always be optional.
This file follows a simple INI format, with the "[section]" and "key: value" notation. Each key-value pair assigns a value to a variable in the section, which otherwise adopts a default value decided by the program. Below are the list of documented sections and its variables.
### resource
- **path** (default: *res*): The root path where resources are loaded from
### game
- **default_map**: Map to be loaded on launch, from the resource folder. If not specified, the program will choose a default map through some other means.
		
## Command Arguments
Certain arguments can be provided on launch through whatever mean provided by the system used to launch the game. The convention is to prepend flags with '--' with '-' between words (ex: --my-setting-example). A flag's arguments (ex: --setting 42 "foo" false), if any, are separated by whitespace, and cannot start with '--'. If a command argument conflicts with a configuration argument, the command one should take priority if the command can override the behavior entirely. If the command conflicts with a configuration argument only partially, the command should be treated as an error
- **window-size {x} {y}**: Size of the main window for the application. The arguments must be strictly positive
- **print-video-drivers**: Print the video drivers of the current system to the standard output
