uniform mediump float fogDistance;

varying mediump vec4 varColor;
varying mediump vec2 varTexCoord;
varying mediump vec3 eyeVec; // divided by fogDistance

const float e = 2.718281828459;
const float BS = 10.0;

void main(void)
{
	varTexCoord = inTexCoord0.xy;
	gl_Position = mWorldViewProj * inVertexPosition;
	eyeVec = -(mWorldView * inVertexPosition).xyz / fogDistance;
	varColor = inVertexColor;
}
