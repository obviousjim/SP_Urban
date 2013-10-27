#version 110
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect colorTex;
varying float positionValid;
const float epsilon = 1e-6;

void main()
{
    if(positionValid < epsilon){
    	discard;
        return;
    }

	gl_FragColor = texture2DRect(colorTex, gl_TexCoord[0].st) * gl_Color;
}
