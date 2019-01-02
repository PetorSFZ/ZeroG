
struct VSInput {
	float3 position : POSITION;
	float3 color : COLOR;
};

struct VSOutput {
	float4 position : SV_Position;
	float4 color : COLOR;
};

struct PSInput {
	float4 color : COLOR;
};

VSOutput VSMain(VSInput input)
{
	VSOutput output;

	output.position = float4(input.position, 1.0f);
	output.color = float4(input.color, 1.0f);

	return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	return input.color;
}
