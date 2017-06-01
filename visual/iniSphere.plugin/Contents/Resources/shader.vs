uniform float v_offset;
uniform bool v_useNoise;
uniform float v_scaleIn;
uniform float v_scaleOut;
uniform bool e_useNoise;
uniform bool useHeightMap;
uniform bool useENHeightMap;
uniform float e_offset;
uniform float e_scaleIn;
uniform float e_scaleOut;
uniform sampler2DRect heightMap;

uniform vec3 LightPosition;

varying vec3 n;
varying vec4 pos;
varying mat3 rotmat;
uniform float displaceOffset;
varying float df;
varying float e_noise;
varying vec3 erosion_noise;

uniform float texelSize;
uniform float normalStrength;

vec4 ComputeNormals(vec2 uv)
{
	//texelSize = 0.1;
	//normalStrength = 0.5;
	
    float tl = abs(texture2DRect( heightMap, gl_TexCoord[0].xy + texelSize * vec2(-1, -1)).x);   // top left
    float  l = abs(texture2DRect( heightMap, gl_TexCoord[0].xy + texelSize * vec2(-1,  0)).x);   // left
    float bl = abs(texture2DRect( heightMap, gl_TexCoord[0].xy + texelSize * vec2(-1,  1)).x);   // bottom left
    float  t = abs(texture2DRect( heightMap, gl_TexCoord[0].xy + texelSize * vec2( 0, -1)).x);   // top
    float  b = abs(texture2DRect( heightMap, gl_TexCoord[0].xy + texelSize * vec2( 0,  1)).x);   // bottom
    float tr = abs(texture2DRect( heightMap, gl_TexCoord[0].xy + texelSize * vec2( 1, -1)).x);   // top right
    float  r = abs(texture2DRect( heightMap, gl_TexCoord[0].xy + texelSize * vec2( 1,  0)).x);   // right
    float br = abs(texture2DRect( heightMap, gl_TexCoord[0].xy + texelSize * vec2( 1,  1)).x);   // bottom right
 
    // Compute dx using Sobel:
    //           -1 0 1 
    //           -2 0 2
    //           -1 0 1
    float dX = tr + 2.0*r + br -tl - 2.0*l - bl;
 
    // Compute dy using Sobel:
    //           -1 -2 -1 
    //            0  0  0
    //            1  2  1
    float dY = bl + 2.0*b + br -tl - 2.0*t - tr;
 
    // Build the normalized normal
    vec4 N = vec4(normalize(vec3(dX, 1.0 / (normalStrength+1.0), dY)), 1.0);
 
    //convert (-1.0 , 1.0) to (0.0 , 1.0), if needed
    return N * 0.5 + 0.5;
}

void main()
{
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

	//Applica vertex noise alla posizione del vertice
	vec3 vertexNoise = vec3(0.0);

	if(v_useNoise)
		vertexNoise = noise3(v_offset + gl_Vertex.xyz * v_scaleIn) * v_scaleOut;
	e_noise = 0.0;
	if(e_useNoise){
		erosion_noise = noise3(e_offset + gl_Vertex.xyz * e_scaleIn) * e_scaleOut;
		e_noise = abs(erosion_noise.x) + abs(erosion_noise.y) + abs(erosion_noise.z) * 1.5;
	}
	
	vec3 vertex = gl_Vertex.xyz + vertexNoise;
	pos = gl_ModelViewProjectionMatrix * vec4(vertex,1.0);

	//Applica vertex noise alla normale del vertice (la normale della sfera coincide con la
	//posizione del suo vertice, quindi possiamo applicargli lo stesso vertexNoise.
	n = gl_NormalMatrix * gl_Normal;
	n = n + vertexNoise;

	//Calcola tangente e binormale se si usa il bump mapping
	vec3 tangent;
	vec3 binormal;

	vec3 c1 = cross( n, vec3(0.0, 0.0, 1.0) );
	vec3 c2 = cross( n, vec3(0.0, 1.0, 0.0) );

	if( length(c1)>length(c2) ) tangent = c1;
	else tangent = c2;

	tangent = normalize(tangent);

	binormal = cross(n, tangent);
	binormal = normalize(binormal);

	rotmat = mat3(tangent,binormal, n);

	//Vertex displacement rispetto alla posizione del vertice senza noise
	vec3 dv;
	vec4 newVertexPos = vec4(vertex, 1.0);
	n = newVertexPos.xyz;

	df = 0.0;
	if(useHeightMap)
		dv = texture2DRect( heightMap, gl_TexCoord[0].xy).rgb;
	else if(useENHeightMap)
		dv = erosion_noise;
	if(useHeightMap || useENHeightMap){
		df = 0.30*dv.x + 0.59*dv.y + 0.11*dv.z;
		df *= displaceOffset;
		newVertexPos = vec4(n * df, 0.0) + vec4(vertex,1.0);
		newVertexPos = vec4(n * df, 0.0) + vec4(gl_Vertex.xyz,1.0);
		//n = newVertexPos.xyz;
		n = ComputeNormals(gl_TexCoord[0].xy).xyz;
	}
	
	gl_Position = gl_ModelViewProjectionMatrix * newVertexPos;
	gl_FrontColor = gl_Color;
}