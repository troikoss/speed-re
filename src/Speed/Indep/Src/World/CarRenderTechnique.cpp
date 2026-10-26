#include "CarRenderTechnique.h"
#include "../../Libs/bWare/Strings.h"

extern "C" double __cdecl sqrt(double);
#pragma intrinsic(sqrt)

namespace {
struct ParameterName { unsigned int index; const char *text; };
// Original 0x5d0f80 copy/hash/store order, including RAMPMAP before the two RAINDROP entries.
const ParameterName parameterNames[60] = {
    {0,"AMBIENT"}, // 0x007a4534
    {1,"AMBIENTCOEFF"}, // 0x007a4524
    {2,"BLENDSTATE"}, // 0x007a4518
    {3,"BASEMINTEXTUREFILTER"}, // 0x007a4500
    {4,"BASEMAGTEXTUREFILTER"}, // 0x007a44e8
    {5,"BASETEXTUREFILTERPARAM"}, // 0x007a44d0
    {6,"BLENDMATRICES"}, // 0x007a44c0
    {7,"DiffuseMap"}, // 0x007a44b4
    {8,"DIFFUSEMIN"}, // 0x007a44a8
    {9,"DIFFUSERANGE"}, // 0x007a4498
    {10,"DIFFUSETEX"}, // 0x007a448c
    {11,"DISPLACEMAP"}, // 0x007a4480
    {12,"DOFFALLOFF"}, // 0x007a4474
    {13,"DOFOFFSET1"}, // 0x007a4468
    {14,"DOFOFFSET2"}, // 0x007a445c
    {15,"ENVMAPANGE"}, // 0x007a4450
    {16,"ENVMAPMIN"}, // 0x007a4444
    {17,"ENVMAPPOWER"}, // 0x007a4438
    {18,"EnvMapTexture"}, // 0x007a4428
    {19,"FECOLORWRITEFLAG"}, // 0x007a4414
    {20,"FILTERBLEND"}, // 0x007a4408
    {21,"FILTERTEXTURE0"}, // 0x007a43f8
    {22,"FILTERTEXTURE1"}, // 0x007a43e8
    {23,"FILTERTEXTURE2"}, // 0x007a43d8
    {24,"FILTERTEXTURE3"}, // 0x007a43c8
    {25,"FILTERWEIGHTS"}, // 0x007a43b8
    {26,"FOCALRANGE"}, // 0x007a43ac
    {27,"FOGCOLOR"}, // 0x007a43a0
    {28,"FOGON"}, // 0x007a4398
    {29,"FOGVALUE"}, // 0x007a438c
    {30,"GAINMAP"}, // 0x007a4384
    {31,"HARMONIC"}, // 0x007a4378
    {32,"HEADLIGHTCLIPMAT"}, // 0x007a4364
    {33,"HEADLIGHTCLIPTEX"}, // 0x007a4350
    {34,"HEADLIGHTCOLOR"}, // 0x007a4340
    {35,"HEADLIGHTTEX"}, // 0x007a4330
    {36,"HEADLIGHTWVP"}, // 0x007a4320
    {37,"LOCALEYEPOS"}, // 0x007a4314
    {38,"MOTIONALPHA"}, // 0x007a4308
    {39,"OVERBRIGHTGREYSCALE"}, // 0x007a42f4
    {40,"OVERBRIGHTOFFSET"}, // 0x007a42e0
    {43,"RAMPMAP"}, // 0x007a42d8
    {41,"RAINDROPOFFSET"}, // 0x007a42c8
    {42,"RAINDROPALPHA"}, // 0x007a42b8
    {44,"REFLECTEDPROJ"}, // 0x007a42a8
    {45,"REFLECTEDTEX"}, // 0x007a4298
    {46,"REFLECTEDTEXMIN"}, // 0x007a4288
    {47,"SATURATIONSCALE"}, // 0x007a4278
    {48,"SCREENOFFSET"}, // 0x007a4268
    {49,"SPECULARMIN"}, // 0x007a425c
    {50,"SPECULARPOWER"}, // 0x007a424c
    {51,"SPECULARRANGE"}, // 0x007a423c
    {52,"SKYMORPHALPHA"}, // 0x007a422c
    {53,"SKYMORPHALPHA2"}, // 0x007a421c
    {54,"SKYMORPHTEXTURE"}, // 0x007a420c
    {55,"TEXTUREOFFSET"}, // 0x007a41fc
    {56,"TEXTUREOFFSETMATRIX"}, // 0x007a41e8
    {57,"TINTCOLOR"}, // 0x007a41dc
    {58,"WORLDVIEW"}, // 0x007a41d0
    {59,"WORLDVIEWPROJECTION"}, // 0x007a41bc
};
unsigned int ReadWord(const unsigned char *record,unsigned int offset) {
    return *reinterpret_cast<const unsigned int *>(record+offset);
}
}

// Original 0x5d0f80. Empty library tag storage is passed by address as in the binary.
CarRenderTechnique *CarRenderTechnique::Construct() {
    value00 = 0x7a41b0;
    value18 = 0;
    CarRenderTechniqueParameters *parameters = static_cast<CarRenderTechniqueParameters *>(UG2TextureMalloc(0x988));
    if (parameters) { unsigned char tag; parameters->map960.Construct(&tag,&tag); }
    parameters1C = parameters;
    for (int i=0;i<60;++i) {
        const ParameterName &entry = parameterNames[i];
        nfs_strcpy(parameters1C->slots00[entry.index].name00,entry.text);
        parameters1C->slots00[entry.index].hash20 = cshash(entry.text);
        parameters1C->slots00[entry.index].handle24 = 0;
    }
    effect20 = 0;
    return this;
}

// Original 0x5d1b70. The first matching static slot is also entered in the library map.
void CarRenderTechniqueParameters::Bind(unsigned int hash,unsigned int handle) {
    for (int i=0;i<60;++i) {
        if (slots00[i].hash20 == hash) {
            slots00[i].handle24 = handle;
            CarRenderTechniqueMapPair pair;
            pair.hash00 = hash;
            pair.value04 = 0;
            CarRenderTechniqueMapResult result;
            map960.Insert(&result,&pair)->node00->value0C = handle;
            return;
        }
    }
}

// Original 0x5d1bd0. Lookup uses the parameter semantic, not its display name.
void CarRenderTechnique::CacheParameters() {
    CarRenderEffectDescription description;
    UG2EffectGetDescription(effect20,&description);
    for (unsigned int i=0;i<description.parameters04;++i) {
        unsigned int parameter = UG2EffectGetParameter(effect20,0,i);
        CarRenderEffectParameterDescription parameterDescription;
        UG2EffectGetParameterDescription(effect20,parameter,&parameterDescription);
        if (parameterDescription.semantic04)
            parameters1C->Bind(cshash(parameterDescription.semantic04),parameter);
    }
}

// Original 0x5be560.
unsigned int CarRenderTechniqueMagFilter() {
    unsigned int mode = static_cast<unsigned int>(ug2TechniqueFilter870D18);
    if (mode == 0) return 1;
    if (mode == 1) return 2;
    if (mode == 2) {
        if (ReadWord(ug2TechniqueAdapter86C120,0x428) == 0x102b) return 2;
        return ((ReadWord(ug2TechniqueCaps86E5C8,0x40)&0x04000000)|0x08000000)>>26;
    }
    return mode;
}

// Original 0x5d1e20. D3D/D3DX calls are typed platform boundaries.
void CarRenderTechnique::Setup(int index) {
    if (effect20) { UG2TechniqueRelease(effect20); effect20 = 0; }
    const unsigned int vendor = ReadWord(ug2TechniqueAdapter86C120,0x428);
    const unsigned int device = ReadWord(ug2TechniqueAdapter86C120,0x42c);
    type04 = index;
    parameters1C->Reset();
    const CarRenderTechniqueRecipe &recipe = ug2TechniqueRecipes7FF7D0[index];
    CarRenderVertexElement elements[16];
    unsigned int stride = 0;
    int count = 0;
    while (recipe.elements00[count][0] != 8) {
        unsigned int type = recipe.elements00[count][0], usage = recipe.elements00[count][1];
        CarRenderVertexElement &element = elements[count];
        element.stream00 = 0;
        element.offset02 = static_cast<unsigned short>(stride);
        element.type04 = static_cast<unsigned char>(ug2TechniqueTypes800494[type]);
        element.method05 = 0;
        element.usage06 = static_cast<unsigned char>(ug2TechniqueUsages8004B4[usage]);
        element.index07 = ug2TechniqueIndices8004D8[usage];
        stride += ug2TechniqueSizes8004E4[type];
        ++count;
    }
    elements[count].stream00 = 0xff;
    elements[count].offset02 = 0;
    elements[count].type04 = 0x11;
    elements[count].method05 = elements[count].usage06 = elements[count].index07 = 0;
    UG2TechniqueCreateDeclaration(ug2SolidDevice870974,elements,&declaration24);
    stride08 = stride;
    unsigned int errors;
    UG2TechniqueCreateEffect(ug2SolidDevice870974,0,recipe.resource84,0,0,0,ug2TechniquePool870914,&effect20,&errors);
    CacheParameters();
    value10 = 0;
    for (int i=0;i<ug2TechniqueAttempts800BF4;++i) {
        unsigned int technique = UG2EffectGetTechnique(effect20,i);
        if (!technique) continue;
        unsigned int annotation = UG2EffectGetAnnotation(effect20,technique,ug2TechniqueShader7A4584);
        int shader = 0;
        if (annotation) UG2EffectGetInt(effect20,annotation,&shader);
        if (ug2TechniqueSkip870908 && shader == 1) continue;
        if ((index == 2 || index == 3) && (vendor == 0x1039 || (vendor == 0x8086 && device == 0x2572)) && i<3) continue;
        if (index == 5 && vendor == 0x102b && i == 0) continue;
        if (index == 15 && vendor == 0x1039 && i == 0) continue;
        unsigned int handle = parameters1C->slots00[2].handle24;
        if (handle) {
            ug2TechniqueBlend86EAD8[0] = ug2TechniqueBlend86EAD8[1] = ug2TechniqueBlend86EAD8[2] = 0;
            ug2TechniqueBlend86EAD8[3] = 5;
            ug2TechniqueBlend86EAD8[4] = 6;
            UG2EffectSetIntArray(effect20,handle,ug2TechniqueBlend86EAD8,5);
        }
        handle = parameters1C->slots00[3].handle24;
        if (handle) {
            // The original leaves this local undefined for selectors outside 0..2.
            int filter;
            if (ug2TechniqueFilter870D18 == 0) filter = 1;
            else if (ug2TechniqueFilter870D18 == 1) filter = 2;
            else if (ug2TechniqueFilter870D18 == 2) filter = 2+(ReadWord(ug2TechniqueAdapter86C120,0x428) != 0x102b);
            UG2EffectSetInt(effect20,handle,filter);
        }
        handle = parameters1C->slots00[4].handle24;
        if (handle) {
            unsigned int effect = effect20;
            unsigned int filter = CarRenderTechniqueMagFilter();
            UG2EffectSetInt(effect,handle,filter);
        }
        handle = parameters1C->slots00[28].handle24;
        if (handle) UG2EffectSetInt(effect20,handle,1);
        handle = parameters1C->slots00[5].handle24;
        if (handle) UG2EffectSetInt(effect20,handle,ReadWord(ug2TechniqueCaps86E5C8,0x6c));
        handle = parameters1C->slots00[19].handle24;
        if (handle) UG2EffectSetInt(effect20,handle,15);
        if (UG2EffectValidate(effect20,technique) == 0) {
            index0C = i;
            value10 = technique;
            UG2EffectSetTechnique(effect20,technique);
            value18 = shader != 0;
            break;
        }
    }
    unsigned int passes = 0;
    while (passes<16 && UG2EffectGetPass(effect20,value10,passes)) ++passes;
    passCount14 = passes;
    if (type04 == 9) UG2EffectSetFloat(effect20,parameters1C->slots00[26].handle24,0.009f);
}

// Original 0x5bc5d0. Keep each x87 operation and the truncating integer conversion.
void CarRenderCreateTechniqueRamp() {
    UG2TextureCreate(ug2SolidDevice870974,64,1,0,0,21,1,&ug2TechniqueRamp86BD18,0);
    CarRenderLockedTexture locked;
    UG2TextureLock(ug2TechniqueRamp86BD18,0,&locked,0,0);
    unsigned int *pixels = reinterpret_cast<unsigned int *>(locked.pixels04);
    int i;
    for (i=0;i<33;++i) pixels[i] = 0;
    for (i=33;i<48;++i) {
        long double value = static_cast<long double>(i-33);
        value *= 0.06666667f;
        value = 1.0f-value;
#ifdef __GNUC__
        value = __builtin_sqrtl(value);
#else
        value = sqrt(value);
#endif
        value *= 255.0f;
        unsigned int channel = static_cast<unsigned int>(static_cast<int>(value));
        pixels[i] = (((channel<<8|channel)<<8|channel)<<8)|channel;
    }
    for (i=48;i<64;++i) pixels[i] = 0;
    UG2TextureUnlock(ug2TechniqueRamp86BD18,0);
}

// Original 0x5d21f0. The tokens are original class identities, never host vtables.
void CarRenderInitialiseTechniques() {
    UG2TechniqueGetCaps(ug2SolidDevice870974,ug2TechniqueCaps86E5C8);
    unsigned int flags = ReadWord(ug2TechniqueCaps86E5C8,0x40);
    int maximum = 0;
    ug2TechniqueMaximum876530 = 0;
    if ((flags&0x02000000) && (flags&0x200) && (flags&0x20000)) {
        maximum = 1;
        ug2TechniqueMaximum876530 = 1;
    }
    if (flags&0x400) { ++maximum; ug2TechniqueMaximum876530 = maximum; }
    if (ug2TechniqueFilter870D18>maximum) ug2TechniqueFilter870D18 = maximum;
    if (ug2TechniquePool870914) UG2TechniqueRelease(ug2TechniquePool870914);
    UG2TechniqueCreatePool(&ug2TechniquePool870914);
    if (!ug2TechniqueReady87090C) {
        for (int i=0;i<23;++i) {
            CarRenderTechnique *technique = static_cast<CarRenderTechnique *>(UG2TextureMalloc(0x28));
            if (technique) {
                technique->Construct();
                unsigned int token = 0x7a4578;
                switch (i) {
                    case 1: case 10: case 14: token = 0x7a453c; break;
                    case 2: case 11: token = 0x7a4548; break;
                    case 3: case 12: token = 0x7a4554; break;
                    case 5: token = 0x7a4560; break;
                    case 15: case 16: case 18: token = 0x7a456c; break;
                }
                technique->value00 = token;
            }
            ug2SolidTechniques86EB28[i] = technique;
        }
    }
    UG2TechniqueGetCaps(ug2SolidDevice870974,ug2TechniqueCaps86E5C8);
    UG2TechniqueGetAdapter(ug2TechniqueSystem870970,ug2TechniqueAdapter8709AC,2,ug2TechniqueAdapter86C120);
    for (int i=0;i<23;++i) ug2SolidTechniques86EB28[i]->Setup(i);
    ug2TechniqueReady87090C = 1;
    CarRenderCreateTechniqueRamp();
}
