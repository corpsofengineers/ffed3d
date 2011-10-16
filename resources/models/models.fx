float4x4 viewprojmat;    // view*proj
float4x4 worldmat;    // world
float4 light;        // light source
float4 lightcol;        // light color
float4 ambient;        // ambient color

int skinnum;

texture skin;        // skin.png

sampler s_tex = sampler_state 
{
    texture = <skin>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

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

  float3 tx;
  float4 tx_base = tex2D(s_tex, tex_vu);
  
  if (skinnum == 1) {
		tx.r = tx_base.g;
		tx.g = tx_base.r;
		tx.b = tx_base.b;
  } else if (skinnum == 2) {
		tx.r = tx_base.b;
		tx.g = tx_base.g;
		tx.b = tx_base.r;
  } else if (skinnum == 3) {
		tx.r = tx_base.r;
		tx.g = tx_base.b;
		tx.b = tx_base.g;
  } else if (skinnum == 4) {
		tx.r = 1.0-tx_base.g;
		tx.g = 1.0-tx_base.r;
		tx.b = 1.0-tx_base.b;
  } else if (skinnum == 5) {
		tx.r = 1.0-tx_base.b;
		tx.g = 1.0-tx_base.g;
		tx.b = 1.0-tx_base.r;
  } else if (skinnum == 6) {
		tx.r = 1.0-tx_base.r;
		tx.g = 1.0-tx_base.b;
		tx.b = 1.0-tx_base.g;
  } else {
		tx = tx_base.rgb;
  }

  ut.a = tx_base.a;

  float3 diffuse = tx * lightcol;
  ut.rgb = saturate(dot(lightDir, normal) * diffuse * 2.0);
  
  float3 R = normalize(reflect(-lightDir, normal));
  float VdotR = saturate(dot(R, normalize(eye)));
  VdotR /= 128.0 - VdotR * 128.0 + VdotR;
  float3 specular = (lightcol * VdotR);

  ut.rgb += specular;
  
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
