#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include <GL\glew.h>
#include <glm\glm.hpp>

using namespace std;

class Model
{
public:
	// Buffers for storing vertices, normals, faces.
	vector<glm::vec4> vBuffer;
	vector<glm::vec2> tBuffer;
	vector<glm::vec3> nBuffer;
	vector<unsigned int> viBuffer;
	vector<unsigned int> tiBuffer;
	vector<unsigned int> niBuffer;

	unsigned int vao, vbo, fbo;
	float scale;
	glm::vec3 position;

	glm::vec3 midVertex;
	float modelLength;

	Model(string filename);
	~Model();

	int readObj(string filename);
	int readVertex(string line);
	int readNormal(string line);
	int readTexture(string line);
	int readFace(string line);

	void orientPoints();
	void computeBounds();
	void bufferData();
};

