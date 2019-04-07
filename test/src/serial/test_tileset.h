#pragma once

#include <string_view>

std::string_view const test_tileset{
	R"(
		{ 
			"columns":2,
			"image":"test_tileset.png",
			"imageheight":64,
			"imagewidth":64,
			"margin":0,
			"name":"test_tileset",
			"spacing":0,
			"tilecount":4,
			"tiledversion":"1.2.1",
			"tileheight":32,
			"tilewidth":32,
			"type":"tileset",
			"version":1.2
		}
	)"
};