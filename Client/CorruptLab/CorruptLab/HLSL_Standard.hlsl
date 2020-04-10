#include "Shaders.hlsl"
struct VS_TEXTURED_LIGHTING_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct VS_TEXTURED_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float2 uv : TEXCOORD;
	float4 vPorjPos : TEXCOORD1; 

	//그림자를 위한 
	float4 LightViewPosition : TEXCOORD2;
	float3 LightPos : TEXCOORD3;
	
};

struct VS_SKINNED_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	uint4 indices : BONEINDEX;
	float4 weights : BONEWEIGHT;
};

#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			128

cbuffer cbBoneOffsets : register(b7)
{
	float4x4 gpmtxBoneOffsets[SKINNED_ANIMATION_BONES];
};

cbuffer cbBoneTransforms : register(b8)
{
	float4x4 gpmtxBoneTransforms[SKINNED_ANIMATION_BONES];
};

VS_TEXTURED_LIGHTING_OUTPUT VSSkinnedAnimationStandard(VS_SKINNED_STANDARD_INPUT input)
{
	VS_TEXTURED_LIGHTING_OUTPUT output;

	output.positionW = float3(0.0f, 0.0f, 0.0f);
	output.normalW = float3(0.0f, 0.0f, 0.0f);
	output.tangentW = float3(0.0f, 0.0f, 0.0f);
	output.bitangentW = float3(0.0f, 0.0f, 0.0f);
	matrix mtxVertexToBoneWorld;
	for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
	{
		mtxVertexToBoneWorld = mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
		output.positionW += input.weights[i] * mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
		output.normalW += input.weights[i] * mul(input.normal, (float3x3)mtxVertexToBoneWorld);
		output.tangentW += input.weights[i] * mul(input.tangent, (float3x3)mtxVertexToBoneWorld);
		output.bitangentW += input.weights[i] * mul(input.bitangent, (float3x3)mtxVertexToBoneWorld);
	}
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;
	output.vPorjPos = output.position;
	
	//shadow 
	output.LightViewPosition = mul(mul(float4(output.positionW, 1.0f), shadowgmtxView), shadowgmtxProjection);
	//output.LightPos = 
	return(output);
}

VS_TEXTURED_LIGHTING_OUTPUT VSLighting(VS_TEXTURED_LIGHTING_INPUT input)
{
	VS_TEXTURED_LIGHTING_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.tangentW = mul(input.tangent, (float3x3)gmtxGameObject);
	output.bitangentW = mul(input.bitangent, (float3x3)gmtxGameObject);
	output.uv = input.uv;
	output.vPorjPos = output.position;
	output.LightViewPosition = mul(mul(float4(output.positionW, 1.0f), shadowgmtxView), shadowgmtxProjection);
	return output;
}

/////////////////////////////////////////////////////////////////////////////////////////////



PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSTexturedLightingToMultipleRTs(VS_TEXTURED_LIGHTING_OUTPUT input)
{
	PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;

	float4 cColorAlbedo = gtxtAlbedoTexture.Sample(gSamplerState, input.uv);
	float4 cColorNormal = gtxtNormalTexture.Sample(gSamplerState, input.uv);

	cColorAlbedo.rgb = cColorAlbedo.rgb;

	float3 normalW;
	float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));

	float3 vNormal = normalize(cColorNormal.rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]

	normalW = normalize(mul(vNormal, TBN));

	//float fdepth = input.vPorjPos.z / 1000.f;
	output.normal = float4(normalW, 1);
	output.color = cColorAlbedo;
	output.depth = float4(input.vPorjPos.z/ input.vPorjPos.w, input.vPorjPos.w /300.0f,0, 1);
	output.ShadowCamera = float4 (1.0f, 0.0f, 0.0f, 1.0f);
	//output.NonLight = float4 (1.0f, 0.0f, 0.0f, 1.0f);

	float2 projectTexCoord;
	float lightDepthValue;
	float depthValue;
	float bias = 0.001f;
	projectTexCoord.x = input.LightViewPosition.x / input.LightViewPosition.w / 2.0f + 0.5f;
	projectTexCoord.y = -input.LightViewPosition.y / input.LightViewPosition.w / 2.0f + 0.5f;

	if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
	{
		float DepthWValue = gtxtShadowCameraViewDepth.Sample(gSamplerClamp, projectTexCoord).g * 600.0f;
		depthValue = gtxtShadowCameraViewDepth.Sample(gSamplerClamp, projectTexCoord).r * DepthWValue;

		// 빛의 깊이를 계산합니다.
		lightDepthValue = input.LightViewPosition.z / input.LightViewPosition.w;

		// lightDepthValue에서 바이어스를 뺍니다.
		lightDepthValue = lightDepthValue - bias;

		output.color = float4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	return output;
}