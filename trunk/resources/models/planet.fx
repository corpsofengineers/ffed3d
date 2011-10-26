float4x4 viewprojmat;	// view*proj
float4x4 worldmat;	// world
float4x4 scalemat;	// scale
float4x4 rotmat;	// scale
float4x4 worldInverse;	// world inv mat
float3 tangent;
float3 light;		// light source
float4 lightcol;	// light color

texture tex;		// current texture
texture wave1;		// 722
texture wave2;		// 723
texture permTexture;
texture permTexture2d;

texture heightmap;

float x_off, y_off, div, div2, type;

float time;

sampler heightSampler = sampler_state 
{
    texture = <heightmap>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

sampler permSampler = sampler_state 
{
    texture = <permTexture>;
    AddressU  = WRAP;        
    AddressV  = CLAMP;
    MIPFILTER = POINT;
    MINFILTER = POINT;
    MAGFILTER = NONE;
};

sampler permSampler2d = sampler_state 
{
    texture = <permTexture2d>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    MIPFILTER = LINEAR;
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

sampler s_bump = sampler_state 
{
    texture = <wave1>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

sampler s_waves = sampler_state 
{
    texture = <wave2>;
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
  
	float3 tx_norm = 2.0f * tex2D(s_bump, tex_vu2) - 1.0f;
	float3 tx_norm2 = 2.0f * tex2D(s_waves, tex_vu3) - 1.0f;  
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

#define M_PI  3.14159265358979323846
#define M_1_PI 0.318309886183790671538

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

	v.x = p.x;
	v.y = p.y;
	v.z = p.z;

	// sides
	if (type == 1) {
		v.y = p.z;
		v.z = p.y;		
	}
	if (type == 2) {
		v.y = -p.z;
		v.z = p.y;		
	}
	if (type == 3) {
		v.x = p.z;
		v.z = p.x;		
	}
	if (type == 4) {
		v.x = -p.z;
		v.z = p.x;		
	}
	if (type == 5) {
		v.z = -p.z;		
	}

	float4 vertex;
	vertex.x = v.x * sqrt(1.0 - v.y * v.y * 0.5 - v.z * v.z * 0.5 + v.y * v.y * v.z * v.z / 3.0);
	vertex.y = v.y * sqrt(1.0 - v.z * v.z * 0.5 - v.x * v.x * 0.5 + v.z * v.z * v.x * v.x / 3.0);
	vertex.z = v.z * sqrt(1.0 - v.x * v.x * 0.5 - v.y * v.y * 0.5 + v.x * v.x * v.y * v.y / 3.0);

	float4 uv;
	uv.x = atan2(vertex.x, vertex.z) / (2. * M_PI) + 0.5;
	uv.y = asin(vertex.y) / M_PI + 0.5;
	uv.z = 1.0f;
	uv.w = 1.0f;

	float height = tex2Dlod(heightSampler, uv).r;

	vertex.xyz = mul(vertex, (float3x3)scalemat);

	float3 normal = normalize(vertex.xyz);

	vertex *= 1.0+(1.0 / 32 * height);

	vertex = mul(float4(vertex.xyz, 1.0), worldmat);

	Out.tex_vu = uv.xy;

    Out.position = mul(vertex, viewprojmat);
    Out.normal = normalize(mul(normal, (float3x3)rotmat)); 
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
    float4 color	: COLOR0
  ) : COLOR0
{

	float4 tex = tex2D(s_tex, tex_vu);

	float offset_x = 1.0/5400;
	float offset_y = 1.0/2700;
	
	float3 d;
	d.x = tex2D( heightSampler, tex_vu + float2( offset_x, 0 ) ) .r*1- tex2D( heightSampler, tex_vu + float2( -offset_x, 0 ) ).r*1; 
	d.y = tex2D( heightSampler, tex_vu + float2( 0, offset_y ) ) .r*1- tex2D( heightSampler, tex_vu + float2( 0, -offset_y ) ).r*1;
	d.z = 0.0;

	d = mul(d, -(float3x3)rotmat); 

	float3 norm = normalize( float3( normal.x  + 5.0 * d.x, normal.y + 5.0 * d.y, normal.z) ); 

	float nrmd_light = dot(norm, normalize(lightDir));
	float4 diffuse = tex;// * nrmd_light;

//	float3 R = normalize(reflect(-lightDir, norm));
//	float VdotR = saturate(dot(R, normalize(eye)));
//	VdotR /= 32.0 - VdotR * 32.0 + VdotR;
//	float4 specular = (lightcol * VdotR) * 0.25;

//	diffuse += specular;

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
        VertexShader = compile vs_3_0 V_Test();
        PixelShader  = compile ps_3_0 P_Test();
    } 
}
