#include "Mesh.h"
#include <iostream>
#include <sstream>
#include <fstream>

std::vector<std::string> split(std::string s, std::string t)
{
	std::vector<std::string> res;
	while (1)
	{
		int pos = s.find(t);
		if (pos == -1)
		{
			res.push_back(s);
			break;
		}
		res.push_back(s.substr(0, pos));
		s = s.substr(pos + 1, s.size() - pos - 1);
	}
	return res;
}

Mesh::Mesh()
	:mLoaded(false)
{
}

Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
}

bool Mesh::loadOBJ(const std::string& filename)
{
	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> tempVertices;
	std::vector<glm::vec2> tempUVs;
	std::vector<glm::vec3> tempNormals;


	if (filename.find(".obj") != std::string::npos)
	{
		std::ifstream fin(filename, std::ios::in);
		if (!fin)
		{
			std::cerr << "Cannot open " << filename << std::endl;
			return false;
		}

		std::cout << "Loading OBJ file " << filename << " ..." << std::endl;

		std::string lineBuffer;
		while (std::getline(fin, lineBuffer))
		{
			std::stringstream ss(lineBuffer);
			std::string cmd;
			ss >> cmd;

			if (cmd == "v")
			{
				glm::vec3 vertex;
				int dim = 0;
				while (dim < 3 && ss >> vertex[dim])
					dim++;

				tempVertices.push_back(vertex);
			}
			else if (cmd == "vt")
			{
				glm::vec2 uv;
				int dim = 0;
				while (dim < 2 && ss >> uv[dim])
					dim++;

				tempUVs.push_back(uv);
			}
			else if (cmd == "vn")
			{
				glm::vec3 normal;
				int dim = 0;
				while (dim < 3 && ss >> normal[dim])
					dim++;
				normal = glm::normalize(normal);
				tempNormals.push_back(normal);
			}
			else if (cmd == "f")
			{
				std::string faceData;
				int vertexIndex, uvIndex, normalIndex;

				while (ss >> faceData)
				{
					std::vector<std::string> data = split(faceData, "/");

					if (data[0].size() > 0)
					{
						sscanf_s(data[0].c_str(), "%d", &vertexIndex);
						vertexIndices.push_back(vertexIndex);
					}

					if (data.size() >= 1)
					{
						// Is face format v//vn?  If data[1] is empty string then
						// this vertex has no texture coordinate
						if (data[1].size() > 0)
						{
							sscanf_s(data[1].c_str(), "%d", &uvIndex);
							uvIndices.push_back(uvIndex);
						}
					}

					if (data.size() >= 2)
					{
						// Does this vertex have a normal?
						if (data[2].size() > 0)
						{
							sscanf_s(data[2].c_str(), "%d", &normalIndex);
							normalIndices.push_back(normalIndex);
						}
					}
				}
			}
		}

		// Close the file
		fin.close();


		// For each vertex of each triangle
		for (unsigned int i = 0; i < vertexIndices.size(); i++)
		{
			Vertex meshVertex;

			// Get the attributes using the indices

			if (tempVertices.size() > 0)
			{
				glm::vec3 vertex = tempVertices[vertexIndices[i] - 1];
				meshVertex.position = vertex;
			}

			if (tempNormals.size() > 0)
			{
				glm::vec3 normal = tempNormals[normalIndices[i] - 1];
				meshVertex.normal = normal;
			}

			if (tempUVs.size() > 0)
			{
				glm::vec2 uv = tempUVs[uvIndices[i] - 1];
				meshVertex.texCoords = uv;
			}

			mVertices.push_back(meshVertex);
		}

		// Create and initialize the buffers
		initBuffers();

		return (mLoaded = true);
	}

	// We shouldn't get here so return failure
	return false;
}

void Mesh::draw()
{
	if (!mLoaded) return;

	glBindVertexArray(mVAO);
	glDrawArrays(GL_TRIANGLES, 0, mVertices.size());
	glBindVertexArray(0);
}

void Mesh::initBuffers()
{
	glGenBuffers(1, &mVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(Vertex), &mVertices[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);

	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(0));
	glEnableVertexAttribArray(0);

	// noramls attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// tex coord
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // unbind to make sure other code doesn't change it
}

