#include "BoidsAvoidCollision.h"

#include <vector>
#include <string>
#include <iostream>
#include <ctime>

#include <Core/Engine.h>

using namespace std;

struct Cone
{
	glm::vec4 pos;
	glm::vec4 vel;
	glm::vec4 newpos;
	glm::vec4 newvel;

	Cone(const glm::vec3& position, const glm::vec3& velocity)
	{
		pos = glm::vec4(position, 1);
		vel = glm::vec4(velocity, 1);
	}
};

BoidsAvoidCollision::BoidsAvoidCollision()
{
}

BoidsAvoidCollision::~BoidsAvoidCollision()
{
}

float BoidsAvoidCollision::generateRandom(glm::vec2 vec) {
	float result;
	float LO = vec.x;
	float HI = vec.y;
	int rand_value = rand();
	result = LO + static_cast <float> (rand_value) / (static_cast <float> (RAND_MAX / (HI - LO)));
	return result;
}

void BoidsAvoidCollision::Init()
{
	light_position = glm::vec3(0, boxScale / 2, 0);
	srand(static_cast <unsigned> (time(0)));
	xmax.x = zmax.x = -boxScale / 2 + 1.0/2.0 * coneScale.x;
	xmax.y = zmax.y = boxScale / 2 - 1.0/2.0 * coneScale.x;
	ymax.x = 0 + 1.0 / 2.0 * coneScale.x;
	ymax.y = boxScale - 1.0 / 2.0 * coneScale.x;
	const string pathFolder = "ProiectSPG3D/BoidsAvoidCollision";
	coffee = glm::vec3(101, 53, 15) / glm::vec3(255.0);
	tortilla = glm::vec3(0x9a, 0x7b, 0x4f) / glm::vec3(255.0);
	darkslategray = glm::vec3(0x2f, 0x4f, 0x4f) / glm::vec3(255.0);
	auto camera = GetSceneCamera();
	camera->SetPosition(glm::vec3(boxScale * 1, boxScale * 1.5, boxScale));

	{
		Mesh* mesh = new Mesh("sphere");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "sphere.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}
	{
		Mesh* mesh = new Mesh("cone");
		mesh->LoadMesh(pathFolder + "/../Models/", "cone.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}
	{
		Mesh* mesh = new Mesh("box");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "box.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}
	lowerConeColor = glm::vec4(0x89, 0x38, 0x43, 1) / glm::vec4(0xff);
	upperConeColor = glm::vec4(0xff, 0xdd, 0xf4, 1) / glm::vec4(0xff);
	boxColor = glm::vec4(tortilla/glm::vec3(5), 1);


	// Create a shader program for drawing face polygon with the color of the normal
	{
		Shader *shader = new Shader("ShaderPerete");
		shader->AddShader(pathFolder + "/VertexShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader(pathFolder + "/FragmentShader.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}
	{
		Shader *shader = new Shader("ShaderObiecte");
		shader->AddShader(pathFolder + "/VertexShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader(pathFolder + "/FragmentShader.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}
	// new compute shader
	{
		Shader *shader = new Shader("MyComputeShaderProgram");
		shader->AddShader(pathFolder + "/ComputeShader.glsl", GL_COMPUTE_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;

		SetUpSSBO();
	}
}

int randInt(int lower, int upper) {
	if (upper > lower) {
		int aux = upper;
		upper = lower;
		lower = aux;
	}
	int result = rand() % (upper - lower + 1) + lower;
	return result;
}

void BoidsAvoidCollision::SetUpSSBO()
{
	glGenBuffers(1, &coneSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, coneSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numCones * sizeof(struct Cone), NULL,  GL_DYNAMIC_DRAW);	
	GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_READ_BIT ;
	cones = (struct Cone *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numCones * sizeof(struct Cone), bufMask);
	for (int i = 0; i < this->numCones; i++)
	{
		coneColor[0] = generateRandom(glm::vec2(lowerConeColor[0], upperConeColor[0]));
		coneColor[1] = generateRandom(glm::vec2(lowerConeColor[1], upperConeColor[1]));
		coneColor[2] = generateRandom(glm::vec2(lowerConeColor[2], upperConeColor[2]));
		coneColor[3] = 1;
		coneColors.push_back(coneColor);

		glm::vec3 pos, vel;
		pos.x = generateRandom(xmax);
		pos.y = generateRandom(ymax);
		pos.z = generateRandom(zmax);
		vel.x = generateRandom(glm::vec2(-1, 1));
		vel.y = generateRandom(glm::vec2(-1, 1));
		vel.z = generateRandom(glm::vec2(-1, 1));
		vel = glm::normalize(vel);
		cones[i].pos = glm::vec4(pos, 1);
		cones[i].vel = glm::vec4(vel, 1);
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void BoidsAvoidCollision::FrameStart()
{
	// clears the color buffer (using the previously set color) and depth buffer
	glClearColor(darkslategray.x, darkslategray.y, darkslategray.z, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::ivec2 resolution = window->GetResolution();
	// sets the screen area where to draw
	glViewport(0, 0, resolution.x, resolution.y);	
}

void BoidsAvoidCollision::Update(float deltaTimeSeconds)
{
	glLineWidth(3);

	{
		Shader* shader = shaders["MyComputeShaderProgram"];
		glUseProgram(shader->GetProgramID());

		GLint totalCones = shader->GetUniformLocation("totalCones");
		glUniform1i(totalCones, numCones);
		int locDt = shader->GetUniformLocation("dt");
		glUniform1f(locDt, deltaTimeSeconds);
		int locxmax = shader->GetUniformLocation("xmax");
		glUniform2fv(locxmax, 1, glm::value_ptr(xmax));
		int locymax = shader->GetUniformLocation("ymax");
		glUniform2fv(locymax, 1, glm::value_ptr(ymax));
		int loczmax = shader->GetUniformLocation("zmax");
		glUniform2fv(loczmax, 1, glm::value_ptr(zmax));

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, coneSSBO);

		glDispatchCompute((GLuint) numCones, (GLuint) 1, (GLuint) 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0 );
	}
	{
		for (int i = 0; i < this->numCones; i++)
		{
			glm::mat4 modelMatrix = glm::mat4(1);
			RenderSimpleMesh(meshes["cone"], shaders["ShaderObiecte"], modelMatrix, 'c', i, deltaTimeSeconds, i);
		}
	}
	/*
	{
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, light_position);
		RenderSimpleMesh(meshes["sphere"], shaders["ShaderPerete"], modelMatrix, 's', 0, deltaTimeSeconds, 0);
	}
	*/
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);

	{
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0, boxScale / 2, 0));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(boxScale));
		RenderSimpleMesh(meshes["box"], shaders["ShaderPerete"], modelMatrix, 'b', 0, deltaTimeSeconds, 0);
	}

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void BoidsAvoidCollision::FrameEnd()
{
	DrawCoordinatSystem();
}

void BoidsAvoidCollision::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, int type, int index, float dt, int cone_num)
{
	if (!mesh || !shader || !shader->program)
		return;
	auto camera = GetSceneCamera();

	// render an object using the specified shader and the specified position
	shader->Use();
	glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
	glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));
	glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	int locColor = shader->GetUniformLocation("objectColor");
	if (type == 'c') {
		glUniform4fv(locColor, 1, glm::value_ptr(coneColors[cone_num]));
	}
	else if (type == 'b')
		glUniform4fv(locColor, 1, glm::value_ptr(boxColor));

	int locType = shader->GetUniformLocation("objectType");
	glUniform1i(locType, type);
	int locIndex = shader->GetUniformLocation("objectIndex");
	glUniform1i(locIndex, index);

	int locConeScale = shader->GetUniformLocation("coneScale");
	glUniform3fv(locConeScale, 1, glm::value_ptr(coneScale));

	glm::vec3 eye_position = GetSceneCamera()->transform->GetWorldPosition();
	int loclight_position = shader->GetUniformLocation("light_position");
	glUniform3fv(loclight_position, 1, glm::value_ptr(light_position));
	int loceye_position = shader->GetUniformLocation("eye_position");
	glUniform3fv(loceye_position, 1, glm::value_ptr(eye_position));

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, coneSSBO);

	mesh->Render();

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0 );
}

// Documentation for the input functions can be found in: "/Source/Core/Window/InputController.h" or
// https://github.com/UPB-Graphics/Framework-EGC/blob/master/Source/Core/Window/InputController.h

void BoidsAvoidCollision::OnInputUpdate(float deltaTime, int mods)
{
}

void BoidsAvoidCollision::OnKeyPress(int key, int mods)
{
	// add key press event
}

void BoidsAvoidCollision::OnKeyRelease(int key, int mods)
{
	// add key release event
}

void BoidsAvoidCollision::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// add mouse move event
}

void BoidsAvoidCollision::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button press event
}

void BoidsAvoidCollision::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button release event
}

void BoidsAvoidCollision::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void BoidsAvoidCollision::OnWindowResize(int width, int height)
{
}
