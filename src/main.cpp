
#include <ctime>

#include <GL/glew.h>
#include <GL/glut.h>

#include <donny/logger.hpp>

#include "../lib/vmathex.hpp"
#include "../lib/donny/GLShaders.hpp"
#include "../lib/donny/GLObjects.hpp"

#include "standard3d.hpp"

using namespace vmath;
using namespace donny;
using namespace donny::OpenGL;

const char* sWindowTitle = "Primitive Restart";
const bool bDebug = true;
const vec2 v2Resolution(1280, 720);

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

enum { SpacecraftInd = 0, CubeInd, TrianglesInd, PyramidInd, Light0Ind, ResourcesCount}; // Resource Indecies
enum { PlayerId = 0, CubeId, TrianglesId, PyramidId, Light0Id = 8, AllObjectsId = 9, SpacecraftId = 10, ControlsCount }; // Object Id

const vec3 v3InitPostions[] = {
	{  0.0f,  0.0f,  0.0f }, // Player
	{  0.0f,  0.0f, -3.0f }, // Cube
	{  0.0f,  0.0f,  16.0f }, // Triangles
	{  3.0f, -0.5f,  0.0f }, // Pyramid
	{  0.0f,  0.0f,  0.0f }, // 4
	{  0.0f,  0.0f,  0.0f }, // 5
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
bool bWorldView = true;
bool bTransparent = false;
vec2 v2LBeginPos(0.f, 0.f);
vec2 v2RBeginPos(0.f, 0.f);

uint nSelectedObject = 0;
enum SettingsEnum { SetSpeed, SetLightAmbient, SetLightDiffuse, SetLightSpecular };
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
} properties[ControlsCount];
// bool bVisibility[ControlsCount];
// vec3 v3Translations[ControlsCount];
// vec3 v3Rotations[ControlsCount];
// vec4 v4GlobalScale = vec4(1.f, 1.f, 1.f, 1.f);
// vec3 &v3GlobalRot = v3Rotations[9]; // Index 0 controls global rotation
ObjectProperty &globalProperty = properties[AllObjectsId];
mat4 m4GlobalRot;

GLint iLightSwitch = 1;
vec3 &v3LightPosition = properties[Light0Id].translation;
vec3 v3LightAttribute = vec3(1.0f, 0.009f, 0.0032f); // constant, linear, quadratic
vec3 v3LightAmbient = vec3(1.0f, 1.0f, 1.0f) * 0.35f;
vec3 v3LightDiffuse = vec3(1.0f, 1.0f, 1.0f) * 6.0f;
vec3 v3LightSpecular = vec3(1.0f, 1.0f, 1.0f) * 1.0f;
GLfloat fLightShininess = 128;

vec4 v4ViewScale = vec4(1.f, 1.f, 1.f, 1.f);
vec3 v3ViewTrans = vec3(0.f, 0.f, 0.f);
vec3 v3ViewRot = vec3(0.f, 0.f, 0.f);

const int nSpacecraftElems = 18;
void InitSpacecraft()
{
	const vec3 Geometry = vec3(100.f, 100.f, 100.f);
	static const GLfloat positions[] =
	{
		-Geometry.x(), -Geometry.y(), -Geometry.z(),  1.0f, //0  - left-bottom-back
		-Geometry.x(), -Geometry.y(),  Geometry.z(),  1.0f, //1  - left-bottom-front
		-Geometry.x(),  Geometry.y(), -Geometry.z(),  1.0f, //2  - left-top-back
		-Geometry.x(),  Geometry.y(),  Geometry.z(),  1.0f, //3  - left-top-front
		 Geometry.x(), -Geometry.y(), -Geometry.z(),  1.0f, //4  - right-bottom-back
		 Geometry.x(), -Geometry.y(),  Geometry.z(),  1.0f, //5  - right-bottom-front
		 Geometry.x(),  Geometry.y(), -Geometry.z(),  1.0f, //6  - right-top-back
		 Geometry.x(),  Geometry.y(),  Geometry.z(),  1.0f, //7  - right-top-front
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
	Standard3D::calculateNormalEAO(positions, indices, normals, 4, 0xFFFF);
	logstdout << "Spacecraft normals: " << endl;
	for (int a = 0; a < length_of_array(normals); ++a)
	{
		logstdout.print("%f ", normals[a]);
		if (a % 4 == 3) logstdout.println("");
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
}

void DrawSpacecraft()
{
	auto &property = properties[SpacecraftId];

	mat4 m4ModelRot(rotateXYZ(property.rotation));
	glUniformMatrix4fv(uModelRotMatrixLoc, 1, GL_FALSE, m4ModelRot);
	mat4 m4Model(translate(property.translation) *
				 scale(property.scale));
	glUniformMatrix4fv(uModelMatrixLoc, 1, GL_FALSE, m4Model);

	// Set up for a glDrawElements call
	glBindVertexArray(VAO[SpacecraftInd]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[SpacecraftInd]);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// When primitive restart is on, we can call one draw command
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFF);

	glDrawElements(GL_TRIANGLE_STRIP, nSpacecraftElems, GL_UNSIGNED_SHORT, NULL);
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
	Standard3D::calculateNormalEAO(positions, indices, normals, 4, 0xFFFF);
	logstdout << "Cube normals: " << endl;
	for (int a = 0; a < length_of_array(normals); ++a)
	{
		logstdout.print("%f ", normals[a]);
		if (a % 4 == 3) logstdout.println("");
	}

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
	Standard3D::calculateNormalEAO(positions, indices, normals, 4, 0xFFFF);
	logstdout << "Triangles normals: " << endl;
	for (int a = 0; a < length_of_array(normals); ++a)
	{
		logstdout.print("%f ", normals[a]);
		if (a % 4 == 3) logstdout.println("");
	}

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
	Standard3D::calculateNormalEAO(positions, indices, normals, 4, 0xFFFF);
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
		1.0f, 1.0f, 1.0f, fCubeAero,
		1.0f, 1.0f, 1.0f, fCubeAero,
		1.0f, 1.0f, 1.0f, fCubeAero,
		1.0f, 1.0f, 1.0f, fCubeAero,
		1.0f, 1.0f, 1.0f, fCubeAero,
		1.0f, 1.0f, 1.0f, fCubeAero,
		1.0f, 1.0f, 1.0f, fCubeAero,
		1.0f, 1.0f, 1.0f, fCubeAero,
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
	Standard3D::calculateNormalEAO(positions, indices, normals, 4, 0xFFFF);

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
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(positions) + sizeof(normals)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	properties[Light0Id].translation = v3InitPostions[Light0Id];
	properties[Light0Id].scale = vec4(0.1f, 0.1f, 0.1f, 1.0f);
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

void Initialize()
{
	static const ShaderFileInfo shaderInfo[] =
	{
		{ GL_VERTEX_SHADER, "../shaders/vert.shad" },
		{ GL_FRAGMENT_SHADER, "../shaders/frag.shad" }
	};

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
	InitCube();
	InitTriangles();
	InitPyramid();
	InitLight0();

	// vec4 dist = normalize(vec4(0.0f, 0.0f, 0.0f, 1.0f) - vec4(-1.0f, -1.0f, -2.0f, 1.0f));
	// vec4 norm = vec4(normalize(vec3(-1.0f, -1.0f, 1.0f)), 1.0f);
	// float factor = dot(dist, norm);
	// vec4 effect = vec4(v3LightDiffuse * max(0.0f, factor), 1.0f);
	// logstdout << "Distance: " << dist << endl;
	// logstdout << "Normal: " << norm << endl;
	// logstdout << "Factor: " << factor << endl;
	// logstdout << "Effect: " << effect << endl;

	// Setup
	// glEnable(GL_CULL_FACE);
	// glEnable(GL_DEPTH_TEST);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
	static float q = 0.0f;

	auto &selectedObject = properties[nSelectedObject];

	// Global rotation
	if (bAutoRotate) {
		float t = clock();
		if (bRotateX) selectedObject.rotation[0] += speed * 0.5 * float(t - last_clock) / period;
		if (bRotateY) selectedObject.rotation[1] += speed * 1 * float(t - last_clock) / period;
		if (bRotateZ) selectedObject.rotation[2] += speed * 2 * float(t - last_clock) / period;
	}
	last_clock = clock();

	if (bDebug) {
		// logstdout << "Global Rotation: "
		// 		<< selectedObject.rotation[0] << " "
		// 		<< selectedObject.rotation[1] << " "
		// 		<< selectedObject.rotation[2] << endl;
	}

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
		v3ViewPos = selectedObject.translation;
		if (nSelectedObject != AllObjectsId &&
			nSelectedObject != SpacecraftId &&
			nSelectedObject != Light0Id) {
			v3ViewPos +=  globalProperty.translation;
		}
	}
	glUniform3fv(uViewPosLoc, 1, v3ViewPos);

	// Set up the view matrix
	vmath::mat4 m4WorldView(translate(v3ViewPos * (-1)) *
							rotateX(v3ViewRot[0] * 360.0f) *
							rotateY(v3ViewRot[1] * 360.0f) *
							rotateZ(v3ViewRot[2] * 360.0f));
	vmath::mat4 m4ObjectView(rotateXYZ(selectedObject.face * 360.0f) *
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

	glutSwapBuffers();
	glutPostRedisplay();
}

void MovementFunc(u_char key, int x, int y)
{
	const float fTransSensitivity = 1.0f;

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

void KeyFunc(u_char key, int x, int y)
{
	const float fSpeedSensitivity = 0.5f;
	const float fLightSensitivity = 0.05f;

	auto &selectedProperty = properties[nSelectedObject];

	if (bDebug) logstdout << key << " pressed." << endl;
    switch (key)
	{
	// Exit
	case 0x1B:
        exit(0);
	// Fullscreen
	case 'f':
		bFullScreen ^= 1;
		if (bFullScreen)
			glutFullScreen();
		else {
			glutPositionWindow(0, 0);
			glutReshapeWindow(v2Resolution[0], v2Resolution[1]);
		}
		break;
		
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
		logstdout << "Auto-rotate changed to: " << bAutoRotate << endl;
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

	case 'p':
		nCurSettings = SetSpeed;
		break;
	case 'y':
		nCurSettings = SetLightAmbient;
		break;
	case 'u':
		nCurSettings = SetLightDiffuse;
		break;
	case 'i':
		nCurSettings = SetLightSpecular;
		break;

	// Turn up/down current settings
	case 0x2B: // Faster
	case 0x2D: // Slower
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
		}
		if (bDebug)
			logstdout << endl
					  << "Ambient: " << v3LightAmbient << endl
					  << "Diffuse: " << v3LightDiffuse << endl
					  << "Specular: " << v3LightSpecular << endl;
		break;

	// Reset
	case 'r': // Reset viewer's or selected objects' rotation(/face) and position
		if (bWorldView) {
			v3ViewRot = vec3(0.f, 0.f, 0.f);
			v3ViewTrans = vec3(0.f, 0.f ,0.f);
		} else {
			selectedProperty.face = vec3(0.f, 0.f, 0.f);
			selectedProperty.translation = v3InitPostions[nSelectedObject];
		}
		break;
	case 'g': // Reset selected objects' rotations
		selectedProperty.rotation = vec3(0.f, 0.f, 0.f);
		break;
	case 'h': // Reset selected objects' scale level
		selectedProperty.scale = vec4(1.f, 1.f, 1.f, 1.f);
		break;

	// Select object(s)
	case '0': // Player
		nSelectedObject = 0;
		break;
	case '1': // Cube
		nSelectedObject = 1;
		break;
	case '2': // Triangles
		nSelectedObject = 2;
		break;
	case '3': // Pyramid
		nSelectedObject = 3;
		break;
	case '8': // Light0
		nSelectedObject = 8;
		break;
	case '9': // Select all objects (except Spacecraft)
		nSelectedObject = 9;
		break;

	case 'o': // World view mode
		bWorldView ^= true;
		break;

	case 'v': // Visibility
		selectedProperty.visibility ^= true;
		break;

	case 'l':
		iLightSwitch ^= 1;
		glUniform1i(uLightSwitchLoc, iLightSwitch);
		break;

	// Transparency - deprecated
	case 't':
		bTransparent ^= 1;
		if (bTransparent) {
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		} else {
			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);
		}
		break;
	}

	logstdout << "Selected: " << nSelectedObject << endl
			<< "Position: " << selectedProperty.translation << endl
			<< "Rotation: " << selectedProperty.rotation << endl
			<< "Scale: " << selectedProperty.scale << endl
			<< "Face: " << selectedProperty.face << endl
			<< endl;
}

void MouseFunc(int button, int state, int x, int y)
{
	// if (bDebug)
	// 	logstdout << "Button: " << button << "; State: " << state
	// 			  << "; (x, y): " << x << ", " << y << endl;
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
	// logstdout << selectedProperty.scale[0] << ' '
	// 		  << selectedProperty.scale[1] << ' '
	// 		  << selectedProperty.scale[2] << ' '
	// 		  << selectedProperty.scale[3]
	// 		  << endl;
}

void MotionFunc(int x, int y)
{
	const float fRotSensitivity = 0.001;

	auto &selectedProperty = properties[nSelectedObject];

	// if (bDebug)
	// 	logstdout << "Motion: " << x << ", " << y << endl;
	
	if (bLMoving) {
		vec3 &v3CurRot = (bWorldView) ? v3ViewRot : selectedProperty.face;
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
    glutInit(&argc, argv);
    glutInitWindowSize(v2Resolution[0], v2Resolution[1]);
    glutInitWindowPosition(10, 10);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutCreateWindow(sWindowTitle);

	glewExperimental = GL_TRUE;
	glewInit();

    glutKeyboardFunc(KeyFunc);
	glutMouseFunc(MouseFunc);
	glutMotionFunc(MotionFunc);
    glutDisplayFunc(DisplayFunc);
    glutReshapeFunc(ReshapeFunc);

	Initialize();

	glutMainLoop();

	Finalize();
	
	return 0;
}
