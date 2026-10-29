#include "CarWheelSecondary.h"
#include "CarRenderSolid.h"
#include "../../Libs/bWare/Strings.h"

void RotateMatrixY(WheelMatrix *,WheelMatrix *,unsigned int); // recovered 0x5bada0

// Original 0x61b500: raw record payload, not a floating conversion.
unsigned int CarRenderPart::AttributeRaw(unsigned int hash,unsigned int fallback) {
    const unsigned int *record=static_cast<const unsigned int *>(FindRecord(hash,0));
    return record ? record[1] : fallback;
}
// Original 0x4fe120. Keep the complete EAX payload; 0x62061f uses signed AL.
unsigned int CarRenderPart::RimValue() { return AttributeRaw(0xeb0101e2,0); }

unsigned int CarWheelCallerFlags(int gate) { return gate==6 ? 0x8000 : 0; }

// Original 0x5bb1c0. The four position W words and bytes 0x71..0x73 stay untouched.
void CarRenderQuad::Initialize() {
    position00[1][0]=position00[2][0]=position00[2][1]=position00[3][1]=64.0f;
    for (int i=0;i<4;++i) colour60[i]=0x80808080;
    position00[0][0]=position00[0][1]=position00[0][2]=0;
    position00[1][1]=position00[1][2]=0;position00[2][2]=0;
    position00[3][0]=position00[3][2]=0;
    uv40[0][0]=uv40[0][1]=0;uv40[1][0]=1;uv40[1][1]=0;
    uv40[2][0]=uv40[2][1]=1;uv40[3][0]=0;uv40[3][1]=1;
    value70=0;
}
// Original 0x5c7e80. The receiver and fifth stack argument are unused.
void CarRenderView::DrawQuad(CarRenderQuad *quad,CarRenderTexture *texture,WheelMatrix *world,unsigned int fourth,unsigned int) {
    CarRenderSubmitQuad(quad,texture,world,fourth);
}

// Original complete 0x620600..0x620ade. The world matrix is forwarded; these
// vertices are generated in local space. Retain the component-specific float
// spills instead of replacing them with a generic matrix/vector transform.
void CarRenderWheelInfo::Fn620600(CarRenderView *view,WheelMatrix *world) {
    CarRenderPart *part=*reinterpret_cast<CarRenderPart **>(carObject+0x604);
    float rim=part ? static_cast<float>(static_cast<signed char>(part->RimValue())) : 17.0f;
    unsigned int hash=cshash(ug2WheelSecondaryTexture7A88A0);
    CarRenderTexture *texture=UG2SolidFindTexture(hash,1,0);
    CarRenderQuad quad;quad.Initialize();
    for (int j=0;j<4;++j) quad.colour60[j]=0x80808080;
    quad.uv40[0][0]=quad.uv40[0][1]=0;quad.uv40[1][0]=1;quad.uv40[1][1]=0;
    quad.uv40[2][0]=quad.uv40[2][1]=1;quad.uv40[3][0]=0;quad.uv40[3][1]=1;
    WheelMatrix rotation;
    for (int j=0;j<16;++j) rotation.elements[j]=(j%5==0) ? 1.0f : 0.0f;
    RotateMatrixY(&rotation,&rotation,0x1999);
    const float *m=rotation.elements;
    volatile long double scaledRim=static_cast<long double>(rim)*ug2WheelSecondaryScale7A889C;
    volatile long double size=scaledRim*ug2WheelSecondaryScale7A6728;
    volatile float positive=size;
    volatile long double negativeSize=-size;
    volatile float negative=negativeSize;
    const float height=ug2WheelSecondaryY7A672C;
    volatile float x0=((static_cast<long double>(m[8])*negative+static_cast<long double>(m[4])*height)+static_cast<long double>(m[0])*negative)+m[12];
    const long double y0=((static_cast<long double>(m[9])*negative+static_cast<long double>(m[5])*height)+static_cast<long double>(m[1])*negative)+m[13];
    volatile float z0=((static_cast<long double>(m[10])*negative+static_cast<long double>(m[6])*height)+static_cast<long double>(m[2])*negative)+m[14];
    volatile float x1Term=static_cast<long double>(m[0])*positive;
    volatile float x1=((static_cast<long double>(m[8])*negative+static_cast<long double>(m[4])*height)+x1Term)+m[12];
    volatile float y1Term=static_cast<long double>(m[1])*positive;
    const long double y1=((static_cast<long double>(m[9])*negative+static_cast<long double>(m[5])*height)+y1Term)+m[13];
    volatile float z1Term=static_cast<long double>(m[2])*positive;
    volatile float z1=((static_cast<long double>(m[10])*negative+static_cast<long double>(m[6])*height)+z1Term)+m[14];
    volatile float x3Base=static_cast<long double>(m[8])*positive+m[12];
    volatile float y3Base=static_cast<long double>(m[9])*positive+m[13];
    volatile float z3Base=static_cast<long double>(m[10])*positive+m[14];
    volatile float x3=(static_cast<long double>(m[4])*height+static_cast<long double>(m[0])*negative)+x3Base;
    const long double y3=(static_cast<long double>(m[5])*height+static_cast<long double>(m[1])*negative)+y3Base;
    volatile float z3=(static_cast<long double>(m[6])*height+static_cast<long double>(m[2])*negative)+z3Base;
    const long double x2=(static_cast<long double>(m[4])*height+x3Base)+x1Term;
    const long double y2=(static_cast<long double>(m[5])*height+y3Base)+y1Term;
    volatile float z2=(static_cast<long double>(m[6])*height+z3Base)+z1Term;
    quad.position00[0][0]=x0;quad.position00[0][1]=y0;quad.position00[0][2]=z0;
    quad.position00[1][0]=x1;quad.position00[1][1]=y1;quad.position00[1][2]=z1;
    quad.position00[2][0]=x2;quad.position00[2][1]=y2;quad.position00[2][2]=z2;
    quad.position00[3][0]=x3;quad.position00[3][1]=y3;quad.position00[3][2]=z3;
    view->DrawQuad(&quad,texture,world,0,0);
}
