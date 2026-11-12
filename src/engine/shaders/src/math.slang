static const float PI_F = 3.14159265359f;

float3 perturbNormal(float3 n, float3 worldPos, float3 normalSample, float2 uv) {
    float3 localNormal = normalSample * 2.0 - 1.0;

    float3 dp1 = ddx(worldPos);
    float3 dp2 = ddy(worldPos);
    float2 duv1 = ddx(uv);
    float2 duv2 = ddy(uv);

    float3 dp2perp = cross(dp2, n);
    float3 dp1perp = cross(n, dp1);
    float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    float3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    float invmax = rsqrt(max(dot(T, T), dot(B, B)) + 0.000001);
    return normalize(T * (localNormal.x * invmax) + B * (localNormal.y * invmax) + n * localNormal.z);
}
