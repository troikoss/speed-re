#include "OriginalImage.h"
#include "../../Speed/Indep/Libs/D3DX/CrtExceptions.h"
#include <stdio.h>
#include <string.h>
// Test-only oracles. Retail code is never mapped executable. Integer payload
// models use 64-bit shifts rather than the recovered two-DWORD implementation;
// instruction oracles retain x87 signaling, spill and status semantics.
namespace {
typedef unsigned long long U64;
struct Case { unsigned int id,flags,saved,control,mxcsr,status,exponent,mode,operation,edit,redirect,recordBits,recordMasks,seed,alias;U64 bits,other,result; };
typedef char CrtExceptionFixtureSize[(sizeof(Case)==84)?1:-1];
struct State { unsigned short env[14];unsigned int mxcsr; };
struct Event { unsigned int code,words[32];State state; };
struct Trial { unsigned char data[128];unsigned int record[32],alternate[32];unsigned char returned[12];int integer,error;State state;Event event;bool raised; };
const Case *current;Trial *trial;unsigned int kind;
State Snapshot() { State s;asm volatile("fnstenv %0; stmxcsr %1" : "=m"(s.env),"=m"(s.mxcsr));return s; }
void Reset(const Case &c) { unsigned short control=c.control;asm volatile("fninit; fldcw %0; ldmxcsr %1" : : "m"(control),"m"(c.mxcsr) : "st");State s=Snapshot();s.env[2]=c.status;asm volatile("fldenv %0" : : "m"(s.env)); }
bool SameState(const State &a,const State &b) { return a.env[0]==b.env[0]&&(a.env[2]&0x383f)==(b.env[2]&0x383f)&&a.env[4]==b.env[4]&&a.mxcsr==b.mxcsr; }
U64 Spill(U64 bits) { U64 value;asm volatile("fldl %1; fstpl %0" : "=m"(value) : "m"(bits) : "st");return value; }
long double Load(U64 bits) { long double value;asm volatile("fldl %1; fstpt %0" : "=m"(value) : "m"(bits) : "st");return value; }
unsigned short CompareZero(U64 bits) { const U64 zero=0;unsigned short status;asm volatile("fldl %1; fcompl %2; fnstsw %0" : "=am"(status) : "m"(bits),"m"(zero) : "st");return status; }
U64 Negate(U64 bits) { U64 value;asm volatile("fldl %1; fchs; fstpl %0" : "=m"(value) : "m"(bits) : "st");return value; }
void RaiseStatus(unsigned int flags) {
    const unsigned char large[]={0,0,0,0,0,0,0,0x80,0x10,0x44},tiny[]={1,0,0,0,0,0,0,0x80,0,0x30};U64 scratch;unsigned short status;
    if (flags&1) asm volatile("fldt %1; fistpl %0; fwait" : "=m"(scratch) : "m"(large) : "st");
    if (flags&8) asm volatile("fstsw %0; fldt %2; fstpl %1; fwait; fstsw %0" : "=m"(status),"=m"(scratch) : "m"(large) : "st");
    if (flags&16) asm volatile("fldt %1; fstpl %0; fwait" : "=m"(scratch) : "m"(tiny) : "st");
    if (flags&4) asm volatile("fldz; fld1; fdivp; fstp %%st(0); fwait" : : : "st");
    if (flags&32) asm volatile("fldpi; fstpl %0; fwait" : "=m"(scratch) : : "st");
}
int ReadStatus(bool clear) { unsigned short status;if (clear) asm volatile("fnstsw %0; fnclex" : "=m"(status));else asm volatile("fstsw %0" : "=m"(status));return static_cast<short>(status); }
int ErrorCode(unsigned int flags) { const unsigned int bits[]={32,8,4,1,2};const int errors[]={5,1,2,3,4};for (unsigned int j=0;j<5;++j) if (flags&bits[j]) return errors[j];return 0; }
long double Exponent(U64 bits,int exponent) {
    U64 value=Spill(bits);unsigned short top=static_cast<unsigned short>((static_cast<unsigned int>(exponent)+1022)*16)|(bits>>48&0x800f);
    value=(value&0xffffffffffffull)|(static_cast<U64>(top)<<48);return Load(value);
}
long double Decompose(U64 bits,int *output) {
    U64 value;int exponent=0;unsigned short status=CompareZero(bits);
    if ((status&0x4400)==0x4000) value=0;
    else if ((bits&0x7ff0000000000000ull)==0&&(bits&0xfffffffffffffull)!=0) {
        bool negative=(CompareZero(bits)&0x500)==0x100;exponent=-1021;
        while (!(bits&0x10000000000000ull)) { bits<<=1;--exponent; }
        bits&=~0x10000000000000ull;if (negative) bits|=0x8000000000000000ull;
        U64 argument=Spill(bits);long double replaced=Exponent(argument,0);asm volatile("fldt %1; fstpl %0" : "=m"(value) : "m"(replaced) : "st");
    } else {
        exponent=static_cast<int>(bits>>52&0x7ff)-1022;U64 argument=Spill(bits);long double replaced=Exponent(argument,0);asm volatile("fldt %1; fstpl %0" : "=m"(value) : "m"(replaced) : "st");
    }
    long double result=Load(value);*output=exponent;return result;
}
int Masks(unsigned int flags,U64 *result,unsigned int control) {
    unsigned int left=flags&31;const unsigned int logical[]={8,4,1,2},hardware[]={1,4,8,16};unsigned int selected=4;
    for (unsigned int j=0;j<4;++j) if ((flags&logical[j])&&(control&hardware[j])) { selected=j;break; }
    if (selected<2) RaiseStatus(hardware[selected]);
    if (selected==2) {
        RaiseStatus(8);bool positive=!(CompareZero(*result)&0x4100);unsigned int direction=control>>10&3;
        bool finite=direction==3||(positive ? direction==1 : direction==2);
        U64 magnitude=finite ? 0x7fefffffffffffffull : 0x7ff0000000000000ull;
        U64 value=positive ? Spill(magnitude) : Negate(magnitude);*result=Spill(value);
    }
    if (selected==3) {
        bool sticky=flags&16;
        if ((CompareZero(*result)&0x4400)==0x4000) sticky=true;
        else {
            U64 argument=Spill(*result),value;int exponent;long double fraction=Decompose(argument,&exponent);
            asm volatile("fldt %1; fstpl %0" : "=m"(value) : "m"(fraction) : "st");exponent-=1536;
            if (exponent<-1074) { const U64 zero=0;asm volatile("fldl %1; fmull %2; fstpl %0" : "=m"(value) : "m"(value),"m"(zero) : "st");sticky=true; }
            else {
                bool negative=(CompareZero(value)&0x500)==0x100;value=(value&0xfffffffffffffull)|0x10000000000000ull;
                unsigned int shift=exponent<-1021 ? -1021-exponent : 0;
                if (shift) { if (value&((1ull<<shift)-1)) sticky=true;value>>=shift; }
                if (negative) value=Negate(value);
            }
            *result=Spill(value);
        }
        if (sticky) RaiseStatus(16);
    }
    if (selected<4) left&=~logical[selected];
    if ((flags&16)&&(control&32)) { RaiseStatus(32);left&=~16u; }
    return !left;
}
void Delivery(unsigned int code,unsigned int **record) {
    trial->raised=true;trial->event.code=code;trial->event.state=Snapshot();memcpy(trial->event.words,*record,128);
    if (current->edit) {
        if (current->redirect) *record=trial->alternate;
        (*record)[0]=current->recordBits;(*record)[2]=current->recordMasks;memcpy(*record+20,&current->other,8);
    }
}
void Record(unsigned int *record,unsigned int *control,unsigned int flags,unsigned int operation,U64 *input,U64 *output) {
    record[1]=record[2]=record[3]=0;
    const unsigned int logical[]={16,2,1,4,8},codes[]={0xc000008f,0xc0000093,0xc0000091,0xc000008e,0xc0000090};unsigned int code=flags;
    for (unsigned int j=0;j<5;++j) if (flags&logical[j]) { record[1]|=1u<<j;code=codes[j]; }
    const unsigned int hw[]={1,4,8,16,32},rec[]={16,8,4,2,1};
    for (unsigned int j=0;j<5;++j) { if (*control&hw[j]) record[2]&=~rec[j];else record[2]|=rec[j]; }
    unsigned int status=ReadStatus(false);
    for (unsigned int j=0;j<5;++j) if (status&hw[j]) record[3]|=rec[j];
    record[0]=(record[0]&~3u)|((*control>>10)&3);
    unsigned int precision=*control>>8&3;
    if (precision!=1) record[0]=(record[0]&~28u)|(precision==0 ? 8 : precision==2 ? 4 : 0);
    record[0]=(record[0]&~0x1ffe0u)|((operation&0xfff)<<5);
    record[8]=(record[8]&~31u)|3;U64 copied=Spill(*input);memcpy(record+4,&copied,8);
    record[24]=(record[24]&~31u)|3;copied=Spill(*output);memcpy(record+20,&copied,8);
    ReadStatus(true);Delivery(code,&record);
    for (unsigned int j=0;j<5;++j) if (record[2]&rec[j]) *control&=~hw[j];
    *control=(*control&~0xc00u)|((record[0]&3)<<10);
    unsigned int decoded=record[0]>>2&7;
    if (decoded<3) *control=(*control&~0xc00u)|(decoded==0 ? 0x300 : decoded==1 ? 0x200 : 0);
    memcpy(&copied,record+20,8);*output=Spill(copied);
}
long double Result(unsigned int flags,unsigned int operation,U64 input,U64 output,unsigned int control) {
    if (!Masks(flags,&output,control)) { unsigned int record[32]={0};Record(record,&control,flags,operation,&input,&output); }
    int error=ErrorCode(flags);
    if (!ug2CrtMathErrorFlag81ACE0&&error) {
        output=Spill(output);input=Spill(input);D3DXCrtDoubleBits a,b,zero={0,0};memcpy(&a,&input,8);memcpy(&b,&output,8);
        // Previously verified dependency; this probe targets the nine new bodies.
        return D3DXCrtUserMathError(error,operation,a,zero,b,control);
    }
    D3DXCrtSetMathErrno(error);D3DXCrtRawControl(control,0xffff);return Load(output);
}
void Prepare(Trial &t,const Case &c) {
    memset(&t,0,sizeof(t));memset(t.data,0xa5,128);memcpy(t.data,&c.bits,8);memcpy(t.data+16,&c.other,8);memcpy(t.data+32,&c.result,8);memcpy(t.data+64,&c.saved,4);
    unsigned int state=c.seed;for (unsigned int j=0;j<64;++j) { state=1664525u*state+1013904223u;(j<32 ? t.record[j] : t.alternate[j-32])=state; }
    *D3DXCrtErrno()=c.seed;ug2CrtMathErrorFlag81ACE0=c.mode;trial=&t;Reset(c);
}
void Run(Trial &t,const Case &c,bool actual,unsigned int operation) {
    Prepare(t,c);kind=operation;long double value=0;D3DXCrtDoubleBits bits,result;memcpy(&bits,&c.bits,8);memcpy(&result,&c.result,8);
    switch (operation) {
    case 0:{ unsigned int saved=c.saved|0x7f;if ((saved&0x300)==0x100) saved&=~0x300u;
        value=actual ? D3DXCrtHandleResult(c.flags,c.operation,bits,result,saved) : Result(c.flags,c.operation,c.bits,c.result,saved);break; }
    case 1:t.integer=actual ? D3DXCrtApplyMasks(c.flags,reinterpret_cast<double *>(t.data),c.saved) : Masks(c.flags,reinterpret_cast<U64 *>(t.data),c.saved);break;
    case 2:{ unsigned int *control=c.alias==0xffffffff ? reinterpret_cast<unsigned int *>(t.data+64) : reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned char *>(t.record)+c.alias);
        const unsigned int offsets[]={4,8,16,20,32,80,96,100};unsigned char *record=reinterpret_cast<unsigned char *>(t.record);
        U64 *input=reinterpret_cast<U64 *>(c.id%7==0 ? record+offsets[c.id/7%8] : t.data),*output=reinterpret_cast<U64 *>(c.id%11==0 ? record+offsets[c.id/11%8] : t.data+32);
        if (actual) D3DXCrtRaiseException(t.record,control,c.flags,c.operation,reinterpret_cast<double *>(input),reinterpret_cast<double *>(output));
        else Record(t.record,control,c.flags,c.operation,input,output);break; }
    case 3:value=actual ? D3DXCrtDecompose(bits,reinterpret_cast<int *>(t.data+80)) : Decompose(c.bits,reinterpret_cast<int *>(t.data+80));break;
    case 4:value=actual ? D3DXCrtSetExponent(bits,c.exponent) : Exponent(c.bits,c.exponent);break;
    case 5:t.integer=actual ? D3DXCrtErrorCode(c.flags) : ErrorCode(c.flags);break;
    case 6:if (actual) D3DXCrtSetStatus(c.flags);else RaiseStatus(c.flags);break;
    case 7:t.integer=actual ? D3DXCrtReadStatus() : ReadStatus(false);break;
    case 8:t.integer=actual ? D3DXCrtClearStatus() : ReadStatus(true);break;
    }
    if (operation==0||operation==3||operation==4) memcpy(t.returned,&value,10);
    t.state=Snapshot();t.error=*D3DXCrtErrno();
}
unsigned int StatusWords() {
    Case c={0};c.control=0x37f;c.mxcsr=0x1f80;
    for (unsigned int word=0;word<65536;++word) {
        c.status=word;Reset(c);int read=ReadStatus(false);State before=Snapshot();Reset(c);int actual=D3DXCrtReadStatus();State after=Snapshot();
        if (read!=actual||!SameState(before,after)) return 0;
        Reset(c);read=ReadStatus(true);before=Snapshot();Reset(c);actual=D3DXCrtClearStatus();after=Snapshot();
        if (read!=actual||!SameState(before,after)) return 0;
    }
    return 65536*2;
}

bool SameEvent(const Event &a,const Event &b,unsigned int operation) {
    if (a.code!=b.code||!SameState(a.state,b.state)) return false;
    if (operation==2) return memcmp(a.words,b.words,128)==0;
    const unsigned int index[]={0,1,2,3,4,5,8,16,20,21,24};
    for (unsigned int j=0;j<11;++j) { unsigned int p=index[j],mask=p==0 ? 0x1ffff : p==8||p==24 ? 31 : p==16 ? 1 : 0xffffffff;if ((a.words[p]&mask)!=(b.words[p]&mask)) return false; }
    return true;
}
}
extern "C" void __stdcall UG2FxCrtOsRaiseException(unsigned int code,unsigned int flags,unsigned int count,unsigned int *arguments) {
    if (flags||count!=1) { fprintf(stderr,"hardware RaiseException ABI mismatch\n");__builtin_trap(); }
    Delivery(code,reinterpret_cast<unsigned int **>(arguments));
}
int main(int argc,char **argv) {
    char error[256];if (argc!=3||!MapOriginalImage(argv[1],error,sizeof(error))) return 2;
    FILE *f=fopen(argv[2],"rb");unsigned char magic[8];unsigned int count;
    if (!f||fread(magic,1,8,f)!=8||memcmp(magic,"UG2FXEI1",8)||fread(&count,4,1,f)!=1) return 2;
    for (unsigned int i=0;i<count;++i) {
        Case c;if (fread(&c,1,sizeof(c),f)!=sizeof(c)||c.id!=i) return 2;current=&c;
        for (unsigned int operation=0;operation<9;++operation) {
            Trial expected,actual;Run(expected,c,false,operation);Run(actual,c,true,operation);
            if (expected.integer!=actual.integer||memcmp(expected.returned,actual.returned,10)||!SameState(expected.state,actual.state)||expected.error!=actual.error||memcmp(expected.data,actual.data,128)||memcmp(expected.record,actual.record,128)||memcmp(expected.alternate,actual.alternate,128)||expected.raised!=actual.raised||(expected.raised&&!SameEvent(expected.event,actual.event,operation))) {
                fprintf(stderr,"hardware CRT exception mismatch: row %u body %u flags %x saved %x value %llx return %x/%x sw %x/%x cw %x/%x event %d/%d mem %d/%d/%d ret80 %d errno %x/%x\n",i,operation,c.flags,c.saved,c.bits,expected.integer,actual.integer,expected.state.env[2],actual.state.env[2],expected.state.env[0],actual.state.env[0],expected.raised,actual.raised,memcmp(expected.data,actual.data,128),memcmp(expected.record,actual.record,128),memcmp(expected.alternate,actual.alternate,128),memcmp(expected.returned,actual.returned,10),expected.error,actual.error);return 1;
            }
        }
    }
    if (fgetc(f)!=EOF) return 2;fclose(f);unsigned int words=StatusWords();asm volatile("fninit");if (!words) { fprintf(stderr,"hardware status-word mismatch\n");return 1; }printf("CRT exceptions hardware: %u rows, %u body/instruction-model checks, %u status-word checks; zero mismatches\n",count,count*9,words);return 0;
}
