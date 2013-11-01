#version 110
#extension GL_ARB_texture_rectangle : enable

uniform vec2 dim;
uniform vec2 textureScale;

// CORRECTION
uniform vec2 scale;

// DEPTH
uniform sampler2DRect depthTex;
uniform sampler2DRect varianceTex;
uniform sampler2DRect speedVarianceTex;
uniform sampler2DRect paletteTex;

uniform vec2 principalPoint;
uniform vec2 fov;

uniform float farClip;
uniform float nearClip;

uniform float edgeClip;

uniform float leftClip;
uniform float rightClip;
uniform float topClip;
uniform float bottomClip;

uniform vec2 simplify;

//COLOR INTRINSICS
uniform mat4 extrinsics;
uniform vec2 colorFOV;
uniform vec2 colorPP;
uniform vec3 dK;
uniform vec2 dP;

uniform float flowPosition;

uniform float maxExtend;
uniform float extendThreshold;
uniform float extendFalloff;
uniform float varianceEffect;

uniform float pureColorFlicker;

uniform vec2 headPosition;
varying float headDistance;

varying float positionValid;
varying float pureColor;

const float epsilon = 1e-6;

float depthAtPosition(vec2 samplePosition){
  
  if(samplePosition.x >= 640.0*leftClip &&
	 samplePosition.x <= 640.0*rightClip &&
	 samplePosition.y >= 480.0*topClip &&
	 samplePosition.y <= 480.0*bottomClip)
  {
	  return texture2DRect(depthTex, samplePosition).r * 65535.;
  }
  return 0.0;
}

vec2 flowedPosition(vec2 basePos, vec2 center){
	return vec2(basePos.x, mod(basePos.y + flowPosition * texture2DRect(speedVarianceTex,vec2(center.x,.5)).r, 480.0));
	//return vec2(basePos.x, mod(basePos.y + flowPosition, 480.0));
}

float extendForPoint(vec2 point, vec2 center){
//	return 1.0;
	float headDistance2d = distance(headPosition, flowedPosition(point,center) );
	return max(maxExtend - ( max(headDistance2d - extendThreshold, 0.) / extendFalloff), 0. ) * mix(1.0, texture2DRect(varianceTex,center).r, varianceEffect);
	//return texture2DRect(varianceTex,center).r;
}

///MAIN ---------------------------
void main(void)
{
    //align to texture
    vec2 halfvec = vec2(.5,.5);
	
	//head position
	float headDepth = depthAtPosition(floor(headPosition) + halfvec);
	vec3 headPos3d = vec3((headPosition.x - principalPoint.x) * headDepth / fov.x,
						  (headPosition.y - principalPoint.y) * headDepth / fov.y, headDepth);
	
	vec2 center = gl_MultiTexCoord0.st - gl_Normal.xy;
	float extend = extendForPoint(gl_MultiTexCoord0.st, center);
	
	vec2 vertexPos = center + gl_Normal.xy * extend;

	vec2 samplePos = flowedPosition(vertexPos, center);
	vec2 neighboralocation = center + gl_Color.xy;
	vec2 neighborblocation = center + gl_Color.zw;
	vec2 neighborsamplea = flowedPosition(center + gl_Color.xy * extendForPoint(neighboralocation,center),center );
	vec2 neighborsampleb = flowedPosition(center + gl_Color.zw * extendForPoint(neighborblocation,center),center );

    float depth = depthAtPosition(floor(samplePos.xy) + halfvec);
	float neighbora = depthAtPosition(floor(neighborsamplea) + halfvec);
	float neighborb = depthAtPosition(floor(neighborsampleb) + halfvec);

	vec4 pos = vec4((samplePos.x - principalPoint.x) * depth / fov.x,
                    (samplePos.y - principalPoint.y) * depth / fov.y, depth, 1.0);
	
	headDistance = distance(pos.xyz,headPos3d);
	
    //cull invalid verts
    positionValid = (depth < farClip &&
					 neighbora < farClip &&
					 neighborb < farClip &&
					 
					 depth > nearClip &&
					 neighbora > nearClip &&
					 neighborb > nearClip &&
					 
					 abs(neighborsamplea.y - samplePos.y) < 30. &&
					 abs(neighborsampleb.y - samplePos.y) < 30. &&
				
					 abs(neighbora - depth) < edgeClip &&
					 abs(neighborb - depth) < edgeClip)
						? 1.0 : 0.0;
	
	
	
    //projective texture on the geometry
	//http://opencv.willowgarage.com/documentation/camera_calibration_and_3d_reconstruction.html
	vec4 texCd;
	vec4 projection = extrinsics * pos;
	
	if(projection.z != 0.0) {
		
		vec2 xyp = projection.xy / projection.z;
		float r2 = pow(xyp.x, 2.0) + pow(xyp.y, 2.0);
		float r4 = r2*r2;
		float r6 = r4*r2;
		vec2 xypp = xyp;
		xypp.x = xyp.x * (1.0 + dK.x*r2 + dK.y*r4 + dK.z*r6) + 2.0*dP.x * xyp.x * xyp.y + dP.y*(r2 + 2.0 * pow(xyp.x,2.0) );
		xypp.y = xyp.y * (1.0 + dK.x*r2 + dK.y*r4 + dK.z*r6) + dP.x * (r2 + 2.0*pow(xyp.y, 2.0) ) + 2.0*dP.y*xyp.x*xyp.y;
		vec2 uv = (colorFOV * xypp + colorPP) * textureScale;
		texCd.xy = ((uv-dim/2.0) * scale) + dim/2.0;
	}

	
	gl_TexCoord[0] = texCd;
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * pos;
    gl_FrontColor = texture2DRect(paletteTex,gl_Vertex.xy);
	pureColor = mod(gl_Vertex.z+pureColorFlicker,1.0);
	
	gl_FrontColor.a = extend;
}
