struct CBData {
	float4 aspectRatio;
};
ConstantBuffer<CBData> inputData : register(b0);

struct VSInput {
	float2 position : ATTRIBUTE_LOCATION_0;
	float2 coord : ATTRIBUTE_LOCATION_1;
};

struct VSOutput {
	float2 coord : PARAM_0;
	float4 position : SV_Position;
};

struct PSInput {
	float2 coord : PARAM_0;
};

VSOutput VSMain(VSInput input)
{
	VSOutput output;
	output.coord = input.coord;
	output.position = float4(input.position, 0.0f, 1.0f);
	return output;
}



float signedDistanceFunction(float3 p) {
	const float sphereRadius = 1.5f;
	const float noiseAmplitude = 0.5f;
	const float3 spherePos = float3(0.0f, 0.0f, 5.0f);
	float displacement = ((sin(16.0f * p.x) * sin(16.0f * p.y) * sin(16.0f * p.z) + 1.0f) / 2.0f) * noiseAmplitude;
	return length(spherePos - p) - (sphereRadius + displacement);
}


float march(float3 origin, float3 dir)
{
	const int MAX_STEPS = 2048;
	const float DIST = 0.005f;
	float3 currentPoint = origin;
	float currentDist = 0.0f;
	for (int i = 0; i < MAX_STEPS; i++) {
		currentPoint += dir * DIST;
		currentDist += DIST;

		float dist = signedDistanceFunction(currentPoint);
		if (dist < 0.1f) {
			return currentDist;
		}
	}
	return -1.0f;
}

float3 calcNormal(float3 p)
{
	// https://en.wikipedia.org/wiki/Finite_difference
	const float NORMAL_EPS = 0.001f;
	float3 normal;
	normal.x = signedDistanceFunction(p + float3(NORMAL_EPS,0,0)) - signedDistanceFunction(p - float3(NORMAL_EPS,0,0));
	normal.y = signedDistanceFunction(p + float3(0,NORMAL_EPS,0)) - signedDistanceFunction(p - float3(0,NORMAL_EPS,0));
	normal.z = signedDistanceFunction(p + float3(0,0,NORMAL_EPS)) - signedDistanceFunction(p - float3(0,0,NORMAL_EPS));
	return normalize(normal);
}

float4 PSMain(PSInput input) : SV_TARGET
{
	const float3 camPos = float3(0.0f, 0.0f, 0.0f);
	float3 camTarget = float3(input.coord.x * inputData.aspectRatio.x, input.coord.y, 1.0f);
	float3 camDir = normalize(camTarget - camPos);

	// Ray march!
	float dist = march(camPos, camDir);
	if (dist < 0.0f) {
		return float4(0.15f, 0.15f, 0.25f, 1.0f);
	}

	// Find hit pos and normal
	float3 hitPos = camPos + camDir * dist;
	float3 normal = calcNormal(hitPos);

	// Shade hit
	const float3 lightPos = float3(2.0f, 10.0f, 3.0f);
	float3 res = float3(0.0f, 0.0f, 0.0f);
	//res = normal;
	float3 toLight = normalize(lightPos - hitPos);

	float intensity = dot(normal, toLight);
	intensity = max(intensity, 0.3);
	res = float3(1.0f, 1.0f, 1.0f) * intensity;

	//float displacement = (sin(16.0f * hitPos.x) * sin(16.0f * hitPos.y) * sin(16.0f * hitPos.z) + 1.0f) / 2.0f;
	//res = float3(1.0f, 1.0f, 1.0f) * displacement * intensity;


	return float4(res, 1.0f);
}
