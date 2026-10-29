#include "OriginalImage.h"
#include "../../Speed/Indep/Libs/D3DX/CrtPower.h"
#include <stdio.h>
#include <string.h>
// Test-only models. The SSE arithmetic body is a controlled service, never a
// numerical substitute in production. All x87 numerical instructions execute.
extern "C" long double __cdecl ProbePowerBits(D3DXCrtDoubleBits,D3DXCrtDoubleBits) __asm__("UG2FxVmPower");
namespace {
typedef unsigned long long U64;
struct Case { unsigned int id,control,mxcsr,status,saved,sse,bypass,mode,operation,exitOperation,type,seed,edit,redirect,recordBits;U64 base,exponent,sseResult;D3DXCrtExtended base80,exponent80; };
typedef char FixtureSize[sizeof(Case)==108 ? 1 : -1];
struct Stored80 { unsigned char bytes[10]; };
struct State { unsigned short env[14];unsigned int mxcsr;unsigned char live[20]; };
struct Trial { D3DXCrtExtended value,output;U64 stored;unsigned int carry,alternate[32],event[13],code,sseArgs[4];bool raised,sseCalled;int integer,error;State state,eventState,sseState; };
const Case *current;Trial *trial;unsigned int kind;
State Snapshot() { State s;asm volatile("fnstenv %0; fld %%st(0); fstpt %2; fld %%st(1); fstpt %3; stmxcsr %1" : "=m"(s.env),"=m"(s.mxcsr),"=m"(*reinterpret_cast<Stored80 *>(s.live)),"=m"(*reinterpret_cast<Stored80 *>(s.live+10)) : : "st");return s; }
bool SameState(const State &a,const State &b) { return a.env[0]==b.env[0]&&(a.env[2]&0x383f)==(b.env[2]&0x383f)&&a.env[4]==b.env[4]&&a.mxcsr==b.mxcsr&&!memcmp(a.live,b.live,20); }
void Reset(const Case &c) { unsigned short control=c.control,env[14];asm volatile("fninit; fldcw %1; ldmxcsr %2; fld1; fld1; fchs; fnstenv %0" : "=m"(env) : "m"(control),"m"(c.mxcsr) : "st");env[2]=(env[2]&0x3800)|c.status;asm volatile("fldenv %0" : : "m"(env)); }
void Control(unsigned int word) { unsigned short control=word;asm volatile("fldcw %0" : : "m"(control)); }
unsigned int ReadControl() { unsigned short word;asm volatile("fstcw %0" : "=m"(word));return word; }
unsigned int Status() { unsigned short status;asm volatile("fstsw %0" : "=m"(status));return status; }
void Normalize(D3DXCrtExtended &out,const D3DXCrtExtended &input) { asm volatile("fldt %1; fstpt %0" : "=m"(out) : "m"(input) : "st"); }
void Load(D3DXCrtExtended &out,U64 bits) { asm volatile("fldl %1; fstpt %0" : "=m"(out) : "m"(bits) : "st"); }
U64 Store(const D3DXCrtExtended &value) { U64 bits;asm volatile("fldt %1; fstpl %0" : "=m"(bits) : "m"(value) : "st");return bits; }
U64 Spill(U64 bits) { U64 value;asm volatile("fldl %1; fstpl %0" : "=m"(value) : "m"(bits) : "st");return value; }
U64 Negate64(U64 bits) { U64 value;asm volatile("fldl %1; fchs; fstpl %0" : "=m"(value) : "m"(bits) : "st");return value; }
void Negate(D3DXCrtExtended &v) { asm volatile("fldt %0; fchs; fstpt %0" : "+m"(v) : : "st"); }
unsigned int Compare(U64 a,U64 b) { unsigned short status;asm volatile("fldl %1; fcompl %2; fnstsw %0" : "=am"(status) : "m"(a),"m"(b) : "st");return status; }
bool Less(U64 a,U64 b) { return (Compare(a,b)&0x500)==0x100; }
bool Greater(U64 a,U64 b) { return !(Compare(a,b)&0x4100); }
bool Equal(U64 a,U64 b) { return (Compare(a,b)&0x4400)==0x4000; }
int Classify(U64 bits) {
    unsigned int exponent=bits>>52&0x7ff;bool negative=bits>>63;
    if (exponent==0x7ff) { U64 prepared=Spill(bits);unsigned int type=D3DXCrtSpecialType(*reinterpret_cast<double *>(&prepared));const int types[]={1,0x200,4,2,1};return types[type]; }
    if (!exponent&&(bits&0xfffffffffffffull)) return negative ? 16 : 128;
    if (Equal(bits,0)) return negative ? 32 : 64;return negative ? 8 : 256;
}
U64 Round64(U64 bits) { U64 value;asm volatile("fldl %1; frndint; fstpl %0" : "=m"(value) : "m"(bits) : "st");return value; }
int Parity(U64 bits) {
    if (Classify(Spill(bits))&0x90) return 0;
    if (!Equal(Round64(Spill(bits)),bits)) return 0;U64 half;const double two=2;
    asm volatile("fldl %1; fdivl %2; fstpl %0" : "=m"(half) : "m"(bits),"m"(two) : "st");return Equal(Round64(Spill(half)),half) ? 2 : 1;
}
unsigned int Parity80(const D3DXCrtExtended &v) {
    unsigned int result;const double half=0.5;
    asm volatile("fldt %1; fld %%st(0); frndint; fcomp; fstsw %%ax; sahf; jne 1f; fmull %2; fld %%st(0); frndint; fcompp; fstsw %%ax; sahf; jne 2f; mov $2,%0; jmp 3f; 2: mov $1,%0; jmp 3f; 1: fstp %%st(0); xor %0,%0; 3:" : "=r"(result) : "m"(v),"m"(half) : "eax","st","cc");return result;
}
void Exp2(D3DXCrtExtended &v) {
    // GNU's two-register FSUB spelling encodes Intel FSUBR ST1,ST0 (DC E1).
    asm volatile("fldt %0; fld %%st(0); frndint; fsub %%st(0),%%st(1); fxch; fchs; f2xm1; fld1; faddp; fscale; fstp %%st(1); fstpt %0" : "+m"(v) : : "st");
}
unsigned int Operand(D3DXCrtExtended &out,U64 bits,unsigned int &carry) {
    unsigned int high=bits>>32,exponent=high&0x7ff00000;
    if (exponent!=0x7ff00000) { Load(out,bits);return exponent; }
    D3DXCrtExtended raw;raw.low=static_cast<unsigned int>(bits)<<11;raw.high=bits>>21;raw.exponent=(high>>16)|0x7fff;carry=static_cast<unsigned int>(bits)<<11;Normalize(out,raw);return high;
}
int Special(U64 base,U64 exponent,U64 &out) {
    U64 magnitude=Less(base,0) ? Negate64(base) : Spill(base);const U64 one=0x3ff0000000000000ull,inf=0x7ff0000000000000ull,nan=0xfff8000000000000ull;
    if (exponent==inf||exponent==0xfff0000000000000ull) {
        if (Greater(magnitude,one)) { out=exponent==inf ? Spill(inf) : 0;return 0; }
        if (Less(magnitude,one)) { out=exponent==inf ? 0 : Spill(inf);return 0; }out=Spill(nan);return 1;
    }
    if (base==inf) { if (Greater(exponent,0)) out=Spill(inf);else out=Less(exponent,0) ? 0 : one; }
    else if (base==0xfff0000000000000ull) {
        int parity=Parity(Spill(exponent));
        if (Greater(exponent,0)) out=Spill(parity==1 ? Negate64(inf) : Spill(inf));
        else if (Less(exponent,0)) out=Spill(parity==1 ? Spill(0x8000000000000000ull) : 0);else out=one;
    }
    return 0;
}
D3DXCrtDoubleBits Pair(U64 bits) { D3DXCrtDoubleBits value;memcpy(&value,&bits,8);return value; }
void Error(unsigned int op,unsigned int type,U64 first,U64 second,unsigned int saved,D3DXCrtExtended &value) {
    // Previously verified dispatch/record/errno dependencies; the new prefix
    // performs integer second-input copies and the floating result spill first.
    D3DXCrtMathException error;error.type00=type;memcpy(&error.input10,&second,8);U64 result=Store(value);memcpy(&error.result18,&result,8);error.name04=reinterpret_cast<const char *>(0x819c88);memcpy(&error.input08,&first,8);
    D3DXCrtOneArgDispatch(op,&error,reinterpret_cast<unsigned short *>(&saved));memcpy(&result,&error.result18,8);Load(value,result);if (static_cast<unsigned short>(saved)!=0x27f) Control(saved);
}
void Exit(unsigned int op,U64 first,U64 second,unsigned int saved,D3DXCrtExtended &v) {
    U64 bits=Store(v);unsigned int exponent=bits>>52&0x7ff,type=0;
    if (!exponent||exponent==0x7ff) {
        const double scale=exponent==0 ? 1536 : -1536;unsigned short status;
        if (!exponent) {
            asm volatile("fldl %1; fldt %0; fscale; fstp %%st(1); fld %%st(0); fabs; fcompl %2; fstpt %0" : "+m"(v) : "m"(scale),"m"(ug2CrtPowerMinimum7D7C5C) : "st");
            // Re-read status from the comparison; the scale result store has no
            // effect on the C0/C3 bits used by the original branch.
            status=Status();type=4;if (status&0x100) { const double zero=0;asm volatile("fldt %0; fmull %1; fstpt %0" : "+m"(v) : "m"(zero) : "st"); }
        } else {
            asm volatile("fldl %1; fldt %0; fscale; fstp %%st(1); fld %%st(0); fabs; fcompl %2; fstpt %0" : "+m"(v) : "m"(scale),"m"(ug2CrtPowerMaximum7D7C54) : "st");status=Status();type=3;
            if (!(status&0x4100)) asm volatile("fldt %0; fmull %1; fstpt %0" : "+m"(v) : "m"(ug2CrtPowerInfinity7D7C74) : "st");
        }
    } else if (static_cast<unsigned short>(saved)!=0x27f) { if (!(saved&32)&&(Status()&32)) type=8;else Control(saved); }
    if (type) {
        if (op==29) Error(op,type,first,second,saved,v);
        else { D3DXCrtDoubleBits input=Pair(first);D3DXCrtOneArgError(op,type,reinterpret_cast<const char *>(0x819c88),input,saved,&v); }
    }
}
void Core(D3DXCrtExtended &out,U64 first,U64 second,const D3DXCrtExtended &exponent) {
    unsigned int saved=ReadControl(),carry=second>>32,type=1;if (saved!=0x27f) Control((saved&0x300)|0x7f);
    bool ys=(second&0x7ff0000000000000ull)==0x7ff0000000000000ull;carry=ys ? 0 : carry&0x7ff00000;
    D3DXCrtExtended base,y=exponent,v;Operand(base,first,carry);bool xs=(first&0x7ff0000000000000ull)==0x7ff0000000000000ull,error=false,quick=false;
    if (xs||ys) {
        bool yn=ys&&(second&0xfffffffffffffull),xn=xs&&(first&0xfffffffffffffull);if (ys) { if (yn&&!(second&0x8000000000000ull)) ++carry; }else carry=0;
        if ((yn&&(second&0x8000000000000ull))||xn||carry) {
            asm volatile("fldt %1; fldt %2; faddp; fstpt %0" : "=m"(v) : "m"(y),"m"(base) : "st");type=(yn&&(second&0x8000000000000ull))||(xn&&(first&0x8000000000000ull)) ? 1 : 7;error=true;
        } else {
            U64 a=Store(base),b=Store(y),result;unsigned char environment[108];asm volatile("fsave %0" : "=m"(environment) : : "st","memory");int failed=Special(a,b,result);asm volatile("frstor %0" : : "m"(environment) : "st","memory");Load(v,result);error=failed!=0;quick=!error;
        }
    } else if (!(first&0x7fffffffffffffffull)) {
        if (!(second&0x7fffffffffffffffull)) { Load(v,0x3ff0000000000000ull);quick=true; }
        else { unsigned int parity=Parity80(y);bool negative=(first>>63)&&(parity&1);Load(v,second>>63 ? 0x7ff0000000000000ull : 0);if (negative) Negate(v);if (second>>63) { type=2;error=true; }else quick=true; }
    } else {
        unsigned int parity=0;if (first>>63) { parity=Parity80(y);Negate(base); }
        if ((first>>63)&&!parity) { memcpy(&v,&ug2CrtIndefinite81A4D0,10);error=true; }
        else { asm volatile("fldt %1; fldt %2; fyl2x; fstpt %0" : "=m"(v) : "m"(y),"m"(base) : "st");Exp2(v);if (parity==1) Negate(v); }
    }
    if (quick||ug2CrtBypassMathExit8B8424) { if (saved!=0x27f) Control(saved); }
    else if (error) Error(29,type,first,second,saved,v);else Exit(29,first,second,saved,v);memcpy(&out,&v,10);
}
void Sse(U64 a,U64 b,U64 &result) {
    trial->sseCalled=true;memcpy(trial->sseArgs,&a,8);memcpy(trial->sseArgs+2,&b,8);trial->sseState=Snapshot();result=current->sseResult;
}
void SseWrapper(D3DXCrtExtended &out,const D3DXCrtExtended &a,const D3DXCrtExtended &b) { U64 x=Store(a),y=Store(b),result;Sse(x,y,result);Load(out,result); }
void Power(D3DXCrtExtended &out,const D3DXCrtExtended &a,const D3DXCrtExtended &b) {
    unsigned short cw;unsigned int mxcsr;asm volatile("fnstcw %0; stmxcsr %1" : "=m"(cw),"=m"(mxcsr));
    if (ug2CrtSseEnabled8BA480&&(mxcsr&0x1f80)==0x1f80&&(cw&0x7f)==0x7f) SseWrapper(out,a,b);else { U64 x=Store(a),y=Store(b);Core(out,x,y,b); }
}
void Prepare(Trial &t,const Case &c) { memset(&t,0,sizeof(t));t.value=c.base80;t.carry=c.seed;t.stored=c.saved|0xa5a5a5a500000000ull;unsigned int seed=c.seed;for (unsigned int j=0;j<32;++j) { seed=1664525u*seed+1013904223u;t.alternate[j]=seed; }trial=&t;ug2CrtSseEnabled8BA480=c.sse;ug2CrtBypassMathExit8B8424=c.bypass;ug2CrtMathErrorFlag81ACE0=c.mode;*D3DXCrtErrno()=c.seed;Reset(c); }
void Run(Trial &t,const Case &c,bool actual,unsigned int operation) {
    Prepare(t,c);kind=operation;D3DXCrtExtended base=c.base80,exponent=c.exponent80;
    if (operation==0||operation==1) { Normalize(base,c.base80);Normalize(exponent,c.exponent80); }
    if (operation==2) Normalize(exponent,c.exponent80);
    if (operation==4||operation==5||operation==9||operation==10) Normalize(t.value,c.base80);
    D3DXCrtDoubleBits a=Pair(c.base),b=Pair(c.exponent);
    switch (operation) {
    case 0:if (actual) D3DXCrtPower80(&t.output,&base,&exponent);else Power(t.output,base,exponent);break;
    case 1:if (actual) D3DXCrtPowerSseWrapper80(&t.output,&base,&exponent);else SseWrapper(t.output,base,exponent);break;
    case 2:if (actual) D3DXCrtPowerX8780(&t.output,a,b,&exponent);else Core(t.output,c.base,c.exponent,exponent);break;
    case 3:t.integer=actual ? D3DXCrtPowerLoadOperand(&t.output,a,&t.carry) : Operand(t.output,c.base,t.carry);break;
    case 4:if (actual) D3DXCrtReconstructExp2(&t.value);else Exp2(t.value);break;
    case 5:t.integer=actual ? D3DXCrtPowerParity80(&t.value) : Parity80(t.value);break;
    case 6:t.integer=actual ? D3DXCrtPowerParity(a) : Parity(c.base);break;
    case 7:t.integer=actual ? D3DXCrtFpClass(a) : Classify(c.base);break;
    case 8:t.integer=actual ? D3DXCrtPowerSpecial(a,b,reinterpret_cast<double *>(&t.stored)) : Special(c.base,c.exponent,t.stored);break;
    case 9:{ unsigned int saved=c.saved,mask=c.type==1||c.type==5 ? 1 : c.type==2 ? 4 : c.type==3 ? 8 : c.type==4 ? 16 : c.type==8 ? 32 : 0;if (c.id%2) saved&=~mask;
        if (actual) D3DXCrtTwoArgError(c.operation,c.type,reinterpret_cast<const char *>(0x819c88),a,b,saved,&t.value);else Error(c.operation,c.type,c.base,c.exponent,saved,t.value);break; }
    case 10:if (actual) D3DXCrtPowerMathExit(c.exitOperation,reinterpret_cast<const char *>(0x819c88),a,b,c.saved,&t.value);else Exit(c.exitOperation,c.base,c.exponent,c.saved,t.value);break;
    }
    t.state=Snapshot();t.error=*D3DXCrtErrno();
}
void Delivery(unsigned int code,unsigned int **record) {
    trial->raised=true;trial->code=code;trial->eventState=Snapshot();unsigned int *w=*record;unsigned int op=kind==9 ? current->operation : kind==10 ? current->exitOperation : 29;
    unsigned int fields[]={w[0]&0x1ffff,w[1],w[2],w[3],w[4],w[5],w[8]&31,w[16]&1,w[20],w[21],w[24]&31,0,0};memcpy(trial->event,fields,sizeof(fields));
    if (op==16||op==22||op==29) { trial->event[7]=w[16]&31;trial->event[11]=w[12];trial->event[12]=w[13]; }
    if (current->edit||kind==9) { if (current->edit&&current->redirect) *record=trial->alternate;if (current->edit) (*record)[0]=current->recordBits;(*record)[2]=0;const double finite=-6.5;if (kind==9) memcpy(*record+20,&finite,8);else memcpy(*record+20,&current->exponent,8); }
}
bool Same(const Trial &a,const Trial &b) { return !memcmp(&a.value,&b.value,10)&&!memcmp(&a.output,&b.output,10)&&a.stored==b.stored&&a.carry==b.carry&&!memcmp(a.alternate,b.alternate,128)&&a.integer==b.integer&&a.error==b.error&&SameState(a.state,b.state)&&a.raised==b.raised&&a.sseCalled==b.sseCalled&&(!a.sseCalled||(!memcmp(a.sseArgs,b.sseArgs,16)&&SameState(a.sseState,b.sseState)))&&(!a.raised||(a.code==b.code&&!memcmp(a.event,b.event,sizeof(a.event))&&SameState(a.eventState,b.eventState))); }
bool Bridge(const Case &c) {
    Trial reference;Prepare(reference,c);kind=0;D3DXCrtExtended a,b,result;Load(a,c.base);Load(b,c.exponent);Power(result,a,b);long double expected;asm volatile("fldt %1; fstpt %0" : "=m"(expected) : "m"(result) : "st");State before=Snapshot();int error=*D3DXCrtErrno();
    Trial actual;Prepare(actual,c);volatile long double value=ProbePowerBits(Pair(c.base),Pair(c.exponent));State after=Snapshot();return !memcmp(&expected,const_cast<const long double *>(&value),10)&&SameState(before,after)&&error==*D3DXCrtErrno();
}
bool Vm(const Case &c,unsigned int &calls) {
    unsigned char expected[96],actual[96];memset(expected,0xa5,96);for (unsigned int j=0;j<3;++j) memcpy(expected+16+j*8,&c.exponent,8);memcpy(actual,expected,96);const int offsets[]={-4,0,4,8,32};unsigned int width=c.id%4,offset=16+offsets[c.id%5];
    Trial reference;Prepare(reference,c);kind=0;ug2CrtSseEnabled8BA480=0;
    for (unsigned int lane=0;lane<width;++lane) { D3DXCrtExtended a,b,value;Load(a,0x4000000000000000ull);U64 exponent;memcpy(&exponent,expected+16+lane*8,8);Load(b,exponent);Power(value,a,b);U64 result=Store(value);memcpy(expected+offset+lane*8,&result,8); }
    State before=Snapshot();int error=*D3DXCrtErrno();Trial state;Prepare(state,c);ug2CrtSseEnabled8BA480=0;
    unsigned int code[]={1,0x10500000|width,1,0,3,0,0,4,0},literal[]={1,0,0};int result=UG2FxExecuteVm(code,literal,reinterpret_cast<unsigned int>(actual+16),0,0,reinterpret_cast<unsigned int>(actual+offset),0,0,0,0,0,0,0,0);State after=Snapshot();calls+=width;
    return !result&&!memcmp(expected,actual,96)&&SameState(before,after)&&error==*D3DXCrtErrno();
}
}
extern "C" void __cdecl UG2FxCrtPowerSseBody(double *out,D3DXCrtDoubleBits a,D3DXCrtDoubleBits b) { U64 x,y,result;memcpy(&x,&a,8);memcpy(&y,&b,8);Sse(x,y,result);memcpy(out,&result,8); }
extern "C" void __stdcall UG2FxCrtOsRaiseException(unsigned int code,unsigned int flags,unsigned int count,unsigned int *arguments) { if (flags||count!=1) __builtin_trap();Delivery(code,reinterpret_cast<unsigned int **>(arguments)); }
int main(int argc,char **argv) {
    char error[256];if (argc!=3||!MapOriginalImage(argv[1],error,sizeof(error))) return 2;FILE *f=fopen(argv[2],"rb");char magic[8];unsigned int count;
    if (!f||fread(magic,1,8,f)!=8||memcmp(magic,"UG2FXPI1",8)||fread(&count,4,1,f)!=1) return 2;unsigned int calls=0,sse=0,raises=0;
    for (unsigned int i=0;i<count;++i) {
        Case c;if (fread(&c,1,sizeof(c),f)!=sizeof(c)||c.id!=i) return 2;current=&c;
        for (unsigned int operation=0;operation<11;++operation) {
            Trial reference,actual;Run(reference,c,false,operation);Run(actual,c,true,operation);sse+=actual.sseCalled;raises+=actual.raised;
            if (!Same(reference,actual)) { fprintf(stderr,"hardware CRT power mismatch: row %u body %u base %llx exponent %llx flags %x/%x control %x/%x ret %x/%x output %d value %d owner %d integer %d/%d sse %d/%d event %d errno %x/%x\n",i,operation,c.base,c.exponent,reference.state.env[2],actual.state.env[2],reference.state.env[0],actual.state.env[0],reference.integer,actual.integer,memcmp(&reference.output,&actual.output,10),memcmp(&reference.value,&actual.value,10),memcmp(reference.state.live,actual.state.live,20),reference.integer,actual.integer,reference.sseCalled,actual.sseCalled,memcmp(reference.event,actual.event,sizeof(actual.event)),reference.error,actual.error);return 1; }
        }
        if (!Bridge(c)) { fprintf(stderr,"hardware CRT power bridge mismatch: row %u\n",i);return 1; }
        if (i<512&&!Vm(c,calls)) { fprintf(stderr,"hardware CRT power VM mismatch: row %u\n",i);return 1; }
    }
    if (fgetc(f)!=EOF) return 2;fclose(f);asm volatile("fninit");printf("CRT power hardware: %u body/instruction-model checks, %u extended bridges, 512 VM graphs / %u calls, %u controlled SSE calls, %u OS records; zero mismatches\n",count*11,count,calls,sse,raises);return 0;
}
