#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;


uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;

uniform int receiveShadows; 


uniform int shadingMode; // 0=flat, 1=smooth


uniform vec3 lightDir;
uniform vec3 lightColor;


uniform vec3 pointLightPos;
uniform vec3 pointLightColor;
uniform float attConstant;
uniform float attLinear;
uniform float attQuadratic;

uniform vec3 spotDir;
uniform float cutOff;
uniform float outerCutOff;


uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;


uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;


uniform int fogEnabled;
uniform vec3 fogColor;
uniform float fogStart;
uniform float fogEnd;


uniform vec3 fogCenter;
uniform float fogRadius;

uniform float fogHeightMin;
uniform float fogHeightMax;



uniform sampler2D shadowMap;
in vec4 fragPosLightSpace;

float computeShadow(vec4 fragPosLS, vec3 normalEye, vec3 lightDirEye)
{
    vec3 projCoords = fragPosLS.xyz / fragPosLS.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0) return 0.0;

    float bias = max(0.003 * (1.0 - dot(normalEye, lightDirEye)), 0.001);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));

    float radius = 2.0; // 1=3x3, 2=5x5, 3=7x7
    float count = 0.0;

    for (float x = -radius; x <= radius; x++)
    for (float y = -radius; y <= radius; y++)
    {
        float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
        shadow += (projCoords.z - bias > pcfDepth) ? 1.0 : 0.0;
        count += 1.0;
    }
    shadow /= count;

    return shadow;
}








float sphereMask(vec3 p, vec3 c, float r)
{
    float d = length(p - c);
    return 1.0 - smoothstep(r * 0.70, r, d);
}


vec3 computeDirLight(vec3 normalEye, vec3 viewDirEye, vec3 texD, vec3 texS)
{
    vec3 lightDirEye = normalize(vec3(view * vec4(lightDir, 0.0)));

    float shadow = 0.0;
    if (receiveShadows == 1)
    	shadow = computeShadow(fragPosLightSpace, normalEye, lightDirEye);


    vec3 ambient = materialAmbient * lightColor * texD;

    float diff = max(dot(normalEye, lightDirEye), 0.0);
    vec3 diffuse = materialDiffuse * diff * lightColor * texD;

    vec3 reflectDir = reflect(-lightDirEye, normalEye);
    float spec = pow(max(dot(viewDirEye, reflectDir), 0.0), materialShininess);
    vec3 specular = materialSpecular * spec * lightColor * texS;

    vec3 sun = ambient + (1.0 - shadow) * (diffuse + specular);

    return sun;
}

vec3 computeSpotLight(vec3 fragPosEye, vec3 normalEye, vec3 viewDirEye, vec3 texD, vec3 texS)
{
    vec3 lightPosEye = vec3(view * vec4(pointLightPos, 1.0));
    vec3 L = normalize(lightPosEye - fragPosEye);

    vec3 spotDirEye = normalize(mat3(view) * spotDir);

    float theta = dot(L, normalize(-spotDirEye));
    float epsilon = cutOff - outerCutOff;
    float intensity = clamp((theta - outerCutOff) / max(epsilon, 0.0001), 0.0, 1.0);

    float diff = max(dot(normalEye, L), 0.0);

    vec3 reflectDir = reflect(-L, normalEye);
    float spec = pow(max(dot(viewDirEye, reflectDir), 0.0), materialShininess);

    float dist = length(lightPosEye - fragPosEye);
    float att  = 1.0 / (attConstant + attLinear * dist + attQuadratic * dist * dist);

    vec3 ambient  = materialAmbient  * pointLightColor * texD;
    vec3 diffuse  = materialDiffuse  * diff           * pointLightColor * texD;
    vec3 specular = materialSpecular * spec           * pointLightColor * texS;

    return (ambient + diffuse + specular) * att * intensity;
}

void main()
{
    
    vec3 fragPosEye = vec3(view * vec4(fPosition, 1.0));


    
    vec3 normalEye;
    if (shadingMode == 1) {
        normalEye = normalize(normalMatrix * fNormal);
    } else {
        vec3 dx = dFdx(fragPosEye);
        vec3 dy = dFdy(fragPosEye);
        normalEye = normalize(cross(dx, dy));
        if (!gl_FrontFacing) normalEye = -normalEye;
    }

    vec3 viewDirEye = normalize(-fragPosEye);

    vec3 texD = texture(diffuseTexture,  fTexCoords).rgb;
    vec3 texS = texture(specularTexture, fTexCoords).rgb;

    vec3 sun  = computeDirLight(normalEye, viewDirEye, texD, texS);
    vec3 lamp = computeSpotLight(fragPosEye, normalEye, viewDirEye, texD, texS);

    vec3 color = sun + lamp;

    
    if (fogEnabled == 1)
    {
        
        vec3 fogSpacePos = fPosition;

        
        float d = length(fragPosEye);
        float fog = clamp((d - fogStart) / (fogEnd - fogStart), 0.0, 1.0);

        
        float region = sphereMask(fogSpacePos, fogCenter, fogRadius);
        float hMask  = 1.0 - smoothstep(fogHeightMin, fogHeightMax, fogSpacePos.y);

        float fogAmount = fog * region * hMask;

        color = mix(color, fogColor, fogAmount);
    }

    color = min(color, vec3(1.0));
    fColor = vec4(color, 1.0);
}
