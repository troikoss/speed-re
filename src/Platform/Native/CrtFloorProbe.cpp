#include "OriginalImage.h"
#include "../../Speed/Indep/Libs/D3DX/CrtFloor.h"
#include <emmintrin.h>
#include <stdio.h>
#include <string.h>
// Test-only raw CDECL argument view: preserve SNaN bits before callee entry.
extern "C" long double __cdecl ProbeFloorBits(D3DXCrtDoubleBits) __asm__("D3DXCrtFloor");
namespace {
struct State { unsigned short words[14];unsigned int mxcsr; };
State Snapshot() { State s;asm volatile("fnstenv %0; stmxcsr %1" : "=m"(s.words),"=m"(s.mxcsr));return s; }
void Reset(unsigned short control,unsigned int mxcsr) { asm volatile("fninit; fldcw %0; ldmxcsr %1" : : "m"(control),"m"(mxcsr) : "st"); }
bool Same(const State &a,const State &b) { return a.words[0]==b.words[0]&&(a.words[2]&0x383f)==(b.words[2]&0x383f)&&a.words[4]==b.words[4]&&a.mxcsr==b.mxcsr; }
long double LoadBits(unsigned long long bits) { long double value;asm volatile("fldl %1; fstpt %0" : "=m"(value) : "m"(bits) : "st");return value; }
// Independent SSE instruction oracle: retain the original packed shift/count
// arithmetic and signaling packed comparisons, including the otherwise-dead
// comparison before the negative large-exponent gate.
long double SseOracle(unsigned long long bits,int &error) {
    __m128i raw=_mm_loadl_epi64(reinterpret_cast<const __m128i *>(&bits));
    __m128i exponent=_mm_srli_epi64(raw,52);
    __m128i shift=_mm_sub_epi32(_mm_load_si128(reinterpret_cast<const __m128i *>(ug2CrtFloorShift7D3CA0)),_mm_and_si128(exponent,_mm_load_si128(reinterpret_cast<const __m128i *>(ug2CrtFloorExponentMask7D3CD0))));
    __m128i chopped=_mm_sll_epi64(_mm_srl_epi64(raw,shift),shift);unsigned int e=_mm_cvtsi128_si32(exponent);
    if (!(e&0x800)) {
        if (e<0x3ff) return 0.0;
        if (e<=0x432) { unsigned long long out;_mm_storel_epi64(reinterpret_cast<__m128i *>(&out),chopped);return LoadBits(out); }
    } else {
        __m128d less=_mm_cmplt_pd(_mm_castsi128_pd(raw),_mm_castsi128_pd(chopped));
        // Force the comparison even where its value is not subsequently used.
        asm volatile("" : "+x"(less));
        if (e<0xbff) {
            less=_mm_cmplt_pd(_mm_castsi128_pd(raw),_mm_load_pd(&ug2CrtFloorNegativeZero7D3CC0));
            __m128d selected=_mm_and_pd(_mm_or_pd(less,_mm_load_pd(&ug2CrtFloorNegativeZero7D3CC0)),_mm_load_pd(&ug2CrtFloorNegativeOne7D3CB0));
            unsigned long long out;_mm_storel_epi64(reinterpret_cast<__m128i *>(&out),_mm_castpd_si128(selected));return LoadBits(out);
        }
        if (e<=0xc32) {
            __m128d selected=_mm_sub_sd(_mm_castsi128_pd(chopped),_mm_and_pd(less,_mm_load_pd(&ug2CrtFloorOne7D3C90)));
            unsigned long long out;_mm_storel_epi64(reinterpret_cast<__m128i *>(&out),_mm_castpd_si128(selected));return LoadBits(out);
        }
    }
    unsigned char unordered;__m128d value=_mm_castsi128_pd(raw);
    asm volatile("ucomisd %1,%1; setp %0" : "=qm"(unordered) : "x"(value) : "cc");
    if (unordered) { unsigned long long quiet;asm volatile("fldl %1; fstpl %0" : "=m"(quiet) : "m"(bits) : "st");bits=quiet;error=0x21; }
    return LoadBits(bits);
}
long double Oracle(unsigned long long bits,unsigned int enabled,int &error) {
    unsigned int mxcsr;unsigned short saved;asm volatile("stmxcsr %0; fnstcw %1" : "=m"(mxcsr),"=m"(saved));
    if (enabled&&(mxcsr&0x1f80)==0x1f80&&(saved&0x7f)==0x7f) return SseOracle(bits,error);
    unsigned short internal=ug2CrtFloorControl81AA60;unsigned long long prepared;
    asm volatile("fldcw %2; fldl %1; fstpl %0" : "=m"(prepared) : "m"(bits),"m"(internal) : "st");
    unsigned int high=static_cast<unsigned int>(bits>>32);
    if ((high&0x7ff00000)==0x7ff00000) {
        if ((bits&0xfffffffffffffull)!=0) { error=0x21;asm volatile("fldl %1; fstpl %0" : "=m"(prepared) : "m"(bits) : "st"); }
        asm volatile("fldcw %0" : : "m"(saved));return LoadBits(prepared);
    }
    unsigned long long result;
    asm volatile("fldl %1; frndint; fstpl %0; fldl %0; fcompl %2; fldcw %3" : "=m"(result) : "m"(prepared),"m"(bits),"m"(saved) : "st");return LoadBits(result);
}
unsigned int Controls() {
    for (unsigned int word=0;word<65536;++word) {
        Reset(word,0x1f80);unsigned short before;asm volatile("fstcw %0" : "=m"(before));
        unsigned int value=word*1664525u+1013904223u,mask=word%4==0 ? 0 : word%4==1 ? 0xffffffffu : word%4==2 ? word*0x10001u : 0xf3f;
        unsigned short request=0;
        for (unsigned int bit=0;bit<16;++bit) if (((mask>>bit)&1) ? ((value>>bit)&1) : ((before>>bit)&1)) request|=1u<<bit;
        unsigned short expected;asm volatile("fldcw %1; fstcw %0" : "=m"(expected) : "m"(request));
        Reset(word,0x1f80);int result=D3DXCrtRawControl(value,mask);unsigned short actual;asm volatile("fstcw %0" : "=m"(actual));
        if (result!=static_cast<short>(before)||actual!=expected) { fprintf(stderr,"hardware raw control mismatch: word %x return %x/%x control %x/%x\n",word,result,static_cast<int>(static_cast<short>(before)),actual,expected);return 0; }
    }return 65536;
}
bool Copies() {
    const unsigned int modes[]={0x1f80,0x1fc0,0x9f80,0x9fc0};
    const unsigned long long inputs[]={0x800fffffffffffffull,0x7ff0000000001234ull};
    for (unsigned int mode=0;mode<4;++mode) for (unsigned int value=0;value<2;++value) {
        unsigned long long result;Reset(0x37f,modes[mode]);asm volatile("fldl %1; fstpl %0" : "=m"(result) : "m"(inputs[value]) : "st");State s=Snapshot();
        unsigned long long expected=value ? inputs[value]|0x8000000000000ull : inputs[value];
        if (result!=expected||(s.words[2]&0x3f)!=(value ? 1 : 2)||s.mxcsr!=modes[mode]) { fprintf(stderr,"hardware x87 copy mismatch: mode %u value %u\n",mode,value);return false; }
    }return true;
}
unsigned int Payloads() {
    const unsigned long long values[]={0x7ff0000000001234ull,0xfff0000000001234ull,0x800fffffffffffffull,1,0x7ff8000000004321ull,0xfff8000000004321ull,0,0x8000000000000000ull};unsigned int count=0;
    for (unsigned int mode=0;mode<12;++mode) for (unsigned int input=0;input<8;++input) for (unsigned int named=0;named<2;++named) {
        unsigned short before=(mode%3==0 ? 0x7f : mode%3==1 ? 0x27f : 0x37f)|((mode/3)<<10),target=(mode%3==0 ? 0x27f : mode%3==1 ? 0x37f : 0x7f)|(((mode/3+1)%4)<<10);
        unsigned int mxcsr=0x1f80|((input%4&1) ? 0x40 : 0)|((input%4&2) ? 0x8000 : 0);
        D3DXCrtDoubleBits a={static_cast<unsigned int>(values[input]),static_cast<unsigned int>(values[input]>>32)},b={static_cast<unsigned int>(values[(input+3)%8]),static_cast<unsigned int>(values[(input+3)%8]>>32)},result={0,0x3ff40000};
        Reset(target,mxcsr);volatile long double reference=LoadBits(0x3ff4000000000000ull);State expected=Snapshot();
        *D3DXCrtErrno()=0x12345678;Reset(before,mxcsr);volatile long double actual=D3DXCrtUserMathError(input%4,named ? 11 : 0xffffffff,a,b,result,target);State state=Snapshot();
        int error=input%4==1 ? 0x21 : input%4>=2 ? 0x22 : 0x12345678;
        if (memcmp(const_cast<const long double *>(&reference),const_cast<const long double *>(&actual),10)||!Same(expected,state)||*D3DXCrtErrno()!=error) { fprintf(stderr,"hardware matherr payload mismatch: mode %u input %u named %u flags %x/%x\n",mode,input,named,expected.words[2],state.words[2]);return 0; }++count;
    }return count;
}
bool Fraction(unsigned long long bits,unsigned int width,unsigned int row,unsigned short control,unsigned int mxcsr,unsigned int enabled,unsigned int &checks) {
    unsigned char expected[96],actual[96];memset(expected,0xa5,96);for (unsigned int i=0;i<3;++i) memcpy(expected+16+i*8,&bits,8);memcpy(actual,expected,96);
    const int offsets[]={-4,0,4,8,32};unsigned int offset=16+offsets[row%5];int error=0x12345678;
    Reset(control,mxcsr);
    for (unsigned int lane=0;lane<width;++lane) {
        unsigned long long original,argument,result;memcpy(&original,expected+16+lane*8,8);
        asm volatile("fldl %1; fstpl %0" : "=m"(argument) : "m"(original) : "st");
        long double floor=Oracle(argument,enabled,error);
        asm volatile("fldt %1; fsubrl %2; fstpl %0" : "=m"(result) : "m"(floor),"m"(original) : "st");memcpy(expected+offset+lane*8,&result,8);
    }
    State reference=Snapshot();*D3DXCrtErrno()=0x12345678;Reset(control,mxcsr);
    unsigned int code[]={1,0x10400000|width,1,0,3,0,0,4,0},literal[]={1,0,0};
    int result=UG2FxExecuteVm(code,literal,reinterpret_cast<unsigned int>(actual+16),0,0,reinterpret_cast<unsigned int>(actual+offset),0,0,0,0,0,0,0,0);State state=Snapshot();
    if (result||memcmp(expected,actual,96)||!Same(reference,state)||*D3DXCrtErrno()!=error) { fprintf(stderr,"hardware floor fraction mismatch: row %u width %u flags %x/%x MXCSR %x/%x errno %x/%x\n",row,width,reference.words[2],state.words[2],reference.mxcsr,state.mxcsr,error,*D3DXCrtErrno());return false; }
    ++checks;return true;
}
}
int main(int argc,char **argv) {
    if (argc!=4) { fprintf(stderr,"usage: ug2crtfloor_probe speed2.exe fixtures.bin capture.bin\n");return 2; }char error[512];if (!MapOriginalImage(argv[1],error,sizeof(error))) { fprintf(stderr,"%s\n",error);return 1; }
    FILE *input=fopen(argv[2],"rb"),*output=fopen(argv[3],"wb");if (!input||!output) return 3;char magic[8];unsigned int count;
    if (fread(magic,1,8,input)!=8||memcmp(magic,"UG2FXFI1",8)||fread(&count,4,1,input)!=1||count>100000||fwrite("UG2FXFD1",1,8,output)!=8||fwrite(&count,4,1,output)!=1) return 3;
    unsigned short saved;unsigned int savedMxcsr;asm volatile("fnstcw %0; stmxcsr %1" : "=m"(saved),"=m"(savedMxcsr));unsigned int controls=Controls(),fractions=0,payloads=Payloads();if (!controls||!Copies()||!payloads) return 1;
    for (unsigned int i=0;i<count;++i) {
        unsigned int config[7];if (fread(config,4,7,input)!=7||config[0]!=i) return 3;
        unsigned long long bits=static_cast<unsigned long long>(config[6])<<32|config[5];ug2CrtSseEnabled8BA480=config[3];ug2CrtMathErrorFlag81ACE0=config[4];ug2CrtMathErrorCallback81AA5C=0x727e6b;
        int expectedErrno=0x12345678;Reset(config[1],config[2]);volatile long double reference=Oracle(bits,config[3],expectedErrno);State expected=Snapshot();
        *D3DXCrtErrno()=0x12345678;D3DXCrtDoubleBits argument={config[5],config[6]};Reset(config[1],config[2]);volatile long double actual=ProbeFloorBits(argument);State state=Snapshot();
        if (memcmp(const_cast<const long double *>(&reference),const_cast<const long double *>(&actual),10)||!Same(expected,state)||*D3DXCrtErrno()!=expectedErrno||state.words[4]!=0xffff) { fprintf(stderr,"hardware floor mismatch: row %u flags %x/%x MXCSR %x/%x errno %x/%x\n",i,expected.words[2],state.words[2],expected.mxcsr,state.mxcsr,expectedErrno,*D3DXCrtErrno());return 1; }
        unsigned int header[]={i,state.words[0],state.mxcsr,(state.words[2]>>11)&7,state.words[4],static_cast<unsigned int>(*D3DXCrtErrno())};
        if (fwrite(header,4,6,output)!=6||fwrite(const_cast<const long double *>(&actual),1,10,output)!=10) return 3;
        if (i<256&&!Fraction(bits,i%4,i,config[1],config[2],config[3],fractions)) return 1;
    }
    Reset(saved,savedMxcsr);if (fgetc(input)!=EOF||fclose(input)||fclose(output)) return 3;printf("CRT floor: %u hardware returns/flags, %u raw-control updates, %u native VM fraction graphs, 8 x87 copy audits, %u matherr payload checks\n",count,controls,fractions,payloads);return 0;
}
