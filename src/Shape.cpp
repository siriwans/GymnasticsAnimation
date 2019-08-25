
#include "Shape.h"
#include <iostream>
#include <cassert>

#include "GLSL.h"
#include "Program.h"

using namespace std;
using namespace glm;

// copy the data from the shape to this object
void Shape::createShape(tinyobj::shape_t & shape)
{
	posBuf = shape.mesh.positions;
	norBuf = shape.mesh.normals;
	texBuf = shape.mesh.texcoords;
	eleBuf = shape.mesh.indices;
}

void Shape::measure()
{
	float minX, minY, minZ;
	float maxX, maxY, maxZ;

	minX = minY = minZ = std::numeric_limits<float>::max();
	maxX = maxY = maxZ = -std::numeric_limits<float>::max();

	//Go through all vertices to determine min and max of each dimension
	for (size_t v = 0; v < posBuf.size() / 3; v++)
	{
		if (posBuf[3*v+0] < minX) minX = posBuf[3 * v + 0];
		if (posBuf[3*v+0] > maxX) maxX = posBuf[3 * v + 0];

		if (posBuf[3*v+1] < minY) minY = posBuf[3 * v + 1];
		if (posBuf[3*v+1] > maxY) maxY = posBuf[3 * v + 1];

		if (posBuf[3*v+2] < minZ) minZ = posBuf[3 * v + 2];
		if (posBuf[3*v+2] > maxZ) maxZ = posBuf[3 * v + 2];
	}

	min.x = minX;
	min.y = minY;
	min.z = minZ;
	max.x = maxX;
	max.y = maxY;
	max.z = maxZ;
}

void normalizeAll(std::vector<float> &norBuf) 
{
	int i;
   for (i = 0; i  < norBuf.size(); i += 3) 
   {
      vec3 norm = normalize(vec3(norBuf[i], norBuf[i+1], norBuf[i+2]));
		norBuf[i] = norm.x;
		norBuf[i+1] = norm.y;
		norBuf[i+2] = norm.z;
	}
}

void makeNorms(std::vector<float> &posBuf, std::vector<unsigned int> &eleBuf, std::vector<float> &norBuf) 
{
	int i;
	norBuf.resize(posBuf.size());
	for (int i = 0; i < norBuf.size(); i++) 
		norBuf[i] = 0;
   for (i = 0; i < eleBuf.size(); i += 3) 
   {
		float v1x = posBuf[3 * eleBuf[i]];
		float v1y = posBuf[3 * eleBuf[i] + 1];
		float v1z = posBuf[3 * eleBuf[i] + 2];
      vec3 v1 = vec3(v1x, v1y, v1z);
		float v2x = posBuf[3 * eleBuf[i + 1]];
		float v2y = posBuf[3 * eleBuf[i + 1] + 1];
		float v2z = posBuf[3 * eleBuf[i + 1] + 2];
      vec3 v2 = vec3(v2x, v2y, v2z);
		float v3x = posBuf[3 * eleBuf[i + 2]];
		float v3y = posBuf[3 * eleBuf[i + 2] + 1];
		float v3z = posBuf[3 * eleBuf[i + 2] + 2];
      vec3 v3 = vec3(v3x, v3y, v3z);
      vec3 crossed = cross(v2 - v1, v3 - v1);
		norBuf[3 * eleBuf[i]] += crossed.x;
		norBuf[3 * eleBuf[i] + 1] += crossed.y;
		norBuf[3 * eleBuf[i] + 2] += crossed.z;
		norBuf[3 * eleBuf[i + 1]] += crossed.x;
		norBuf[3 * eleBuf[i + 1] + 1] += crossed.y;
		norBuf[3 * eleBuf[i + 1] + 2] += crossed.z;
		norBuf[3 * eleBuf[i + 2]] += crossed.x;
		norBuf[3 * eleBuf[i + 2] + 1] += crossed.y;
		norBuf[3 * eleBuf[i + 2] + 2] += crossed.z;
	}
	normalizeAll(norBuf);
}


void Shape::init()
{
	// Initialize the vertex array object
	CHECKED_GL_CALL(glGenVertexArrays(1, &vaoID));
	CHECKED_GL_CALL(glBindVertexArray(vaoID));

	// Send the position array to the GPU
	CHECKED_GL_CALL(glGenBuffers(1, &posBufID));
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
	CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW));

	// Send the normal array to the GPU
	if (norBuf.empty())
	{
		makeNorms(posBuf, eleBuf, norBuf);
	}
   CHECKED_GL_CALL(glGenBuffers(1, &norBufID));
   CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
   CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW));

	// Send the texture array to the GPU
	if (texBuf.empty())
	{
		texBufID = 0;
	}
	else
	{
		CHECKED_GL_CALL(glGenBuffers(1, &texBufID));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufID));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW));
	}

	// Send the element array to the GPU
	CHECKED_GL_CALL(glGenBuffers(1, &eleBufID));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID));
	CHECKED_GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW));

	// Unbind the arrays
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void Shape::draw(const shared_ptr<Program> prog) const
{
	int h_pos, h_nor, h_tex;
	h_pos = h_nor = h_tex = -1;

	CHECKED_GL_CALL(glBindVertexArray(vaoID));

	// Bind position buffer
	h_pos = prog->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
	CHECKED_GL_CALL(glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));

	// Bind normal buffer
	h_nor = prog->getAttribute("vertNor");
	if (h_nor != -1 && norBufID != 0)
	{
		GLSL::enableVertexAttribArray(h_nor);
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
		CHECKED_GL_CALL(glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
	}

	if (texBufID != 0)
	{
		// Bind texcoords buffer
		h_tex = prog->getAttribute("vertTex");

		if (h_tex != -1 && texBufID != 0)
		{
			GLSL::enableVertexAttribArray(h_tex);
			CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufID));
			CHECKED_GL_CALL(glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0));
		}
	}

	// Bind element buffer
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID));

	// Draw
	CHECKED_GL_CALL(glDrawElements(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT, (const void *)0));

	// Disable and unbind
	if (h_tex != -1)
	{
		GLSL::disableVertexAttribArray(h_tex);
	}
	if (h_nor != -1)
	{
		GLSL::disableVertexAttribArray(h_nor);
	}
	GLSL::disableVertexAttribArray(h_pos);
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
