#include "NativeFxInput.h"
#include "../../Speed/Indep/Libs/D3DX/EffectEvaluation.h"
#include <stdio.h>
#include <string.h>
#include <vector>
namespace {
unsigned int arenaBase,heapBase,uses,vmResult;bool vmWrite;std::vector<unsigned int> calls;
unsigned int Address(const void *p) { return reinterpret_cast<unsigned int>(p); }
unsigned int Word(unsigned int at) { return *reinterpret_cast<unsigned int *>(at); }
unsigned int Digest(const unsigned char *data,unsigned int count) { unsigned int value=2166136261;for (unsigned int i=0;i<count;++i) value=(value^data[i])*16777619;return value; }
void Clear() { asm volatile("fnclex"); }
std::vector<unsigned int> UseWords() { std::vector<unsigned int> words;for (unsigned int i=0;i<uses;++i) { words.push_back(Word(heapBase+i*32));words.push_back(Word(heapBase+i*32+4)); }return words; }
void Relocate(std::vector<unsigned char> &memory,unsigned int offset) {
    unsigned int *word=reinterpret_cast<unsigned int *>(memory.data()+offset);
    if (*word>=0x2000000&&*word<0x2010000) *word=arenaBase+*word-0x2000000;
    else if (*word>=0x3200000&&*word<0x3420000) *word=heapBase+*word-0x3200000;
}
}
// Explicit test-only VM service. It captures the complete caller contract and
// controlled result/write. Production native VM execution remains unsupported.
extern "C" int __stdcall UG2FxExecuteVm(unsigned int *program,unsigned int *literals,unsigned int a,unsigned int b,unsigned int c,unsigned int d,unsigned int e,unsigned int f,unsigned int g,unsigned int h,unsigned int i,unsigned int j,unsigned int k,unsigned int l) {
    unsigned int args[14]={Address(program),Address(literals),a,b,c,d,e,f,g,h,i,j,k,l};
    for (unsigned int n=0;n<14;++n) calls.push_back(n<8&&args[n] ? args[n]-arenaBase : args[n]);
    calls.push_back(UG2FxReadControlWord()&~0x40u);calls.push_back(Word(arenaBase+0x1114));calls.push_back(Word(arenaBase+0x1118));calls.push_back(Digest(reinterpret_cast<unsigned char *>(arenaBase+0x3000),0x4000));std::vector<unsigned int> values=UseWords();calls.push_back(Digest(reinterpret_cast<unsigned char *>(values.data()),values.size()*4));
    if (vmWrite) { *reinterpret_cast<unsigned int *>(d)=0x89abcdef;*reinterpret_cast<unsigned int *>(d+4)=0x01234567; }return static_cast<int>(vmResult);
}
int main(int argc,char **argv) {
    if (argc!=3) { fprintf(stderr,"usage: ug2effectevaluation_probe fixtures.bin capture.bin\n");return 2; }
    FILE *fixtures=fopen(argv[1],"rb"),*capture=fopen(argv[2],"wb");if (!fixtures||!capture) return 3;char magic[8];unsigned int count;
    if (fread(magic,1,8,fixtures)!=8||memcmp(magic,"UG2FXVF1",8)||fread(&count,4,1,fixtures)!=1||count!=64||fwrite("UG2FXVD1",1,8,capture)!=8) return 3;
    unsigned int saved=UG2FxReadControlWord();
    for (unsigned int row=0;row<count;++row) {
        unsigned int config[13];if (fread(config,4,13,fixtures)!=13||config[0]!=row||config[9]!=0x10000||config[10]>0x220000) return 3;std::vector<unsigned char> arena(config[9]),heap(config[10] ? config[10] : 1);std::vector<unsigned int> arenaOffsets(config[11]),heapOffsets(config[12]);
        if (fread(arena.data(),1,config[9],fixtures)!=config[9]||fread(heap.data(),1,config[10],fixtures)!=config[10]||fread(arenaOffsets.data(),4,arenaOffsets.size(),fixtures)!=arenaOffsets.size()||fread(heapOffsets.data(),4,heapOffsets.size(),fixtures)!=heapOffsets.size()) return 3;
        arenaBase=Address(arena.data());heapBase=Address(heap.data());uses=config[3];vmResult=config[7];vmWrite=config[8]!=0;calls.clear();
        for (unsigned int n=0;n<arenaOffsets.size();++n) Relocate(arena,arenaOffsets[n]);for (unsigned int n=0;n<heapOffsets.size();++n) Relocate(heap,heapOffsets[n]);
        D3DXEffectObject *effect=reinterpret_cast<D3DXEffectObject *>(arenaBase+0x1000);Clear();UG2FxWriteControlWord(config[4]);unsigned int first=UG2FxEvaluateRecord(effect,config[1],config[2]),second=0xffffffff;
        if (config[6]) { UG2FxWriteControlWord(config[5]);second=UG2FxEvaluateRecord(effect,config[1],config[2]); }
        unsigned int control=UG2FxReadControlWord()&~0x40u;Clear();UG2FxWriteControlWord(saved);
        std::vector<unsigned int> words;words.push_back(first);words.push_back(second);words.push_back(control);words.push_back(effect->value114);words.push_back(effect->value118);words.push_back(calls.size()/19);words.insert(words.end(),calls.begin(),calls.end());std::vector<unsigned int> values=UseWords();unsigned int header[2]={row,static_cast<unsigned int>(words.size()*4+0x4000+values.size()*4)};
        if (fwrite(header,4,2,capture)!=2||fwrite(words.data(),4,words.size(),capture)!=words.size()||fwrite(arena.data()+0x3000,1,0x4000,capture)!=0x4000||fwrite(values.data(),4,values.size(),capture)!=values.size()) return 3;
    }
    if (fread(&count,4,1,fixtures)!=1||count!=65536) return 3;
    for (unsigned int row=0;row<count;++row) { unsigned int value[4];if (fread(value,4,4,fixtures)!=4||D3DXEffectAbstractControl(value[0])!=value[1]||D3DXEffectHardwareControl(value[2])!=value[3]) { fprintf(stderr,"control mapping mismatch %u\n",row);return 4; } }
    if (fread(&count,4,1,fixtures)!=1||count!=2048) return 3;
    for (unsigned int row=0;row<count;++row) { unsigned int value[6];if (fread(value,4,6,fixtures)!=6) return 3;Clear();UG2FxWriteControlWord(value[0]);unsigned int result=value[3] ? D3DXEffectControlFp(value[1],value[2]) : D3DXEffectControl87(value[1],value[2]);unsigned int word=UG2FxReadControlWord()&~0x40u;Clear();UG2FxWriteControlWord(saved);if (result!=value[4]||word!=value[5]) { fprintf(stderr,"hardware control mismatch %u: %x/%x %x/%x\n",row,result,value[4],word,value[5]);return 4; } }
    const unsigned int checks[2]={131072,2048};if (fwrite(checks,4,2,capture)!=2||fgetc(fixtures)!=EOF||fclose(fixtures)||fclose(capture)) return 3;
    printf("effect evaluation: 64 native controlled-VM graphs; 131072 mapping checks; 2048 hardware control checks\n");return 0;
}
