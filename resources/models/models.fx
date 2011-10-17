float4x4 viewprojmat;    // view*proj
float4x4 worldmat;    // world
float4 light;        // light source
float4 lightcol;        // light color
float4 ambient;        // ambient color

int skinnum;

texture skin;        // skin.png
texture permTexture2d;

sampler s_tex = sampler_state 
{
    texture = <skin>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
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

float getnoise(float2 tex_vu)
{
	float2 tex_vu_n = tex_vu;

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

	float noise = 1.0-saturate((r.x+r.y+r.z+r.w)/4.0);
	
	return noise;
}

float3 getskin(float3 tx_base)
{
	float3 tx;

  if (floor(skinnum / 64) == 1) {
		tx.r = tx_base.g;
		tx.g = tx_base.r;
		tx.b = tx_base.b;
  } else if (floor(skinnum / 64) == 2) {
		tx.r = tx_base.b;
		tx.g = tx_base.g;
		tx.b = tx_base.r;
  } else if (floor(skinnum / 64) == 3) {
		tx.r = tx_base.r;
		tx.g = tx_base.b;
		tx.b = tx_base.g;
  } else {
		tx = tx_base.rgb;
  }

  if (frac((float)floor(skinnum / 1) / 2) > 0) {
		tx.r = (1.0-tx_base.r) * 0.5;
  }
  if (frac((float)floor(skinnum / 2) / 2) > 0) {
		tx.g = (1.0-tx_base.g) * 0.5;
  }
  if (frac((float)floor(skinnum / 4) / 2) > 0) {
		tx.b = (1.0-tx_base.b) * 0.5;
  }

  if (frac((float)floor(skinnum / 8) / 2) > 0) {
		tx.r = tx_base.r * 0.5;
  }
  if (frac((float)floor(skinnum / 16) / 2) > 0) {
		tx.g = tx_base.g * 0.5;
  }
  if (frac((float)floor(skinnum / 32) / 2) > 0) {
		tx.b = tx_base.b * 0.5;
  }

  return tx;
}

struct VS_OUTPUT
{
    float4 position : POSITION;
    float3 normal    : TEXCOORD1;
    float2 tex_vu    : TEXCOORD0;
    float3 lightDir : TEXCOORD2;
    float3 eye    : TEXCOORD3;
};

VS_OUTPUT Vexel_Sh(
    float4 pos        : POSITION,
    float3 nor        : NORMAL,
    float3 tangent        : TANGENT,
    float2 tex_vu        : TEXCOORD0
    )
{
    VS_OUTPUT Out = (VS_OUTPUT)0;

    float4 vertex = mul(pos, worldmat);	

    Out.position = mul(vertex, viewprojmat);
    Out.normal = normalize(mul(nor, (float3x3)worldmat)); 
    Out.tex_vu = tex_vu;
     
    Out.lightDir = normalize(light);
    Out.eye = -normalize(vertex.xyz);
    
    return Out;
}

float4 Pixel_Sh(
    float2 tex_vu    : TEXCOORD0,
    float3 normal    : TEXCOORD1,
    float3 lightDir : TEXCOORD2,
    float3 eye        : TEXCOORD3
  ) : COLOR0
{
  float4 ut;

  float4 tx_base = tex2D(s_tex, tex_vu);
  
  float3 tx = getskin(tx_base.rgb);

  ut.a = tx_base.a;

  float noise = getnoise(tex_vu);
  float4 tx_noise = float4(noise,noise,noise,0.0) * 0.5 + 0.5;

  float3 diffuse = tx * tx_noise * lightcol;
  ut.rgb = saturate(dot(lightDir, normal) * diffuse * 2.0);
  
  float3 R = normalize(reflect(-lightDir, normal));
  float VdotR = saturate(dot(R, normalize(eye)));
  VdotR /= 128.0 - VdotR * 128.0 + VdotR;
  float3 specular = (lightcol * VdotR);

  ut.rgb += specular;
  ut.rgb = max(ut.rgb, diffuse * 0.2);
  
  return ut;
}

technique PixelLight
{
    pass P0
    {
        VertexShader = compile vs_3_0 Vexel_Sh();
        PixelShader  = compile ps_3_0 Pixel_Sh();
    }
}
