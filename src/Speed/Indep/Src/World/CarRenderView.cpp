#include "CarRenderView.h"
#include "CarRenderConsume.h"
#include "../Camera/Camera.h"
#include "../Physics/EngineAngleMath.h"
#include "../Physics/D3DXMatrixScalar.h"
#ifdef _MSC_VER
extern "C" double sqrt(double);
#pragma intrinsic(sqrt)
typedef __int64 ViewInteger;
#else
#include <math.h>
typedef long long ViewInteger;
#endif
extern float cameraAspectScaleX7A27DC,cameraAspectScaleY7A27E0,cameraAspectDivisor7A27D8;

CarRenderClipData *CarRenderClipDataByIndex(int index) {
    return ug2RenderClipData873380+index;
}
// Original 0x491940. The CRT constructs 22 records at 0x832de0, stride 0x70
// (initializer 0x77ccd0). Preserve the self-linked head after clearing the record.
CarRenderView *CarRenderView::Construct() {
    unsigned int *words=reinterpret_cast<unsigned int *>(this);
    for (int i=0;i<0x70/4;++i) words[i]=0;
    link44.next00=link44.previous04=&link44;
    value18=1.0f;
    type04=(reinterpret_cast<unsigned char *>(this)-reinterpret_cast<unsigned char *>(ug2RenderViews832DE0))/0x70;
    clip00=CarRenderClipDataByIndex(type04);
    value60=value64=0;
    return this;
}

// Original 0x5c7ec0. The fourth argument is unused. Both nonzero overrides write
// the near value in the binary; the second does not override the far value.
void CarRenderClipData::Update(CarRenderView *view,float firstOverride,float secondOverride,unsigned int unused) {
    const CameraState *camera=reinterpret_cast<const CameraState *>(view->frame40);
    if (!camera) {
        WheelIdentity(&matrices00[0]);
        WheelIdentity(&matrices00[1]);
        return;
    }
    BodyMatrixCopy(&matrices00[0],const_cast<WheelMatrix *>(&camera->matrix0));
    float nearValue=camera->nearBC;
    float farValue=view->type04==1 ? 15000.0f : camera->farC0;
    if (firstOverride!=0.0f) nearValue=firstOverride;
    if (secondOverride!=0.0f) nearValue=secondOverride;
    volatile float aspect=static_cast<long double>(view->viewport58->height18)/view->viewport58->width14;
    float scaleX=1.0f;
    int type=view->type04;
    if (type==1 || type==4 || type==2 || type==5) {
        scaleX=cameraAspectScaleX7A27DC;
        aspect=static_cast<long double>(cameraAspectScaleY7A27E0)/cameraAspectDivisor7A27D8*aspect;
    }
    if (type==9 || type==10) aspect=0.60024011135101318359375f; // binary32 0x3f19a956
    int halfX=static_cast<int>(static_cast<unsigned int>(static_cast<ViewInteger>(static_cast<long double>(camera->fovC4)*scaleX*0.5f)));
    volatile float sine=EngineSineUnitsExtended(halfX);
    long double cosine=EngineCosineUnitsExtended(halfX);
    volatile float xScale=1.0f/(static_cast<long double>(sine)/cosine);
    int halfY=static_cast<int>(static_cast<unsigned int>(static_cast<ViewInteger>(static_cast<long double>(camera->fovC4)*aspect*0.5f)));
    sine=EngineSineUnitsExtended(halfY);
    cosine=EngineCosineUnitsExtended(halfY);
    volatile float yScale=1.0f/(static_cast<long double>(sine)/cosine);
    volatile float depthScale=static_cast<long double>(farValue)/(static_cast<long double>(farValue)-nearValue);
    WheelIdentity(&matrices00[1]);
    float *projection=matrices00[1].elements;
    projection[5]=-yScale;
    projection[0]=xScale;
    projection[10]=depthScale;
    projection[11]=1.0f;
    projection[15]=0;
    projection[14]=-(static_cast<long double>(depthScale)*nearValue);
    unsigned int half=camera->fovC4>>1;
    sine=EngineSineUnitsExtended(half);
    cosine=EngineCosineUnitsExtended(half);
    long double tangent=static_cast<long double>(sine)/cosine;
    view->focal0C=tangent==0.0f ? 10.0f : 256.0f/tangent;
    matrices00[0].elements[3]=matrices00[0].elements[7]=matrices00[0].elements[11]=0;
    D3DXMatrixMultiplyScalar(&matrices00[2],&matrices00[0],&matrices00[1]);
    for (int i=0;i<6;++i) {
        volatile float *plane=planesC0[i];
        const float *combined=matrices00[2].elements;
        for (int j=0;j<4;++j) {
            int row=j*4;
            if (i==0) plane[j]=static_cast<long double>(combined[row+3])+combined[row];
            else if (i==1) plane[j]=static_cast<long double>(combined[row+3])-combined[row];
            else if (i==2) plane[j]=static_cast<long double>(combined[row+3])+combined[row+1];
            else if (i==3) plane[j]=static_cast<long double>(combined[row+3])-combined[row+1];
            else if (i==4) reinterpret_cast<unsigned int *>(planesC0[i])[j]=reinterpret_cast<const unsigned int *>(combined)[row+2];
            else plane[j]=static_cast<long double>(combined[row+3])-combined[row+2];
        }
        long double x=plane[0],y=plane[1],z=plane[2];
        long double squared=(x*x+y*y)+z*z;
#ifdef __GNUC__
        long double length=__builtin_sqrtl(squared);
#else
        long double length=sqrt(squared);
#endif
        long double inverse=1.0f/length;
        plane[0]=inverse*plane[0];
        plane[1]=inverse*plane[1];
        plane[2]=inverse*plane[2];
        plane[3]=inverse*plane[3];
    }
}
