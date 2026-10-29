#include "OriginalImage.h"
#include "../../Speed/Indep/Libs/D3DX/CrtPowerSse.h"
#include <stdio.h>
#include <string.h>
// Test-only binary protocol. Opcodes 0..7 and 9..10 are independent physical
// instruction oracles for emulated original control flow; opcode 8 runs source.
// No retail machine code is mapped executable or called by this process.
namespace {
struct Request { unsigned int operation,control,mxcsr,status;D3DXCrtSseValue a,b; };
struct Response { D3DXCrtSseValue a;D3DXCrtExtended value;unsigned int control,mxcsr,status,tag;unsigned char owners[20];unsigned int error,called;unsigned char record[32];unsigned int callControl,callMxcsr,callStatus; };
typedef char ProtocolSize[(sizeof(Request)==48&&sizeof(Response)==116)?1:-1];
struct Stored80 { unsigned char bytes[10]; };
Response *current;
void State(unsigned int &control,unsigned int &mxcsr,unsigned int &status) {
    unsigned short cw,sw;asm volatile("fnstcw %0; fnstsw %1; stmxcsr %2" : "=m"(cw),"=m"(sw),"=m"(mxcsr));control=cw;status=sw&0x3f;
}
void Reset(const Request &q,bool owners) {
    unsigned short cw=q.control,env[14];asm volatile("fninit; fldcw %0; ldmxcsr %1" : : "m"(cw),"m"(q.mxcsr) : "st");
    if (owners) asm volatile("fld1; fld1; fchs" : : : "st");
    asm volatile("fnstenv %0" : "=m"(env));env[2]=(env[2]&0x3800)|(q.status&0x3f);asm volatile("fldenv %0" : : "m"(env));
}
__attribute__((target("sse2"))) void Instruction(const Request &q,Response &r) {
#define BINARY(OP) asm volatile("movdqu %1,%%xmm4; movdqu %2,%%xmm5; " OP " %%xmm5,%%xmm4; movdqu %%xmm4,%0" : "=m"(r.a) : "m"(q.a),"m"(q.b) : "xmm4","xmm5")
    switch(q.operation) {
    case 0:BINARY("addsd");break;case 1:BINARY("subsd");break;
    case 2:BINARY("mulsd");break;case 3:BINARY("divsd");break;
    case 4:BINARY("addpd");break;case 5:BINARY("mulpd");break;
    case 6:asm volatile("movdqu %1,%%xmm4; cvtdq2pd %%xmm4,%%xmm5; movdqu %%xmm5,%0" : "=m"(r.a) : "m"(q.a) : "xmm4","xmm5");break;
    case 7:asm volatile("movdqu %1,%%xmm5; cvtsd2si %%xmm5,%0" : "=r"(r.a.word[0]) : "m"(q.a) : "xmm5");break;
    }
#undef BINARY
}
}
// Observe the same default-zero callback used by the original verifier. Its
// publication boundary, record and FP state are compared, with no record edits.
extern "C" int __cdecl D3DXCrtDefaultMathError(D3DXCrtMathException *record) {
    ++current->called;memcpy(current->record,record,32);State(current->callControl,current->callMxcsr,current->callStatus);return 0;
}
int main(int argc,char **argv) {
    char error[256];if (argc!=2||!MapOriginalImage(argv[1],error,sizeof(error))) return 2;
    setvbuf(stdin,0,_IONBF,0);setvbuf(stdout,0,_IONBF,0);Request q;
    while (fread(&q,1,sizeof(q),stdin)==sizeof(q)) {
        Response r;memset(&r,0,sizeof(r));current=&r;
        if ((q.mxcsr&0x1f80)!=0x1f80||(q.control&0x3f)!=0x3f) return 3;
        Reset(q,q.operation==8);
        if (q.operation<8) Instruction(q,r);
        else if (q.operation==8) {
            D3DXCrtDoubleBits a={q.a.word[0],q.a.word[1]},b={q.a.word[2],q.a.word[3]};
            ug2CrtMathErrorCallback81AA5C=0x727e6b;*D3DXCrtErrno()=0x12345678;
            D3DXCrtPowerSse80(&r.value,a,b);r.error=*D3DXCrtErrno();
        } else if (q.operation==9) asm volatile("fldl %1; fstpt %0" : "=m"(r.value) : "m"(*reinterpret_cast<const double *>(&q.a)) : "st");
        else if (q.operation==10) asm volatile("fldt %1; fstpl %0" : "=m"(*reinterpret_cast<double *>(&r.a)) : "m"(*reinterpret_cast<const D3DXCrtExtended *>(&q.a)) : "st");
        else return 4;
        State(r.control,r.mxcsr,r.status);
        if (q.operation==8) {
            unsigned short env[14];asm volatile("fnstenv %0; fldenv %0" : "=m"(env));r.tag=env[4];
            asm volatile("fld %%st(0); fstpt %0; fld %%st(1); fstpt %1" : "=m"(*reinterpret_cast<Stored80 *>(r.owners)),"=m"(*reinterpret_cast<Stored80 *>(r.owners+10)) : : "st");
            // TOP accompanies the low exception bits in this response only.
            r.status|=env[2]&0x3800;
        }
        asm volatile("fninit");if (fwrite(&r,1,sizeof(r),stdout)!=sizeof(r)) return 5;
    }
    return ferror(stdin) ? 6 : 0;
}
