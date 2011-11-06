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

float x_off, y_off, div, type, width;
int uid;

float time;

sampler s_chunkheight = sampler_state 
{
    texture = <chunkheight>;
    AddressU  = CLAMP;        
    AddressV  = CLAMP;
    MIPFILTER = NONE;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

sampler s_grad = sampler_state 
{
    texture = <permGrad>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
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
        //return t * t * t * (t * (t * 6 - 15) + 10); // new curve
		return t * t * (3 - 2 * t);
}

float4 perm2d(float2 p)
{
        return tex2Dlod(permSampler2d, float4(p, 0.0, 0.0));
}

float4 perm3d(float3 p, float seed = 0)
{
        return tex2Dlod(permSampler2d, float4(p.xy, 0.0, 0.0)) + p.z + seed;
}

float gradperm(float x, float3 p)
{
        return dot(tex1Dlod(s_grad, float4(x, 0.0, 0.0, 0.0)).xyz, p);
}

const float dim = 1.0 / 256.0;

// Improved 3d noise basis function
float inoise(float3 p, float seed = 0)
{
        float3 P = fmod(floor(p), 256.0);       // FIND UNIT CUBE THAT CONTAINS POINT
        p -= floor(p);                      // FIND RELATIVE X,Y,Z OF POINT IN CUBE.
        float3 f = fade(p);                     // COMPUTE FADE CURVES FOR EACH OF X,Y,Z.

        P = P * dim;

    // HASH COORDINATES OF THE 8 CUBE CORNERS
        float4 AA = perm3d(P, seed);

        // AND ADD BLENDED RESULTS FROM 8 CORNERS OF CUBE
        return lerp( lerp( lerp( gradperm(AA.x, p ),
                                gradperm(AA.z, p + float3(-1, 0, 0) ), f.x),
                        lerp( gradperm(AA.y, p + float3(0, -1, 0) ),
                                gradperm(AA.w, p + float3(-1, -1, 0) ), f.x), f.y),

                        lerp( lerp( gradperm(AA.x + dim, p + float3(0, 0, -1) ),
                                gradperm(AA.z + dim, p + float3(-1, 0, -1) ), f.x),
                        lerp( gradperm(AA.y + dim, p + float3(0, -1, -1) ),
                                gradperm(AA.w + dim, p + float3(-1, -1, -1) ), f.x), f.y), f.z);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// FRACTAL FUNCTIONS
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// fractal sum
float fBm(float3 p, int octaves, float lacunarity = 2.0, float gain = 0.5, float seed = 0)
{
        float freq = 1.0f, amp  = 0.5f;
        float sum  = 0.0;
        for(int i=0; i < octaves; i++) {
                sum += inoise(p*freq, seed)*amp;
                freq *= lacunarity;
                amp *= gain;
        }
        return sum;
}

float turbulence(float3 p, int octaves, float lacunarity = 2.0, float gain = 0.5, float seed = 0)
{
		float freq = 1.0, amp = 1.0;
        float sum = 0.0; 
        for(int i=0; i < octaves; i++) {
                sum += abs(inoise(p*freq, seed))*amp;
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

float ridgedmf(float3 p, int octaves, float lacunarity, float gain = 0.05, float offset = 1.0, float seed = 0)
{
        float sum = 0;
        float freq = 1.0;
        float amp = 0.5;
        float prev = 1.0;
        for(int i=0; i < octaves; i++)
        {
                float n = ridge(inoise(p*freq, seed), offset);
                sum += n*amp*prev;
                prev = n;
                freq *= lacunarity;
                amp *= gain;
        }
        return sum;
}

float getHeight(float3 v)
{
	float seed = frac(uid*0.000001);
	float height = fBm(v, 6, 2.7, 0.5, seed);
	float montes = ridgedmf(v, 6, 2.7, 1.0, 1.0, seed);
    height += montes * height;
	height = saturate(height);
    return height;
}

float3 GetSide(float side, float3 p)
{
    if (side == 0.0)
        return float3( p.x, p.y, p.z);
    else if (side == 1.0)
        return float3( p.x, p.z, p.y );
    else if (side == 2.0)
        return float3( p.x,-p.z, p.y);
    else if (side == 3.0)
        return float3( p.z, p.y, p.x);
    else if (side == 4.0)
        return float3(-p.z, p.y, p.x);
    else if (side == 5.0)
        return float3( p.x, p.y,-p.z);
    else
        return float3(0.0, 0.0, 0.0);
}

#define M_PI  3.14159265358979323846
#define M_1_PI 0.318309886183790671538

VS_OUTPUT V_Height(
    float3 pos		: POSITION
    )
{
    VS_OUTPUT Out = (VS_OUTPUT)0;

	float3 v, p;

	// divide
	p.x = pos.x / div;
	p.y = pos.y / div;
	p.z = pos.z;

	// offsets
	p.x = p.x + x_off;
	p.y = p.y + y_off;

	v = GetSide(type, p);

	float4 vertex;
	vertex.x = v.x * sqrt(1.0 - v.y * v.y * 0.5 - v.z * v.z * 0.5 + v.y * v.y * v.z * v.z / 3.0);
	vertex.y = v.y * sqrt(1.0 - v.z * v.z * 0.5 - v.x * v.x * 0.5 + v.z * v.z * v.x * v.x / 3.0);
	vertex.z = v.z * sqrt(1.0 - v.x * v.x * 0.5 - v.y * v.y * 0.5 + v.x * v.x * v.y * v.y / 3.0);

	float height = getHeight(vertex.xyz);

    Out.position = float4((pos.x * 2.0 - 1.0), (pos.y * 2.0 - 1.0), 0.0, 1.0);
    Out.color = float4(height,height,height,0.0);
	
    return Out;
}

float4 P_Height(
    float4 color	: COLOR0
  ) : COLOR0
{
	return color;
}

VS_OUTPUT V_Test(
    float3 pos		: POSITION,
    float3 nor		: NORMAL
    )
{
    VS_OUTPUT Out = (VS_OUTPUT)0;

	float3 v, p;

	// divide
	p.x = pos.x / div;
	p.y = pos.y / div;
	p.z = pos.z;

	// offsets
	p.x = p.x + x_off;
	p.y = p.y + y_off;

	v = GetSide(type, p);

	float4 vertex;
	vertex.x = v.x * sqrt(1.0 - v.y * v.y * 0.5 - v.z * v.z * 0.5 + v.y * v.y * v.z * v.z / 3.0);
	vertex.y = v.y * sqrt(1.0 - v.z * v.z * 0.5 - v.x * v.x * 0.5 + v.z * v.z * v.x * v.x / 3.0);
	vertex.z = v.z * sqrt(1.0 - v.x * v.x * 0.5 - v.y * v.y * 0.5 + v.x * v.x * v.y * v.y / 3.0);

	float4 uv;
	uv.x = atan2(vertex.x, vertex.z) / (2.0 * M_PI) + 0.5;
	uv.y = asin(vertex.y) / M_PI + 0.5;
	uv.z = 0.0f;
	uv.w = 0.0f;

	float4 buv = float4(pos.x,1.0-pos.y,0.0,1.0);

	//float height = tex2Dlod(s_chunkheight, buv).r;
	float height = getHeight(vertex.xyz);

	vertex *= 1.0 + height/64;

	vertex.xyz = mul(vertex, (float3x3)scalemat);

	float3 normal = normalize(vertex.xyz);

	float d = 1.0/32;

	float x_depth = tex2Dlod( s_chunkheight, buv + float4( d, 0, 0, 1 ) ).r - 
					tex2Dlod( s_chunkheight, buv + float4(-d, 0, 0, 1 ) ).r; 
	float y_depth = tex2Dlod( s_chunkheight, buv + float4( 0, d, 0, 1 ) ).r - 
					tex2Dlod( s_chunkheight, buv + float4( 0,-d, 0, 1 ) ).r; 
	float3 norm = normalize( float3( normal.x + 3.0*x_depth, normal.y + 3.0*y_depth, normal.z ) );

	vertex = mul(float4(vertex.xyz, 1.0), worldmat);
	vertex.xyz += 0.5;

	Out.tex_vu = uv.xy;

    Out.position = mul(vertex, viewprojmat);
    Out.normal = normalize(mul(norm, (float3x3)rotmat)); 
    Out.color = float4(height,height,height,0.0);
	
	Out.lightDir = normalize(light);//normalize(mul(-normalize(light), -(float3x3)rotmat));
	Out.eye = -normalize(vertex.xyz);

    return Out;
}

float4 P_Test(
	float2 tex_vu	: TEXCOORD0,
   float3 normal	: TEXCOORD1,
    float3 lightDir : TEXCOORD2,
    float3 eye	: TEXCOORD3,
	float4 position : TEXCOORD4,
    float4 color	: COLOR0
  ) : COLOR0
{

	float4 tex;
	//float r = tex2D(heightSampler, tex_vu).r;

	tex = float4(1.0,1.0,1.0,1.0);

	
	float nrmd_light = dot(normal, normalize(lightDir));


	float4 diffuse;

	if (color.r == 0.0)
		diffuse = float4(0.1, 0.1, 0.3, 1.0) * nrmd_light;
	else
		diffuse = saturate((color + 0.5) * tex * nrmd_light);

	//float4 diffuse = color * nrmd_light;

	//if (r < 0.02 || r > 0.4) {
		//float3 R = normalize(reflect(-lightDir, normal));
		//float VdotR = saturate(dot(R, normalize(eye)));
		//VdotR /= 32.0 - VdotR * 32.0 + VdotR;
		//float4 specular = (lightcol * VdotR) * 0.25;

		//diffuse += specular;
	//}

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
