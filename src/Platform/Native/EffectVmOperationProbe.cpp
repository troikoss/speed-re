#include "OriginalImage.h"
#include "../../Speed/Indep/Libs/D3DX/EffectVm.h"
#include <stdio.h>
#include <string.h>
extern "C" {
extern const float ug2VmOperationOne784250,ug2VmOperationBias784500,ug2VmOperationThree787ADC,ug2VmOperationScale785110;
extern const unsigned int ug2VmOperationPermutation7A9460[514];
extern const float ug2VmOperationGradients7A9C68[514][3];
}
namespace {
struct FpState { unsigned short words[14]; };
FpState State() { FpState value;asm volatile("fnstenv %0" : "=m"(value));return value; }
void Reset(unsigned short word) { asm volatile("fninit; fldcw %0" : : "m"(word) : "st"); }
long double Sum(const float &input) {
    long double result;asm volatile("flds %1; fadds %2; fstpt %0" : "=m"(result) : "m"(input),"m"(ug2VmOperationBias784500) : "st");return result;
}
int Index(const long double &input) {
    unsigned short saved,truncation;asm volatile("fstcw %0" : "=m"(saved));truncation=saved|0xc00;
    long long integer;asm volatile("fldt %1; fldcw %2; fistpll %0; fldcw %3" : "=m"(integer) : "m"(input),"m"(truncation),"m"(saved) : "st");return static_cast<int>(integer);
}
long double Fraction(const long double &input,const int &integer) {
    long double result;asm volatile("fldt %1; fisubl %2; fstpt %0" : "=m"(result) : "m"(input),"m"(integer) : "st");return result;
}
long double Minus(const long double &input) {
    long double result;asm volatile("fldt %1; fsubs %2; fstpt %0" : "=m"(result) : "m"(input),"m"(ug2VmOperationOne784250) : "st");return result;
}
void Narrow(float &result,const long double &input) {
    asm volatile("fldt %1; fstps %0" : "=m"(result) : "m"(input) : "st");
}
void Weight(float &result,const long double &input) {
    asm volatile("fldt %1; fld %%st(0); fadd %%st,%%st; fsubrs %2; fmul %%st(1),%%st; fmul %%st(1),%%st; fstps %0; fstp %%st(0)" : "=m"(result) : "m"(input),"m"(ug2VmOperationThree787ADC) : "st");
}
long double Blend(const long double &a,const long double &b,const float &weight) {
    long double result;asm volatile("fldt %1; fldt %2; fsub %%st(1),%%st; fmuls %3; fadd %%st(1),%%st; fstpt %0; fstp %%st(0)" : "=m"(result) : "m"(a),"m"(b),"m"(weight) : "st");return result;
}
// Independent instruction micro-oracles. Explicit float/extended loads retain
// each original operand type, including the final narrowed x-minus-one load.
#define DOT(to,x,y,z,g,loadX,loadY,loadZ) asm volatile(loadZ " %3; fmuls %6; " loadY " %2; fmuls %5; faddp; " loadX " %1; fmuls %4; faddp; fstpt %0" : "=m"(to) : "m"(x),"m"(y),"m"(z),"m"((g)[0]),"m"((g)[1]),"m"((g)[2]) : "st")
long double Oracle(const float *values) {
    long double sx=Sum(values[0]);int nx=Index(sx);long double x=Fraction(sx,nx),xm=Minus(x);float xmf;Narrow(xmf,xm);
    long double sy=Sum(values[1]);int ny=Index(sy);long double y=Fraction(sy,ny);float ym;Narrow(ym,Minus(y));
    long double sz=Sum(values[2]);int nz=Index(sz);long double z=Fraction(sz,nz);float zf,zm;Narrow(zf,z);Narrow(zm,Minus(z));
    const unsigned int *p=ug2VmOperationPermutation7A9460;unsigned int x0=static_cast<unsigned int>(nx)&255,y0=static_cast<unsigned int>(ny)&255,z0=static_cast<unsigned int>(nz)&255,z1=(z0+1)&255;
    unsigned int corners[4]={p[p[x0]+y0],p[p[(x0+1)&255]+y0],p[p[x0]+((y0+1)&255)],p[p[(x0+1)&255]+((y0+1)&255)]};
    float wx,wy;Weight(wx,x);Weight(wy,y);long double dots[8];const float (*g)[3]=ug2VmOperationGradients7A9C68;
    DOT(dots[0],x,y,z,g[corners[0]+z0],"fldt","fldt","fldt");
    DOT(dots[1],xm,y,z,g[corners[1]+z0],"fldt","fldt","fldt");
    long double row00=Blend(dots[0],dots[1],wx);
    DOT(dots[2],x,ym,z,g[corners[2]+z0],"fldt","flds","fldt");
    DOT(dots[3],xm,ym,z,g[corners[3]+z0],"fldt","flds","fldt");
    float plane0;Narrow(plane0,Blend(row00,Blend(dots[2],dots[3],wx),wy));
    DOT(dots[4],x,y,zm,g[corners[0]+z1],"fldt","fldt","flds");
    DOT(dots[5],xm,y,zm,g[corners[1]+z1],"fldt","fldt","flds");
    float row01;Narrow(row01,Blend(dots[4],dots[5],wx));
    DOT(dots[6],x,ym,zm,g[corners[2]+z1],"fldt","flds","flds");
    DOT(dots[7],xmf,ym,zm,g[corners[3]+z1],"flds","flds","flds");
    long double row11=Blend(dots[6],dots[7],wx),result;
    asm volatile("fldt %1; fsubs %2; fmuls %3; fadds %2; fsubs %4; flds %5; fadd %%st,%%st; fsubrs %6; fmuls %5; fmuls %5; fmulp; fadds %4; fmuls %7; fstpt %0" : "=m"(result) : "m"(row11),"m"(row01),"m"(wy),"m"(plane0),"m"(zf),"m"(ug2VmOperationThree787ADC),"m"(ug2VmOperationScale785110) : "st");return result;
}
#undef DOT
bool Composition(const unsigned int *bits,unsigned int row,unsigned short control,unsigned int &checks) {
    unsigned char initial[64];memset(initial,0xa5,sizeof(initial));float packet[3];
    for (unsigned int lane=0;lane<3;++lane) {
        float value;memcpy(&value,bits+lane,4);volatile double widened=value;
        memcpy(initial+16+lane*8,const_cast<const double *>(&widened),8);memcpy(packet+lane,&value,4);
    }
    const int offsets[]={-4,0,4,8,32};
    for (unsigned int count=0;count<5;++count) {
        float values[3];memcpy(values,packet,12);for (unsigned int lane=count;lane<3;++lane) memset(values+lane,0,4);
        unsigned char expected[64];memcpy(expected,initial,64);double *destination=reinterpret_cast<double *>(expected+16+offsets[row%5]);
        Reset(control);long double result=Oracle(values);asm volatile("fldt %1; fstpl %0" : "=m"(*destination) : "m"(result) : "st");FpState reference=State();
        for (unsigned int route=0;route<2;++route) {
            unsigned char actual[64];memcpy(actual,initial,64);double *input=reinterpret_cast<double *>(actual+16),*output=reinterpret_cast<double *>(actual+16+offsets[row%5]);
            Reset(control);int status=0;
            if (!route) D3DXEffectVmOperation502(count,&input,output);
            else {
                unsigned int code[]={1,0x50200000|count,1,0,3,0,0,4,0},literal[]={1,0,0};
                status=UG2FxExecuteVm(code,literal,reinterpret_cast<unsigned int>(input),0,0,reinterpret_cast<unsigned int>(output),0,0,0,0,0,0,0,0);
            }
            FpState state=State();
            if (status||memcmp(expected,actual,64)||reference.words[0]!=state.words[0]||(reference.words[2]&0x383f)!=(state.words[2]&0x383f)||reference.words[4]!=state.words[4]||state.words[4]!=0xffff) {
                fprintf(stderr,"hardware VM operation composition mismatch: row %u count %u route %u flags %x/%x\n",row,count,route,reference.words[2],state.words[2]);return false;
            }++checks;
        }
    }return true;
}
}
int main(int argc,char **argv) {
    if (argc!=4) { fprintf(stderr,"usage: ug2effectvmoperation_probe speed2.exe fixtures.bin capture.bin\n");return 2; }
    char error[512];if (!MapOriginalImage(argv[1],error,sizeof(error))) { fprintf(stderr,"%s\n",error);return 1; }
    FILE *input=fopen(argv[2],"rb"),*output=fopen(argv[3],"wb");if (!input||!output) return 3;
    char magic[8];unsigned int count;if (fread(magic,1,8,input)!=8||memcmp(magic,"UG2FXOI1",8)||fread(&count,4,1,input)!=1||count>100000||fwrite("UG2FXOD1",1,8,output)!=8||fwrite(&count,4,1,output)!=1) return 3;
    unsigned short saved;asm volatile("fnstcw %0" : "=m"(saved));unsigned int compositions=0;
    for (unsigned int i=0;i<count;++i) {
        unsigned int config[5];if (fread(config,4,5,input)!=5||config[0]!=i) return 3;float values[3];memcpy(values,config+2,12);
        Reset(config[1]);volatile long double reference=Oracle(values);FpState expected=State();
        Reset(config[1]);volatile long double actual=UG2FxVmOperation63F2B5(values);FpState state=State();
        if (memcmp(const_cast<const long double *>(&reference),const_cast<const long double *>(&actual),10)||expected.words[0]!=state.words[0]||(expected.words[2]&0x383f)!=(state.words[2]&0x383f)||expected.words[4]!=state.words[4]||state.words[4]!=0xffff) {
            fprintf(stderr,"hardware VM operation mismatch: row %u flags %x/%x tags %x/%x\n",i,expected.words[2],state.words[2],expected.words[4],state.words[4]);asm volatile("fninit; fldcw %0" : : "m"(saved));return 1;
        }
        unsigned int header[4]={i,state.words[0],(state.words[2]>>11)&7,state.words[4]};
        if (fwrite(header,4,4,output)!=4||fwrite(const_cast<const long double *>(&actual),1,10,output)!=10||fwrite(values,1,12,output)!=12) return 3;
        if (i<256&&!Composition(config+2,i,config[1],compositions)) { asm volatile("fninit; fldcw %0" : : "m"(saved));return 1; }
    }
    asm volatile("fninit; fldcw %0" : : "m"(saved));if (fgetc(input)!=EOF||fclose(input)||fclose(output)) return 3;
    printf("effect VM operation: %u exact native returns and hardware arithmetic/FP-state checks, %u native wrapper/dispatcher checks\n",count,compositions);return 0;
}
