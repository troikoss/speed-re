#include "EffectVm.h"
#ifdef _MSC_VER
typedef __int64 VmInteger;
#else
typedef long long VmInteger;
#endif

// Original 0x688ce2..0x688d2b. Narrow at most three input doubles to floats,
// zero-fill the remaining words, then call the original operation dependency.
extern "C" void __stdcall D3DXEffectVmOperation502(unsigned int count,double **inputs,double *output) {
    float values[3];unsigned int i=0;
    for (;i<count && i<3;++i) values[i]=static_cast<float>(inputs[0][i]);
    for (;i<3;++i) values[i]=0.0f;
    *output=UG2FxVmOperation63F2B5(values);
}
// Original 0x688e79..0x68978c. Preserve pointer-list and scratch lifetimes,
// operand/output rules, low-word widths and each original arithmetic order.
extern "C" int __stdcall D3DXEffectVm(unsigned int *program,unsigned int *literals,unsigned int base3,unsigned int base2,unsigned int base9,
    unsigned int base4,unsigned int base6,unsigned int base5,unsigned int mask3,unsigned int mask2,unsigned int mask9,unsigned int mask4,unsigned int mask6,unsigned int mask5) {
    double scratch[256];double *inputs[256],*output;
    unsigned int instructions=program[0],literalCount=literals[0],*cursor=program+1;
    for (unsigned int instruction=0;instruction<instructions;++instruction) {
        unsigned int operation=*cursor++,inputCount=*cursor++,width=operation&0xfffff;
        for (unsigned int input=0;input<inputCount;++input) {
            unsigned int terms=*cursor++,index=0;
            for (;terms;--terms) {
                unsigned int kind=*cursor++,offset=*cursor++;index=offset+index*4;
                if (kind==7) index=static_cast<unsigned int>(static_cast<VmInteger>(scratch[index&0xff]));
                else if (kind==1) index=static_cast<unsigned int>(static_cast<VmInteger>(*reinterpret_cast<double *>(literals+1+(index%literalCount)*2)));
                else if (kind==2) index=static_cast<unsigned int>(static_cast<VmInteger>(*reinterpret_cast<double *>(base2+(index&mask2)*8)));
                else if (kind==3) index=static_cast<unsigned int>(static_cast<VmInteger>(*reinterpret_cast<double *>(base3+(index&mask3)*8)));
                else if (kind==4) index=static_cast<unsigned int>(static_cast<VmInteger>(*reinterpret_cast<double *>(base4+(index&mask4)*8)));
                else if (kind==5) index=static_cast<unsigned int>(static_cast<VmInteger>(*reinterpret_cast<double *>(base5+(index&mask5)*8)));
                else if (kind==6) index=static_cast<unsigned int>(static_cast<VmInteger>(*reinterpret_cast<double *>(base6+(index&mask6)*8)));
            }
            unsigned int kind=*cursor++,offset=*cursor++;index=offset+index*4;
            if (kind==7) inputs[input]=scratch+(index&0xff);
            else if (kind==2) inputs[input]=reinterpret_cast<double *>(base2+(index&mask2)*8);
            else if (kind==3) inputs[input]=reinterpret_cast<double *>(base3+(index&mask3)*8);
            else if (kind==1) inputs[input]=reinterpret_cast<double *>(literals+1+(index%literalCount)*2);
            else if (kind==9) inputs[input]=reinterpret_cast<double *>(base9+(index&mask9)*4);
            else if (kind==4) inputs[input]=reinterpret_cast<double *>(base4+(index&mask4)*8);
            else if (kind==5) inputs[input]=reinterpret_cast<double *>(base5+(index&mask5)*8);
            else if (kind==6) inputs[input]=reinterpret_cast<double *>(base6+(index&mask6)*8);
        }
        ++cursor;unsigned int outputKind=*cursor++,outputIndex=*cursor++;
        if (outputKind==7) output=scratch+(outputIndex&0xff);
        else if (outputKind==4) output=reinterpret_cast<double *>(base4+(outputIndex&mask4)*8);
        else if (outputKind==5) output=reinterpret_cast<double *>(base5+(outputIndex&mask5)*8);
        else if (outputKind==6) output=reinterpret_cast<double *>(base6+(outputIndex&mask6)*8);
        unsigned int opcode=operation&0x7ff00000;
        if (static_cast<int>(operation)<0) {
            if (opcode==0x20000000 || opcode==0x20100000 || opcode==0x20200000 || opcode==0x20300000 || opcode==0x20400000 || opcode==0x20500000 || opcode==0x20600000) {
                for (unsigned int lane=0;lane<width;++lane) {
                    if (opcode==0x20000000) { volatile long double value=UG2_FX_VM_LT(*inputs[0],inputs[1][lane]) ? *inputs[0] : inputs[1][lane];output[lane]=value; }
                    else if (opcode==0x20100000) { volatile long double value=UG2_FX_VM_GT(*inputs[0],inputs[1][lane]) ? *inputs[0] : inputs[1][lane];output[lane]=value; }
                    else if (opcode==0x20200000) output[lane]=UG2_FX_VM_LT(*inputs[0],inputs[1][lane]) ? 1.0 : 0.0;
                    else if (opcode==0x20300000) output[lane]=UG2_FX_VM_GE(*inputs[0],inputs[1][lane]) ? 1.0 : 0.0;
                    else if (opcode==0x20400000) UG2_FX_VM_STORE_ADD(output[lane],inputs[1][lane],*inputs[0]);
                    else if (opcode==0x20500000) UG2_FX_VM_STORE_MUL(output[lane],inputs[1][lane],*inputs[0]);
                    else UG2_FX_VM_STORE_ATAN2(output[lane],*inputs[0],inputs[1][lane]);
                }
            } else if (opcode==0x50000000) {
                *output=0.0;
                for (unsigned int lane=0;lane<width;++lane) UG2_FX_VM_STORE_MADD(*output,inputs[1][lane],*inputs[0],*output);
            }
        } else if (opcode==0x10000000) {
            unsigned int *to=reinterpret_cast<unsigned int *>(output),*from=reinterpret_cast<unsigned int *>(inputs[0]);
            for (unsigned int word=0;word<width*2;++word) to[word]=from[word];
        } else if (opcode==0x10100000) {
            for (unsigned int lane=0;lane<width;++lane) UG2_FX_VM_STORE_NEG(output[lane],inputs[0][lane]);
        } else if (opcode==0x10300000) {
            for (unsigned int lane=0;lane<width;++lane) UG2_FX_VM_STORE_RECIP(output[lane],inputs[0][lane]);
        } else if (opcode==0x10400000) {
            for (unsigned int lane=0;lane<width;++lane) { long double value=UG2FxVmFloor(inputs[0][lane]);UG2_FX_VM_STORE_FRACTION(output[lane],inputs[0][lane],value); }
        } else if (opcode==0x10500000) {
            for (unsigned int lane=0;lane<width;++lane) output[lane]=UG2FxVmPower(ug2VmTwo787AD0,inputs[0][lane]);
        } else if (opcode==0x10600000) {
            for (unsigned int lane=0;lane<width;++lane) { if (UG2_FX_VM_GE(*inputs[lane],ug2VmZero786E60)) UG2_FX_VM_STORE_LOG(output[lane],inputs[0][lane]);else output[lane]=0.0; }
        } else if (opcode==0x10700000) {
            for (unsigned int lane=0;lane<width;++lane) UG2_FX_VM_STORE_RSQRT(output[lane],inputs[0][lane]);
        } else if (opcode==0x10800000) {
            for (unsigned int lane=0;lane<width;++lane) UG2_FX_VM_STORE_SIN(output[lane],inputs[0][lane]);
        } else if (opcode==0x10900000) {
            for (unsigned int lane=0;lane<width;++lane) UG2_FX_VM_STORE_COS(output[lane],inputs[0][lane]);
        } else if (opcode==0x10a00000) {
            for (unsigned int lane=0;lane<width;++lane) output[lane]=UG2FxVmAsin(inputs[0][lane]);
        } else if (opcode==0x10b00000) {
            for (unsigned int lane=0;lane<width;++lane) output[lane]=UG2FxVmAcos(inputs[0][lane]);
        } else if (opcode==0x10c00000) {
            for (unsigned int lane=0;lane<width;++lane) UG2_FX_VM_STORE_ATAN(output[lane],inputs[0][lane]);
        } else if (opcode==0x20000000 || opcode==0x20100000 || opcode==0x20200000 || opcode==0x20300000 || opcode==0x20400000 || opcode==0x20500000 || opcode==0x20600000) {
            for (unsigned int lane=0;lane<width;++lane) {
                if (opcode==0x20000000) { volatile long double value=UG2_FX_VM_LT(inputs[0][lane],inputs[1][lane]) ? inputs[0][lane] : inputs[1][lane];output[lane]=value; }
                else if (opcode==0x20100000) { volatile long double value=UG2_FX_VM_GT(inputs[0][lane],inputs[1][lane]) ? inputs[0][lane] : inputs[1][lane];output[lane]=value; }
                else if (opcode==0x20200000) output[lane]=UG2_FX_VM_LT(inputs[0][lane],inputs[1][lane]) ? 1.0 : 0.0;
                else if (opcode==0x20300000) output[lane]=UG2_FX_VM_GE(inputs[0][lane],inputs[1][lane]) ? 1.0 : 0.0;
                else if (opcode==0x20400000) UG2_FX_VM_STORE_ADD(output[lane],inputs[0][lane],inputs[1][lane]);
                else if (opcode==0x20500000) UG2_FX_VM_STORE_MUL(output[lane],inputs[0][lane],inputs[1][lane]);
                else UG2_FX_VM_STORE_ATAN2(output[lane],inputs[0][lane],inputs[1][lane]);
            }
        } else if (opcode==0x30000000) {
            for (unsigned int lane=0;lane<width;++lane) { volatile long double value=UG2_FX_VM_GE(inputs[0][lane],ug2VmZero786E60) ? inputs[1][lane] : inputs[2][lane];output[lane]=value; }
        } else if (opcode==0x50000000) {
            *output=0.0;
            for (unsigned int lane=0;lane<width;++lane) UG2_FX_VM_STORE_MADD(*output,inputs[0][lane],inputs[1][lane],*output);
        } else if (opcode==0x50200000) D3DXEffectVmOperation502(width,inputs,output);
        else if (opcode==0x70e00000) {
            *output=0.0;unsigned int half=inputCount>>1;
            for (unsigned int input=0;input<half;++input) UG2_FX_VM_STORE_MADD(*output,*inputs[input],*inputs[half+input],*output);
        }
    }
    return 0;
}
