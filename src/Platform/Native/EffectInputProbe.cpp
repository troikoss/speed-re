#include "OriginalImage.h"
#include "NativeFxInput.h"
#include "../../Speed/Indep/Libs/D3DX/EffectInput.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <stdint.h>

namespace {
unsigned int Checksum(const unsigned char *data,unsigned int size) {
    unsigned int hash=2166136261u;
    for (unsigned int i=0;i<size;++i) hash=(hash^data[i])*16777619u;
    return hash;
}
}
int main(int argc,char **argv) {
    if (argc!=2 && argc!=3) { fprintf(stderr,"usage: ug2effectinput_probe speed2.exe [capture.bin]\n");return 2; }
    char error[512];
    if (!MapOriginalImage(argv[1],error,sizeof(error))) { fprintf(stderr,"%s\n",error);return 1; }
    FILE *capture=argc==3 ? fopen(argv[2],"wb") : 0;
    if (argc==3 && !capture) return 15;
    if (capture && fwrite("UG2FXIN1",1,8,capture)!=8) return 15;
    unsigned int total=0;
    for (unsigned int i=0;i<23;++i) {
        // Read the resource-name pointer from the verified original recipe table.
        const char *name=*reinterpret_cast<const char *const *>(0x7ff7d0+i*0x8c+0x84);
        std::vector<unsigned short> wide;
        for (const char *c=name;*c;++c) wide.push_back(static_cast<unsigned char>(*c));
        wide.push_back(0);
        unsigned int checksum=0,length=0;
        for (unsigned int mode=0;mode<2;++mode) {
            D3DXEffectResource resource;resource.Construct();
            if (resource.Load(0,mode ? static_cast<const void *>(wide.data()) : name,1,mode)!=0) return 3;
            unsigned int retained=resource.handle00;
            unsigned int expected=Checksum(resource.data04,resource.size08);
            D3DXEffectBuffer *buffer=0;
            if (D3DXEffectBufferCreateForward(resource.size08,&buffer)!=0) return 4;
            length=D3DXEffectBufferLength(buffer);
            if (length!=resource.size08 || *reinterpret_cast<const unsigned int *>(resource.data04)!=0xfeff0901) return 5;
            memcpy(D3DXEffectBufferPointer(buffer),resource.data04,length);
            checksum=Checksum(D3DXEffectBufferPointer(buffer),length);
            if (checksum!=expected) return 6;
            if (capture) {
                unsigned int header[4]={i,mode,static_cast<unsigned int>(strlen(name)),length};
                if (fwrite(header,1,sizeof(header),capture)!=sizeof(header) ||
                    fwrite(name,1,header[2],capture)!=header[2] ||
                    fwrite(D3DXEffectBufferPointer(buffer),1,length,capture)!=length) return 15;
            }
            D3DXEffectBuffer *queried=0;
            if (D3DXEffectBufferQuery(buffer,ug2FxBufferIid7B7430,&queried)!=0 || queried!=buffer) return 7;
            if (D3DXEffectBufferRelease(queried)!=1 || D3DXEffectBufferRelease(buffer)!=0) return 8;
            resource.Destroy();
            if (resource.handle00!=retained || Checksum(resource.data04,length)!=expected || NativeFxAllocationCount()) return 9;
            // Reload clears the old handle via the recovered Clear call, while
            // the underlying module resource bytes retain their lifetime.
            if (resource.Load(0,name,0,0)!=0 || Checksum(resource.data04,length)!=expected) return 10;
            resource.Clear();if (resource.handle00 || resource.data04 || resource.size08) return 11;
            total+=length;
        }
        printf("effect input %u %s %u %08x\n",i,name,length,checksum);
    }
    D3DXEffectResource missing;missing.Construct();
    if (static_cast<unsigned int>(missing.Load(0,"MISSING_EFFECT",1,0))!=0x88760b59) return 12;
    // The PE's numeric icon and #number lookup exercise the adapter's two
    // integer-identifier forms without treating them as compiled effects.
    bool numericTested=false;
    for (unsigned int id=1;id<256;++id) {
        unsigned int info=UG2FxFindResourceA(0,reinterpret_cast<const void *>(id),3);
        if (!info) continue;
        char numeric[16];snprintf(numeric,sizeof(numeric),"#%u",id);
        if (UG2FxFindResourceA(0,numeric,3)!=info || !UG2FxSizeofResource(0,info)) return 13;
        numericTested=true;break;
    }
    if (!numericTested) return 14;
    if (capture && fclose(capture)!=0) return 15;
    printf("effect inputs: 23 recipes, 46 ANSI/UTF16 buffer cycles, %u copied bytes, %u live allocations\n",total,static_cast<unsigned int>(NativeFxAllocationCount()));
    return 0;
}
