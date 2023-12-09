#include "procedural.hpp"

TerrainEditor::TerrainEditor() {}

void TerrainEditor::generate() {}

void TerrainEditor::save(std::string filename) {
	// Open file for output, ensuring file is cleared
	std::ofstream f(filename, std::ofstream::out | std::ofstream::trunc);

	if (!f.is_open()) {
		throw std::runtime_error("Failed to open file: " + filename);
	}
	for (size_t i = 0; i < getAddedTerrainArraySize(); i++) {
		glm::vec3 p = _pArray[i];
		float r = _rArray[i];
		float h = _hArray[i];
		f << p.x << std::endl;
		f << p.y << std::endl;
		f << p.z << std::endl;
		f << r << std::endl;
		f << h << std::endl;

	}
	f.close();
}

void TerrainEditor::load(std::string filename) {
	// Open file for input
	std::ifstream f(filename);

	if (!f.is_open()) {
		throw std::runtime_error("Failed to open file: " + filename);
	}

	// Load lines into a vector
	std::vector<std::string> lines;
	std::string r;
	while (std::getline(f, r, '\n')) {
		lines.push_back(r);
	}
	f.close();

	_parseLines(lines);
}

void TerrainEditor::_parseLines(std::vector<std::string> lines) {
	// check validity of config
	if ((lines.size() % 5) != 0) {
		return;
	}

	// Parse
	int lineNum = 1; // Current relative line number, file should in sets of 5 lines
	_pArray.clear();
	_rArray.clear();
	_hArray.clear();
	for (std::string line : lines) {
		glm::vec3 p;
		if (lineNum > 5) {
			lineNum = 1;
		}
		float value = stof(line);

		switch (lineNum)
		{
		case 1: // Center x value
			p.x = value;
			break;
		case 2: // Center y value
			p.y = value;
			break;
		case 3: // Center z value, also add completed vec3 to vector
			p.z = value;
			_pArray.push_back(p);
			break;
		case 4: // Add radius to vector
			_rArray.push_back(value);
			break;
		case 5: // Add height to vector
			_hArray.push_back(value);
			break;
		default:
			break;
		}
		lineNum += 1;

	}
}

void TerrainEditor::addTerrain(glm::vec3 center, float radius, float height) {
	_pArray.push_back(center);
	_rArray.push_back(radius);
	_hArray.push_back(height);
}

void TerrainEditor::undoAddTerrain() {
	if (!_pArray.empty()) {
		_pArray.pop_back();
		_rArray.pop_back();
		_hArray.pop_back();
	}
}