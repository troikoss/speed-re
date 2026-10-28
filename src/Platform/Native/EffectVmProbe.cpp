#include "OriginalImage.h"
#include "../../Speed/Indep/Libs/D3DX/EffectVm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
namespace {
unsigned int Address(const void *p) { return reinterpret_cast<unsigned int>(p); }
double controlledResult;std::vector<unsigned char> operationInputs;
struct FpState { unsigned short words[14]; };
FpState State() { FpState value;asm volatile("fnstenv %0" : "=m"(value));return value; }
void Reset(unsigned short word) { asm volatile("fninit; fldcw %0" : : "m"(word) : "st"); }
void Add(std::vector<unsigned int> &words,unsigned int value) { words.push_back(value); }
std::vector<unsigned int> Program(unsigned int op,unsigned int width,bool broadcast,unsigned int inputCount) {
    std::vector<unsigned int> code;Add(code,1);Add(code,op|width|(broadcast ? 0x80000000u : 0));Add(code,inputCount);
    for (unsigned int i=0;i<inputCount;++i) { Add(code,0);Add(code,op==0x70e00000||i==0 ? 3 : op==0x10600000||i==1 ? 2 : 6);Add(code,op==0x70e00000||op==0x10600000 ? i : 0); }
    Add(code,0);Add(code,4);Add(code,0);return code;
}
// Each oracle below transcribes the original instruction micro-operation,
// independently of the recovered C++ dispatcher. No PE code is executed.
#define LANE(instructions) asm volatile("fldl %1; " instructions "; fstpl %0" : "=m"(*out) : "m"(*a),"m"(*b),"m"(*c) : "st")
#define SELECT(test,branch) asm volatile("fldl %1; fcompl %2; fnstsw %%ax; testb $" test ",%%ah; " branch " 1f; fldl %1; jmp 2f; 1: fldl %2; 2: fstpl %0" : "=m"(*out) : "m"(*a),"m"(*b) : "ax","cc","st")
#define BOOL(test,branch) asm volatile("fldl %1; fcompl %2; fnstsw %%ax; testb $" test ",%%ah; " branch " 1f; fld1; jmp 2f; 1: fldz; 2: fstpl %0" : "=m"(*out) : "m"(*a),"m"(*b) : "ax","cc","st")
void Lane(unsigned int op,double *a,double *b,double *c,double *out) {
    if (op==0x10100000) { LANE("fchs"); }
    else if (op==0x10300000) { asm volatile("fld1; fdivl %1; fstpl %0" : "=m"(*out) : "m"(*a) : "st"); }
    else if (op==0x10600000) { asm volatile("fldl %4; fcompl %2; fnstsw %%ax; testb $1,%%ah; jnz 1f; fldl %1; fldln2; fxch; fyl2x; fmull %3; jmp 2f; 1: fldz; 2: fstpl %0" : "=m"(*out) : "m"(*a),"m"(ug2VmZero786E60),"m"(ug2VmLogScale7BAA88),"m"(*b) : "ax","cc","st"); }
    else if (op==0x10700000) { asm volatile("fldl %1; fabs; fsqrt; fdivrl %2; fstpl %0" : "=m"(*out) : "m"(*a),"m"(ug2VmOne786E68) : "st"); }
    else if (op==0x10800000) { LANE("fsin"); }
    else if (op==0x10900000) { LANE("fcos"); }
    else if (op==0x10c00000) { LANE("fld1; fpatan"); }
    else if (op==0x20000000) { SELECT("5","jp"); }
    else if (op==0x20100000) { SELECT("65","jnz"); }
    else if (op==0x20200000) { BOOL("5","jp"); }
    else if (op==0x20300000) { BOOL("1","jnz"); }
    else if (op==0x20400000) { LANE("faddl %2"); }
    else if (op==0x20500000) { LANE("fmull %2"); }
    else if (op==0x20600000) { LANE("fldl %2; fpatan"); }
    else if (op==0x30000000) { asm volatile("fldl %1; fcompl %4; fnstsw %%ax; testb $1,%%ah; jnz 1f; fldl %2; jmp 2f; 1: fldl %3; 2: fstpl %0" : "=m"(*out) : "m"(*a),"m"(*b),"m"(*c),"m"(ug2VmZero786E60) : "ax","cc","st"); }
}
#undef LANE
#undef SELECT
#undef BOOL
void Oracle(unsigned int op,unsigned int width,bool broadcast,unsigned int count,double *a,double *b,double *c,double *out) {
    if (broadcast&&op!=0x20000000&&op!=0x20100000&&op!=0x20200000&&op!=0x20300000&&op!=0x20400000&&op!=0x20500000&&op!=0x20600000&&op!=0x50000000) return;
    if (op==0x10000000) { unsigned int *from=reinterpret_cast<unsigned int *>(a),*to=reinterpret_cast<unsigned int *>(out),words=width*2;asm volatile("rep movsl" : "+S"(from),"+D"(to),"+c"(words) : : "memory"); }
    else if (op==0x50000000||op==0x70e00000) {
        *out=0.0;unsigned int terms=op==0x50000000 ? width : count>>1;
        for (unsigned int i=0;i<terms;++i) { double *left=op==0x50000000 ? (broadcast ? b+i : a+i) : a+i,*right=op==0x50000000 ? (broadcast ? a : b+i) : a+(count>>1)+i;asm volatile("fldl %1; fmull %2; faddl %0; fstpl %0" : "+m"(*out) : "m"(*left),"m"(*right) : "st"); }
    } else for (unsigned int lane=0;lane<width;++lane) {
        if (broadcast&&(op==0x20400000||op==0x20500000)) Lane(op,b+lane,a,c+lane,out+lane);
        else Lane(op,a+(broadcast ? 0 : lane),op==0x10600000&&!lane ? a : b+lane,c+lane,out+lane);
    }
}
bool Same(const FpState &a,const FpState &b,bool range) { return a.words[0]==b.words[0]&&(a.words[2]&0x383f)==(b.words[2]&0x383f)&&a.words[4]==b.words[4]&&(!range||((a.words[2]^b.words[2])&0x400)==0); }
bool Hardware(unsigned int &arithmetic,unsigned int &indices,unsigned int &narrowing) {
    const unsigned int ops[]={0x10000000,0x10100000,0x10300000,0x10600000,0x10700000,0x10800000,0x10900000,0x10c00000,0x20000000,0x20100000,0x20200000,0x20300000,0x20400000,0x20500000,0x20600000,0x30000000,0x50000000,0x70e00000,0x10200000};
    const unsigned long long values[]={0,0x8000000000000000ull,1,0x000fffffffffffffull,0x0010000000000000ull,0x3ff0000000000000ull,0xbff0000000000000ull,0x3fe0000000000000ull,0x4000000000000000ull,0x7fefffffffffffffull,0x7ff0000000000000ull,0xfff0000000000000ull,0x7ff8123456789abcull,0xfff8543212345678ull,0x7ff0123456789abcull,0xfff0123456789abcull,0x41e0000000200000ull,0xc1e0000000200000ull};
    const int aliases[]={-8,-4,0,4,8,0x400,0x800,0xc00};unsigned short saved;asm volatile("fnstcw %0" : "=m"(saved));arithmetic=indices=narrowing=0;controlledResult=5.5;
    for (unsigned int mode=0;mode<12;++mode) {
        unsigned short control=(mode%3==0 ? 0x7f : mode%3==1 ? 0x27f : 0x37f)|((mode/3)<<10);
        for (unsigned int opIndex=0;opIndex<sizeof(ops)/sizeof(ops[0]);++opIndex) for (unsigned int sample=0;sample<776;++sample) {
            bool edge=sample>=128;unsigned int pair=sample-128,alias=edge ? 7 : (sample/8)%8;
            unsigned int op=ops[opIndex],width=edge ? 1 : (sample%8<5 ? sample%8 : sample%8==5 ? 7 : sample%8==6 ? 8 : 16),count=op==0x70e00000 ? sample%9 : op==0x10600000 ? width : op==0x30000000 ? 3 : op>=0x20000000 ? 2 : 1;bool broadcast=edge ? pair>=324 : (sample/64)!=0;
            std::vector<unsigned char> initial(0x2000,0xa5);
            for (unsigned int bank=0;bank<3;++bank) for (unsigned int i=0;i<32;++i) { unsigned int selected=edge ? (bank==0 ? pair%18 : bank==1 ? (pair/18)%18 : (pair*7+3)%18) : (sample+i+bank*5)%18;unsigned long long bits=values[selected];memcpy(initial.data()+0x100+bank*0x400+i*8,&bits,8); }
            std::vector<unsigned char> expected=initial,actual=initial;double *a=reinterpret_cast<double *>(expected.data()+0x100),*b=reinterpret_cast<double *>(expected.data()+0x500),*c=reinterpret_cast<double *>(expected.data()+0x900),*out=reinterpret_cast<double *>(expected.data()+0x100+aliases[alias]);
            Reset(control);Oracle(op,width,broadcast,count,a,b,c,out);FpState reference=State();std::vector<unsigned int> code=Program(op,width,broadcast,count);unsigned int literal[3]={1,0,0};
            Reset(control);int result=UG2FxExecuteVm(code.data(),literal,Address(actual.data()+0x100),Address(actual.data()+0x500),0,Address(actual.data()+0x100+aliases[alias]),Address(actual.data()+0x900),0,255,255,0,255,255,0);FpState rebuilt=State();
            if (result||expected!=actual||!Same(reference,rebuilt,op==0x10800000||op==0x10900000)||reference.words[4]!=0xffff) { fprintf(stderr,"hardware VM mismatch: mode %u opcode %x sample %u flags %x/%x tags %x/%x\n",mode,op,sample,reference.words[2],rebuilt.words[2],reference.words[4],rebuilt.words[4]);unsigned int printed=0;for (unsigned int byte=0;byte<expected.size()&&printed<8;++byte) if (expected[byte]!=actual[byte]) { fprintf(stderr," byte %x: %02x/%02x\n",byte,expected[byte],actual[byte]);++printed; }asm volatile("fninit; fldcw %0" : : "m"(saved));return false; }++arithmetic;
        }
        for (unsigned int sample=0;sample<256;++sample) {
            unsigned long long bits=values[sample%(sizeof(values)/sizeof(values[0]))];double input;memcpy(&input,&bits,8);long long integer;unsigned short truncation=control|0xc00;
            Reset(control);asm volatile("fldl %2; fldcw %3; fistpll %0; fldcw %1" : "=m"(integer) : "m"(control),"m"(input),"m"(truncation) : "st");FpState reference=State();unsigned int selected=(static_cast<unsigned int>(integer)*4)&255;
            unsigned long long table[256];for (unsigned int j=0;j<256;++j) table[j]=0x3ff0000000000000ull+j*0x12345ull;unsigned long long output=0;unsigned int code[]={1,0x10000001,1,1,3,0,2,0,0,4,0},literal[3]={1,0,0};
            Reset(control);int result=UG2FxExecuteVm(code,literal,Address(&input),Address(table),0,Address(&output),0,0,255,255,0,0,0,0);FpState rebuilt=State();
            if (result||output!=table[selected]||!Same(reference,rebuilt,false)||reference.words[4]!=0xffff) { fprintf(stderr,"hardware VM index mismatch: mode %u sample %u flags %x/%x\n",mode,sample,reference.words[2],rebuilt.words[2]);asm volatile("fninit; fldcw %0" : : "m"(saved));return false; }++indices;
        }
        for (unsigned int sample=0;sample<128;++sample) {
            unsigned int count=sample%8;double input[3];for (unsigned int lane=0;lane<3;++lane) { unsigned long long bits=values[(sample+lane)%(sizeof(values)/sizeof(values[0]))];memcpy(input+lane,&bits,8); }
            float expected[3];double oracleOutput;Reset(control);
            for (unsigned int lane=0;lane<3;++lane) if (lane<count) asm volatile("fldl %1; fstps %0" : "=m"(expected[lane]) : "m"(input[lane]) : "st");else memset(expected+lane,0,4);
            asm volatile("fldl %1; fstpl %0" : "=m"(oracleOutput) : "m"(controlledResult) : "st");FpState reference=State();
            double output=0,*pointer=input;operationInputs.clear();Reset(control);D3DXEffectVmOperation502(count,&pointer,&output);FpState rebuilt=State();
            if (operationInputs.size()!=12||memcmp(operationInputs.data(),expected,12)||memcmp(&output,&oracleOutput,8)||!Same(reference,rebuilt,false)||reference.words[4]!=0xffff) { fprintf(stderr,"hardware VM narrowing mismatch: mode %u sample %u flags %x/%x\n",mode,sample,reference.words[2],rebuilt.words[2]);asm volatile("fninit; fldcw %0" : : "m"(saved));return false; }++narrowing;
        }
    }
    asm volatile("fninit; fldcw %0" : : "m"(saved));return true;
}
}
// Test-only operation dependency. The other CRT boundaries remain traps here.
extern "C" long double __cdecl UG2FxVmFloor(double) { abort(); }
extern "C" long double __cdecl UG2FxVmPower(double,double) { abort(); }
extern "C" long double __cdecl UG2FxVmAsin(double) { abort(); }
extern "C" long double __cdecl UG2FxVmAcos(double) { abort(); }
extern "C" long double __stdcall UG2FxVmOperation63F2B5(float *values) { const unsigned char *bytes=reinterpret_cast<const unsigned char *>(values);operationInputs.insert(operationInputs.end(),bytes,bytes+12);return controlledResult; }
int main(int argc,char **argv) {
    if (argc!=4) { fprintf(stderr,"usage: ug2effectvm_probe speed2.exe fixtures.bin capture.bin\n");return 2; }char error[512];if (!MapOriginalImage(argv[1],error,sizeof(error))) { fprintf(stderr,"%s\n",error);return 1; }
    unsigned int arithmetic,indices,narrowing;if (!Hardware(arithmetic,indices,narrowing)) return 1;printf("effect VM: %u hardware opcode/alias/flag checks, %u hardware index checks, %u hardware narrowing checks\n",arithmetic,indices,narrowing);
    FILE *fixtures=fopen(argv[2],"rb"),*capture=fopen(argv[3],"wb");if (!fixtures||!capture) return 3;char magic[8];unsigned int count;
    if (fread(magic,1,8,fixtures)!=8||memcmp(magic,"UG2FXMF1",8)||fread(&count,4,1,fixtures)!=1||count>1024||fwrite("UG2FXMD1",1,8,capture)!=8) return 3;
    unsigned short saved;asm volatile("fnstcw %0" : "=m"(saved));
    for (unsigned int row=0;row<count;++row) {
        unsigned int config[4];double resultValue;if (fread(config,4,4,fixtures)!=4||config[0]!=row||config[1]>1||config[3]!=(config[1] ? 3u : 14u)||fread(&resultValue,8,1,fixtures)!=1) return 3;controlledResult=resultValue;std::vector<unsigned int> arguments(config[3]);std::vector<unsigned char> memory(0x10000);
        if (fread(arguments.data(),4,arguments.size(),fixtures)!=arguments.size()||fread(memory.data(),1,memory.size(),fixtures)!=memory.size()) return 3;unsigned int base=Address(memory.data());
        if (config[1]) { for (unsigned int i=1;i<3;++i) arguments[i]=base+arguments[i]-0x2000000;*reinterpret_cast<unsigned int *>(memory.data()+0x1000)=base+0x5000; }
        else for (unsigned int i=0;i<8;++i) if (arguments[i]) arguments[i]=base+arguments[i]-0x2000000;
        operationInputs.clear();Reset(config[2]);unsigned int result=0;
        if (config[1]) D3DXEffectVmOperation502(arguments[0],reinterpret_cast<double **>(arguments[1]),reinterpret_cast<double *>(arguments[2]));
        else result=UG2FxExecuteVm(reinterpret_cast<unsigned int *>(arguments[0]),reinterpret_cast<unsigned int *>(arguments[1]),arguments[2],arguments[3],arguments[4],arguments[5],arguments[6],arguments[7],arguments[8],arguments[9],arguments[10],arguments[11],arguments[12],arguments[13]);
        FpState state=State();std::vector<unsigned int> words;Add(words,result);Add(words,state.words[0]);Add(words,(state.words[2]>>11)&7);Add(words,state.words[4]);Add(words,operationInputs.size()/12);unsigned int header[2]={row,static_cast<unsigned int>(words.size()*4+operationInputs.size()+0x6000)};
        if (fwrite(header,4,2,capture)!=2||fwrite(words.data(),4,words.size(),capture)!=words.size()||fwrite(operationInputs.data(),1,operationInputs.size(),capture)!=operationInputs.size()||fwrite(memory.data()+0x5000,1,0x6000,capture)!=0x6000) return 3;
    }
    asm volatile("fninit; fldcw %0" : : "m"(saved));if (fgetc(fixtures)!=EOF||fclose(fixtures)||fclose(capture)) return 3;printf("effect VM: %u native dispatch/operation graphs\n",count);return 0;
}
