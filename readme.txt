Telhar Tactical
	- Working Title
	- Software project with the goal of implementing a Tactical RPG in the setting of Telhar

Scope
	- Ceci n'est pas un game engine. While the code aims to be as generic and reusable as possible, having a full game engine is not the goal of this project, however you define a game engine
	- This means no custom editor, no portable wrapper of system primitives unless needed, and no executable except for the game
	- The project aims to use libraries as much as possible wherever it makes sense. For example, the project uses SDL for handling much media requirements (rendering, windows, input, sound, image loading, etc...). However, the project should aim to minimize the use of intrusive frameworks, which dictate the architecture of a program for ease of use.
	
Modules
	GameLib
		- Collection of generic game components, used for modeling and updating the game simulation
		- This module has no awareness of media (graphics, sound), I/O (networking, files), or system resources (memory allocation, threading). Such things should be done externally, by the module using GameLib
		- This means that game entities will have no graphics entity components, which might be surprising to some
	AppLib
		- Collection of components which handle Application logic in a generic way, without necessarily dictating how they are used
		- While the module is aware of media, I/O, or system resources, it should not dictate their use internally. Rather, application components should take providers and allocators as parameters, and execute their logic with them
	Main
		- The actual Application, which will result in an executable binary
		- All resources and I/O should come from this module only
		- Configuration and command arguments should be processed and acted upon only in this module
		- The application will then run a game simulation on a given map, listen to user input, update the simulation, and render it to a window
		
Design Choices
	Maps
		- Maps are edited from the Tiled editor
		- Maps have a dynamic size, meaning that they can be as big as their tile chunks go
		- Each tile on the map has 32 per 32 pixels
	Media
		- Most media goes through SDL libraries

Terminology
	Sofware
		- In the context of software, component will refer to a logical functionality within a module. Not to be confused with entity components, which represent unique data an entity can contain
	Game
		Tile
			- A tile is an arbitrary atomic unit of terrain. For example, each tile might be a group of 32 per 32 pixels. While certain game logic might operate on pixel or even sub-pixel levels (or even have "analog" logic), many gameplay components will align themselves at tile boundaries
		Chunk
			- A fixed group of tiles. The tiles will be adjacent to each other spacially
			- Very few logic should interact with chunks. They are mostly a way to reason with dynamically-sized maps
		Map
			- The actual playing field. Entities move on the map, and tiles are used to represent physical areas
		Entity
			- A unique identity in the simulation
			
Controls
	+ F2: Reload map
	+ Arrow Keys: Move the Camera
	
Configuration Arguments
	- A config file named 'config.ini' can be placed in the CWD, which help parameterize the game without recompilation, in a persistent way
	- This file should always be optional
	- This file follows a simple INI format
	+ Section 'resource'
		+ Key 'path' (default: 'res')
			- The root path where resources are loaded from
	+ Section 'game'
		+ Key 'default_map'
			- Map to be loaded on launch, from the resource folder
		
Command Arguments
	- Certain arguments can be provided on launch through whatever mean provided by the system used to launch the game
	- The convention is to prepend flags with '--' with '-' between words (ex: --my-setting-example). A flag's arguments (if any) are separated by whitespace, and cannot start with '--' (ex: --setting 1 "foo" false)
	- If a command argument conflicts with a configuration argument, the command one should take priority if the command can override the behavior entirely. If the command conflicts with a configuration argument only partially, the command should be treated as an error
	+ window-size x y
		- Size of the main window for the application. The arguments must be strictly positive
	+ print-video-drivers
		- Print the video drivers of the current system to the standard output