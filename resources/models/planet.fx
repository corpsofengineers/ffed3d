
#define FRACTALS_IN_PIXEL_SHADER 1

float4x4 viewprojmat;	// view*proj
float4x4 worldmat;	// world
float4x4 scalemat;	// scale
float4x4 rotmat;	// scale
float4x4 worldInverse;	// world inv mat
float3 tangent;
float3 light;		// light source
float4 lightcol;	// light color

texture tex;		// current texture

texture permGrad;
texture permTexture2d;

texture chunkheight;

float x_off, y_off, div, width, seed, dist;
float h_map_size, chunk_size;
int side;

float time;

sampler s_chunkheight = sampler_state 
{
    texture = <chunkheight>;
    AddressU  = CLAMP;        
    AddressV  = CLAMP;
    MIPFILTER = NONE;
    MINFILTER = ANISOTROPIC;
    MAGFILTER = ANISOTROPIC;
};

sampler s_grad = sampler_state 
{
    texture = <permGrad>;
    AddressU  = WRAP;        
    AddressV  = CLAMP;
    MIPFILTER = NONE;
    MINFILTER = POINT;
    MAGFILTER = POINT;
};

sampler permSampler2d = sampler_state 
{
    texture = <permTexture2d>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    MIPFILTER = NONE;
    MINFILTER = POINT;
    MAGFILTER = POINT;
};

sampler s_tex = sampler_state 
{
    texture = <tex>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

struct VS_OUTPUT
{
    float4 position	: POSITION;
    float3 normal	: TEXCOORD1;
    float4 color	: COLOR0;    
    float2 tex_vu	: TEXCOORD0;
    float2 tex_vu2	: TEXCOORD4;
    float2 tex_vu3	: TEXCOORD5;
	float3 lightDir	: TEXCOORD2;    
    float3 eye	: TEXCOORD3;
};

VS_OUTPUT V_Water(
    float3 pos		: POSITION,
    float3 nor		: NORMAL,
    float4 col		: COLOR0,
	float2 tex_vu	: TEXCOORD0
    )
{
    VS_OUTPUT Out = (VS_OUTPUT)0;

	float4 vertex = mul(float4(pos, 1), worldmat);

    Out.position = mul(vertex, viewprojmat);
    Out.normal = normalize(mul(nor, (float3x3)worldmat)); 
    Out.color = col;
	Out.tex_vu = tex_vu;
	
	float3x3 matTangentSpace;
		
	matTangentSpace[0] = normalize(mul(-tangent, (float3x3)worldmat));
	matTangentSpace[1] = normalize(cross(tangent, Out.normal));
	matTangentSpace[2] = normalize(Out.normal);
	
	Out.lightDir = mul(matTangentSpace, normalize(light));
	Out.eye = mul(matTangentSpace, -normalize(vertex.xyz));

	//вычисляем новые текстурные координаты
	Out.tex_vu2.x = tex_vu.x + time*0.1;
	//т.е. смещаем данные текстурные координаты
	Out.tex_vu2.y = tex_vu.y + time*0.1;
	//для обеих карт по и против
	Out.tex_vu3.x = tex_vu.x - time*0.1;
	//часовой стрелки по окружности соответственно
	Out.tex_vu3.y = tex_vu.y - time*0.1;

    return Out;
}

VS_OUTPUT V_Land(
    float3 pos		: POSITION,
    float3 nor		: NORMAL,
    float4 col		: COLOR0,
	float2 tex_vu	: TEXCOORD0
    )
{
    VS_OUTPUT Out = (VS_OUTPUT)0;

	float4 vertex = mul(float4(pos, 1), worldmat);

    Out.position = mul(vertex, viewprojmat);
    Out.normal = normalize(mul(nor, (float3x3)worldmat)); 
    Out.color = col;
	Out.tex_vu = tex_vu;
	
	Out.lightDir = normalize(light);
	Out.eye = -normalize(vertex.xyz);


    return Out;
}


float4 P_Water(
	float2 tex_vu	: TEXCOORD0,
	float2 tex_vu2	: TEXCOORD4,	
	float2 tex_vu3	: TEXCOORD5,		
    float3 normal	: TEXCOORD1,
    float3 lightDir : TEXCOORD2,
    float3 eye	: TEXCOORD3,
    float4 color	: COLOR0
  ) : COLOR0
{

	float4 tx_base = tex2D(s_tex, tex_vu2);
  
	float3 tx_norm = float3(0,1,0);//2.0f * tex2D(s_bump, tex_vu2) - 1.0f;
	float3 tx_norm2 = float3(0,1,0);//2.0f * tex2D(s_waves, tex_vu3) - 1.0f;  
	tx_norm = normalize(tx_norm+tx_norm2);
  
	float nrmd_light = dot(tx_norm, lightDir);
	float4 diffuse = (color * tx_base * lightcol) * nrmd_light;
  
	float3 R = normalize(reflect(-lightDir, tx_norm));
	float VdotR = saturate(dot(R, normalize(eye)));
	VdotR /= 128.0 - VdotR * 128.0 + VdotR;
	float4 specular = (lightcol * VdotR);

	diffuse += specular;
  
	return diffuse;
}

float4 mnoise(float2 st)
{
	float g = 1.0; 
	float4 r = 0.0;
	for (int i = 0.0; i < 8.0; i++) 
	{
		r += g * (2.0 * tex2D( permSampler2d, st) - 1.0);
		st = st * 2.0;   
		g *= 0.5; 
	} 
	return r;
}

float4 P_Land(
	float2 tex_vu	: TEXCOORD0,
	float2 tex_vu2	: TEXCOORD4,	
	float2 tex_vu3	: TEXCOORD5,		
    float3 normal	: TEXCOORD1,
    float3 lightDir : TEXCOORD2,
    float3 eye	: TEXCOORD3,
    float4 color	: COLOR0
  ) : COLOR0
{

	float2 tex_vu_n = tex_vu * 50.0;

	float texel_size = 1.0 / (float)256.0;

	float2 f;
    f.x = frac( tex_vu_n.x * (float)256.0);
    f.y = frac( tex_vu_n.y * (float)256.0);

	float4 t00 = mnoise(tex_vu_n + float2(0.0, 0.0));
	float4 t10 = mnoise(tex_vu_n + float2(texel_size, 0.0));
	float4 t01 = mnoise(tex_vu_n + float2(0.0, texel_size));
	float4 t11 = mnoise(tex_vu_n + float2(texel_size, texel_size));

	float4 tA = lerp( t00, t10, f.x);
	float4 tB = lerp( t01, t11, f.x);

	float4 r = lerp( tA, tB, f.y );

	float4 r2 = (r + mnoise(tex_vu_n * 50)) / 2.0;

	float noise = 1.0-saturate((r2.x+r2.y+r2.z+r2.w+color.x+color.y+color.z)/7.0);

	float4 tx_base = tex2D(s_tex, tex_vu) * 0.5 + 0.5;
	float4 tx_noise = float4(noise,noise,noise,0.0) * 0.5 + 0.5;
	color.xyz = saturate(color.xyz + (r.xyz-0.5) * 0.05); 

	float nrmd_light = dot(normal, lightDir) * 2.0;
	float4 diffuse = saturate((color * tx_base * tx_noise * lightcol) * nrmd_light);

	float3 R = normalize(reflect(-lightDir, normal));
	float VdotR = saturate(dot(R, normalize(eye)));
	VdotR /= 128.0 - VdotR * 128.0 + VdotR;
	float4 specular = (lightcol * VdotR) * 0.17;

	diffuse += specular;

	return diffuse;
}

float4 P_Atmo(
	float2 tex_vu	: TEXCOORD0,
	float2 tex_vu2	: TEXCOORD4,	
	float2 tex_vu3	: TEXCOORD5,		
    float3 normal	: TEXCOORD1,
    float3 lightDir : TEXCOORD2,
    float3 eye	: TEXCOORD3,
    float4 color	: COLOR0
  ) : COLOR0
{
	float4 tx_base = tex2D(s_tex, tex_vu);

	float nrmd_light = dot(normal, normalize(lightDir));
	float4 diffuse = (tx_base * lightcol) * nrmd_light;
	diffuse.a = 0.2;

	return diffuse;
}

float3 fade(float3 t)
{
        return t * t * t * (t * (t * 6 - 15) + 10); // new curve
		//return t * t * (3 - 2 * t);
}

float4 perm2d(float2 p)
{
	return tex2D(permSampler2d, p);
}

float4 perm3d(float3 p)
{
#ifdef FRACTALS_IN_PIXEL_SHADER
	return tex2D(permSampler2d, p.xy) + p.z;
#else
	return tex2Dlod(permSampler2d, float4(p.xy, 0.0, 1.0)) + p.z;
#endif
}

float gradperm(float x, float3 p)
{
#ifdef FRACTALS_IN_PIXEL_SHADER
	return dot(tex1D(s_grad, x).xyz, p);
#else
	return dot(tex2Dlod(s_grad, float4(x, 0.0, 0.0, 1.0)).xyz, p);
#endif
}

const float dim = 1.0 / 256.0;
const float3 goffsets[7] = {float3(-1, 0, 0),
							float3( 0,-1, 0),
							float3(-1,-1, 0),
							float3( 0, 0,-1),
							float3(-1, 0,-1),
							float3( 0,-1,-1),
							float3(-1,-1,-1)};

// Improved 3d noise basis function
float inoise(float3 p)
{
	p += seed;
	float3 P = fmod(floor(p), 256.0);   // FIND UNIT CUBE THAT CONTAINS POINT
	p -= floor(p);                      // FIND RELATIVE X,Y,Z OF POINT IN CUBE.
	float3 f = fade(p);                 // COMPUTE FADE CURVES FOR EACH OF X,Y,Z.

	P = P * dim;

	// HASH COORDINATES OF THE 8 CUBE CORNERS
	float4 AA = perm3d(P);

	// AND ADD BLENDED RESULTS FROM 8 CORNERS OF CUBE
	return lerp( lerp( lerp( gradperm(AA.x, p ),
							gradperm(AA.z, p + goffsets[0]), f.x),
					lerp( gradperm(AA.y, p + goffsets[1]),
							gradperm(AA.w, p + goffsets[2] ), f.x), f.y),

					lerp( lerp( gradperm(AA.x + dim, p + goffsets[3]),
							gradperm(AA.z + dim, p + goffsets[4]), f.x),
					lerp( gradperm(AA.y + dim, p + goffsets[5]),
							gradperm(AA.w + dim, p + goffsets[6]), f.x), f.y), f.z);
}

float fBm(float3 p, int octaves, float lacunarity = 2.0, float gain = 0.5)
{
        float freq = 1.0f, amp  = 0.5f;
        float sum  = 0.0;
        for(int i=0; i < octaves; i++) {
                sum += inoise(p*freq)*amp;
                freq *= lacunarity;
                amp *= gain;
        }
        return sum;
}

float turbulence(float3 p, int octaves, float lacunarity = 2.0, float gain = 0.5)
{
		float freq = 1.0, amp = 1.0;
        float sum = 0.0; 
        for(int i=0; i < octaves; i++) {
                sum += abs(inoise(p*freq))*amp;
                freq *= lacunarity;
                amp *= gain;
        }
        return sum;
}

// Ridged multifractal
// See "Texturing & Modeling, A Procedural Approach", Chapter 12
float ridge(float h, float offset)
{
    h = abs(h);
    h = offset - h;
    h = h * h;
    return h;
}

float ridgedmf(float3 p, int octaves, float lacunarity, float gain = 0.05, float offset = 1.0)
{
        float sum = 0;
        float freq = 1.0;
        float amp = 0.5;
        float prev = 1.0;
        for(int i=0; i < octaves; i++)
        {
                float n = ridge(inoise(p*freq), offset);
                sum += n*amp*prev;
                prev = n;
                freq *= lacunarity;
                amp *= gain;
        }
        return sum;
}

const float hscale = (1.0 / 48.0);

float getHeight(float3 v)
{
	float height = fBm(v, 5, 2.7, 0.5);
	height = saturate(height);
	if (height > 0.0) {
		float montes = ridgedmf(v.yxz, 6, 2.7, 1.0, 1.0);
		height += montes * height;
		height = saturate(height);
		//if (height < 0.2) {
		//	float canions = ridgedmf(v.zxy, 3, 2.9521, 8.5, 0.3);
		//	height -= canions * height;
		//	height = saturate(height);
		//}
		
	}
    return height * hscale;
}

float3 GetSide(int side, float3 p)
{
    if (side == 0)
        return float3(-p.x, p.y, p.z);
    else if (side == 1)
        return float3( p.x, p.z, p.y);
    else if (side == 2)
        return float3( p.y,-p.z, p.x);
    else if (side == 3)
        return float3( p.z, p.y, p.x);
    else if (side == 4)
        return float3(-p.z, p.y,-p.x);
    else if (side == 5)
        return float3( p.x, p.y,-p.z);
    else
        return float3(0.0, 0.0, 0.0);
}

float3 MapToSphere(float3 v)
{
	float3 vertex;

	vertex.x = v.x * sqrt(1.0 - v.y * v.y * 0.5 - v.z * v.z * 0.5 + v.y * v.y * v.z * v.z / 3.0);
	vertex.y = v.y * sqrt(1.0 - v.z * v.z * 0.5 - v.x * v.x * 0.5 + v.z * v.z * v.x * v.x / 3.0);
	vertex.z = v.z * sqrt(1.0 - v.x * v.x * 0.5 - v.y * v.y * 0.5 + v.x * v.x * v.y * v.y / 3.0);
	
	return vertex;
}

#define M_PI  3.14159265358979323846
#define M_1_PI 0.318309886183790671538

VS_OUTPUT V_Height(
    float3 pos		: POSITION
    )
{
    VS_OUTPUT Out = (VS_OUTPUT)0;

	float3 vertex;

	pos.xy *= (h_map_size) / (h_map_size - 2);

	// divide
	vertex.x = (pos.x + 1.0) / div - 1.0;
	vertex.y = (pos.y + 1.0) / div - 1.0;
	vertex.z = -1.0;

	// offsets
	vertex.x += x_off;
	vertex.y += y_off;

	vertex = MapToSphere(GetSide(side, vertex));

	pos.xy *= (h_map_size - 2) / (h_map_size);
	Out.position = float4(pos.x, -pos.y, 0.0, 1.0);

#ifdef FRACTALS_IN_PIXEL_SHADER
    Out.normal = float3(vertex.x, vertex.y, vertex.z);
#else
	float height = getHeight(vertex);
	Out.color = float4(height, height, height, 0.0);
#endif
    return Out;
}

float4 P_Height(
#ifdef FRACTALS_IN_PIXEL_SHADER
	float3 vertex	: TEXCOORD1
#else
    float4 color	: COLOR0
#endif
  ) : COLOR0
{
#ifdef FRACTALS_IN_PIXEL_SHADER
	float height = getHeight(vertex);
	return float4(height, height, height, 0.0);
#else
	return color;
#endif
}

VS_OUTPUT V_Test(
    float3 pos		: POSITION,
    float3 nor		: NORMAL
    )
{
    VS_OUTPUT Out = (VS_OUTPUT)0;

	float3 v;

	// divide
	v.x = (pos.x + 1.0) / div - 1.0;
	v.y = (pos.y + 1.0) / div - 1.0;
	v.z = -1.0;

	// offsets
	v.x += x_off;
	v.y += y_off;

	float3 tan = MapToSphere(GetSide(side, float3(v.x + 0.0001, v.y, v.z)));

	float4 vertex = float4(MapToSphere(GetSide(side, v)), 1.0);

	if (abs(pos.z) > 1.0)
		vertex.xyz *= (chunk_size - 2) / (chunk_size);

	tan = normalize(tan - vertex.xyz);

	//float4 uv;
	//uv.x = atan2(vertex.x, vertex.z) / (2.0 * M_PI) + 0.5;
	//uv.y = asin(vertex.y) / M_PI + 0.5;
	//uv.z = 0.0f;
	//uv.w = 0.0f;

	pos.xy *= (chunk_size - 2) / (chunk_size);
	float4 buv = float4((pos.x + 1.0) * 0.5, (pos.y + 1.0) * 0.5,0.0,0.0);
	buv.xy += 0.5 / (h_map_size);

	vertex.xyz = mul(vertex.xyz, (float3x3)scalemat);

	float height = tex2Dlod(s_chunkheight, buv).r;

	vertex.xyz *= 1.0 + height;
	
	float3 normal = normalize(vertex.xyz);

	vertex = mul(vertex, worldmat);

	float3x3 matTangentSpace;
		
	normal = normalize(mul(normal, (float3x3)rotmat));
	tan = normalize(mul(tan, (float3x3)rotmat));

	matTangentSpace[0] = tan;
	matTangentSpace[1] = cross(tan, normal);
	matTangentSpace[2] = normal;
	
	Out.tex_vu = buv;
	Out.lightDir = mul(matTangentSpace, normalize(light));
	Out.eye = mul(matTangentSpace, -normalize(vertex.xyz));
    Out.position = mul(vertex, viewprojmat);
    //Out.normal = normalize(mul(normal, (float3x3)rotmat)); 
    Out.color = float4(height,height,height,0.0);

	float dscale = 1.0;//(1.0+(1.0-width)) * 2.0;
	float3 Scale = float3(1.0 / (h_map_size), 1.0 / (h_map_size), 512.0 * dscale);
	float4 dx = float4(0.0, Scale.y, 0.0, 0.0);
	float4 dy = float4(Scale.x, 0.0, 0.0, 0.0);

	float pc = height * Scale.z;
	float pl = tex2Dlod(s_chunkheight, buv - dx).r * Scale.z;
	float pr = tex2Dlod(s_chunkheight, buv + dx).r * Scale.z;
	float pu = tex2Dlod(s_chunkheight, buv - dy).r * Scale.z;
	float pd = tex2Dlod(s_chunkheight, buv + dy).r * Scale.z;

	float3 s;
	s  = normalize(float3(pu-pc, pc-pl, 1.0));
	s += normalize(float3(pc-pd, pc-pl, 1.0));
	s += normalize(float3(pc-pd, pr-pc, 1.0));
	s += normalize(float3(pu-pc, pr-pc, 1.0));

	Out.normal = normalize(s);

    return Out;
}

float4 P_Test(
	float2 tex_vu	: TEXCOORD0,
    float3 lightDir : TEXCOORD2,
	float3 normal	: TEXCOORD1,
	float4 color	: COLOR0,
    float3 eye	: TEXCOORD3
  ) : COLOR0
{

	float4 tex;
	
	//tex = tex2D(s_tex, tex_vu);

	float nrmd_light = dot(normal, normalize(lightDir));

	tex = float4(1.0,1.0,1.0,1.0);

	float4 diffuse;

	float h = color.r * 32;
	if (h < 0.01)
		diffuse = float4(0.1+h*20, 0.1+h*20, 0.3+h*20, 1.0) * tex * nrmd_light;
	else
		diffuse =  (0.7 + color.r * 12.0) * tex * nrmd_light;

	diffuse = saturate(diffuse);

	//float4 diffuse = color * nrmd_light;

	if (h < 0.01) {
		float3 R = normalize(reflect(-lightDir, normal));
		float VdotR = saturate(dot(R, normalize(eye)));
		VdotR /= 32.0 - VdotR * 32.0 + VdotR;
		float4 specular = (lightcol * VdotR) * 0.25;

		diffuse += specular;
	}

	return diffuse;
}

technique Main
{
    pass P0
    {
        VertexShader = compile vs_3_0 V_Land();
        PixelShader  = compile ps_3_0 P_Land();
    }
    pass P1
    {
        VertexShader = compile vs_3_0 V_Water();
        PixelShader  = compile ps_3_0 P_Water();
    }
    pass P2
    {
        VertexShader = compile vs_3_0 V_Land();
        PixelShader  = compile ps_3_0 P_Land();
    }
    pass P3
    {
        VertexShader = compile vs_3_0 V_Land();
        PixelShader  = compile ps_3_0 P_Atmo();
    } 
    pass P4
    {
        VertexShader = compile vs_3_0 V_Height();
        PixelShader  = compile ps_3_0 P_Height();
    }
    pass P5
    {
        VertexShader = compile vs_3_0 V_Test();
        PixelShader  = compile ps_3_0 P_Test();
    }  
}
