#version 110
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect colorTex;
uniform float brightness;
uniform float contrast;
uniform float headSphereRadius;
uniform float headEffectFalloff;
varying float positionValid;
varying float headDistance;

const float epsilon = 1e-6;

void main()
{
    if(positionValid < epsilon){
    	discard;
        return;
    }
	vec4 col = texture2DRect(colorTex, gl_TexCoord[0].st);
	
	//Apply contrast
	col.rgb = ((col.rgb - 0.5) * max(contrast, 0.0)) + 0.5;
	
	// Apply brightness.
	col.rgb += brightness;
		
	gl_FragColor = mix(gl_Color, col, max(1. - ( max(headDistance - headSphereRadius,0.) / headEffectFalloff), 0.)) * gl_Color.a;
	gl_FragColor.a = 1.0;
;
}
