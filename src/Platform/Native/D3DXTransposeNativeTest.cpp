#include "../../Speed/Indep/Src/Physics/D3DXMatrixScalar.h"
#include <fenv.h>
#include <stdio.h>
#include <string.h>

// Hardware check of the x87 conversion lanes seen in 0x63d132. Unicorn does
// not quiet signaling NaNs or raise invalid for its FLD/FSTP implementation.
// The fixed lane sets below come from the original instructions, independently
// of the recovered source. No original executable code is mapped or run here.
int main() {
    const unsigned int payloads[]={0,0x80000000u,1,0x80000001u,0x007fffffu,
        0x00800000u,0x7f7fffffu,0xff7fffffu,0x7f800000u,0xff800000u,
        0x7f800123u,0xff800123u,0x7fbfffffu,0x7fc00123u,0xffc00123u};
    const int rounding[]={FE_TONEAREST,FE_DOWNWARD,FE_UPWARD,FE_TOWARDZERO};
    fenv_t initial;
    fegetenv(&initial);
    unsigned int cases=0;
    for (unsigned int r=0;r<4;++r) {
        fesetround(rounding[r]);
        for (unsigned int p=0;p<sizeof(payloads)/sizeof(payloads[0]);++p) {
            for (unsigned int lane=0;lane<16;++lane) {
                for (unsigned int alias=0;alias<2;++alias) {
                    WheelMatrix input,output;
                    unsigned int words[16],expected[16],actual[16];
                    for (unsigned int i=0;i<16;++i) words[i]=0x3f800000u+i;
                    words[lane]=payloads[p];
                    for (unsigned int i=0;i<16;++i) expected[(i%4)*4+i/4]=words[i];
                    const bool converted=lane==1 || lane==4 || lane==2 || lane==3 || lane==6 || lane==7 || lane==11;
                    const bool signaling=(payloads[p]&0x7f800000u)==0x7f800000u &&
                        (payloads[p]&0x007fffffu)!=0 && (payloads[p]&0x00400000u)==0;
                    if (converted && signaling) expected[(lane%4)*4+lane/4]|=0x00400000u;
                    memcpy(&input,words,sizeof(words));
                    memset(&output,0xa5,sizeof(output));
                    WheelMatrix *target=alias ? &input : &output;
                    feclearexcept(FE_ALL_EXCEPT);
                    WheelMatrix *result=D3DXMatrixTransposeScalar(target,&input);
                    const bool invalid=fetestexcept(FE_INVALID)!=0;
                    memcpy(actual,target,sizeof(actual));
                    if (result!=target || memcmp(actual,expected,sizeof(actual)) || invalid!=(converted && signaling)) {
                        fprintf(stderr,"native behavior mismatch: rounding=%u payload=%08x lane=%u alias=%u invalid=%u\n",
                            r,payloads[p],lane,alias,invalid);
                        fesetenv(&initial);
                        return 1;
                    }
                    ++cases;
                }
            }
        }
    }
    fesetenv(&initial);
    printf("D3DX scalar transpose: %u native lane/payload/rounding cases passed\n",cases);
    return 0;
}
