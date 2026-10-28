#include "D3DXMatrixScalar.h"

// Original scalar transpose 0x63d132..0x63d1a2. The first pair passes through
// x87 in both directions; the other pairs preserve one DWORD and widen the
// other through x87. Volatile extended temporaries retain those conversions
// (including signaling NaN quieting) and the observed reads before stores.
// Partial overlaps deliberately retain the original pair-by-pair order.
WheelMatrix *__stdcall D3DXMatrixTransposeScalar(WheelMatrix *output,const WheelMatrix *input) {
    unsigned int *to=reinterpret_cast<unsigned int *>(output->elements);
    const unsigned int *from=reinterpret_cast<const unsigned int *>(input->elements);
    volatile long double saved=input->elements[1];
    volatile long double other=input->elements[4];
    output->elements[1]=other;
    output->elements[4]=saved;
    unsigned int word=from[8];
    saved=input->elements[2];
    to[2]=word;
    output->elements[8]=saved;
    word=from[12];
    saved=input->elements[3];
    to[3]=word;
    output->elements[12]=saved;
    word=from[9];
    saved=input->elements[6];
    to[6]=word;
    output->elements[9]=saved;
    word=from[13];
    saved=input->elements[7];
    to[7]=word;
    output->elements[13]=saved;
    word=from[14];
    saved=input->elements[11];
    to[11]=word;
    output->elements[14]=saved;
    if (output!=input) {
        to[0]=from[0];
        to[5]=from[5];
        to[10]=from[10];
        to[15]=from[15];
    }
    return output;
}

// Original scalar D3DX library implementation 0x63cee1..0x63d118. Exact aliases
// use a temporary; distinct partially overlapping records retain direct stores.
WheelMatrix *__stdcall D3DXMatrixMultiplyScalar(WheelMatrix *output,const WheelMatrix *left,const WheelMatrix *right) {
    WheelMatrix temporary;
    float *r=(output==left || output==right) ? temporary.elements : output->elements;
    const float *a=left->elements,*b=right->elements;
    r[0]=((((static_cast<long double>(a[3])*static_cast<long double>(b[12]))+(static_cast<long double>(a[0])*static_cast<long double>(b[0])))+(static_cast<long double>(a[2])*static_cast<long double>(b[8])))+(static_cast<long double>(b[4])*static_cast<long double>(a[1])));
    r[1]=((((static_cast<long double>(a[1])*static_cast<long double>(b[5]))+(static_cast<long double>(a[3])*static_cast<long double>(b[13])))+(static_cast<long double>(a[2])*static_cast<long double>(b[9])))+(static_cast<long double>(b[1])*static_cast<long double>(a[0])));
    r[2]=((((static_cast<long double>(a[3])*static_cast<long double>(b[14]))+(static_cast<long double>(b[2])*static_cast<long double>(a[0])))+(static_cast<long double>(a[2])*static_cast<long double>(b[10])))+(static_cast<long double>(b[6])*static_cast<long double>(a[1])));
    r[3]=((((static_cast<long double>(a[3])*static_cast<long double>(b[15]))+(static_cast<long double>(b[11])*static_cast<long double>(a[2])))+(static_cast<long double>(a[0])*static_cast<long double>(b[3])))+(static_cast<long double>(b[7])*static_cast<long double>(a[1])));
    r[4]=((((static_cast<long double>(a[7])*static_cast<long double>(b[12]))+(static_cast<long double>(a[4])*static_cast<long double>(b[0])))+(static_cast<long double>(b[4])*static_cast<long double>(a[5])))+(static_cast<long double>(a[6])*static_cast<long double>(b[8])));
    r[5]=((((static_cast<long double>(a[6])*static_cast<long double>(b[9]))+(static_cast<long double>(a[7])*static_cast<long double>(b[13])))+(static_cast<long double>(b[5])*static_cast<long double>(a[5])))+(static_cast<long double>(b[1])*static_cast<long double>(a[4])));
    r[6]=((((static_cast<long double>(b[6])*static_cast<long double>(a[5]))+(static_cast<long double>(a[7])*static_cast<long double>(b[14])))+(static_cast<long double>(b[2])*static_cast<long double>(a[4])))+(static_cast<long double>(a[6])*static_cast<long double>(b[10])));
    r[7]=((((static_cast<long double>(b[7])*static_cast<long double>(a[5]))+(static_cast<long double>(a[7])*static_cast<long double>(b[15])))+(static_cast<long double>(a[6])*static_cast<long double>(b[11])))+(static_cast<long double>(a[4])*static_cast<long double>(b[3])));
    r[8]=((((static_cast<long double>(a[11])*static_cast<long double>(b[12]))+(static_cast<long double>(a[8])*static_cast<long double>(b[0])))+(static_cast<long double>(b[4])*static_cast<long double>(a[9])))+(static_cast<long double>(a[10])*static_cast<long double>(b[8])));
    r[9]=((((static_cast<long double>(a[11])*static_cast<long double>(b[13]))+(static_cast<long double>(a[10])*static_cast<long double>(b[9])))+(static_cast<long double>(b[5])*static_cast<long double>(a[9])))+(static_cast<long double>(b[1])*static_cast<long double>(a[8])));
    r[10]=((((static_cast<long double>(b[6])*static_cast<long double>(a[9]))+(static_cast<long double>(b[2])*static_cast<long double>(a[8])))+(static_cast<long double>(a[11])*static_cast<long double>(b[14])))+(static_cast<long double>(a[10])*static_cast<long double>(b[10])));
    r[11]=((((static_cast<long double>(a[11])*static_cast<long double>(b[15]))+(static_cast<long double>(a[10])*static_cast<long double>(b[11])))+(static_cast<long double>(b[7])*static_cast<long double>(a[9])))+(static_cast<long double>(a[8])*static_cast<long double>(b[3])));
    r[12]=((((static_cast<long double>(a[15])*static_cast<long double>(b[12]))+(static_cast<long double>(a[14])*static_cast<long double>(b[8])))+(static_cast<long double>(a[12])*static_cast<long double>(b[0])))+(static_cast<long double>(b[4])*static_cast<long double>(a[13])));
    r[13]=((((static_cast<long double>(a[15])*static_cast<long double>(b[13]))+(static_cast<long double>(b[5])*static_cast<long double>(a[13])))+(static_cast<long double>(a[14])*static_cast<long double>(b[9])))+(static_cast<long double>(b[1])*static_cast<long double>(a[12])));
    r[14]=((((static_cast<long double>(a[14])*static_cast<long double>(b[10]))+(static_cast<long double>(b[6])*static_cast<long double>(a[13])))+(static_cast<long double>(b[2])*static_cast<long double>(a[12])))+(static_cast<long double>(a[15])*static_cast<long double>(b[14])));
    r[15]=((((static_cast<long double>(a[15])*static_cast<long double>(b[15]))+(static_cast<long double>(b[7])*static_cast<long double>(a[13])))+(static_cast<long double>(a[12])*static_cast<long double>(b[3])))+(static_cast<long double>(b[11])*static_cast<long double>(a[14])));
    if (r!=output->elements) {
        unsigned int *to=reinterpret_cast<unsigned int *>(output->elements);
        const unsigned int *from=reinterpret_cast<const unsigned int *>(temporary.elements);
        for (int i=0;i<16;++i) to[i]=from[i];
    }
    return output;
}
