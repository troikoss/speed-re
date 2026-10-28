#include "../../Speed/Indep/Libs/D3DX/EffectEvaluationCopy.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <fpu_control.h>

namespace {
unsigned int Address(const void *p) { return reinterpret_cast<unsigned int>(p); }
unsigned short Status() { unsigned short value;asm volatile("fnstsw %0" : "=am"(value));return value; }
void Clear() { asm volatile("fnclex"); }
// Hardware oracle for the exact FLD m32 / FSTP m64 conversion sequences in
// 0x662ecc/0x663265/0x65fab5/0x65febf. This test does not execute mapped PE code.
double Convert(unsigned int bits) { double output;asm volatile("flds %1; fstpl %0" : "=m"(output) : "m"(bits) : "st");return output; }
void Copy(unsigned int kind,D3DXEffectBaseRecord *parameter,unsigned int **type,unsigned int **source,double **destination,unsigned int base,unsigned int offset,unsigned int *left,unsigned int elements,unsigned int *written) {
    if (kind==0) D3DXEffectWalkEvaluationVectors(type,source,destination,base,offset,left,elements);
    else if (kind==1) D3DXEffectWalkEvaluationScalars(type,source,destination,base,offset,left,elements);
    else {
        unsigned int entry[5]={0,0,0,offset,0};
        if (kind==2) D3DXEffectCopyEvaluationVectors(parameter,*destination,*left,base,Address(entry),written,elements);
        else D3DXEffectCopyEvaluationScalars(parameter,*destination,*left,base,Address(entry),written,elements);
    }
}
bool Hardware() {
    const unsigned int edge[]={0,0x80000000,1,0x007fffff,0x00800000,0x3f800000,0xbf800000,0x7f7fffff,0xff7fffff,0x7f800000,0xff800000,0x7fc00000,0x7fc12345,0xffc54321,0x7f800001,0x7fbfffff,0xff800001,0xffbfffff};
    unsigned int random=0x65fab5,checks=0;fpu_control_t saved;_FPU_GETCW(saved);
    for (unsigned int precision=0;precision<2;++precision) {
        fpu_control_t control=precision ? 0x27f : 0x7f;_FPU_SETCW(control);
        for (unsigned int i=0;i<512;++i) {
            unsigned int input[4];for (unsigned int lane=0;lane<4;++lane) { random^=random<<13;random^=random>>17;random^=random<<5;input[lane]=i<sizeof(edge)/sizeof(edge[0]) ? edge[(i+lane)%(sizeof(edge)/sizeof(edge[0]))] : random; }
            double expected[4];Clear();for (unsigned int lane=0;lane<4;++lane) expected[lane]=Convert(input[lane]);unsigned short status=Status()&0x3f;
            for (unsigned int kind=0;kind<4;++kind) {
                unsigned int descriptor[7]={3,0,0,0,1,1,1},*type=descriptor,*source=input,left=1,written=0x12345678;double output[8],*destination=output;memset(output,0xa5,sizeof(output));D3DXEffectBaseRecord parameter;memset(&parameter,0,sizeof(parameter));parameter.value00=Address(input);parameter.value04=Address(descriptor);
                Clear();Copy(kind,&parameter,&type,&source,&destination,0,0,&left,0xffffffff,&written);unsigned short actualStatus=Status()&0x3f;fpu_control_t actual;_FPU_GETCW(actual);
                if (memcmp(expected,output,sizeof(expected))||actualStatus!=status||actual!=control||((kind<2)&&(left||type!=descriptor+7||source!=input+4||destination!=output+(kind==1 ? 1 : 4)))||((kind>=2)&&written!=1)) { fprintf(stderr,"hardware evaluation copy mismatch: precision %u case %u kind %u flags %x/%x\n",precision,i,kind,status,actualStatus);_FPU_SETCW(saved);return false; }
                for (unsigned int byte=sizeof(expected);byte<sizeof(output);++byte) if (reinterpret_cast<unsigned char *>(output)[byte]!=0xa5) { _FPU_SETCW(saved);return false; }
                ++checks;
            }
        }
    }
    Clear();_FPU_SETCW(saved);printf("effect evaluation copy: %u hardware conversions/flags/cursors, including signaling NaNs\n",checks);return true;
}
}
int main(int argc,char **argv) {
    if (argc!=3) { fprintf(stderr,"usage: ug2effectevaluationcopy_probe fixtures.bin capture.bin\n");return 2; }
    if (!Hardware()) return 1;FILE *fixtures=fopen(argv[1],"rb"),*capture=fopen(argv[2],"wb");if (!fixtures||!capture) return 3;char magic[8];unsigned int count;
    if (fread(magic,1,8,fixtures)!=8||memcmp(magic,"UG2FXEF1",8)||fread(&count,4,1,fixtures)!=1||count!=128||fwrite("UG2FXED1",1,8,capture)!=8) return 3;
    unsigned int row=0;
    for (unsigned int i=0;i<count;++i) {
        unsigned int config[7];if (fread(config,4,7,fixtures)!=7||config[0]!=i||config[6]!=0x10000) return 3;std::vector<unsigned char> initial(0x10000);if (fread(initial.data(),1,initial.size(),fixtures)!=initial.size()) return 3;
        for (unsigned int kind=0;kind<4;++kind) {
            std::vector<unsigned char> memory=initial;unsigned int address=Address(memory.data()),*type=reinterpret_cast<unsigned int *>(address+0x2000),*source=reinterpret_cast<unsigned int *>(address+0x6000),left=config[3],written=0x12345678;double *destination=reinterpret_cast<double *>(address+(config[5] ? 0x6000+config[5] : 0x9000));D3DXEffectBaseRecord parameter;memset(&parameter,0,sizeof(parameter));parameter.value00=Address(source);parameter.value04=Address(type);
            fpu_control_t saved,control=0x27f;_FPU_GETCW(saved);_FPU_SETCW(control);Copy(kind,&parameter,&type,&source,&destination,config[2] ? address+0x3000 : 0,config[1],&left,config[4],&written);fpu_control_t actual;_FPU_GETCW(actual);_FPU_SETCW(saved);if (actual!=control) return 4;
            unsigned int fields[5]={Address(type)-address,Address(source)-address,Address(destination)-address,left,written},header[2]={row++,sizeof(fields)+0x8000};if (fwrite(header,4,2,capture)!=2||fwrite(fields,4,5,capture)!=5||fwrite(memory.data()+0x5000,1,0x8000,capture)!=0x8000) return 3;
        }
    }
    if (fgetc(fixtures)!=EOF||fclose(fixtures)||fclose(capture)) return 3;printf("effect evaluation copy: %u native descriptor/reflection/alias graphs\n",row);return 0;
}
