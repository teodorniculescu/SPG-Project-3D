#pragma once
#include <Component/SimpleScene.h>
#include <Component/Transform/Transform.h>
#include <Core/GPU/Mesh.h>

class BoidsAvoidCollision : public SimpleScene
{
	public:
		BoidsAvoidCollision();
		~BoidsAvoidCollision();

		float generateRandom(glm::vec2 vec);

		void Init() override;

		void SetUpSSBO();

	private:
		void FrameStart() override;
		void Update(float deltaTimeSeconds) override;
		void FrameEnd() override;

		void RenderSimpleMesh(Mesh *mesh, Shader *shader, const glm::mat4 &modelMatrix, int type, int index, float dt, int cone_num);

		void OnInputUpdate(float deltaTime, int mods) override;
		void OnKeyPress(int key, int mods) override;
		void OnKeyRelease(int key, int mods) override;
		void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
		void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
		void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
		void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
		void OnWindowResize(int width, int height) override;

		std::unordered_map<std::string, Texture2D*> mapTextures;
		GLuint randomTextureID;
		glm::vec3 tortilla, coffee, darkslategray;
		glm::vec4 coneColor, boxColor;
		int numCones = 1024;
		glm::vec2 xmax, ymax, zmax;
		float boxScale = 10;
		glm::vec3 coneScale = glm::vec3(1) / glm::vec3(15);
		struct Cone *cones;
		glm::vec3 light_position = glm::vec3(0, boxScale / 2, 0);
		glm::vec4 lowerConeColor, upperConeColor;
		std::vector<glm::vec4> coneColors;


		GLuint coneSSBO;
};
