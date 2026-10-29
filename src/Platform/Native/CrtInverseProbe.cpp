#include "OriginalImage.h"
#include "../../Speed/Indep/Libs/D3DX/CrtInverse.h"
#include <stdio.h>
#include <string.h>
// Test-only original-instruction and policy models. Shared CRT mask/record/
// errno bodies from the preceding checkpoint remain verified dependencies.
extern "C" long double __cdecl ProbeAsinBits(D3DXCrtDoubleBits) __asm__("UG2FxVmAsin");
extern "C" long double __cdecl ProbeAcosBits(D3DXCrtDoubleBits) __asm__("UG2FxVmAcos");
namespace {
typedef unsigned long long U64;
struct Case { unsigned int id,control,mxcsr,status,saved,bypass,mode,operation,type,name,edit,redirect,recordBits,seed,alias;U64 bits,other,result;D3DXCrtExtended input; };
typedef char FixtureSize[sizeof(Case)==96 ? 1 : -1];
struct State { unsigned short env[14];unsigned int mxcsr; };
struct Trial { D3DXCrtExtended input,output;D3DXCrtMathException error;unsigned int saved,alternate[32],event[13],code;bool raised;int integer,errnoValue;State state,eventState; };
const Case *current;Trial *trial;unsigned int kind;
State Snapshot() { State s;asm volatile("fnstenv %0; stmxcsr %1" : "=m"(s.env),"=m"(s.mxcsr));return s; }
void Reset(const Case &c) { unsigned short word=c.control;asm volatile("fninit; fldcw %0; ldmxcsr %1" : : "m"(word),"m"(c.mxcsr) : "st");State s=Snapshot();s.env[2]=c.status;asm volatile("fldenv %0" : : "m"(s.env)); }
bool SameState(const State &a,const State &b) { return a.env[0]==b.env[0]&&(a.env[2]&0x383f)==(b.env[2]&0x383f)&&a.env[4]==b.env[4]&&a.mxcsr==b.mxcsr; }
void Store64(double *out,const D3DXCrtExtended *value) { asm volatile("fldt %1; fstpl %0" : "=m"(*out) : "m"(*value) : "st"); }
void Load64(D3DXCrtExtended *out,const double *value) { asm volatile("fldl %1; fstpt %0" : "=m"(*out) : "m"(*value) : "st"); }
void Normalize(D3DXCrtExtended *out,const D3DXCrtExtended *value) { asm volatile("fldt %1; fstpt %0" : "=m"(*out) : "m"(*value) : "st"); }
void Control(unsigned int word) { unsigned short value=word;asm volatile("fldcw %0" : : "m"(value)); }
unsigned int Status() { unsigned short status;asm volatile("fstsw %0" : "=m"(status));return status; }
unsigned int ReadControl() { unsigned short word;asm volatile("fstcw %0" : "=m"(word));return word; }
unsigned int Nan(D3DXCrtExtended *value,unsigned int high) {
    if (high&0x80000) return 7;const double one=1;asm volatile("fldt %0; faddl %1; fstpt %0" : "+m"(*value) : "m"(one) : "st");return 1;
}
void Dispatch(unsigned int operation,D3DXCrtMathException *error,unsigned short *saved) {
    unsigned int control=*saved,flags=0;const int types[]={1,2,3,4,5,8};const unsigned int logical[]={8,4,17,18,8,16};
    for (unsigned int j=0;j<6;++j) if (error->type00==types[j]) flags=logical[j];
    if (error->type00==7) error->type00=1;
    if (flags&&!D3DXCrtApplyMasks(flags,&error->result18,control)) {
        unsigned int record[32]={0};
        if (operation==16||operation==22||operation==29) { asm volatile("fldl %1; fstpl %0" : "=m"(*reinterpret_cast<double *>(record+12)) : "m"(error->input10) : "st");record[16]=3; }
        D3DXCrtRaiseException(record,&control,flags,operation,&error->input08,&error->result18);
    }
    D3DXCrtRawControl(control,0xffff);
    // The original default matherr body returns zero. Its established native
    // body, errno and mask/record operations are dependencies of this oracle.
    if (error->type00!=8&&!ug2CrtMathErrorFlag81ACE0&&D3DXCrtDefaultMathError(error)) return;
    D3DXCrtSetMathErrno(error->type00);
}
void Start(unsigned int operation,unsigned int type,const char *name,U64 input,unsigned int saved,D3DXCrtExtended *value) {
    D3DXCrtMathException error={0};error.type00=type;Store64(&error.result18,value);error.name04=name;memcpy(&error.input08,&input,8);
    Dispatch(operation,&error,reinterpret_cast<unsigned short *>(&saved));Load64(value,&error.result18);if (static_cast<unsigned short>(saved)!=0x27f) Control(saved);
}
void Exit(unsigned int operation,const char *name,U64 input,unsigned int saved,D3DXCrtExtended *value) {
    if (static_cast<unsigned short>(saved)==0x27f) return;
    if ((saved&32)&&(Status()&32)) Start(operation,8,name,input,saved,value);else Control(saved);
}
void Inverse(D3DXCrtExtended *out,const D3DXCrtExtended *input,bool cosine) {
    double argument;Store64(&argument,input);U64 bits;memcpy(&bits,&argument,8);unsigned int exponent=static_cast<unsigned int>(bits>>52&0x7ff),high=static_cast<unsigned int>(bits>>32),saved=ReadControl(),type=1;
    D3DXCrtExtended value=*input;bool error=false;
    if (exponent==0x7ff) {
        if (bits&0xfffffffffffffull) type=Nan(&value,high);else memcpy(&value,&ug2CrtIndefinite81A4D0,10);error=true;
    } else {
        if (saved!=0x27f) Control((saved&0x300)|0x7f);
        if (exponent<0x3ff) {
            // Exact original arithmetic stack order, independently of the
            // production adapters and their separate extended storage calls.
            if (cosine) asm volatile("fldt %1; fld1; fadd %%st(1),%%st; fld1; fsub %%st(2),%%st; fmulp; fsqrt; fxch; fpatan; fstpt %0" : "=m"(value) : "m"(*input) : "st");
            else asm volatile("fldt %1; fld1; fadd %%st(1),%%st; fld1; fsub %%st(2),%%st; fmulp; fsqrt; fpatan; fstpt %0" : "=m"(value) : "m"(*input) : "st");
        } else if (exponent>0x3ff||(bits&0xfffffffffffffull)) { memcpy(&value,&ug2CrtIndefinite81A4D0,10);error=true; }
        else if (cosine) {
            if (bits>>63) asm volatile("fldpi; fstpt %0" : "=m"(value) : : "st");else asm volatile("fldz; fstpt %0" : "=m"(value) : : "st");
        } else {
            if (bits>>63) asm volatile("fldt %1; fchs; fstpt %0" : "=m"(value) : "m"(ug2CrtHalfPi81A4DA) : "st");
            else memcpy(&value,&ug2CrtHalfPi81A4DA,10);
        }
    }
    unsigned int operation=cosine ? 13 : 14;const char *name=reinterpret_cast<const char *>(cosine ? 0x819cc0 : 0x819f00);
    if (ug2CrtBypassMathExit8B8424) { if (saved!=0x27f) Control(saved); }
    else if (error) Start(operation,type,name,bits,saved,&value);else Exit(operation,name,bits,saved,&value);
    memcpy(out,&value,10);
}
void Prepare(Trial &t,const Case &c) {
    memset(&t,0,sizeof(t));t.input=c.input;t.saved=c.saved;unsigned int seed=c.seed;
    for (unsigned int j=0;j<32;++j) { seed=1664525u*seed+1013904223u;t.alternate[j]=seed; }
    t.error.type00=c.type;t.error.name04=reinterpret_cast<const char *>(c.name);memcpy(&t.error.input08,&c.bits,8);memcpy(&t.error.input10,&c.other,8);memcpy(&t.error.result18,&c.result,8);
    ug2CrtBypassMathExit8B8424=c.bypass;ug2CrtMathErrorFlag81ACE0=c.mode;*D3DXCrtErrno()=c.seed;trial=&t;Reset(c);
    // Model the original incoming ST0 load, including NaN quieting. The explicit
    // extended API receives the value already present in that original register.
    Normalize(&t.input,&c.input);
}
void Run(Trial &t,const Case &c,bool actual,unsigned int operation) {
    Prepare(t,c);kind=operation;const char *name=reinterpret_cast<const char *>(c.name);
    switch (operation) {
    case 0:if (actual) D3DXCrtAsin80(&t.output,&t.input);else Inverse(&t.output,&t.input,false);break;
    case 1:if (actual) D3DXCrtAcos80(&t.output,&t.input);else Inverse(&t.output,&t.input,true);break;
    case 2:{ D3DXCrtDoubleBits bits;memcpy(&bits,&c.bits,8);unsigned int e=bits.high&0x7ff00000;t.integer=actual ? D3DXCrtInspectOperand(bits) : e==0x7ff00000 ? bits.high : e;break; }
    case 3:if (actual) D3DXCrtPrepareControl(c.saved);else Control((c.saved&0x300)|0x7f);break;
    case 4:t.integer=actual ? D3DXCrtPrepareNan(&t.input,c.bits>>32) : Nan(&t.input,c.bits>>32);break;
    case 5:{ D3DXCrtDoubleBits bits;memcpy(&bits,&c.bits,8);if (actual) D3DXCrtOneArgError(13,c.type,name,bits,c.saved,&t.input);else Start(13,c.type,name,c.bits,c.saved,&t.input);break; }
    case 6:if (actual) D3DXCrtQuickExit(c.saved);else if (static_cast<unsigned short>(c.saved)!=0x27f) Control(c.saved);break;
    case 7:{ D3DXCrtDoubleBits bits;memcpy(&bits,&c.bits,8);if (actual) D3DXCrtMathExit(13,name,bits,c.saved,&t.input);else Exit(13,name,c.bits,c.saved,&t.input);break; }
    case 8:{ unsigned short *saved=c.alias==0xffffffff ? reinterpret_cast<unsigned short *>(&t.saved) : reinterpret_cast<unsigned short *>(reinterpret_cast<unsigned char *>(&t.error)+c.alias);
        *saved|=0x7f;if ((*saved&0x300)==0x100) *saved&=~0x300;
        // Exercise real CRT record construction while arithmetic executes with
        // masked current hardware. The test OS handler supplies a safe finite
        // result before the dispatcher restores this independently saved mask.
        unsigned int mask=c.type==1||c.type==5 ? 1 : c.type==2 ? 4 : c.type==3 ? 8 : c.type==4 ? 16 : c.type==8 ? 32 : 0;
        if (c.id%2) *saved&=~mask;
        if (actual) D3DXCrtOneArgDispatch(c.operation,&t.error,saved);else Dispatch(c.operation,&t.error,saved);break; }
    }
    t.state=Snapshot();t.errnoValue=*D3DXCrtErrno();
}
void Delivery(unsigned int code,unsigned int **record) {
    trial->raised=true;trial->code=code;trial->eventState=Snapshot();unsigned int *w=*record;
    unsigned int fields[]={w[0]&0x1ffff,w[1],w[2],w[3],w[4],w[5],w[8]&31,w[16]&1,w[20],w[21],w[24]&31,0,0};memcpy(trial->event,fields,sizeof(fields));
    if (kind==8&&(current->operation==16||current->operation==22||current->operation==29)) { trial->event[7]=w[16]&31;trial->event[11]=w[12];trial->event[12]=w[13]; }
    if (current->edit||kind==8) {
        if (current->edit&&current->redirect) *record=trial->alternate;
        if (current->edit) (*record)[0]=current->recordBits;
        (*record)[2]=0;
        const double finite=-6.5;if (kind==8) memcpy(*record+20,&finite,8);else memcpy(*record+20,&current->other,8);
    }
}
bool Same(const Trial &a,const Trial &b) {
    return !memcmp(&a.input,&b.input,10)&&!memcmp(&a.output,&b.output,10)&&!memcmp(&a.error,&b.error,32)&&a.saved==b.saved&&!memcmp(a.alternate,b.alternate,128)&&a.integer==b.integer&&a.errnoValue==b.errnoValue&&SameState(a.state,b.state)&&a.raised==b.raised&&(!a.raised||(a.code==b.code&&!memcmp(a.event,b.event,sizeof(a.event))&&SameState(a.eventState,b.eventState)));
}
bool Vm(const Case &c,bool cosine,unsigned int &calls) {
    unsigned char expected[96],actual[96];memset(expected,0xa5,96);
    for (unsigned int j=0;j<3;++j) memcpy(expected+16+j*8,&c.bits,8);
    memcpy(actual,expected,96);const int offsets[]={-4,0,4,8,32};unsigned int offset=16+offsets[c.id%5],width=c.id%4;
    Trial reference;Prepare(reference,c);kind=cosine;
    for (unsigned int lane=0;lane<width;++lane) {
        D3DXCrtExtended input,output;Load64(&input,reinterpret_cast<const double *>(expected+16+lane*8));Inverse(&output,&input,cosine);Store64(reinterpret_cast<double *>(expected+offset+lane*8),&output);
    }
    State before=Snapshot();int error=*D3DXCrtErrno();Trial state;Prepare(state,c);
    unsigned int code[]={1,(cosine ? 0x10b00000u : 0x10a00000u)|width,1,0,3,0,0,4,0},literal[]={1,0,0};
    int result=UG2FxExecuteVm(code,literal,reinterpret_cast<unsigned int>(actual+16),0,0,reinterpret_cast<unsigned int>(actual+offset),0,0,0,0,0,0,0,0);State after=Snapshot();calls+=width;
    return !result&&!memcmp(expected,actual,96)&&SameState(before,after)&&error==*D3DXCrtErrno();
}

bool Bridge(const Case &c,bool cosine) {
    Trial expected;Prepare(expected,c);kind=cosine;D3DXCrtExtended input,output;Load64(&input,reinterpret_cast<const double *>(&c.bits));Inverse(&output,&input,cosine);long double reference;asm volatile("fldt %1; fstpt %0" : "=m"(reference) : "m"(output) : "st");State before=Snapshot();int error=*D3DXCrtErrno();
    Trial actual;Prepare(actual,c);D3DXCrtDoubleBits bits;memcpy(&bits,&c.bits,8);volatile long double result=cosine ? ProbeAcosBits(bits) : ProbeAsinBits(bits);State after=Snapshot();
    return !memcmp(&reference,const_cast<const long double *>(&result),10)&&SameState(before,after)&&error==*D3DXCrtErrno();
}
}
extern "C" void __stdcall UG2FxCrtOsRaiseException(unsigned int code,unsigned int flags,unsigned int count,unsigned int *arguments) {
    if (flags||count!=1) __builtin_trap();Delivery(code,reinterpret_cast<unsigned int **>(arguments));
}
int main(int argc,char **argv) {
    char error[256];if (argc!=3||!MapOriginalImage(argv[1],error,sizeof(error))) return 2;FILE *f=fopen(argv[2],"rb");char magic[8];unsigned int count;
    if (!f||fread(magic,1,8,f)!=8||memcmp(magic,"UG2FXAI1",8)||fread(&count,4,1,f)!=1) return 2;
    unsigned int graphs=0,calls=0,raises=0,twoArgs=0;for (unsigned int i=0;i<count;++i) {
        Case c;if (fread(&c,1,sizeof(c),f)!=sizeof(c)||c.id!=i) return 2;current=&c;
        for (unsigned int operation=0;operation<9;++operation) {
            Trial reference,actual;Run(reference,c,false,operation);Run(actual,c,true,operation);
            raises+=actual.raised;twoArgs+=actual.raised&&operation==8&&(c.operation==16||c.operation==22||c.operation==29);
            if (!Same(reference,actual)) { fprintf(stderr,"hardware CRT inverse mismatch: row %u body %u value %llx flags %x/%x control %x/%x return %x/%x input %d output %d error %d event %d errno %x/%x\n",i,operation,c.bits,reference.state.env[2],actual.state.env[2],reference.state.env[0],actual.state.env[0],reference.integer,actual.integer,memcmp(&reference.input,&actual.input,10),memcmp(&reference.output,&actual.output,10),memcmp(&reference.error,&actual.error,32),memcmp(reference.event,actual.event,sizeof(actual.event)),reference.errnoValue,actual.errnoValue);return 1; }
        }
        if (!Bridge(c,false)||!Bridge(c,true)) { fprintf(stderr,"hardware CRT inverse bridge mismatch: row %u value %llx\n",i,c.bits);return 1; }
        if (i<512) { if (!Vm(c,false,calls)||!Vm(c,true,calls)) { fprintf(stderr,"hardware CRT inverse VM mismatch: row %u value %llx\n",i,c.bits);return 1; }graphs+=2; }
    }
    if (fgetc(f)!=EOF) return 2;fclose(f);asm volatile("fninit");printf("CRT inverse hardware: %u rows, %u body/instruction-model checks, %u extended-return bridge checks, %u VM graphs / %u calls, %u OS fixtures / %u two-argument records; zero mismatches\n",count,count*9,count*2,graphs,calls,raises,twoArgs);return 0;
}
