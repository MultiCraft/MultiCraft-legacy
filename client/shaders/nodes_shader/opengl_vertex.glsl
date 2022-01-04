// Color of the light emitted by the sun.
uniform highp mat4 mWorld;
uniform vec3 dayLight;
uniform mediump float fogDistance;
uniform float animationTimer;

varying mediump vec4 varColor;
varying mediump vec2 varTexCoord;
varying mediump vec3 eyeVec;

// Color of the light emitted by the light sources.
const vec3 artificialLight = vec3(1.04, 1.04, 1.04);
const float e = 2.718281828459;
const float BS = 10.0;


float smoothCurve(float x)
{
	return x * x * (3.0 - 2.0 * x);
}


float triangleWave(float x)
{
	return abs(fract(x + 0.5) * 2.0 - 1.0);
}


float smoothTriangleWave(float x)
{
	return smoothCurve(triangleWave(x)) * 2.0 - 1.0;
}


void main(void)
{
	varTexCoord = inTexCoord0.xy;
	//TODO: make offset depending on view angle and parallax uv displacement
	//thats for textures that doesnt align vertically, like dirt with grass
	//varTexCoord.y += 0.008;

float disp_x;
float disp_z;
#if (MATERIAL_TYPE == TILE_MATERIAL_WAVING_LEAVES && ENABLE_WAVING_LEAVES) || (MATERIAL_TYPE == TILE_MATERIAL_WAVING_PLANTS && ENABLE_WAVING_PLANTS)
	vec4 pos2 = mWorld * inVertexPosition;
	float tOffset = (pos2.x + pos2.y) * 0.001 + pos2.z * 0.002;
	disp_x = (smoothTriangleWave(animationTimer * 23.0 + tOffset) +
		smoothTriangleWave(animationTimer * 11.0 + tOffset)) * 0.4;
	disp_z = (smoothTriangleWave(animationTimer * 31.0 + tOffset) +
		smoothTriangleWave(animationTimer * 29.0 + tOffset) +
		smoothTriangleWave(animationTimer * 13.0 + tOffset)) * 0.5;
#endif


#if (MATERIAL_TYPE == TILE_MATERIAL_LIQUID_TRANSPARENT || MATERIAL_TYPE == TILE_MATERIAL_LIQUID_OPAQUE) && ENABLE_WAVING_WATER
	vec4 pos = inVertexPosition;
	pos.y -= 2.0;
	float posYbuf = (pos.z / WATER_WAVE_LENGTH + animationTimer * WATER_WAVE_SPEED * WATER_WAVE_LENGTH);
	pos.y -= sin(posYbuf) * WATER_WAVE_HEIGHT + sin(posYbuf / 7.0) * WATER_WAVE_HEIGHT;
	gl_Position = mWorldViewProj * pos;
#elif MATERIAL_TYPE == TILE_MATERIAL_WAVING_LEAVES && ENABLE_WAVING_LEAVES
	vec4 pos = inVertexPosition;
	pos.x += disp_x;
	pos.y += disp_z * 0.1;
	pos.z += disp_z;
	gl_Position = mWorldViewProj * pos;
#elif MATERIAL_TYPE == TILE_MATERIAL_WAVING_PLANTS && ENABLE_WAVING_PLANTS
	vec4 pos = inVertexPosition;
	if (varTexCoord.y < 0.05) {
		pos.x += disp_x;
		pos.z += disp_z;
	}
	gl_Position = mWorldViewProj * pos;
#else
	gl_Position = mWorldViewProj * inVertexPosition;
#endif

	eyeVec = -(mWorldView * inVertexPosition).xyz / fogDistance;

	// Calculate color.
	// Red, green and blue components are pre-multiplied with
	// the brightness, so now we have to multiply these
	// colors with the color of the incoming light.
	// The pre-baked colors are halved to prevent overflow.
	vec4 color = inVertexColor;
#if GL_ES
	color.xyz = color.zyx; // swap RGB order
#endif
	// The alpha gives the ratio of sunlight in the incoming light.
	color.rgb *= 2.0 * mix(artificialLight.rgb, dayLight.rgb, color.a);
	color.a = 1.0;
	
	// Emphase blue a bit in darker places
	// See C++ implementation in mapblock_mesh.cpp finalColorBlend()
	float brightness = (color.r + color.g + color.b) / 3.0;
	color.b += max(0.0, 0.021 - abs(0.2 * brightness - 0.021) +
		0.07 * brightness);
	
	varColor = clamp(color, 0.0, 1.0);
}
