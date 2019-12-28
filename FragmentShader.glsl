#version 330
 
layout(location = 0) out vec4 out_color;

in vec4 vertexColor;
in vec3 world_position;
in vec3 world_normal;


// Uniforms for light properties
uniform vec3 light_position;
uniform vec3 eye_position;

float material_kd = 1;
float material_ks = 2;
int material_shininess = 2;


void main()
{
	/*
	vec3 L = normalize(light_position - world_position);
	vec3 V = normalize(eye_position - world_position);
	vec3 H = normalize(L + V);

	// TODO: define ambient light component
	float ambient_light = 0.25;

	// TODO: compute diffuse light component
	float diffuse_light = material_kd * max(dot(world_normal, L), 0);

	// TODO: compute specular light component
	float primeste_lumina = 0;
	if (diffuse_light > 0)
		primeste_lumina = 1;
	float specular_light = material_ks * primeste_lumina * pow(max(dot(world_normal, H), 0), material_shininess);

	// TODO: compute light
	float light = ambient_light + diffuse_light + specular_light;

	// TODO: write pixel out color
	out_color = vertexColor * light;
	*/
	out_color = vertexColor;
}