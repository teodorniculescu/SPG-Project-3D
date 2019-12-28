#version 430 compatibility
#extension GL_ARB_compute_shader :             enable
#extension GL_ARB_shader_storage_buffer_object : enable

uniform int totalCones;
uniform float dt;
uniform vec2 xmax;
uniform vec2 ymax;
uniform vec2 zmax;

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
 
layout (local_size_x = 1) in;

uint gid;

float distBtwn(vec4 pos1, vec4 pos2) {
	float dist = sqrt(pow(pos1.x - pos2.x, 2) + pow(pos1.y - pos2.y, 2) + pow(pos1.z - pos2.z, 2));
	return dist;
}

const float sphereradius = 0.5;
const vec4 ruleDamping = vec4(16);

// separation
vec4 rule1() {
	vec4 curpos = data[gid].pos;
	vec4 sum = vec4(0);
	float numsum = -1;

	int i;
	for (i = 0; i < totalCones; i++)
	{
		vec4 otherpos = data[i].pos;
		float dist = distBtwn(curpos, otherpos);
		if (dist < sphereradius)
		{
			sum = sum - (otherpos - curpos);
			numsum = numsum + 1;
		}
	}
	if (numsum > 0)
	{
		sum = sum / numsum;
		sum = sum / ruleDamping;
	}
	else 
		sum = vec4(0);
	return sum;
}

//alignment
vec4 rule2() {
	vec4 result = vec4(0);
	vec4 curvel = data[gid].vel;
	vec4 curpos = data[gid].pos;
	float num = -1;
	int i;
	for (i = 0; i < totalCones; i++)
	{
		vec4 othervel = data[i].vel;
		vec4 otherpos = data[i].pos;
		float dist = distBtwn(curpos, otherpos);
		if (dist < sphereradius) {
			num++;
			result = result + othervel;
		}
	}
	if (num > 0) {
		result -= curvel;
		result /= num;
		result /= ruleDamping;
	}
	else {
		result = vec4(0);
	}
	return result;
}

//cohesion
vec4 rule3() {
	vec4 result = vec4(0);
	vec4 curpos = data[gid].pos;
	float num = -1;
	int i;
	for (i = 0; i < totalCones; i++)
	{
		vec4 otherpos = data[i].pos;
		float dist = distBtwn(curpos, otherpos);
		if (dist < sphereradius) {
			result = result + otherpos;
			num++;
		}
	}
	if (num > 0) {
		result -= curpos; // remove this vector
		result /= num;
		result -= curpos; // get vector
		result /= ruleDamping;
	}
	else {
		result = vec4(0);
	}
	return result;
}

vec4 normalize(vec4 val) {
	float magnitude = distBtwn(val, vec4(0));
	return val / magnitude;
}

void main() {
	const float speedUp = 2;
	gid = gl_GlobalInvocationID.x;

	vec4 velocity = data[gid].vel;
	vec4 position = data[gid].pos;

	velocity += (rule1() + rule2() + rule3());
	velocity = normalize(velocity);

	position = position + velocity * vec4(vec3(speedUp * dt), 1);
	// check out of bounds

	if (position.x > xmax.y)
		position.x = xmax.x;
	else if (position.x < xmax.x)
		position.x = xmax.y;

	if (position.y > ymax.y)
		position.y = ymax.x;
	else if (position.y < ymax.x)
		position.y = ymax.y;

	if (position.z > zmax.y)
		position.z = zmax.x;
	else if (position.z < zmax.x)
		position.z = zmax.y;

	velocity.w = 1;
	position.w = 1;
	data[gid].newvel = velocity;
	data[gid].newpos = position;

	barrier();

	data[gid].pos = data[gid].newpos;
	data[gid].vel = data[gid].newvel;

}