#include "CrtPowerSse.h"

// Original UG2 PC v1.2 SSE power, 0x764749..0x765275. Integer encodings,
// three reciprocal/log tables, packed polynomial order and exception exits
// follow the binary. Values below retain both lanes, without binding compiler
// registers. FP instruction semantics live in the native ISA adapter.
namespace {
#ifdef _MSC_VER
typedef unsigned __int64 UInt64;
#else
typedef unsigned long long UInt64;
#endif
D3DXCrtSseValue Table(unsigned int address,unsigned int words=4) {
    D3DXCrtSseValue value={{0,0,0,0}};
    const unsigned int *data=reinterpret_cast<const unsigned int *>(ug2CrtPowerSseData7D4310+address-0x7d4310);
    for (unsigned int j=0;j<words;++j) value.word[j]=data[j];
    return value;
}
D3DXCrtSseValue Vector(D3DXCrtDoubleBits bits) {
    D3DXCrtSseValue value={{bits.low,bits.high,0,0}};return value;
}
D3DXCrtDoubleBits Low(const D3DXCrtSseValue &value) {
    D3DXCrtDoubleBits bits={value.word[0],value.word[1]};return bits;
}
void CopyDouble(double &out,D3DXCrtDoubleBits bits) {
    unsigned int *words=reinterpret_cast<unsigned int *>(&out);words[0]=bits.low;words[1]=bits.high;
}
void CopyBits(D3DXCrtDoubleBits &out,const double &value) {
    const unsigned int *words=reinterpret_cast<const unsigned int *>(&value);out.low=words[0];out.high=words[1];
}
double &Number(D3DXCrtDoubleBits &bits) { return *reinterpret_cast<double *>(&bits); }
void LowMove(D3DXCrtSseValue &value,const D3DXCrtSseValue &other) { value.word[0]=other.word[0];value.word[1]=other.word[1]; }
UInt64 Quad(const D3DXCrtSseValue &value,unsigned int lane) { return value.word[lane*2]|(static_cast<UInt64>(value.word[lane*2+1])<<32); }
void SetQuad(D3DXCrtSseValue &value,UInt64 bits,unsigned int lane) { value.word[lane*2]=static_cast<unsigned int>(bits);value.word[lane*2+1]=static_cast<unsigned int>(bits>>32); }
void SetDword(D3DXCrtSseValue &value,unsigned int bits) { value.word[0]=bits;value.word[1]=value.word[2]=value.word[3]=0; }
unsigned int Word(const D3DXCrtSseValue &value,unsigned int lane) { return (value.word[lane/2]>>((lane%2)*16))&0xffff; }
void SetWord(D3DXCrtSseValue &value,unsigned int bits,unsigned int lane) {
    unsigned int shift=(lane%2)*16;value.word[lane/2]=(value.word[lane/2]&~(0xffffu<<shift))|((bits&0xffff)<<shift);
}
void And(D3DXCrtSseValue &value,const D3DXCrtSseValue &other) { for (int j=0;j<4;++j) value.word[j]&=other.word[j]; }
void Or(D3DXCrtSseValue &value,const D3DXCrtSseValue &other) { for (int j=0;j<4;++j) value.word[j]|=other.word[j]; }
void Xor(D3DXCrtSseValue &value,const D3DXCrtSseValue &other) { for (int j=0;j<4;++j) value.word[j]^=other.word[j]; }
void ShiftLeft(D3DXCrtSseValue &value,UInt64 count) {
    for (int j=0;j<4;j+=2) {
        unsigned int low=value.word[j],high=value.word[j+1],n=static_cast<unsigned int>(count);
        if (count>63) low=high=0;
        else if (n>=32) { high=low<<(n-32);low=0; }
        else if (n) { high=(high<<n)|(low>>(32-n));low<<=n; }
        value.word[j]=low;value.word[j+1]=high;
    }
}
void ShiftRight(D3DXCrtSseValue &value,UInt64 count) {
    for (int j=0;j<4;j+=2) {
        unsigned int low=value.word[j],high=value.word[j+1],n=static_cast<unsigned int>(count);
        if (count>63) low=high=0;
        else if (n>=32) { low=high>>(n-32);high=0; }
        else if (n) { low=(low>>n)|(high<<(32-n));high>>=n; }
        value.word[j]=low;value.word[j+1]=high;
    }
}
void SubtractDwords(D3DXCrtSseValue &value,const D3DXCrtSseValue &other) { for (int j=0;j<4;++j) value.word[j]-=other.word[j]; }
void SubtractQuads(D3DXCrtSseValue &value,const D3DXCrtSseValue &other) { for (int j=0;j<2;++j) SetQuad(value,Quad(value,j)-Quad(other,j),j); }
void EqualDwords(D3DXCrtSseValue &value,const D3DXCrtSseValue &other) { for (int j=0;j<4;++j) value.word[j]=value.word[j]==other.word[j] ? 0xffffffff : 0; }
void MaximumWords(D3DXCrtSseValue &value,const D3DXCrtSseValue &other) {
    for (int j=0;j<8;++j) { unsigned int a=Word(value,j),b=Word(other,j);if (static_cast<short>(a)<static_cast<short>(b)) SetWord(value,b,j); }
}
unsigned int ByteMask(const D3DXCrtSseValue &value) {
    unsigned int mask=0;for (int j=0;j<16;++j) mask|=((value.word[j/4]>>((j%4)*8+7))&1)<<j;return mask;
}
void Shuffle(D3DXCrtSseValue &value,const D3DXCrtSseValue &other,unsigned int order) {
    D3DXCrtSseValue source=other;for (int j=0;j<4;++j) value.word[j]=source.word[(order>>(j*2))&3];
}
void UnpackLow(D3DXCrtSseValue &value,const D3DXCrtSseValue &other) { value.word[2]=other.word[0];value.word[3]=other.word[1]; }
unsigned int ArithmeticRight(unsigned int value,unsigned int count) { return (value>>count)|((value&0x80000000) ? ~(0xffffffffu>>count) : 0); }
int Signed(unsigned int value) { return static_cast<int>(value); }
}

extern "C" void __cdecl D3DXCrtPowerSseResult(double *out,D3DXCrtDoubleBits first,D3DXCrtDoubleBits second) {
    // Incoming scratch lanes are unobservable along reachable arithmetic paths;
    // see the dataflow notes and entry-state audit in docs/D3DX_CRT_POWER_SSE.md.
    D3DXCrtSseValue zero={{0,0,0,0}};
    D3DXCrtSseValue value0=zero,value1=zero,value2=zero,value3=zero,value4=zero,value5=zero,value6=zero,value7=zero,source=zero;
    D3DXCrtDoubleBits result={0,0};
    unsigned int index=0,signScale=0,bound=0,splitExponent=0,extraExponent=0,compareLeft=0,compareRight=0;
    LowMove(value0,Vector(first));
    LowMove(value7,Table(0x007d7ba0,2));
    LowMove(value2,Table(0x007d7bb0,2));
    And(value7,value0);
    LowMove(value4,value0);
    ShiftRight(value0,0x2c);
    index=Word(value0,0x0);
    Or(value7,value2);
    signScale=Word(value4,0x3);
    index&=0xff;
    index+=0x1;
    index&=0x1fe;
    source=Table(index*0x4 + 0x7d4310,2);UG2FxCrtSseMultiplyScalar(&value7,&source);
    LowMove(value5,Table(index*0x4 + 0x7d4310,2));
    index+=index;
    value6=Table(index*0x4 + 0x7d4720);
    bound=0x7fef;
    bound-=signScale;
    signScale-=0x10;
    signScale|=bound;
    compareLeft=signScale;compareRight=0x80000000;
    if (compareLeft>=compareRight) goto block_764a97;
    signScale=0x0;
    bound=0x3fe7f;
block_7647c6: // Three-stage logarithm reduction and split head/tail product.
    SetDword(value1,bound);
    SubtractQuads(value0,value1);
    ShiftRight(value0,0x8);
    UG2FxCrtSseConvertDwords(&value0);
    LowMove(value1,Table(0x007d7bf8,2));
    LowMove(value3,value7);
    ShiftRight(value7,0x26);
    index=Word(value7,0x0);
    And(value4,Table(0x007d7ba0));
    index&=0xff;
    index+=0x1;
    index&=0x1fe;
    source=Table(index*0x4 + 0x7d4f30,2);UG2FxCrtSseMultiplyScalar(&value3,&source);
    source=Table(index*0x4 + 0x7d4f30,2);UG2FxCrtSseMultiplyScalar(&value5,&source);
    index+=index;
    source=Table(index*0x4 + 0x7d5340);UG2FxCrtSseAddPacked(&value6,&source);
    Or(value4,Table(0x007d7bb0));
    UG2FxCrtSseAddScalar(&value6,&value0);
    And(value1,value4);
    LowMove(value2,value3);
    ShiftRight(value3,0x1f);
    index=Word(value3,0x0);
    LowMove(value0,Table(0x007d7bf8,2));
    UG2FxCrtSseSubtractScalar(&value4,&value1);
    LowMove(value7,Table(0x007d7c00,2));
    index&=0x1ff;
    index+=0x1;
    index&=0x3fe;
    source=Table(index*0x4 + 0x7d5b50,2);UG2FxCrtSseMultiplyScalar(&value5,&source);
    source=Table(index*0x4 + 0x7d5b50,2);UG2FxCrtSseMultiplyScalar(&value2,&source);
    source=Table(index*0x8 + 0x7d6360);UG2FxCrtSseAddPacked(&value6,&source);
    And(value0,value5);
    UG2FxCrtSseSubtractScalar(&value5,&value0);
    UG2FxCrtSseAddScalar(&value7,&value2);
    LowMove(value3,value0);
    UG2FxCrtSseMultiplyScalar(&value0,&value1);
    UG2FxCrtSseMultiplyScalar(&value1,&value5);
    UG2FxCrtSseMultiplyScalar(&value3,&value4);
    UG2FxCrtSseSubtractScalar(&value2,&value0);
    UG2FxCrtSseMultiplyScalar(&value4,&value5);
    LowMove(value0,value6);
    UG2FxCrtSseSubtractScalar(&value2,&value1);
    UG2FxCrtSseAddScalar(&value6,&value7);
    LowMove(value1,Vector(second));
    index=Word(value1,0x3);
    UG2FxCrtSseSubtractScalar(&value2,&value3);
    UG2FxCrtSseSubtractScalar(&value0,&value6);
    LowMove(value3,Table(0x007d7bf8,2));
    bound=Word(value6,0x3);
    UG2FxCrtSseSubtractScalar(&value2,&value4);
    LowMove(value4,value6);
    UG2FxCrtSseAddScalar(&value0,&value7);
    UG2FxCrtSseSubtractScalar(&value7,&value2);
    UG2FxCrtSseSubtractScalar(&value6,&value2);
    UnpackLow(value7,value7);
    index&=0x7ff0;
    compareLeft=index;compareRight=0x7ff0;
    if (compareLeft>=compareRight) goto block_764e08;
    bound&=0x7ff0;
    index-=0x3ff0;
    index+=bound;
    bound=0x40a0;
    bound-=index;
    index-=0x3c70;
    bound|=index;
    compareLeft=bound;compareRight=0x80000000;
    if (compareLeft>=compareRight) goto block_7651f1;
block_764918: // Scale by 128 and convert with the current MXCSR rounding mode.
    UG2FxCrtSseSubtractScalar(&value4,&value6);
    UG2FxCrtSseSubtractScalar(&value2,&value4);
    LowMove(value4,Table(0x007d7bf8,2));
    And(value3,value1);
    And(value4,value6);
    UG2FxCrtSseSubtractScalar(&value0,&value2);
    Xor(value2,value2);
    bound=0x4060;
    SetWord(value2,bound,0x3);
    LowMove(value5,value3);
    UG2FxCrtSseMultiplyScalar(&value3,&value4);
    UG2FxCrtSseSubtractScalar(&value6,&value4);
    UG2FxCrtSseSubtractScalar(&value1,&value5);
    UG2FxCrtSseMultiplyScalar(&value3,&value2);
    UG2FxCrtSseMultiplyScalar(&value5,&value6);
    value2=Table(0x007d7370);
    UG2FxCrtSseMultiplyScalar(&value4,&value1);
    index=UG2FxCrtSseConvertInteger(&value3);
    UG2FxCrtSseMultiplyScalar(&value6,&value1);
    value1=Table(0x007d7380);
    UG2FxCrtSseAddScalar(&value5,&value4);
    Shuffle(value4,value6,0xee);
    UG2FxCrtSseAddScalar(&value5,&value6);
    bound=0x1ff7f;
    bound-=index;
    index+=0x1e1ff;
    bound|=index;
    index-=0x1e1ff;
    compareLeft=bound;compareRight=0x0;
    if (Signed(compareLeft)<=Signed(compareRight)) goto block_764f81;
    signScale+=index;
    index&=0x7f;
    signScale&=0xffffff80;
    signScale+=0x1ff80;
    UG2FxCrtSseAddScalar(&value4,&value0);
    LowMove(value0,value3);
    source=Table(0x007d7b90,2);UG2FxCrtSseAddScalar(&value3,&source);
    UG2FxCrtSseMultiplyPacked(&value2,&value7);
    source=Table(0x007d7b90,2);UG2FxCrtSseSubtractScalar(&value3,&source);
    UG2FxCrtSseMultiplyPacked(&value7,&value7);
    UG2FxCrtSseSubtractScalar(&value0,&value3);
    index+=index;
    index+=index;
    index+=index;
    index+=index;
    value3=Table(index + 0x7d7390);
    value6=Table(0x007d7bd0);
    UG2FxCrtSseMultiplyPacked(&value1,&value7);
    UG2FxCrtSseAddPacked(&value2,&value1);
    Shuffle(value1,value2,0xee);
    UG2FxCrtSseMultiplyScalar(&value2,&value7);
    Xor(value7,value7);
    bound=0x3f80;
    UG2FxCrtSseAddScalar(&value2,&value1);
    SetWord(value7,bound,0x3);
    UG2FxCrtSseAddScalar(&value2,&value4);
    LowMove(value4,Table(0x007d7bf0,2));
    SetDword(value1,signScale);
    source=Vector(second);UG2FxCrtSseMultiplyScalar(&value2,&source);
    UG2FxCrtSseMultiplyScalar(&value0,&value7);
    ShiftLeft(value1,0x2d);
    Shuffle(value1,value1,0x44);
    value7=Table(0x007d7be0);
    UG2FxCrtSseAddScalar(&value5,&value2);
    UG2FxCrtSseMultiplyPacked(&value3,&value1);
    UG2FxCrtSseAddScalar(&value0,&value5);
    UnpackLow(value0,value0);
    UG2FxCrtSseMultiplyPacked(&value6,&value0);
    UG2FxCrtSseMultiplyScalar(&value4,&value0);
    UG2FxCrtSseMultiplyPacked(&value0,&value0);
    UG2FxCrtSseAddPacked(&value7,&value6);
    UG2FxCrtSseMultiplyPacked(&value7,&value0);
    UG2FxCrtSseMultiplyScalar(&value0,&value3);
    Shuffle(value6,value7,0xee);
    UG2FxCrtSseMultiplyScalar(&value0,&value7);
    Shuffle(value5,value3,0xee);
    UG2FxCrtSseMultiplyScalar(&value6,&value3);
    UG2FxCrtSseMultiplyScalar(&value4,&value3);
    UG2FxCrtSseAddScalar(&value0,&value5);
    UG2FxCrtSseAddScalar(&value0,&value6);
    UG2FxCrtSseAddScalar(&value0,&value4);
    UG2FxCrtSseAddScalar(&value0,&value3);
    result=Low(value0);
    CopyDouble(*out,result);return;
block_764a97: // Signed/zero/subnormal/nonfinite operand classification and parity.
    LowMove(value1,Vector(second));
    LowMove(value3,Table(0x007d7bc0,2));
    index=value1.word[0];
    LowMove(value2,value1);
    And(value1,value3);
    ShiftRight(value1,0x20);
    signScale=value1.word[0];
    compareLeft=signScale;compareRight=0x7ff00000;
    if (compareLeft>=compareRight) goto block_764ba0;
    index|=signScale;
    compareLeft=index;compareRight=0x0;
    if (compareLeft==compareRight) goto block_764ee4;
    compareLeft=bound;compareRight=0x0;
    if (Signed(compareLeft)>=Signed(compareRight)) goto block_764bdf;
    bound=0u-bound;
    bound+=0x7fef;
    ShiftLeft(value3,0x34);
    Or(value2,value3);
    signScale=0x3f3;
    SetDword(value3,signScale);
    ShiftRight(value1,0x14);
    SubtractDwords(value1,value3);
    Xor(value3,value3);
    MaximumWords(value1,value3);
    ShiftLeft(value2,Quad(value1,0));
    EqualDwords(value2,value3);
    index=ByteMask(value2);
    signScale=bound;
    bound&=0x7fff;
    compareLeft=bound;compareRight=0x7ff0;
    if (compareLeft>=compareRight) goto block_764cc8;
    index&=0xff;
    compareLeft=index;compareRight=0xff;
    if (compareLeft!=compareRight) goto block_764dc1;
    LowMove(value1,Vector(second));
    LowMove(value2,Vector(second));
    signScale=0x3f4;
    SetDword(value3,signScale);
    And(value1,Table(0x007d7bc0));
    ShiftRight(value1,0x34);
    SubtractDwords(value1,value3);
    LowMove(value3,Table(0x007d7c20,2));
    ShiftLeft(value2,Quad(value1,0));
    EqualDwords(value2,value3);
    index=ByteMask(value2);
    index&=0xff;
    signScale=0x3ff01;
    signScale+=index;
    signScale&=0x40000;
    compareLeft=bound;compareRight=0x10;
    if (compareLeft<compareRight) goto block_764be4;
    bound=0xbfe7f;
    LowMove(value3,Table(0x007d7ba0,2));
    LowMove(value2,Table(0x007d7bb0,2));
    goto block_7647c6;
block_764ba0:
    LowMove(value7,Vector(first));
    LowMove(value4,Vector(first));
    bound=value7.word[0];
    ShiftRight(value7,0x20);
    index=value7.word[0];
    signScale=index;
    index&=0x7fffffff;
    compareLeft=index;compareRight=0x7ff00000;
    if (compareLeft<compareRight) goto block_764e5c;
    if (compareLeft>compareRight) goto block_764daf;
    compareLeft=bound;compareRight=0x0;
    if (compareLeft>compareRight) goto block_764daf;
    goto block_764e5c;
block_764bdf:
    signScale=0x0;
block_764be4: // Normalize a subnormal base by 2^64, retaining the original flag effects.
    Xor(value0,value0);
    index=0x43f0;
    SetWord(value0,index,0x3);
    LowMove(value7,Table(0x007d7ba0,2));
    LowMove(value2,Table(0x007d7bb0,2));
    UG2FxCrtSseMultiplyScalar(&value0,&value4);
    bound=value4.word[0];
    ShiftRight(value4,0x20);
    index=value4.word[0];
    compareLeft=bound;compareRight=0x0;
    if (compareLeft==compareRight) goto block_764c6a;
block_764c18:
    And(value7,value0);
    LowMove(value4,value0);
    And(value0,Table(0x007d7bc0));
    ShiftRight(value0,0x2c);
    index=Word(value0,0x0);
    Or(value7,value2);
    index&=0xff;
    index+=0x1;
    index&=0x1fe;
    source=Table(index*0x4 + 0x7d4310,2);UG2FxCrtSseMultiplyScalar(&value7,&source);
    LowMove(value5,Table(index*0x4 + 0x7d4310,2));
    index+=index;
    value6=Table(index*0x4 + 0x7d4720);
    bound=0x43e7f;
    goto block_7647c6;
block_764c6a:
    bound=index;
    bound&=0x7fffffff;
    compareLeft=bound;compareRight=0x0;
    if (compareLeft!=compareRight) goto block_764c18;
    bound=second.high;
    bound&=0x80000000;
    compareLeft=bound;compareRight=0x0;
    if (compareLeft==compareRight) goto block_764cb7;
    signScale<<=0xd;
    index&=signScale;
    bound=0x7ff00000;
    bound|=index;
    SetDword(value0,bound);
    ShiftLeft(value0,0x20);
    LowMove(value1,Table(0x007d7bb0,2));
    LowMove(value2,Vector(first));
    UG2FxCrtSseDivideScalar(&value1,&value2);
    bound=0x1b;
    goto range_error;
block_764cb7:
    signScale<<=0xd;
    index&=signScale;
    compareLeft=index;compareRight=0x0;
    if (compareLeft!=compareRight) goto block_764d69;
    result.low=0;result.high=0;
    CopyDouble(*out,result);return;
block_764cc8:
    LowMove(value3,Table(0x007d7ba0,2));
    Xor(value1,value1);
    And(value3,value4);
    EqualDwords(value1,value3);
    signScale=ByteMask(value1);
    signScale&=0xff;
    compareLeft=signScale;compareRight=0xff;
    if (compareLeft!=compareRight) goto block_764daf;
    signScale=Word(value4,0x3);
    signScale&=0x8000;
    compareLeft=signScale;compareRight=0x0;
    if (compareLeft==compareRight) goto block_764d93;
    index&=0xff;
    compareLeft=index;compareRight=0xff;
    if (compareLeft!=compareRight) goto block_764d77;
    LowMove(value1,Vector(second));
    LowMove(value2,Vector(second));
    signScale=0x3f4;
    SetDword(value3,signScale);
    And(value1,Table(0x007d7bc0));
    ShiftRight(value1,0x34);
    SubtractDwords(value1,value3);
    Xor(value3,value3);
    ShiftLeft(value2,Quad(value1,0));
    EqualDwords(value2,value3);
    index=ByteMask(value2);
    index&=0xff;
    compareLeft=index;compareRight=0xff;
    if (compareLeft==compareRight) goto block_764d77;
    LowMove(value1,Vector(second));
    index=Word(value1,0x3);
    index&=0x8000;
    compareLeft=index;compareRight=0x0;
    if (compareLeft==compareRight) goto block_764d70;
block_764d69:
    result=Low(Table(0x007d7c20));
    CopyDouble(*out,result);return;
block_764d70:
    result=Low(Table(0x007d7c18));
    CopyDouble(*out,result);return;
block_764d77:
    LowMove(value1,Vector(second));
    index=Word(value1,0x3);
    index&=0x8000;
    compareLeft=index;compareRight=0x0;
    if (compareLeft==compareRight) goto block_764ece;
    result.low=0;result.high=0;
    CopyDouble(*out,result);return;
block_764d93:
    LowMove(value1,Vector(second));
    index=Word(value1,0x3);
    index&=0x8000;
    compareLeft=index;compareRight=0x0;
    if (compareLeft==compareRight) goto block_764ece;
    result.low=0;result.high=0;
    CopyDouble(*out,result);return;
block_764daf:
    UG2FxCrtSseAddScalar(&value4,&value4);
    LowMove(value0,value4);
    bound=0x3ee;
    goto range_error;
block_764dc1:
    LowMove(value2,Vector(first));
    index=value2.word[0];
    ShiftRight(value2,0x20);
    bound=value2.word[0];
    bound&=0x7fffffff;
    index|=bound;
    signScale=0x0;
    compareLeft=index;compareRight=0x0;
    if (compareLeft==compareRight) goto block_764c6a;
    LowMove(value1,Table(0x007d7c08,2));
    LowMove(value0,Table(0x007d7c30,2));
    UG2FxCrtSseMultiplyScalar(&value1,&value1);
    bound=0x1c;
    goto range_error;
block_764e08:
    LowMove(value4,Vector(first));
    LowMove(value2,Vector(second));
    index=value4.word[0];
    compareLeft=index;compareRight=0x0;
    if (compareLeft!=compareRight) goto block_764e5c;
    ShiftRight(value4,0x20);
    bound=value4.word[0];
    compareLeft=bound;compareRight=0x3ff00000;
    if (compareLeft==compareRight) goto block_764f3a;
    compareLeft=bound;compareRight=0xbff00000;
    if (compareLeft!=compareRight) goto block_764e5c;
    LowMove(value3,Table(0x007d7ba0,2));
    Xor(value1,value1);
    And(value3,value2);
    EqualDwords(value1,value3);
    index=ByteMask(value1);
    compareLeft=index;compareRight=0xff;
    if (compareLeft!=compareRight) goto block_764ed5;
    result.low=0;result.high=0x3ff00000;
    CopyDouble(*out,result);return;
block_764e5c:
    LowMove(value3,Table(0x007d7ba0,2));
    Xor(value1,value1);
    And(value3,value2);
    EqualDwords(value1,value3);
    index=ByteMask(value1);
    index&=0xff;
    compareLeft=index;compareRight=0xff;
    if (compareLeft!=compareRight) goto block_764ed5;
    index=Word(value2,0x3);
    LowMove(value4,Vector(first));
    index&=0x8000;
    signScale^=0xbff00000;
    bound|=signScale;
    compareLeft=bound;compareRight=0x0;
    if (compareLeft==compareRight) goto block_764f3a;
    compareLeft=index;compareRight=0x0;
    if (compareLeft==compareRight) goto block_764eba;
    index=Word(value4,0x3);
    index&=0x7ff0;
    compareLeft=index;compareRight=0x3ff0;
    if (compareLeft<compareRight) goto block_764ece;
    result.low=0;result.high=0;
    CopyDouble(*out,result);return;
block_764eba:
    index=Word(value4,0x3);
    index&=0x7ff0;
    compareLeft=index;compareRight=0x3ff0;
    if (compareLeft>=compareRight) goto block_764ece;
    result.low=0;result.high=0;
    CopyDouble(*out,result);return;
block_764ece:
    result=Low(Table(0x007d7c10));
    CopyDouble(*out,result);return;
block_764ed5:
    UG2FxCrtSseAddScalar(&value2,&value2);
    LowMove(value0,value2);
    bound=0x3ee;
    goto range_error;
block_764ee4:
    index=value4.word[0];
    ShiftRight(value4,0x20);
    bound=value4.word[0];
    bound&=0x7fffffff;
    signScale=index;
    index|=bound;
    LowMove(value0,Table(0x007d7bb0,2));
    bound=0x1a;
    compareLeft=index;compareRight=0x0;
    if (compareLeft==compareRight) goto range_error;
    index=value4.word[0];
    bound=0x1d;
    index&=0x7fffffff;
    compareLeft=index;compareRight=0x7ff00000;
    if (compareLeft>compareRight) goto range_error;
    if (compareLeft<compareRight) goto block_764f29;
    compareLeft=signScale;compareRight=0x0;
    if (compareLeft>compareRight) goto range_error;
block_764f29:
    result=Low(value0);
    CopyDouble(*out,result);return;
block_764f3a:
    LowMove(value1,Table(0x007d7c30,2));
    LowMove(value0,Vector(second));
    UG2FxCrtSseAddScalar(&value0,&value1);
    bound=0x1c;
range_error: // Original 0x764f51: one terminal call, then FLD result.
    result=Low(value0);
    { double output;CopyDouble(output,result);D3DXCrtSseMathError(&Number(first),&Number(second),&output,static_cast<int>(bound));CopyBits(result,output); }
    CopyDouble(*out,result);return;
block_764f81: // Large result: split scaling and masked mantissa reconstruction.
    compareLeft=index;compareRight=0x0;
    if (Signed(compareLeft)<=Signed(compareRight)) goto block_764fae;
    compareLeft=index;compareRight=0x40000;
    if (compareLeft>=compareRight) goto block_7651cd;
    bound=index;
    index&=0x7f;
    signScale+=0x3ff00;
    bound-=0x80;
    bound&=0xffffff80;
    extraExponent=0x3ff0;
    goto block_764fd4;
block_764fae:
    compareLeft=index;compareRight=0xfffc0200;
    if (Signed(compareLeft)<=Signed(compareRight)) goto block_7651aa;
    bound=index;
    index&=0x7f;
    signScale+=0x80;
    bound&=0xffffff80;
    bound+=0x3fe80;
    extraExponent=0x0;
block_764fd4:
    UG2FxCrtSseAddScalar(&value4,&value0);
    LowMove(value0,value3);
    source=Table(0x007d7b90,2);UG2FxCrtSseAddScalar(&value3,&source);
    splitExponent=bound;
    splitExponent-=0x1ff80;
    UG2FxCrtSseMultiplyPacked(&value2,&value7);
    source=Table(0x007d7b90,2);UG2FxCrtSseSubtractScalar(&value3,&source);
    UG2FxCrtSseMultiplyPacked(&value7,&value7);
    UG2FxCrtSseSubtractScalar(&value0,&value3);
    index+=index;
    index+=index;
    index+=index;
    index+=index;
    value3=Table(index + 0x7d7390);
    value6=Table(0x007d7bd0);
    UG2FxCrtSseMultiplyPacked(&value1,&value7);
    UG2FxCrtSseAddPacked(&value2,&value1);
    Shuffle(value1,value2,0xee);
    UG2FxCrtSseMultiplyScalar(&value2,&value7);
    UG2FxCrtSseAddScalar(&value2,&value1);
    UG2FxCrtSseAddScalar(&value2,&value4);
    LowMove(value4,Table(0x007d7bf0,2));
    SetDword(value1,bound);
    bound-=0x1ff80;
    bound=0u-bound;
    bound=ArithmeticRight(bound,0x7);
    bound+=0x2;
    index=bound;
    index&=0x20;
    bound+=index;
    Xor(value7,value7);
    index=0x3f80;
    SetWord(value7,index,0x3);
    source=Vector(second);UG2FxCrtSseMultiplyScalar(&value2,&source);
    UG2FxCrtSseMultiplyScalar(&value0,&value7);
    ShiftLeft(value1,0x2d);
    Shuffle(value1,value1,0x44);
    value7=Table(0x007d7be0);
    UG2FxCrtSseAddScalar(&value5,&value2);
    UG2FxCrtSseMultiplyPacked(&value3,&value1);
    UG2FxCrtSseAddScalar(&value0,&value5);
    UnpackLow(value0,value0);
    UG2FxCrtSseMultiplyPacked(&value6,&value0);
    UG2FxCrtSseMultiplyScalar(&value4,&value0);
    UG2FxCrtSseMultiplyPacked(&value0,&value0);
    UG2FxCrtSseAddPacked(&value7,&value6);
    UG2FxCrtSseMultiplyPacked(&value7,&value0);
    UG2FxCrtSseMultiplyScalar(&value0,&value3);
    Shuffle(value6,value7,0xee);
    UG2FxCrtSseMultiplyScalar(&value0,&value7);
    Shuffle(value5,value3,0xee);
    UG2FxCrtSseMultiplyScalar(&value6,&value3);
    UG2FxCrtSseMultiplyScalar(&value4,&value3);
    SetDword(value7,signScale);
    ShiftLeft(value7,0x2d);
    SetDword(value2,bound);
    EqualDwords(value1,value1);
    ShiftLeft(value1,Quad(value2,0));
    UG2FxCrtSseAddScalar(&value0,&value5);
    UG2FxCrtSseAddScalar(&value0,&value6);
    And(value1,value3);
    UG2FxCrtSseAddScalar(&value0,&value4);
    Xor(value6,value6);
    EqualDwords(value4,value4);
    ShiftLeft(value4,Quad(value2,0));
    UG2FxCrtSseSubtractScalar(&value3,&value1);
    LowMove(value2,value1);
    UG2FxCrtSseAddScalar(&value1,&value0);
    And(value1,value4);
    SetWord(value6,extraExponent,0x3);
    UG2FxCrtSseSubtractScalar(&value2,&value1);
    UG2FxCrtSseAddScalar(&value0,&value2);
    UG2FxCrtSseAddScalar(&value0,&value3);
    compareLeft=splitExponent;compareRight=0x0;
    if (Signed(compareLeft)>Signed(compareRight)) goto block_765160;
    UG2FxCrtSseMultiplyScalar(&value0,&value7);
    UG2FxCrtSseMultiplyScalar(&value1,&value7);
    UG2FxCrtSseAddScalar(&value0,&value1);
    UG2FxCrtSseMultiplyScalar(&value6,&value0);
    UG2FxCrtSseAddScalar(&value0,&value6);
    index=Word(value0,0x3);
    index&=0x7ff0;
    bound=0x18;
    compareLeft=index;compareRight=0x7ff0;
    if (compareLeft==compareRight) goto range_error;
    bound=0x19;
    compareLeft=index;compareRight=0x0;
    if (compareLeft==compareRight) goto range_error;
    result=Low(value0);
    CopyDouble(*out,result);return;
block_765160:
    UG2FxCrtSseAddScalar(&value0,&value1);
    UG2FxCrtSseMultiplyScalar(&value0,&value7);
    UG2FxCrtSseMultiplyScalar(&value6,&value0);
    UG2FxCrtSseAddScalar(&value0,&value6);
    index=Word(value0,0x3);
    index&=0x7ff0;
    bound=0x18;
    compareLeft=index;compareRight=0x7ff0;
    if (compareLeft==compareRight) goto range_error;
    bound=0x19;
    compareLeft=index;compareRight=0x0;
    if (compareLeft==compareRight) goto range_error;
    result=Low(value0);
    CopyDouble(*out,result);return;
block_7651aa:
    LowMove(value0,Table(0x007d7c28,2));
    SetDword(value1,signScale);
    UG2FxCrtSseMultiplyScalar(&value0,&value0);
    ShiftLeft(value1,0x2d);
    Or(value0,value1);
    bound=0x19;
    goto range_error;
block_7651cd:
    bound=0x18;
    compareLeft=signScale;compareRight=0x0;
    if (compareLeft==compareRight) goto block_7651e4;
    LowMove(value0,Table(0x007d7c18,2));
    goto range_error;
block_7651e4:
    LowMove(value0,Table(0x007d7c10,2));
    goto range_error;
block_7651f1: // Extreme product shortcut; preserve the packed multiply before testing.
    Shuffle(value4,value1,0x44);
    UG2FxCrtSseMultiplyPacked(&value4,&value6);
    index=Word(value4,0x3);
    index&=0x7ff0;
    bound=0x40a0;
    bound-=index;
    index-=0x3c70;
    bound|=index;
    compareLeft=bound;compareRight=0x80000000;
    if (compareLeft<compareRight) goto block_764918;
    compareLeft=index;compareRight=0x80000000;
    if (compareLeft<compareRight) goto block_765245;
    signScale|=0x1ff80;
    SetDword(value0,signScale);
    ShiftLeft(value0,0x2d);
    result=Low(value0);
    CopyDouble(*out,result);return;
block_765245:
    LowMove(value4,Vector(first));
    bound=Word(value4,0x3);
    bound&=0x7ff0;
    bound-=0x3ff0;
    index=Word(value1,0x3);
    index^=bound;
    index&=0x8000;
    compareLeft=index;compareRight=0x0;
    if (compareLeft!=compareRight) goto block_7651aa;
    goto block_7651cd;
}

extern "C" void __cdecl D3DXCrtPowerSse80(D3DXCrtExtended *out,D3DXCrtDoubleBits first,D3DXCrtDoubleBits second) {
    double result;D3DXCrtPowerSseResult(&result,first,second);UG2FxCrtExtendedFromDouble(out,&result);
}
