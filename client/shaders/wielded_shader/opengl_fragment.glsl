uniform sampler2D baseTexture;
uniform sampler2D normalTexture;
uniform sampler2D textureFlags;

uniform vec4 skyBgColor;
uniform float fogDistance;
uniform vec3 eyePosition;

varying mediump vec4 varColor;
varying mediump vec2 varTexCoord;
varying mediump vec3 eyeVec;

bool normalTexturePresent = false;
bool texTileableHorizontal = false;
bool texTileableVertical = false;
bool texSeamless = false;

const float e = 2.718281828459;
const float BS = 10.0;
const float fogStart = FOG_START;
const float fogShadingParameter = 1.0 / (1.0 - fogStart);

void main(void)
{
	vec2 uv = varTexCoord.st;

	vec4 base = texture2D(baseTexture, uv).rgba;
#ifdef USE_DISCARD
	if (base.a == 0.0)
		discard;
#endif
	vec4 col = base;
	col *= varColor;
	// Due to a bug in some (older ?) graphics stacks (possibly in the glsl compiler ?),
	// the fog will only be rendered correctly if the last operation before the
	// clamp() is an addition. Else, the clamp() seems to be ignored.
	// E.g. the following won't work:
	//      float clarity = clamp(fogShadingParameter
	//		* (fogDistance - length(eyeVec)) / fogDistance), 0.0, 1.0);
	// As additions usually come for free following a multiplication, the new formula
	// should be more efficient as well.
	// Note: clarity = (1 - fogginess)
	if (fogDistance > 0.0) { // -1.0 means disabled
		float clarity = clamp(fogShadingParameter
			- fogShadingParameter * length(eyeVec) / fogDistance, 0.0, 1.0);
		col = mix(skyBgColor, col, clarity);
	}

	gl_FragColor = vec4(col.rgb, base.a);
}
