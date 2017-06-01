uniform sampler2DRect textureMap;
varying vec4 pos;
uniform float mDiffColR;
uniform float mDiffColG;
uniform float mDiffColB;
uniform float mDiffColA;
uniform int useTMap;

varying vec3 n;
uniform float mShininess;
uniform float lightPosX;
uniform float lightPosY;
uniform float lightPosZ;
uniform float mSpecColR;
uniform float mSpecColG;
uniform float mSpecColB;
uniform float mSpecColA;
uniform int lightingMode;
uniform float lightingMul;

uniform float e_treshold;
uniform bool e_useDiscard;
uniform bool e_useAlpha;
uniform bool useENoiseCol;
uniform bool useSpecialENoiseCol;
uniform bool depthLighting;

varying mat3 rotmat;
varying float df;
varying float e_noise;
varying vec3 erosion_noise;

void main()
{	
	vec4 finalColor;

	vec4 textureCol = vec4(0.0,0.0,0.0,1.0);
	if(useTMap == 1)
		textureCol = texture2DRect(textureMap, gl_TexCoord[0].st);
		
	vec3 bumpNorm = textureCol.rgb;
	vec3 normal;
	vec4 noiseCol;
	normal  = bumpNorm;
	
	normal = -n;
	
	vec4 color;
	vec4 mDiffCol = vec4(mDiffColR, mDiffColG, mDiffColB, mDiffColA);
	vec4 mSpecCol = vec4(mSpecColR, mSpecColG, mSpecColB, mSpecColA);
	color = mDiffCol;
	finalColor = color;
	vec3 lightPos = vec3(lightPosX, lightPosY, lightPosZ);
	//diffuse lighting
	if(lightingMode == 0){
  		vec4 matspec = mSpecCol;
 		float shininess = mShininess;
 	 	vec4 lightspec = mSpecCol;
 	 	vec4 lpos = vec4(lightPos, 1.0);
		vec3 s = -normalize(pos-lpos).xyz;
		s = normalize(rotmat * normalize(s));
  		vec3 light = s.xyz;
 		vec3 n = normalize(normal);
  		vec3 r = -reflect(light, n);
  		r = normalize(r);
  		vec3 v = -pos.xyz;
  		v = normalize(v);
   
  		vec4 diffuse  = color * max(0.2, dot(n, s.xyz));
		vec4 specular;
  		if (shininess != 0.0)
    			specular = lightspec * matspec * pow(max(0.0,dot(r, v)), shininess);
  		else
			specular = vec4(0.0, 0.0, 0.0, 0.0);
		finalColor = vec4(diffuse.rgb, color.a) + specular;
	}
	
	//gl_FragColor = vec4(normal, 1.0);

	//Abilita l'erosione
	if(e_noise > 0.0){
		noiseCol = vec4(e_noise);
		noiseCol = vec4(noiseCol.xyz,1.0);
		if(useENoiseCol)
			finalColor *= vec4(erosion_noise.rgb, finalColor.a);
		else if(useSpecialENoiseCol){
			finalColor.rgb /= erosion_noise.rgb;
		}
		if(noiseCol.r < e_treshold){
			if(e_useAlpha)
				finalColor.a = noiseCol.r/e_treshold;
			else if(e_useDiscard)
				discard;
		}
	}
    
    //Aggiunge la texture
	if(useTMap == 1)
		finalColor *= textureCol;
    if(depthLighting){
	finalColor.rgb = finalColor.rgb*df* lightingMul+0.1;
/*
        float depthMul = 1.0-pos.z;
        if(depthMul<0.0) depthMul = 0.2;
    	finalColor.rgb *= depthMul;
        finalColor.rgb *= lightingMul;
*/
    }
	gl_FragColor = finalColor; 
	//gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0); 
	//gl_FragColor = vec4(n, 1.0); 
}
