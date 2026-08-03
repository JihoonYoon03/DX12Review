//게임 객체의 정보를 위한 상수 버퍼를 선언한다.
cbuffer cbGameObjectInfo : register(b0)
{
    matrix gmtxWorld : packoffset(c0);
};

cbuffer cbCameraInfo : register(b1)
{
    matrix gmtxView : packoffset(c0);
    matrix gmtxProjection : packoffset(c4);
};

//정점 셰이더의 입력을 위한 구조체를 선언한다.
struct VS_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
};

//정점 셰이더의 출력을 위한 구조체를 선언한다.
struct VS_OUTPUT
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

//정점 쉐이더를 정의한다.
VS_OUTPUT VSDiffused( VS_INPUT input )
{
    VS_OUTPUT output;
    
    //정점을 변환한다.
    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
    output.color = input.color;
    
    return output;
}

//픽셀 셰이더를 정의한다.
float4 PSDiffused(VS_OUTPUT input) : SV_Target
{
    //입력되는 픽셀 색상을 그대로 OM으로 출력
    return input.color;
}





/*
#define FRAME_BUFFER_WIDTH  640.0f
#define FRAME_BUFFER_HEIGHT 480.0f
#define HALF_WIDTH          (FRAME_BUFFER_WIDTH * 0.5f)
#define HALF_HEIGHT         (FRAME_BUFFER_HEIGHT * 0.5f)
#define EPSILON             1.0e-5f

inline bool IsZero(float fValue)
{
    return abs(fValue) <= EPSILON;

}

inline bool IsZero(float fValue, float fEpsilon)
{
    return abs(fValue) <= fEpsilon;
}

inline bool IsEqual(float fA, float fB, float fEpsilon)
{
    return abs(fA - fB) <= fEpsilon;
}

float Rectangle(float2 f2NDC, float fLeft, float fRight, float fTop, float fBottom)
{
    float2 f2Shape = float2(step(fLeft, f2NDC.x), step(f2NDC.x, fRight));
    f2Shape *= float2(step(fTop, f2NDC.y), step(f2NDC.y, fBottom));
    
    return f2Shape.x * f2Shape.y;
}

float RegularPolygon(float2 f2NDC, float fSides, float fRadius)
{
    float fAngle = atan(f2NDC.y / f2NDC.x);
    float fSlices = (2.0f * 3.14159f) / fSides;
    
    float fShape = step(cos(floor((fAngle / fSlices) + 0.5f) * fSlices - fAngle) * length(f2NDC), fRadius);
    return fShape;
}


//픽셀 쉐이더를 정의한다.
float4 PSMain( float4 input : SV_POSITION) : SV_TARGET
{
    float4 cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    //1
    //cColor.r = input.x / FRAME_BUFFER_WIDTH;
    
    //2
    //cColor.r = input.x / FRAME_BUFFER_WIDTH;
    //cColor.g = input.y / FRAME_BUFFER_HEIGHT;
    
    //3
    //cColor.rgb = distance(float2(0.5f, 0.5f), input.xy / float2(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT));
    
    //4
    //float fDistance = distance(float2(0.5f, 0.5f), input.xy / float2(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT));
    //if (fDistance < 0.25f)
    //    cColor.b = 1.0f;
    
    //5
    //float2 f2NDC = float2(input.x / FRAME_BUFFER_WIDTH, input.y / FRAME_BUFFER_HEIGHT) - 0.5f;
    //f2NDC.x *= (FRAME_BUFFER_WIDTH / FRAME_BUFFER_HEIGHT);
    //cColor.b = (length(f2NDC) <= 0.25f) ? 1.0f : 0.0f;
    
    //6
    //float2 f2NDC = float2(input.x / FRAME_BUFFER_WIDTH, input.y / FRAME_BUFFER_HEIGHT) - 0.5f;
    //f2NDC.x *= (FRAME_BUFFER_WIDTH / FRAME_BUFFER_HEIGHT);
    //float fLength = length(f2NDC);
    //float fMin = 0.3f, fMax = 0.2f;
    //cColor.rgb = smoothstep(fMin, fMax, fLength);
    
    //7
    //if ((int) input.x == (int) HALF_WIDTH)
    //    cColor.g = 1.0f;
    //if ((int) input.y == (int) HALF_HEIGHT)
    //    cColor.r = 1.0f;
    //float fDistance = distance((int2) input.xy, float2(HALF_WIDTH, HALF_HEIGHT));
    //if (IsEqual(fDistance, 100.0f, 0.5f))
    //    cColor.b = 1.0f;
    
    //8
    //float2 f2NDC = float2(input.x / FRAME_BUFFER_WIDTH, input.y / FRAME_BUFFER_HEIGHT) - 0.5f;
    //f2NDC.x *= (FRAME_BUFFER_WIDTH / FRAME_BUFFER_HEIGHT);
    //cColor.b = Rectangle(f2NDC, 0.1f, 0.3f, -0.2f, 0.4f);
    //cColor.b += Rectangle(f2NDC, -0.3f, -0.1f, -0.4f, -0.1f);
    
    //9
    //float2 f2NDC = float2(input.x - FRAME_BUFFER_WIDTH * 0.5f, input.y - FRAME_BUFFER_HEIGHT * 0.5f);
    //f2NDC *= 20.0f;
    //float fLength = length(f2NDC);
    //cColor.rgb = cos(fLength);
    
    //10
    //float2 f2NDC = float2(input.x / FRAME_BUFFER_WIDTH, input.y / FRAME_BUFFER_HEIGHT) - 0.5f;
    //f2NDC.x *= (FRAME_BUFFER_WIDTH / FRAME_BUFFER_HEIGHT);
    //float fRadius = 0.3f;
    //float fRadian = radians(360.0f / 30.0f);
    //for (float f = 0; f < 30.0f; f += 1.0f)
    //{
    //    float fAngle = fRadian * f;
    //    cColor.rgb += (0.0025f / length(f2NDC + float2(fRadius * cos(fAngle), fRadius * sin(fAngle))));
    //}
    
    //11
    //float2 f2NDC = float2(input.x / FRAME_BUFFER_WIDTH, input.y / FRAME_BUFFER_HEIGHT) - 0.5f;
    //f2NDC.x *= (FRAME_BUFFER_WIDTH / FRAME_BUFFER_HEIGHT);
    //cColor.b = RegularPolygon(f2NDC - float2(-0.3f, -0.1f), 8.0f, 0.2f);
    //cColor.r = RegularPolygon(f2NDC - float2(0.3f, 0.2f), 4.0f, 0.15f);
    
    //12
    //float x = abs(frac((input.x * 10.0f) / FRAME_BUFFER_HEIGHT) - 0.5f);
    //float y = abs(frac((input.y * 10.0f) / FRAME_BUFFER_HEIGHT) - 0.5f);
    //cColor.r = ((x <= 0.0125f) || (y <= 0.0125f)) ? 1.0f : 0.0f;
    
    //13
    //float2 f2NDC = float2(input.x / FRAME_BUFFER_WIDTH, input.y / FRAME_BUFFER_HEIGHT) - 0.5f;
    //f2NDC.x *= (FRAME_BUFFER_WIDTH / FRAME_BUFFER_HEIGHT);
    //f2NDC.xy *= 10.0f;
    //cColor.b = smoothstep(0.125f, 0.875f, abs(cos(length(f2NDC) * 3.14159f)));
    
    return cColor;
}*/