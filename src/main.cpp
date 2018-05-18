
#include <ctime>
#include <sstream>
#include <exception>

#include <GL/glew.h>
#include <GL/glut.h>

#include <donny/logger.hpp>
#include <boost/gil/extension/io/png_io.hpp>

#include "../lib/vmathex.hpp"
#include "../lib/donny/GLShaders.hpp"
#include "../lib/donny/GLObjects.hpp"

#include "standard3d.hpp"

using namespace std;
using namespace vmath;
using namespace donny;
using namespace donny::OpenGL;

const char* sWindowTitle = "3D Demo";
const bool bDebug = false;
vec2 v2WndPosition;
const vec2 v2WndResolution(1280, 720);

GLuint uShaderPgm;

GLuint uModelRotMatrixLoc;
GLuint uModelMatrixLoc;
GLuint uViewMatrixLoc;
GLuint uProjectMatrixLoc;

GLuint uViewPosLoc;
GLuint uLightSwitchLoc;
GLuint uLightPositionLoc;
GLuint uLightAttributeLoc;
GLuint uLightAmbientLoc;
GLuint uLightDiffuseLoc;
GLuint uLightSpecularLoc;
GLuint uLightShininessLoc;

GLuint uTextureModeLoc;
GLuint uTextureIdLoc;
GLuint uTextureCubeIdLoc;

enum { SpacecraftInd = 0, CubeInd, TrianglesInd, PyramidInd, Light0Ind, Board1Ind, CrystalInd, ResourcesCount }; // Resource Indecies
enum { PlayerId = 0, CubeId, TrianglesId, PyramidId, Board1Id, CrystalId, Light0Id = 8, AllObjectsId = 9, SpacecraftId = 10, ObjectsCount }; // Object Id

const vec3 v3InitPostions[] = {
	{  0.0f,  0.0f,  1.0f }, // Player
	{  3.0f,  0.0f, -3.0f }, // Cube
	{  0.0f,  0.0f,  8.0f }, // Triangles
	{  3.0f, -0.5f,  0.0f }, // Pyramid
	{  0.0f,  0.0f, -3.0f }, // Board1
	{ -3.0f,  0.0f, -3.0f }, // Crystal
	{  0.0f,  0.0f,  0.0f }, // 6
	{  0.0f,  0.0f,  0.0f }, // 7
	{ -1.0f,  2.0f,  1.0f }, // Light0
	{  0.0f,  0.0f,  0.0f }, // 9
	{  0.0f,  0.0f,  0.0f }, // Spacecraft
};

GLuint VAO[ResourcesCount];
GLuint VBO[ResourcesCount];
GLuint EBO[ResourcesCount];

bool bAutoRotate = false;
bool bRotateX = false;
bool bRotateY = false;
bool bRotateZ = false;
bool bLMoving = false;
bool bRMoving = false;
bool bFullScreen = false;
bool bWorldView = false;
bool bSelectViewer = false;
bool bTransparent = false;
bool bIgnoreKeyRepeat = false;
vec2 v2LBeginPos(0.f, 0.f);
vec2 v2RBeginPos(0.f, 0.f);

uint nSelectedObject = 0;
uint nViewerObject = 0;
enum SettingsEnum { SetSpeed, SetLightAmbient, SetLightDiffuse, SetLightSpecular, SetLightShininess };
SettingsEnum nCurSettings = SetSpeed;

float aspect = 0.f;
float speed = 1.f;

struct ObjectProperty
{
	bool visibility = true;
	vec3 translation = vec3(0.f, 0.f, 0.f);
	vec3 rotation = vec3(0.f, 0.f, 0.f);
	vec4 scale = vec4(1.f, 1.f, 1.f, 1.f);
	vec3 face = vec3(0.f, 0.f, 0.f);
} properties[ObjectsCount];
ObjectProperty &globalProperty = properties[AllObjectsId];
mat4 m4GlobalRot;

GLint iLightSwitch = 1;
vec3 &v3LightPosition = properties[Light0Id].translation;
vec3 v3LightAttribute = vec3(1.0f, 0.009f, 0.0032f); // constant, linear, quadratic
vec3 v3LightAmbient = vec3(1.0f, 1.0f, 1.0f) * 0.30f;
vec3 v3LightDiffuse = vec3(1.0f, 1.0f, 1.0f) * 0.60f; // 6.0f
vec3 v3LightSpecular = vec3(1.0f, 1.0f, 1.0f) * 0.35f; // 1.0f
GLfloat fLightShininess = 4;

vec4 v4ViewScale = vec4(1.f, 1.f, 1.f, 1.f);
vec3 v3ViewTrans = vec3(0.f, 0.f, 0.f);
vec3 v3ViewRot = vec3(0.f, 0.f, 0.f);

const int nSpacecraftElems = 18;
const int nSpacecraftTextures = 3;
GLuint uSpacecraftTex[nSpacecraftTextures];
int nSpacecraftCurTex = 0;
void InitSpacecraft()
{
	const vec3 Geometry = vec3(256.0f, 256.0f, 256.0f);
	static const GLfloat positions[] =
	{
		-Geometry.x(), -Geometry.y(), -Geometry.z(),  1.0f, // 0 - left-bottom-back
		-Geometry.x(), -Geometry.y(),  Geometry.z(),  1.0f, // 1 - left-bottom-front
		-Geometry.x(),  Geometry.y(), -Geometry.z(),  1.0f, // 2 - left-top-back
		-Geometry.x(),  Geometry.y(),  Geometry.z(),  1.0f, // 3 - left-top-front
		 Geometry.x(), -Geometry.y(), -Geometry.z(),  1.0f, // 4 - right-bottom-back
		 Geometry.x(), -Geometry.y(),  Geometry.z(),  1.0f, // 5 - right-bottom-front
		 Geometry.x(),  Geometry.y(), -Geometry.z(),  1.0f, // 6 - right-top-back
		 Geometry.x(),  Geometry.y(),  Geometry.z(),  1.0f, // 7 - right-top-front
	};

	const GLfloat fSpacecraftAero = 1.0f;
	static const GLfloat colors[] =
	{
		0.3f, 0.3f, 0.3f, fSpacecraftAero, // 0
		0.3f, 0.3f, 0.3f, fSpacecraftAero, // 1
		0.6f, 0.6f, 0.3f, fSpacecraftAero, // 2
		0.6f, 0.6f, 0.3f, fSpacecraftAero, // 3
		0.6f, 0.3f, 0.3f, fSpacecraftAero, // 4
		0.6f, 0.3f, 0.3f, fSpacecraftAero, // 5
		0.1f, 0.5f, 0.7f, fSpacecraftAero, // 6
		0.1f, 0.5f, 0.7f, fSpacecraftAero, // 7
	};

	static const GLushort indices[] =
	{
		// Spacecraft
		// 6, 4, 2, 0, 3, 1, 7, 5,
		// 0xFFFF,
		// 1, 0, 5, 4, 7, 6, 3, 2,
		// 0xFFFF,
		4, 6, 0, 2, 1, 3, 5, 7,
		0xFFFF,
		0, 1, 4, 5, 6, 7, 2, 3,
		0xFFFF,
	};

	static GLfloat normals[length_of_array(positions)];
	Standard3D::calculateEANormals(positions, indices, normals, 4, 0xFFFF);
	// logstdout << "Spacecraft normals: " << endl;
	// for (int a = 0; a < length_of_array(normals); ++a)
	// {
	// 	logstdout.print("%f ", normals[a]);
	// 	if (a % 4 == 3) logstdout.println("");
	// }

	static const string texPath = "skybox";
	static const string texFilenames[] =
	{
		"right.png",
		"left.png",
		"top.png",
		"bottom.png",
		"back.png",
		"front.png",
	};
	static const string texPackages[] =
	{
		"plateau",
		"city",
		"illusion",
	};

	glGenTextures(nSpacecraftTextures, uSpacecraftTex);
	for (int b = 0; b < nSpacecraftTextures; ++b)
	{
		glActiveTexture(GL_TEXTURE1 + b);
		glBindTexture(GL_TEXTURE_CUBE_MAP, uSpacecraftTex[b]);

		for (int a = 0; a < length_of_array(texFilenames); ++a)
		{
			stringstream filename; filename << texPath << '/' << texPackages[b] << '/' << texFilenames[a];
			try {
				boost::gil::rgb8_image_t img;
				boost::gil::png_read_and_convert_image(filename.str(), img);
				auto view = boost::gil::const_view(img);

				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + a,
					0, GL_RGB, view.width(), view.height(), 0, GL_RGB,
					GL_UNSIGNED_BYTE, view.begin().x());
			} catch (fstream::failure e) {
				logstderr.e("Error when reading file %s: \"%s\"",
					texFilenames[a].c_str(), e.what());
			}
		}

		GLfloat mxTexAni; // Query for the max anisotropy
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &mxTexAni);

		glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, mxTexAni);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[SpacecraftInd]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(VAO[SpacecraftInd]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[SpacecraftInd]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(colors) + sizeof(normals), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions), sizeof(colors), colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(colors), sizeof(normals), normals);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)sizeof(positions));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(positions) + sizeof(colors)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	properties[SpacecraftId].translation = v3InitPostions[SpacecraftId];
	logstdout.v() << "Spacecraft loaded." << endl;
}

void DrawSpacecraft()
{
	auto &property = properties[SpacecraftId];

	mat4 m4ModelRot(rotateXYZ(property.rotation));
	glUniformMatrix4fv(uModelRotMatrixLoc, 1, GL_FALSE, m4ModelRot);
	mat4 m4Model(translate(property.translation) *
				 scale(property.scale));
	glUniformMatrix4fv(uModelMatrixLoc, 1, GL_FALSE, m4Model);

	glUniform1i(uTextureModeLoc, 2);
	glUniform1i(uTextureCubeIdLoc, 1+nSpacecraftCurTex);

	// Set up for a glDrawElements call
	glBindVertexArray(VAO[SpacecraftInd]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[SpacecraftInd]);
	glBindTexture(GL_TEXTURE_CUBE_MAP, uSpacecraftTex[nSpacecraftCurTex]);

	glEnable(GL_TEXTURE_CUBE_MAP);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// When primitive restart is on, we can call one draw command
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFF);

	glDrawElements(GL_TRIANGLE_STRIP, nSpacecraftElems, GL_UNSIGNED_SHORT, NULL);

	glUniform1i(uTextureModeLoc, 0);
}

void InitPlayer()
{
	auto &property = properties[PlayerId];
	property.translation = v3InitPostions[PlayerId];
}

const int nCubeElems = 18;
void InitCube()
{
	static const GLfloat positions[] =
	{
		-1.0f, -1.0f, -1.0f,  1.0f, //0  - left-bottom-back
		-1.0f, -1.0f,  1.0f,  1.0f, //1  - left-bottom-front
		-1.0f,  1.0f, -1.0f,  1.0f, //2  - left-top-back
		-1.0f,  1.0f,  1.0f,  1.0f, //3  - left-top-front
		 1.0f, -1.0f, -1.0f,  1.0f, //4  - right-bottom-back
		 1.0f, -1.0f,  1.0f,  1.0f, //5  - right-bottom-front
		 1.0f,  1.0f, -1.0f,  1.0f, //6  - right-top-back
		 1.0f,  1.0f,  1.0f,  1.0f, //7  - right-top-front
	};

	const GLfloat fCubeAero = 0.8f;
	static const GLfloat colors[] =
	{
		1.0f, 1.0f, 1.0f, fCubeAero,
		1.0f, 1.0f, 0.0f, fCubeAero,
		1.0f, 0.0f, 1.0f, fCubeAero,
		1.0f, 0.0f, 0.0f, fCubeAero,
		0.0f, 1.0f, 1.0f, fCubeAero,
		0.0f, 1.0f, 0.0f, fCubeAero,
		0.0f, 0.0f, 1.0f, fCubeAero,
		0.5f, 0.5f, 0.5f, fCubeAero,
	};

	// static const GLushort indices[] =
	// {
	// 	0, 1, 2, 3, 6, 7, 4, 5,
	// 	0xFFFF,
	// 	2, 6, 0, 4, 1, 5, 3, 7
	// };
	static const GLushort indices[] =
	{
		// Cube
		6, 4, 2, 0, 3, 1, 7, 5,
		0xFFFF,
		1, 0, 5, 4, 7, 6, 3, 2,
		0xFFFF,
		// 4, 6, 0, 2, 1, 3, 5, 7,
		// 0xFFFF,
		// 0, 1, 4, 5, 6, 7, 2, 3,
		// 0xFFFF,
	};

	static GLfloat normals[length_of_array(positions)];
	Standard3D::calculateEANormals(positions, indices, normals, 4, 0xFFFF);
	// logstdout << "Cube normals: " << endl;
	// for (int a = 0; a < length_of_array(normals); ++a)
	// {
	// 	logstdout.print("%f ", normals[a]);
	// 	if (a % 4 == 3) logstdout.println("");
	// }

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[CubeInd]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(VAO[CubeInd]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[CubeInd]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(colors) + sizeof(normals), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions), sizeof(colors), colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(colors), sizeof(normals), normals);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)sizeof(positions));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(positions) + sizeof(normals)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	properties[CubeId].translation = v3InitPostions[CubeId];

	logstdout.v() << "Cube loaded." << endl;
}

void DrawCube()
{
	auto &property = properties[CubeId];

	if (!property.visibility || !globalProperty.visibility) return;

	mat4 m4ModelRot(m4GlobalRot *
			        rotateX(property.rotation[0] * 360.0f) *
			     	rotateY(property.rotation[1] * 360.0f) *
			     	rotateZ(property.rotation[2] * 360.0f));
	glUniformMatrix4fv(uModelRotMatrixLoc, 1, GL_FALSE, m4ModelRot);
	mat4 m4Model(translate(globalProperty.translation) *
				 translate(property.translation) *
				 scale(globalProperty.scale) *
				 scale(property.scale) *
				 m4ModelRot);
	glUniformMatrix4fv(uModelMatrixLoc, 1, GL_FALSE, m4Model);

	// Set up for a glDrawElements call
	glBindVertexArray(VAO[CubeInd]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[CubeInd]);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// When primitive restart is on, we can call one draw command
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFF);

	glDrawElements(GL_TRIANGLE_STRIP, nCubeElems, GL_UNSIGNED_SHORT, NULL);
}

const int nTrianglesElems = 8;
void InitTriangles()
{
	static const GLfloat positions[] =
	{
		-0.3f,  0.3f,  0.0f,  1.0f, //0
		 0.3f,  0.3f,  0.0f,  1.0f, //1
		 0.0f, -0.6f,  0.0f,  1.0f, //2
		-0.3f, -0.3f,  0.0f,  1.0f, //3
		 0.3f, -0.3f,  0.0f,  1.0f, //4
		 0.0f,  0.6f,  0.0f,  1.0f, //5
	};

	const GLfloat fTrianglesAero = 0.8f;
	static const GLfloat colors[] =
	{
		0.8f, 0.8f, 0.3f, fTrianglesAero,
		0.8f, 0.8f, 0.3f, fTrianglesAero,
		0.8f, 0.8f, 0.3f, fTrianglesAero,
		0.3f, 0.8f, 0.8f, fTrianglesAero,
		0.3f, 0.8f, 0.8f, fTrianglesAero,
		0.3f, 0.8f, 0.8f, fTrianglesAero,
	};

	static const GLushort indices[] =
	{
		// 2-Triangles
		0, 1, 2,
		0xFFFF,
		4, 3, 5,
		0xFFFF,
	};

	static GLfloat normals[length_of_array(positions)];
	Standard3D::calculateEANormals(positions, indices, normals, 4, 0xFFFF);
	// logstdout << "Triangles normals: " << endl;
	// for (int a = 0; a < length_of_array(normals); ++a)
	// {
	// 	logstdout.print("%f ", normals[a]);
	// 	if (a % 4 == 3) logstdout.println("");
	// }

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[TrianglesInd]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(VAO[TrianglesInd]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[TrianglesInd]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(colors) + sizeof(normals), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions), sizeof(colors), colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(colors), sizeof(normals), normals);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)sizeof(positions));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(positions) + sizeof(colors)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	properties[TrianglesId].translation = v3InitPostions[TrianglesId];

	logstdout.v() << "Hexagram loaded." << endl;
}

void DrawTriangles()
{
	auto &property = properties[TrianglesId];

	if (!property.visibility || !globalProperty.visibility) return;

	mat4 m4ModelRot(m4GlobalRot *
			        rotateX(property.rotation[0] * 360.0f) *
			     	rotateY(property.rotation[1] * 360.0f) *
			     	rotateZ(property.rotation[2] * 360.0f));
	glUniformMatrix4fv(uModelRotMatrixLoc, 1, GL_FALSE, m4ModelRot);
	mat4 m4Model(translate(globalProperty.translation) *
				 translate(property.translation) *
				 scale(globalProperty.scale) *
				 scale(property.scale) *
				 m4ModelRot);
	glUniformMatrix4fv(uModelMatrixLoc, 1, GL_FALSE, m4Model);

	glBindVertexArray(VAO[TrianglesInd]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[TrianglesInd]);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFF);

	glDrawElements(GL_TRIANGLE_STRIP, nTrianglesElems, GL_UNSIGNED_SHORT, NULL);
}


const int nPyramidElems = 12;
void InitPyramid()
{
	static const GLfloat positions[] =
	{
		 0.0f,  1.0f,  0.0f,  1.0f, //0 - top
		-1.0f,  0.0f,  1.0f,  1.0f, //1 - left-front
		 1.0f,  0.0f,  1.0f,  1.0f, //2 - right-front
		 1.0f,  0.0f, -1.0f,  1.0f, //3 - right-back
		-1.0f,  0.0f, -1.0f,  1.0f, //4 - left-back
	};

	const GLfloat fPyramidAero = 1.0f;
	static const GLfloat colors[] =
	{
		0.8f, 0.8f, 0.3f, fPyramidAero,
		0.3f, 0.8f, 0.8f, fPyramidAero,
		0.8f, 0.3f, 0.8f, fPyramidAero,
		0.3f, 0.8f, 0.3f, fPyramidAero,
		0.3f, 0.3f, 0.8f, fPyramidAero,
	};

	static const GLushort indices[] =
	{
		// 2-Triangles
		0, 1, 2, 4, 3,
		0xFFFF,
		2, 3, 0, 4, 1,
		0xFFFF,
	};

	static GLfloat normals[length_of_array(positions)];
	Standard3D::calculateEANormals(positions, indices, normals, 4, 0xFFFF);
	// for (int a = 0; a < length_of_array(normals); ++a)
	// {
	// 	if (a % 4 == 0) logstdout.println("");
	// 	logstdout.print("%f ", normals[a]);
	// }

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[PyramidInd]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(VAO[PyramidInd]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[PyramidInd]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(colors) + sizeof(normals), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions), sizeof(colors), colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(colors), sizeof(normals), normals);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)sizeof(positions));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(positions) + sizeof(colors)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	properties[PyramidId].translation = v3InitPostions[PyramidId];

	logstdout.v() << "Pyramid loaded." << endl;
}

void DrawPyramid()
{
	auto &property = properties[PyramidId];

	if (!property.visibility || !globalProperty.visibility) return;

	mat4 m4ModelRot(m4GlobalRot *
			        rotateX(property.rotation[0] * 360.0f) *
			     	rotateY(property.rotation[1] * 360.0f) *
			     	rotateZ(property.rotation[2] * 360.0f));
	glUniformMatrix4fv(uModelRotMatrixLoc, 1, GL_FALSE, m4ModelRot);
	mat4 m4Model(translate(globalProperty.translation) *
				 translate(property.translation) *
				 scale(globalProperty.scale) *
				 scale(property.scale) *
				 m4ModelRot);
	glUniformMatrix4fv(uModelMatrixLoc, 1, GL_FALSE, m4Model);

	glBindVertexArray(VAO[PyramidInd]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[PyramidInd]);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFF);

	glDrawElements(GL_TRIANGLE_STRIP, nPyramidElems, GL_UNSIGNED_SHORT, NULL);
}

const int nLightElems = 18;
void InitLight0()
{
	static const GLfloat positions[] =
	{
		-1.0f, -1.0f, -1.0f,  1.0f, //0  - left-bottom-back
		-1.0f, -1.0f,  1.0f,  1.0f, //1  - left-bottom-front
		-1.0f,  1.0f, -1.0f,  1.0f, //2  - left-top-back
		-1.0f,  1.0f,  1.0f,  1.0f, //3  - left-top-front
		 1.0f, -1.0f, -1.0f,  1.0f, //4  - right-bottom-back
		 1.0f, -1.0f,  1.0f,  1.0f, //5  - right-bottom-front
		 1.0f,  1.0f, -1.0f,  1.0f, //6  - right-top-back
		 1.0f,  1.0f,  1.0f,  1.0f, //7  - right-top-front
	};

	const GLfloat fCubeAero = 1.0f;
	static const GLfloat colors[] =
	{
		// Set value over 1.0f to make sure the light is shiny
		10.0f, 10.0f, 10.0f, fCubeAero,
		10.0f, 10.0f, 10.0f, fCubeAero,
		10.0f, 10.0f, 10.0f, fCubeAero,
		10.0f, 10.0f, 10.0f, fCubeAero,
		10.0f, 10.0f, 10.0f, fCubeAero,
		10.0f, 10.0f, 10.0f, fCubeAero,
		10.0f, 10.0f, 10.0f, fCubeAero,
		10.0f, 10.0f, 10.0f, fCubeAero,
	};

	// static const GLushort indices[] =
	// {
	// 	0, 1, 2, 3, 6, 7, 4, 5,
	// 	0xFFFF,
	// 	2, 6, 0, 4, 1, 5, 3, 7
	// };
	static const GLushort indices[] =
	{
		// Cube
		// 6, 4, 2, 0, 3, 1, 7, 5,
		// 0xFFFF,
		// 1, 0, 5, 4, 7, 6, 3, 2,
		// 0xFFFF,
		4, 6, 0, 2, 1, 3, 5, 7,
		0xFFFF,
		0, 1, 4, 5, 6, 7, 2, 3,
		0xFFFF,
	};

	static GLfloat normals[length_of_array(positions)];
	Standard3D::calculateEANormals(positions, indices, normals, 4, 0xFFFF);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[Light0Ind]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(VAO[Light0Ind]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[Light0Ind]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(colors) + sizeof(normals), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions), sizeof(colors), colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(colors), sizeof(normals), normals);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)sizeof(positions));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(positions) + sizeof(colors)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	properties[Light0Id].translation = v3InitPostions[Light0Id];
	properties[Light0Id].scale = vec4(0.1f, 0.1f, 0.1f, 1.0f);

	logstdout.v() << "Light0 loaded." << endl;
}

void DrawLight0()
{
	auto &property = properties[Light0Id];

	if (!property.visibility || !globalProperty.visibility) return;

	mat4 m4ModelRot(rotateX(0.0f));
	glUniformMatrix4fv(uModelRotMatrixLoc, 1, GL_FALSE, m4ModelRot);
	mat4 m4Model(translate(property.translation) *
				 scale(property.scale) *
				 m4ModelRot);
	glUniformMatrix4fv(uModelMatrixLoc, 1, GL_FALSE, m4Model);

	// Set up for a glDrawElements call
	glBindVertexArray(VAO[Light0Ind]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[Light0Ind]);

	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// When primitive restart is on, we can call one draw command
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFF);

	glDrawElements(GL_TRIANGLE_STRIP, nCubeElems, GL_UNSIGNED_SHORT, NULL);
}

const int nBoardElems = 18;
const int nBoardSlides = 18;
GLuint uBoardTex[nBoardSlides];
int nBoardCurSlide = 4;
void InitBoard1()
{
	static const GLfloat positions[] =
	{
		-1.6f, -1.2f, -0.2f,  1.0f, //0  - left-bottom-back
		-1.6f, -1.2f,  0.2f,  1.0f, //1  - left-bottom-front
		-1.6f,  1.2f, -0.2f,  1.0f, //2  - left-top-back
		-1.6f,  1.2f,  0.2f,  1.0f, //3  - left-top-front
		 1.6f, -1.2f, -0.2f,  1.0f, //4  - right-bottom-back
		 1.6f, -1.2f,  0.2f,  1.0f, //5  - right-bottom-front
		 1.6f,  1.2f, -0.2f,  1.0f, //6  - right-top-back
		 1.6f,  1.2f,  0.2f,  1.0f, //7  - right-top-front
	};

	const GLfloat fAero = 1.0f;
	static const GLfloat colors[] =
	{
		1.0f, 1.0f, 1.0f, fAero,
		1.0f, 1.0f, 1.0f, fAero,
		1.0f, 1.0f, 1.0f, fAero,
		1.0f, 1.0f, 1.0f, fAero,
		1.0f, 1.0f, 1.0f, fAero,
		1.0f, 1.0f, 1.0f, fAero,
		1.0f, 1.0f, 1.0f, fAero,
		1.0f, 1.0f, 1.0f, fAero,
	};

	// static const GLushort indices[] =
	// {
	// 	0, 1, 2, 3, 6, 7, 4, 5,
	// 	0xFFFF,
	// 	2, 6, 0, 4, 1, 5, 3, 7
	// };
	static const GLushort indices[] =
	{
		// Cube
		6, 4, 2, 0, 3, 1, 7, 5,
		0xFFFF,
		1, 0, 5, 4, 7, 6, 3, 2,
		0xFFFF,
		// 4, 6, 0, 2, 1, 3, 5, 7,
		// 0xFFFF,
		// 0, 1, 4, 5, 6, 7, 2, 3,
		// 0xFFFF,
	};

	static GLfloat normals[length_of_array(positions)];
	Standard3D::calculateEANormals(positions, indices, normals, 4, 0xFFFF);

	const float texWidth = 1.0f, texHeight = 1.0f;
	static const GLfloat texCoords[] =
	{
		    0.0f, texHeight,
		    0.0f, texHeight, // Left-Bottom
		    0.0f,      0.0f,
		    0.0f,      0.0f, // Left-Top
		texWidth, texHeight,
		texWidth, texHeight, // Right-Bottom
		texWidth,      0.0f,
		texWidth,      0.0f, // Right-Top
	};

	// Create textures
	glGenTextures(nBoardSlides, uBoardTex);
	glActiveTexture(GL_TEXTURE0);
	for (int a = 0; a < nBoardSlides; ++a)
	{
		// images = SOIL
		stringstream filename; filename << "board/" << a << ".png";
		try {
			boost::gil::rgb8_image_t img;
			boost::gil::png_read_and_convert_image(filename.str(), img);
			auto view = boost::gil::const_view(img);

			glBindTexture(GL_TEXTURE_2D, uBoardTex[a]);
			// glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			// logstdout.log( "%d, %d - %d", view.width(), view.height(), view.width() * view.height() );
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, view.width(), view.height(), 0, GL_RGB,
				GL_UNSIGNED_BYTE, view.begin().x());

			GLfloat mxTexAni; // Query for the max anisotropy
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &mxTexAni);

			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, mxTexAni);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

			glGenerateMipmap(GL_TEXTURE_2D);
		} catch (fstream::failure e) {
			logstderr.e("Error when reading file %s: \"%s\"",
				filename.str().c_str(), e.what());
		}
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[Board1Ind]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(VAO[Board1Ind]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[Board1Ind]);
	glBufferData(GL_ARRAY_BUFFER, 
		sizeof(positions) + sizeof(colors) + sizeof(normals) + sizeof(texCoords), 
		NULL, GL_STATIC_DRAW);
	long offset = 0;
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(positions), positions); offset += sizeof(positions);
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(colors), colors);       offset += sizeof(colors);
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(normals), normals);     offset += sizeof(normals);
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(texCoords), texCoords); offset += sizeof(texCoords);

	offset = 0;
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)offset); offset += sizeof(positions);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)offset); offset += sizeof(colors);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)offset); offset += sizeof(normals);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)offset); offset += sizeof(texCoords);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	properties[Board1Id].translation = v3InitPostions[Board1Id];
	// properties[Board1Id].rotation = vec3(0.0f, 90.0f / 360.0f, 0.0f);

	logstdout.v() << "Board1 loaded." << endl;
}

void DrawBoard1()
{
	auto &property = properties[Board1Id];

	if (!property.visibility || !globalProperty.visibility) return;

	// logstdout << property.rotation << endl;
	mat4 m4ModelRot(m4GlobalRot *
					rotateXYZ(property.rotation * 360.0f));
	glUniformMatrix4fv(uModelRotMatrixLoc, 1, GL_FALSE, m4ModelRot);
	mat4 m4Model(translate(globalProperty.translation) *
				 translate(property.translation) *
				 scale(globalProperty.scale) *
				 scale(property.scale) *
				 m4ModelRot);
	glUniformMatrix4fv(uModelMatrixLoc, 1, GL_FALSE, m4Model);

	glUniform1i(uTextureModeLoc, 1);
	glUniform1i(uTextureIdLoc, 0);

	// Set up for a glDrawElements call
	glBindVertexArray(VAO[Board1Ind]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[Board1Ind]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, uBoardTex[nBoardCurSlide]);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// When primitive restart is on, we can call one draw command
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFF);

	glDrawElements(GL_TRIANGLE_STRIP, nBoardElems, GL_UNSIGNED_SHORT, NULL);

	glUniform1i(uTextureModeLoc, 0);
}

const int nCrystalElems = 20;
void InitCrystal()
{
    static const GLfloat vertices[] =
	{
         0.000,  1.414,  0.000,  1.000f, // 0 - top
        -1.000,  0.000,  1.000,  1.000f, // 1 - left-front
         1.000,  0.000,  1.000,  1.000f, // 2 - right-front
        -1.000,  0.000, -1.000,  1.000f, // 3 - left-back
         1.000,  0.000, -1.000,  1.000f, // 4 - right-back
         0.000, -1.414,  0.000,  1.000f, // 5 - bottom
    };

	const float fAero = 0.7f;
	static const GLfloat colors[] = 
	{
		0.3f, 0.7f, 0.9f, fAero, // 0
		0.1f, 0.3f, 0.7f, fAero, // 1
		0.1f, 0.3f, 0.7f, fAero, // 2
		0.1f, 0.3f, 0.7f, fAero, // 3
		0.1f, 0.3f, 0.7f, fAero, // 4
		0.3f, 0.7f, 0.9f, fAero, // 5		
	};

	static const GLushort indices[] =
	{
		0, 1, 2, 5, 0xFFFF,
		0, 2, 4, 5, 0xFFFF,
		0, 4, 3, 5, 0xFFFF,
		0, 3, 1, 5, 0xFFFF,
	};

	static GLfloat normals[length_of_array(vertices)];
	Standard3D::calculateEANormals(vertices, indices, normals, 4, 0xFFFF);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[CrystalInd]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(VAO[CrystalInd]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[CrystalInd]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(colors) + sizeof(normals), NULL, GL_STATIC_DRAW);
	long offset = 0;
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vertices), vertices); offset += sizeof(vertices);
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(colors), colors);     offset += sizeof(colors);
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(normals), normals);   offset += sizeof(normals);

	offset = 0;
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)offset); offset += sizeof(vertices);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)offset); offset += sizeof(colors);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)offset); offset += sizeof(normals);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	properties[CrystalId].translation = v3InitPostions[CrystalId];

	logstdout.v() << "Crystal loaded." << endl;
}

void DrawCrystal(float delay)
{
	auto &property = properties[CrystalId];

	if (!property.visibility || !globalProperty.visibility) return;

	if (!bAutoRotate || nSelectedObject != CrystalId)
		property.rotation.y() -= speed * 1.0f * delay;

	mat4 m4ModelRot(m4GlobalRot *
					rotateXYZ(property.rotation * 360.0f));
	glUniformMatrix4fv(uModelRotMatrixLoc, 1, GL_FALSE, m4ModelRot);
	mat4 m4Model(translate(globalProperty.translation) *
				 translate(property.translation) *
				 scale(globalProperty.scale) *
				 scale(property.scale) *
				 m4ModelRot);
	glUniformMatrix4fv(uModelMatrixLoc, 1, GL_FALSE, m4Model);

	// Set up for a glDrawElements call
	glBindVertexArray(VAO[CrystalInd]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[CrystalInd]);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// When primitive restart is on, we can call one draw command
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFF);

	glDrawElements(GL_TRIANGLE_STRIP, nCrystalElems, GL_UNSIGNED_SHORT, NULL);
}

void Initialize()
{
	static const ShaderFileInfo shaderInfo[] =
	{
		{ GL_VERTEX_SHADER, "../shaders/vert.shad" },
		{ GL_FRAGMENT_SHADER, "../shaders/frag.shad" }
	};

	logstdout.v() << "Loading..." << endl;

	uShaderPgm = LoadShaders(shaderInfo);
	glUseProgram(uShaderPgm);

	v3LightPosition = v3InitPostions[Light0Id];

	// "model_matrix" is actually an array of 4 matrices
	uModelRotMatrixLoc = glGetUniformLocation(uShaderPgm, "model_rot_matrix");
	uModelMatrixLoc = glGetUniformLocation(uShaderPgm, "model_matrix");
	uViewMatrixLoc = glGetUniformLocation(uShaderPgm, "view_matrix");
	uProjectMatrixLoc = glGetUniformLocation(uShaderPgm, "projection_matrix");

	uViewPosLoc = glGetUniformLocation(uShaderPgm, "view_pos");

	uLightSwitchLoc = glGetUniformLocation(uShaderPgm, "light_switch");
	uLightPositionLoc = glGetUniformLocation(uShaderPgm, "light_position");
	uLightAttributeLoc = glGetUniformLocation(uShaderPgm, "light_attribute");
	uLightAmbientLoc = glGetUniformLocation(uShaderPgm, "light_ambient");
	uLightDiffuseLoc = glGetUniformLocation(uShaderPgm, "light_diffuse");
	uLightSpecularLoc = glGetUniformLocation(uShaderPgm, "light_specular");
	uLightShininessLoc = glGetUniformLocation(uShaderPgm, "light_shininess");

	uTextureModeLoc = glGetUniformLocation(uShaderPgm, "texture_mode");
	uTextureIdLoc = glGetUniformLocation(uShaderPgm, "texture_id");
	uTextureCubeIdLoc = glGetUniformLocation(uShaderPgm, "texture_cube_id");

	glUniform1i(uLightSwitchLoc, iLightSwitch);
	glUniform3fv(uLightPositionLoc, 1, v3LightPosition);
	glUniform3fv(uLightAttributeLoc, 1, v3LightAttribute);
	glUniform3fv(uLightAmbientLoc, 1, v3LightAmbient);
	glUniform3fv(uLightDiffuseLoc, 1, v3LightDiffuse);
	glUniform3fv(uLightSpecularLoc, 1, v3LightSpecular);
	glUniform1f(uLightShininessLoc, fLightShininess);

	glGenBuffers(ResourcesCount, EBO); // Set up the element array buffer
	glGenVertexArrays(ResourcesCount, VAO); // Set up the vertex attributes
	glGenBuffers(ResourcesCount, VBO);  // Set up the vertex array buffer

	InitSpacecraft();
	InitPlayer();
	InitCube();
	InitTriangles();
	InitPyramid();
	InitLight0();
	InitBoard1();
	InitCrystal();

	// Setup
	// glEnable(GL_CULL_FACE);
	// glEnable(GL_DEPTH_TEST);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	logstdout.v() << "Initialize done." << endl;
}

void Finalize()
{
	glUseProgram(0);
	glDeleteProgram(uShaderPgm);
	glDeleteVertexArrays(1, VAO);
	glDeleteBuffers(1, VBO);
	glDeleteBuffers(1, EBO);
}

void DisplayFunc()
{
	static float period = 5 * CLOCKS_PER_SEC;
	static clock_t last_clock = clock();
	float delay = float(clock() - last_clock) / period;

	auto &selectedObject = properties[nSelectedObject];
	auto &viewerObject = properties[nViewerObject];

	// Global rotation
	if (bAutoRotate) {
		float t = clock();
		if (bRotateX) selectedObject.rotation[0] += speed * 0.5f * delay;
		if (bRotateY) selectedObject.rotation[1] += speed * 1.0f * delay;
		if (bRotateZ) selectedObject.rotation[2] += speed * 2.0f * delay;
	}
	last_clock = clock();

	// logstdout.d() << "Global Rotation: "
	// 		<< selectedObject.rotation[0] << " "
	// 		<< selectedObject.rotation[1] << " "
	// 		<< selectedObject.rotation[2] << endl;

	m4GlobalRot = rotateX(globalProperty.rotation[0] * 360.0f) *
				  rotateY(globalProperty.rotation[1] * 360.0f) *
				  rotateZ(globalProperty.rotation[2] * 360.0f);

	// Light position
	glUniform3fv(uLightPositionLoc, 1, v3LightPosition);

	// Set up the view posision
	vec3 v3ViewPos;
	if (bWorldView)
		v3ViewPos = v3ViewTrans;
	else {
		v3ViewPos = viewerObject.translation;
		if (nViewerObject != AllObjectsId &&
			nViewerObject != SpacecraftId &&
			nViewerObject != Light0Id) {
			v3ViewPos +=  globalProperty.translation;
		}
	}
	glUniform3fv(uViewPosLoc, 1, v3ViewPos);

	// Set up the view matrix
	vmath::mat4 m4WorldView(translate(v3ViewPos * (-1)) *
							rotateX(v3ViewRot[0] * 360.0f) *
							rotateY(v3ViewRot[1] * 360.0f) *
							rotateZ(v3ViewRot[2] * 360.0f));
	vmath::mat4 m4ObjectView(rotateXYZ(viewerObject.face * 360.0f) *
							 translate(v3ViewPos * (-1)));
	vmath:mat4 m4View = (bWorldView) ? m4WorldView : m4ObjectView;
	glUniformMatrix4fv(uViewMatrixLoc, 1, GL_FALSE, m4View);

	// Set up the projection matrix
	vmath::mat4 m4Proj(vmath::frustum(-1.0f, 1.0f, -aspect, aspect, 1.0f, 500.0f));
	glUniformMatrix4fv(uProjectMatrixLoc, 1, GL_FALSE, m4Proj);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Activate simple shading program
	glUseProgram(uShaderPgm);

	DrawSpacecraft();
	DrawTriangles();
	DrawCube();
	DrawPyramid();
	DrawLight0();
	DrawBoard1();
	DrawCrystal(delay);

	glutSwapBuffers();
	glutPostRedisplay();
}

void MovementFunc(u_char key, int x, int y)
{
	const float fTransSensitivity = 0.5f;

	auto &selectedProperty = properties[nSelectedObject];
	vec4 v4Movement = vec4(0.f, 0.f, 0.f, 1.f);

	switch (key)
	{
	// Movement
	case 'd': // right
		v4Movement = vec4(fTransSensitivity, 0.0f, 0.0f, 1.0f);
		break;
	case 'a': // left
		v4Movement = vec4(-fTransSensitivity, 0.0f, 0.0f, 1.0f);
		break;
	case 'q': // up
		v4Movement = vec4(0.0f, fTransSensitivity, 0.0f, 1.0f);
		break;
	case 'e': // down
		v4Movement = vec4(0.0f, -fTransSensitivity, 0.0f, 1.0f);
		break;
	case 's': // backward
		v4Movement = vec4(0.0f, 0.0f, fTransSensitivity, 1.0f);
		break;
	case 'w': // forward
		v4Movement = vec4(0.0f, 0.0f, -fTransSensitivity, 1.0f);
		break;
	}

	if (bWorldView)
		v3ViewTrans += v4Movement.xyz();
	else {
		mat4 m4Rot = rotateY(selectedProperty.face.y() * 360.0f);
		vec4 v4Offset = v4Movement * m4Rot;
		selectedProperty.translation += v4Offset.xyz();
	}
}

void SelectObject(u_char key, int x, int y)
{
	auto &selectedProperty = properties[nSelectedObject];
	auto &viewerProperty = properties[nViewerObject];

	if (bSelectViewer) {
		nViewerObject = key - '0';

		logstdout.i() << "Selected Viewer: " << nViewerObject << endl
				<< "Position: " << viewerProperty.translation << endl
				<< "Rotation: " << viewerProperty.rotation << endl
				<< "Scale: " << viewerProperty.scale << endl
				<< "Face: " << viewerProperty.face << endl
				<< endl;
	}
	else {
		nSelectedObject = key - '0';

		logstdout.i() << "Selected Object: " << nSelectedObject << endl
				<< "Position: " << selectedProperty.translation << endl
				<< "Rotation: " << selectedProperty.rotation << endl
				<< "Scale: " << selectedProperty.scale << endl
				<< "Face: " << selectedProperty.face << endl
				<< endl;
	}
}

void KeyFunc(u_char key, int x, int y)
{
	const float fSpeedSensitivity = 0.5f;
	const float fLightSensitivity = 0.05f;

	auto &selectedProperty = properties[nSelectedObject];
	auto &viewerProperty = properties[nViewerObject];

	logstdout.d() << key << " pressed." << endl;
    switch (key)
	{
	// Fullscreen
	case 'f':
		bFullScreen ^= 1;
		if (bFullScreen)
			glutFullScreen();
		else {
			glutPositionWindow(v2WndPosition[0], v2WndPosition[1]);
			glutReshapeWindow(v2WndResolution[0], v2WndResolution[1]);
		}
		break;
	// Exit
	case 0x1B:
        exit(0);

	// Movement
	case 'd': // right
	case 'a': // left
	case 'q': // up
	case 'e': // down
	case 's': // backward
	case 'w': // forward
		MovementFunc(key, x, y);
		break;

	// Auto-rotate
	case 'm': // Enable/Disable auto-rotate
		bAutoRotate ^= 1;
		logstdout.i() << "Auto-rotate changed to: " << bAutoRotate << endl;
		break;
	case 'x': // Enable/Disable auto-rotate on x axis
		if (bAutoRotate) bRotateX ^= 1;
		break;
	case 'c': // Enable/Disable auto-rotate on y axis
		if (bAutoRotate) bRotateY ^= 1;
		break;
	case 'z': // Enable/Disable auto-rotate on z axis
		if (bAutoRotate) bRotateZ ^= 1;
		break;

	case 'i':
		nCurSettings = SetSpeed;
		break;
	case 'g':
		nCurSettings = SetLightAmbient;
		break;
	case 'h':
		nCurSettings = SetLightDiffuse;
		break;
	case 'j':
		nCurSettings = SetLightSpecular;
		break;
	case 'k':
		nCurSettings = SetLightShininess;
		break;

	// Turn up/down current settings
	case '+': // Faster
	case '-': // Slower
		switch (nCurSettings)
		{
		case SetSpeed:
			speed += fSpeedSensitivity * (0x2C - key);
			break;
		case SetLightAmbient:
			v3LightAmbient += fLightSensitivity * (0x2C - key);
			glUniform3fv(uLightAmbientLoc, 1, v3LightAmbient);
			break;
		case SetLightDiffuse:
			v3LightDiffuse += fLightSensitivity * (0x2C - key);
			glUniform3fv(uLightDiffuseLoc, 1, v3LightDiffuse);
			break;
		case SetLightSpecular:
			v3LightSpecular += fLightSensitivity * (0x2C - key);
			glUniform3fv(uLightSpecularLoc, 1, v3LightSpecular);
			break;
		case SetLightShininess:
			fLightShininess *= pow(2.0f, (0x2C - key));
			glUniform1f(uLightShininessLoc, fLightShininess);
			break;
		}
		logstdout.i() << endl
				<< "Ambient: " << v3LightAmbient << endl
				<< "Diffuse: " << v3LightDiffuse << endl
				<< "Specular: " << v3LightSpecular << endl
				<< "Shininess: " << fLightShininess << endl;
		break;

	// Board slide control
	case '<':
	case ',':
		nBoardCurSlide -= 1;
		if (nBoardCurSlide < 0) nBoardCurSlide = 0;
		logstdout.i() << "Current board slide: " << nBoardCurSlide << endl;
		break;
	case '.':
	case '>':
		nBoardCurSlide += 1;
		if (nBoardCurSlide >= nBoardSlides) nBoardCurSlide = nBoardSlides - 1;
		logstdout.i() << "Current board slide: " << nBoardCurSlide << endl;
		break;

	// Reset
	case 'r': // Reset world viewer position and rotation or selected viewer's face
		if (bWorldView) {
			v3ViewRot = vec3(0.f, 0.f, 0.f);
			v3ViewTrans = vec3(0.f, 0.f ,0.f);
		} else {
			viewerProperty.face = vec3(0.f, 0.f, 0.f);
		}
		break;
	case 't': // Reset selected objects' rotations
		selectedProperty.rotation = vec3(0.f, 0.f, 0.f);
		break;
	case 'y': // Reset selected objects' scale level
		selectedProperty.scale = vec4(1.f, 1.f, 1.f, 1.f);
		break;
	case 'u':
		selectedProperty.translation = v3InitPostions[nSelectedObject];
		break;

	// Select object(s)
	case '0': // Player
	case '1': // Cube
	case '2': // Triangles
	case '3': // Pyramid
	case '4': // Board1
	case '5':
	// case '6':
	// case '7':
	case '8': // Light0
	case '9': // Select all objects (except Spacecraft)
		SelectObject(key, x, y);
		break;

	case 'o': // World view mode
		bWorldView ^= true;
		break;
	case 'p':
		bSelectViewer ^= true;
		logstdout.i() << "SelectViewer: " << bSelectViewer << endl;
		break;

	case 'v': // Visibility
		selectedProperty.visibility ^= true;
		break;

	case 'b':
		nSpacecraftCurTex = (nSpacecraftCurTex + 1) % nSpacecraftTextures;
		logstdout.i() << "Current skybox texture: " << nSpacecraftCurTex << endl;
		break;

	case 'l':
		iLightSwitch ^= 1;
		glUniform1i(uLightSwitchLoc, iLightSwitch);
		break;

	// Transparency - deprecated
	// case 't':
	// 	bTransparent ^= 1;
	// 	if (bTransparent) {
	// 		glDisable(GL_CULL_FACE);
	// 		glDisable(GL_DEPTH_TEST);

	// 		glEnable(GL_BLEND);
	// 		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// 	} else {
	// 		glEnable(GL_CULL_FACE);
	// 		glEnable(GL_DEPTH_TEST);
	// 		glDisable(GL_BLEND);
	// 	}
	// 	break;
	}
}

void SpecialFunc(int key, int x, int y)
{
	const float fRotSensitivity = 0.001;
	const float fKeySensitivity = 10.0f;

	auto &viewerProperty = properties[nViewerObject];
	
	float xOff = 0, yOff = 0;
	switch (key)
	{
	case GLUT_KEY_UP:
		yOff -= fKeySensitivity;
		break;
	case GLUT_KEY_DOWN:
		yOff += fKeySensitivity;
		break;
	case GLUT_KEY_LEFT:
		xOff -= fKeySensitivity;
		break;
	case GLUT_KEY_RIGHT:
		xOff += fKeySensitivity;
		break;
	case GLUT_KEY_F1:
		bIgnoreKeyRepeat ^= true;
		glutIgnoreKeyRepeat(bIgnoreKeyRepeat);
		break;
	}
	// logstdout.d() << "Special key lifted. Key: " << key
	// 			 << " (xOff, yOff): " << xOff << ',' << yOff << endl;

	vec3 &v3CurRot = (bWorldView) ? v3ViewRot : viewerProperty.face;
	v3CurRot[1] += fRotSensitivity * xOff;
	v3CurRot[0] += fRotSensitivity * yOff;
}

void MouseFunc(int button, int state, int x, int y)
{
	// logstdout.d() << "Button: " << button << "; State: " << state
	// 		<< "; (x, y): " << x << ", " << y << endl;
	auto &selectedProperty = properties[nSelectedObject];

	switch (button)
	{
	case 0: // Left button
		bLMoving = true ^ state;
		v2LBeginPos = vec2(x, y);
		break;
	case 2: // Right button
		bRMoving = true ^ state;
		v2RBeginPos = vec2(x, y);
		break;
	case 3: // Scroll up
		selectedProperty.scale += selectedProperty.scale * 0.1;
		break;
	case 4: // Scroll down
		selectedProperty.scale -= selectedProperty.scale * 0.1;
		break;
	}
	// logstdout.d() << "Scale of selected object: " << selectedProperty.scale << endl;
}

void MotionFunc(int x, int y)
{
	const float fRotSensitivity = 0.001;

	auto &selectedProperty = properties[nSelectedObject];
	auto &viewerProperty = properties[nViewerObject];

	// logstdout.d() << "Motion: " << x << ", " << y << endl;

	if (bLMoving) {
		vec3 &v3CurRot = (bWorldView) ? v3ViewRot : viewerProperty.face;
		v3CurRot[1] += fRotSensitivity * (x - v2LBeginPos[0]);
		v3CurRot[0] += fRotSensitivity * (y - v2LBeginPos[1]);
		v2LBeginPos = vec2(x, y);
	}

	if (bRMoving) {
		selectedProperty.rotation[1] += fRotSensitivity * (x - v2RBeginPos[0]);
		selectedProperty.rotation[0] += fRotSensitivity * (y - v2RBeginPos[1]);
		v2RBeginPos = vec2(x, y);
	}
}

void ReshapeFunc(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = float(height) / float(width);
}

int main(int argc, char** argv)
{
	logstdout.enableLogLevel(logstdout.DEB, bDebug);

    glutInit(&argc, argv);

	v2WndPosition = vec2(std::max(0.0f, (glutGet(GLUT_SCREEN_WIDTH)-v2WndResolution[0])/2),
						 std::max(0.0f, (glutGet(GLUT_SCREEN_HEIGHT)-v2WndResolution[1])/2));

    glutInitWindowSize(v2WndResolution[0], v2WndResolution[1]);
    glutInitWindowPosition(v2WndPosition[0], v2WndPosition[1]);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutCreateWindow(sWindowTitle);
	glutHideWindow();

	glewExperimental = GL_TRUE;
	glewInit();

	glutIgnoreKeyRepeat(bIgnoreKeyRepeat);
    glutKeyboardFunc(KeyFunc);
	glutSpecialFunc(SpecialFunc);
	glutMouseFunc(MouseFunc);
	glutMotionFunc(MotionFunc);
    glutDisplayFunc(DisplayFunc);
    glutReshapeFunc(ReshapeFunc);

	Initialize();

	glutShowWindow();

	glutMainLoop();

	Finalize();

	return 0;
}
