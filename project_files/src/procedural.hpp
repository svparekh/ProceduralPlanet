#pragma once

#include <iostream>
#include <glm/glm.hpp>

#include <fstream>
#include <sstream>
#include <vector>

class TerrainEditor
{
public:
	TerrainEditor();
	
	inline void incSeed() { _seed += 1; } // Increment the current seed (aka noise sample point offset)
	inline void decSeed() { _seed -= 1; } // Decrement the current seed
	inline void incDetail() { _detail = glm::clamp(_detail + 1, 0, 12); } // Increment the current detail level (fbm iterations), range limited to [0, 12]
	inline void decDetail() { _detail = glm::clamp(_detail - 1, 0, 12); } // Decrement the current detail level
	inline int getDetailLevel() { return _detail; }
	inline int getSeed() { return _seed; }
	inline size_t getAddedTerrainArraySize() { return _pArray.size(); }
	inline glm::vec3 *getAddedTerrainPointsArray() { return _pArray.data(); } // Get array start pointer
	inline float *getAddedTerrainRadiusArray() { return _rArray.data(); }
	inline float *getAddedTerrainHeightArray() { return _hArray.data(); }
	
	void generate();
	void load(std::string filename); // Load config file
	void save(std::string filename); // Save config file
	void addTerrain(glm::vec3 center, float radius, float height); // Add a mound
	void undoAddTerrain(); // Remove last point added

	float clickTCRadius = 0.5f; // radius and height to use when adding terrain in placement mode
	float clickTCHeight = 0.05f;

private:
	// these three arrays must be same size, so it is private
	std::vector<glm::vec3> _pArray = {}; // points array (centers of mounds)
	std::vector<float> _rArray = {}; // radius array (size of mounds)
	std::vector<float> _hArray = {}; // height array (height of mounds)

	int _detail = 5; // FBM iterations
	int _seed = 0; // Noise offset

	void _parseLines(std::vector<std::string> lines); // Called by load
};

