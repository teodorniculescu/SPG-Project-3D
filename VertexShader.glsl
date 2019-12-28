#version 430

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;
layout(location = 3) in vec3 v_color;

out vec3 world_position;
out vec3 world_normal;
out vec4 vertexColor;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform vec4 objectColor;
uniform int objectType;
uniform int objectIndex;
uniform vec3 coneScale;

struct Cone
{
	vec4 pos;
	vec4 vel;
	vec4 newpos;
	vec4 newvel;
};

layout(std430, binding = 4) buffer cones {
	Cone data[];
};

mat4 translate(mat4 model, vec4 position) {
	return model * 
		mat4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			position.x, position.y, position.z, 1);
}
mat4 scale(mat4 model, vec3 scale) {
	return model *
		mat4(
			scale.x, 0, 0, 0,
			0, scale.y, 0, 0,
			0, 0, scale.z, 0,
			0, 0, 0, 1);
}

mat4 rotateOX(mat4 model, float angle) {
	return model *
		mat4(
			1, 0, 0, 0,
			0, cos(angle), sin(angle), 0,
			0, -sin(angle), cos(angle), 0,
			0, 0, 0, 1);
}

mat4 rotateOY(mat4 model, float angle) {
	return model *
		mat4(
			cos(angle), 0, -sin(angle), 0,
			0, 1, 0, 0,
			sin(angle), 0, cos(angle), 0,
			0, 0, 0, 1);
}

mat4 rotateOZ(mat4 model, float angle) {
	return model *
		mat4(
			cos(angle), sin(angle), 0, 0,
			-sin(angle), cos(angle), 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
}

mat4 rotate(mat4 model, vec3 velocity) {
	mat4 result = model;
	velocity = normalize(velocity);
	float oxrot = asin(-velocity.y);
	float oyrot = -atan(velocity.x/velocity.z);
	float ozrot = -1.5708;

	result = rotateOX(result, oxrot);
	result = rotateOY(result, oyrot);
	result = rotateOZ(result, ozrot);

	return result;
}

mat4 lookat(mat4 model, vec3 from, vec3 dir) {
	vec3 tmp = vec3(0, 1, 0);
	vec3 forward = normalize(dir); 
    vec3 right = cross(normalize(tmp), forward); 
    vec3 up = cross(forward, right); 
 
    mat4 camToWorld = mat4(1); 
 
    camToWorld[0][0] = right.x; 
    camToWorld[0][1] = right.y; 
    camToWorld[0][2] = right.z; 
    camToWorld[1][0] = up.x; 
    camToWorld[1][1] = up.y; 
    camToWorld[1][2] = up.z; 
    camToWorld[2][0] = forward.x; 
    camToWorld[2][1] = forward.y; 
    camToWorld[2][2] = forward.z; 
 
	from = vec3(0);
    camToWorld[3][0] = from.x; 
    camToWorld[3][1] = from.y; 
    camToWorld[3][2] = from.z; 
 
    return model * camToWorld; 
}


void main()
{
	vec4 position = data[objectIndex].pos;
	vec4 velocity = data[objectIndex].vel;

	mat4 model;
	// box
	if (objectType == 98) {
		model = Model;
		vertexColor = objectColor * vec4((v_normal + vec3(1)) / vec3(2), 1);
	}
	// cone
	else if (objectType == 99) {
		model = translate(mat4(1), position);
		model = scale(model, coneScale);
		model = lookat(model, vec3(position.x, position.y, position.z), vec3(velocity.x, velocity.y, velocity.z));
		model = rotateOY(model, -1.5708);
		model = rotateOZ(model, -1.5708);
		vertexColor = objectColor;
	} else if (objectType == 115) {
		model = Model;
		vertexColor = vec4(1);
	}

    world_position = (Model * vec4(v_position, 1)).xyz;
    world_normal = normalize(mat3(Model) * v_normal);

	gl_Position = Projection * View * model * vec4(v_position, 1.0);
}
