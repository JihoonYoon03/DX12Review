#define MAX_LIGHTS          8
#define MAX_MATERIALS       8

#define POINT_LIGHT         1
#define SPOT_LIGHT          2
#define DIRECTIONAL_LIGHT   3

#define _WITH_LOCAL_VIEWER_HIGHLIGHTING
#define _WITH_THETA_PHI_CONES
//#define _WITH_REFLECT

struct MATERIAL
{
    float4  m_cAmbient;
    float4  m_cDiffuse;
    float4  m_cSpecular; //a = power
    float4  m_cEmissive;
};

struct LIGHT
{
    float4  m_cAmbient;
    float4  m_cDiffuse;
    float4  m_cSpecular;
    float3  m_vPosition;
    float   m_fFalloff;
    float3  m_vDirection;
    float   m_fTheta; //cos(m_fTheta)
    float3  m_vAttenuation;
    float   m_fPhi; //cos(m_fPhi)
    bool    m_bEnable;
    int     m_nType;
    float   m_fRange;
    float   padding;
};

cbuffer cbMaterial : register(b3)
{
    MATERIAL gMaterials[MAX_MATERIALS];
};

cbuffer cbLights : register(b4)
{
    LIGHT gLights[MAX_LIGHTS];
    float4 gcGlobalAmbientLight;
};
