#include "CarRenderMaterial.h"

#ifdef __GNUC__
typedef long long MaterialInteger;
#else
typedef __int64 MaterialInteger;
#endif
namespace {
// Original pooled binary32 constants; memory loads retain their rounded values
// under GCC's x87 excess-precision evaluation of decimal literals.
const volatile float clockScale784268 = 0.00025f;
const volatile float directionThreshold7843EC = 0.0001f;
const volatile float powerMinimum784358 = 0.1f;
}

// Original 0x5bcb70 / 0x5bcbc0: immediate device states or the five-word effect array.
void CarRenderSetAlphaTest(unsigned int enabled,unsigned int reference) {
    if (!ug2RenderTechnique870910) {
        UG2RenderSetState(ug2SolidDevice870974,15,enabled);
        UG2RenderSetState(ug2SolidDevice870974,24,reference);
    } else {
        ug2TechniqueBlend86EAD8[0] = enabled;
        ug2TechniqueBlend86EAD8[1] = reference;
    }
}
void CarRenderSetBlend(unsigned int enabled,unsigned int source,unsigned int destination) {
    if (!ug2RenderTechnique870910) {
        UG2RenderSetState(ug2SolidDevice870974,27,enabled);
        UG2RenderSetState(ug2SolidDevice870974,19,source);
        UG2RenderSetState(ug2SolidDevice870974,20,destination);
    } else {
        ug2TechniqueBlend86EAD8[2] = enabled;
        ug2TechniqueBlend86EAD8[3] = source;
        ug2TechniqueBlend86EAD8[4] = destination;
    }
}

// Original 0x48b450. _ftol2 returns a 64-bit integer; the code uses its signed low
// 32 bits via FILD32. Truncation here is not floor, including for negative phases.
long double __stdcall CarRenderTextureWave(float time,float frequency,int mode,float period) {
    if (mode == 0) return 0.0f;
    long double phase = time;
    if (mode == 2) {
        phase = static_cast<int>(static_cast<unsigned int>(static_cast<MaterialInteger>(phase/period)));
    }
    long double scaled = phase*frequency;
    int whole = static_cast<int>(static_cast<unsigned int>(static_cast<MaterialInteger>(scaled)));
    return scaled-whole;
}
// Original 0x5b7d40 / 0x5b7d80: both coefficients are spilled as float arguments.
long double CarRenderTexture::ScrollU(float time) {
    float period = static_cast<float>(value58)*0.00390625f;
    float frequency = static_cast<float>(value5A)*0.0009765625f;
    return CarRenderTextureWave(time,frequency,static_cast<signed char>(value52),period);
}
long double CarRenderTexture::ScrollV(float time) {
    float period = static_cast<float>(value58)*0.00390625f;
    float frequency = static_cast<float>(value5C)*0.0009765625f;
    return CarRenderTextureWave(time,frequency,static_cast<signed char>(value52),period);
}

// Original 0x5bcc30. A cache miss requires a live texture-state record. After a
// nonzero token the ordinary path requires a current technique, as in the binary.
void CarRenderBindTexture(CarRenderTexture *texture) {
    CarRenderTextureState *state = texture->state00;
    if (ug2RenderLastTexture870940 == texture && ug2RenderLastTextureTechnique870944 == ug2RenderTechnique870910) return;
    ++ug2RenderTextureChanges872F10;
    ug2RenderLastTexture870940 = texture;
    ug2RenderLastTextureTechnique870944 = ug2RenderTechnique870910;
    unsigned int token = state->texture18;
    if (!token) return;
    if (ug2RenderTechnique870910) {
        UG2EffectSetTexture(ug2RenderTechnique870910->effect20,
            ug2RenderTechnique870910->parameters1C->slots00[7].handle24,token);
    } else UG2RenderSetTexture(ug2SolidDevice870974,0,token);
    if (texture->value55 == 1) CarRenderSetAlphaTest(1,95);
    else CarRenderSetAlphaTest(0,0);
    unsigned int source = 5,destination = 6;
    switch (texture->material56) {
    case 2: destination = 2; break;
    case 5: source = 7; destination = 8; break;
    case 6: source = 7; destination = 2; break;
    }
    CarRenderSetBlend(texture->material56 != 0,source,destination);
    if (texture->material56 == 0) {
        if (ug2RenderTechnique870910->type04 == 5) UG2RenderSetState(ug2SolidDevice870974,22,1);
        UG2RenderSetState(ug2SolidDevice870974,14,1);
    } else {
        if (ug2RenderTechnique870910->type04 == 5) UG2RenderSetState(ug2SolidDevice870974,22,2);
        UG2RenderSetState(ug2SolidDevice870974,14,0);
    }
    UG2EffectSetIntArray(ug2RenderTechnique870910->effect20,
        ug2RenderTechnique870910->parameters1C->slots00[2].handle24,ug2TechniqueBlend86EAD8,5);
    float offset[4] = {0,0,0,0};
    if (ug2RenderTechnique870910->parameters1C->slots00[55].handle24) {
        if (ug2RenderUvEnable870CE8) {
            float time = static_cast<float>(static_cast<long double>(ug2RenderClock86518C)*clockScale784268);
            if (texture->value52 == 3) {
                offset[0] = static_cast<float>(texture->value5E)*-0.0009765625f;
                offset[1] = static_cast<float>(texture->value60)*-0.0009765625f;
            } else if (ug2RenderUvOverride870924 == 0.0f && ug2RenderUvOverride870928 == 0.0f) {
                offset[0] = texture->ScrollU(time);
                offset[1] = texture->ScrollV(time);
            } else {
                float u = ug2RenderUvOverride870924/static_cast<float>(texture->width44);
                float v = ug2RenderUvOverride870928/static_cast<float>(texture->height46);
                offset[0] = CarRenderTextureWave(time,u,ug2RenderUvMode800504,ug2RenderUvPeriod800508);
                offset[1] = CarRenderTextureWave(time,v,ug2RenderUvMode800504,ug2RenderUvPeriod800508);
            }
        }
        ug2RenderUvDirty870948 = 1;
    }
    if (ug2RenderTechnique870910 && ug2RenderUvDirty870948 && ug2RenderTechnique870910->parameters1C->slots00[55].handle24) {
        WheelMatrix matrix;
        WheelIdentity(&matrix);
        matrix.elements[8] = offset[0];
        matrix.elements[9] = offset[1];
        UG2EffectSetMatrix(ug2RenderTechnique870910->effect20,
            ug2RenderTechnique870910->parameters1C->slots00[56].handle24,&matrix);
        UG2EffectSetVector(ug2RenderTechnique870910->effect20,
            ug2RenderTechnique870910->parameters1C->slots00[55].handle24,offset);
    }
    if (!texture->value52) ug2RenderUvDirty870948 = 0;
}

// Original 0x5bd060. The upper comparison maps NaN to one; the lower comparison
// maps negative zero to positive zero. This is not a generic clamp replacement.
void CarRenderClampColour(float *value) {
    for (int i=0;i<4;++i) {
        float upper = value[i]<1.0f ? value[i] : 1.0f;
        value[i] = upper;
        value[i] = upper<=0.0f ? 0.0f : upper;
    }
}

// Original 0x486f50, called through 0x43c9b0. All inputs are consumed before any
// output store, with component-specific sums and the fourth component stored first.
void CarRenderTransform4(float *output,const WheelMatrix *matrix,const float *input) {
    const float *m = matrix->elements;
    float x = ((static_cast<long double>(m[12])*input[3]+static_cast<long double>(m[8])*input[2])+
        static_cast<long double>(m[4])*input[1])+static_cast<long double>(m[0])*input[0];
    float y = ((static_cast<long double>(m[13])*input[3]+static_cast<long double>(m[1])*input[0])+
        static_cast<long double>(m[9])*input[2])+static_cast<long double>(m[5])*input[1];
    float z = ((static_cast<long double>(m[14])*input[3]+static_cast<long double>(m[2])*input[0])+
        static_cast<long double>(m[10])*input[2])+static_cast<long double>(m[6])*input[1];
    float w = ((static_cast<long double>(m[15])*input[3]+static_cast<long double>(m[3])*input[0])+
        static_cast<long double>(m[11])*input[2])+static_cast<long double>(m[7])*input[1];
    output[3] = w; output[0] = x; output[1] = y; output[2] = z;
}

namespace {
unsigned int PackChannel(float value) {
    return static_cast<unsigned int>(static_cast<MaterialInteger>(static_cast<long double>(value)*255.0f));
}
}

// Original 0x5bd140.
void CarRenderUploadRecord(void *record,WheelMatrix *world) {
    const float *input = static_cast<const float *>(record);
    int type = ug2RenderTechnique870910->type04;
    if (type == 5 && !ug2RenderTechnique870910->value18) {
        float colour[4];
        for (int i=0;i<4;++i) colour[i] = ug2RenderRecordScale80050C*input[0x48+i];
        CarRenderClampColour(colour);
        unsigned int alpha = PackChannel(colour[3]);
        unsigned int blue = PackChannel(colour[2]);
        unsigned int green = PackChannel(colour[1]);
        unsigned int red = PackChannel(colour[0]);
        unsigned int packed = (((alpha<<8|blue)<<8|green)<<8)|red;
        float directions[3][4],colors[3][3];
        for (int i=0;i<3;++i) {
            for (int j=0;j<3;++j) {
                directions[i][j] = input[0x10+j*4+i];
                colors[i][j] = input[i*4+j];
            }
            directions[i][3] = 0;
        }
        for (int i=0;i<3;++i) {
            CarRenderDeviceLight light;
            light.type00 = 3;
            for (int j=0;j<3;++j) light.diffuse04[j] = ug2RenderDiffuseScale800510*colors[i][j];
            for (int j=0;j<3;++j) light.specular14[j] = ug2RenderSpecularScale800514*colors[i][j];
            for (int j=0;j<3;++j) light.ambient24[j] = 0.0f;
            light.ambient24[3] = colour[3];
            float transformed[4];
            CarRenderTransform4(transformed,world,directions[i]);
            for (int j=0;j<3;++j) light.direction40[j] = -transformed[j];
            UG2RenderSetLight(ug2SolidDevice870974,i,&light);
            float x = light.direction40[0],y = light.direction40[1],z = light.direction40[2];
            if (!((x<0?-x:x)<directionThreshold7843EC) || !((y<0?-y:y)<directionThreshold7843EC) || !((z<0?-z:z)<directionThreshold7843EC))
                UG2RenderLightEnable(ug2SolidDevice870974,i,1);
        }
        UG2EffectSetInt(ug2RenderTechnique870910->effect20,
            ug2RenderTechnique870910->parameters1C->slots00[1].handle24,packed);
    } else if (type == 5 || type == 4) {
        UG2EffectSetVectorArray(ug2RenderTechnique870910->effect20,
            ug2RenderTechnique870910->parameters1C->slots00[31].handle24,input+0x48,10);
        UG2EffectSetVector(ug2RenderTechnique870910->effect20,
            ug2RenderTechnique870910->parameters1C->slots00[37].handle24,input+0x30);
    }
}

// Original 0x5bd4a0. Retain each float spill between the base and range values.
void CarRenderUploadLightPack(CarRenderLightPack *pack,unsigned int flags) {
    const CarRenderMaterialPackView *p = reinterpret_cast<const CarRenderMaterialPackView *>(pack);
    float a[4],b[4],c[4],ar[4],br[4],cr[4];
    for (int i=0;i<3;++i) a[i] = static_cast<long double>(p->value34[i])*p->value30;
    a[3] = p->value50;
    for (int i=0;i<3;++i) ar[i] = static_cast<long double>(p->value44[i])*p->value40-a[i];
    ar[3] = static_cast<long double>(p->value54)-a[3];
    for (int i=0;i<3;++i) b[i] = static_cast<long double>(p->value60[i])*p->value5C;
    for (int i=0;i<3;++i) br[i] = static_cast<long double>(p->value70[i])*p->value6C-b[i];
    for (int i=0;i<3;++i) c[i] = static_cast<long double>(p->value84[i])*p->value80*0.5f;
    for (int i=0;i<3;++i) cr[i] = static_cast<long double>(p->value94[i])*p->value90*0.5f-c[i];
    b[3] = c[3] = br[3] = cr[3] = 0;
    if (!(flags&0x100) || !ug2RenderFlag870CB8) {
        for (int i=0;i<4;++i) c[i] = cr[i] = 0;
    } else if (static_cast<signed char>(flags)>=0) {
        for (int i=0;i<4;++i) a[i] = ar[i] = b[i] = br[i] = 0;
    }
    int type = ug2RenderTechnique870910->type04;
    if (type == 5 && !ug2RenderTechnique870910->value18) {
        CarRenderDeviceMaterial material;
        float environment[3];
        for (int i=0;i<3;++i) {
            material.diffuse00[i] = static_cast<long double>(ar[i])*0.5f+a[i];
            material.specular20[i] = static_cast<long double>(br[i])*0.5f+b[i];
            environment[i] = static_cast<long double>(cr[i])*0.5f+c[i];
        }
        material.diffuse00[3] = material.specular20[3] = 1;
        for (int i=0;i<4;++i) material.ambient10[i] = 1;
        material.emissive30[0] = material.emissive30[1] = material.emissive30[2] = 0;
        material.emissive30[3] = 1;
        long double power = static_cast<long double>(ug2RenderPower80051C)*p->value58;
        material.power40 = power;
        if (power<powerMinimum784358) material.power40 = powerMinimum784358;
        UG2RenderSetMaterial(ug2SolidDevice870974,&material);
        unsigned int device = ug2SolidDevice870974;
        unsigned int alpha = static_cast<unsigned int>(static_cast<MaterialInteger>(static_cast<long double>(ar[3])*128.0f+static_cast<long double>(a[3])*255.0f));
        unsigned int blue = PackChannel(environment[2]);
        unsigned int green = PackChannel(environment[1]);
        unsigned int red = PackChannel(environment[0]);
        unsigned int packed = (((alpha<<8|blue)<<8|green)<<8)|red;
        UG2RenderSetState(device,60,packed);
    } else if (type == 5 || type == 4) {
        UG2EffectSetVector(ug2RenderTechnique870910->effect20,ug2RenderTechnique870910->parameters1C->slots00[8].handle24,a);
        UG2EffectSetVector(ug2RenderTechnique870910->effect20,ug2RenderTechnique870910->parameters1C->slots00[9].handle24,ar);
        if (ug2RenderTechnique870910->type04 == 5) {
            UG2EffectSetVector(ug2RenderTechnique870910->effect20,ug2RenderTechnique870910->parameters1C->slots00[49].handle24,b);
            UG2EffectSetVector(ug2RenderTechnique870910->effect20,ug2RenderTechnique870910->parameters1C->slots00[51].handle24,br);
            UG2EffectSetVector(ug2RenderTechnique870910->effect20,ug2RenderTechnique870910->parameters1C->slots00[16].handle24,c);
            UG2EffectSetVector(ug2RenderTechnique870910->effect20,ug2RenderTechnique870910->parameters1C->slots00[15].handle24,cr);
            UG2EffectSetFloat(ug2RenderTechnique870910->effect20,ug2RenderTechnique870910->parameters1C->slots00[50].handle24,ug2RenderShaderPower800B94*p->value58);
            UG2EffectSetFloat(ug2RenderTechnique870910->effect20,ug2RenderTechnique870910->parameters1C->slots00[17].handle24,p->value7C);
        }
    }
}
