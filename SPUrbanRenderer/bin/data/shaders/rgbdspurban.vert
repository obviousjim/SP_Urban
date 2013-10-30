#version 110
#extension GL_ARB_texture_rectangle : enable

uniform vec2 dim;
uniform vec2 textureScale;

// CORRECTION
uniform vec2 scale;

// DEPTH
uniform sampler2DRect depthTex;
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
uniform float extend;
uniform vec2 headPosition;
varying float headDistance;

varying float positionValid;

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

///MAIN ---------------------------
void main(void)
{
    //align to texture
    vec2 halfvec = vec2(.5,.5);
	vec2 center = gl_MultiTexCoord0.st - gl_Normal.xy;
	vec2 vertexPos = center + gl_Normal.xy * extend;

	vec2 samplePos = vec2(vertexPos.x, mod(vertexPos.y + flowPosition, 480.0));
	vec2 neighborsamplea = vec2(center.x + gl_Color.x * extend, mod(center.y + gl_Color.y*extend + flowPosition, 480.0));
	vec2 neighborsampleb = vec2(center.x + gl_Color.z * extend, mod(center.y + gl_Color.w*extend + flowPosition, 480.0));
	
    float depth = depthAtPosition(floor(samplePos.xy) + halfvec);
	float neighbora = depthAtPosition(floor(neighborsamplea) + halfvec);
	float neighborb = depthAtPosition(floor(neighborsampleb) + halfvec);
    //float right = depthAtPosition(floor(samplePos.xy + vec2(simplify.x,0.0))  + halfvec );
    //float down  = depthAtPosition(floor(samplePos.xy + vec2(0.0,simplify.y))  + halfvec );
    //float left  = depthAtPosition(floor(samplePos.xy + vec2(-simplify.x,0.0)) + halfvec );
    //float up    = depthAtPosition(floor(samplePos.xy + vec2(0.0,-simplify.y)) + halfvec );
    //float bl    = depthAtPosition(vec2(floor(samplePos.x - simplify.x),floor( samplePos.y + simplify.y)) + halfvec );
    //float ur    = depthAtPosition(vec2(floor(samplePos.x  + simplify.x),floor(samplePos.y - simplify.y)) + halfvec );

    //cull invalid verts
    positionValid = (depth < farClip &&
					 neighbora < farClip &&
					 neighborb < farClip &&
					 
					 depth > nearClip &&
					 neighbora > nearClip &&
					 neighborb > nearClip &&
					 
					 abs(neighbora - depth) < edgeClip &&
					 abs(neighborb - depth) < edgeClip)
						? 1.0 : 0.0;
	
	
	vec4 pos = vec4((samplePos.x - principalPoint.x) * depth / fov.x,
                    (samplePos.y - principalPoint.y) * depth / fov.y, depth, 1.0);
	
	//head position
	float headDepth = depthAtPosition(floor(headPosition) + halfvec);
	vec3 headPos3d = vec3((headPosition.x - principalPoint.x) * headDepth / fov.x,
						  (headPosition.y - principalPoint.y) * headDepth / fov.y, depth);
	

	headDistance = distance(pos.xyz,headPos3d);
	
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
    gl_FrontColor = gl_Vertex;
	gl_FrontColor.a = 1.0;
}
