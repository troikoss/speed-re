#include "OriginalImage.h"
#include "NativeFxInput.h"
#include "../../Speed/Indep/Libs/D3DX/EffectMetadata.h"
#include <stdio.h>
#include <string.h>
#include <vector>

namespace {
unsigned int Address(const void *p) { return reinterpret_cast<unsigned int>(p); }
bool Probe(unsigned int mode,FILE *capture) {
    D3DXEffectObject effect;memset(&effect,0xa5,sizeof(effect));effect.Construct();
    D3DXEffectParameter parameters[3];D3DXEffectGroup64 groups[3],storage;
    D3DXEffectRecord64 records[4];D3DXEffectUseLink uses[4];
    memset(parameters,0xa5,sizeof(parameters));memset(groups,0xa5,sizeof(groups));memset(records,0xa5,sizeof(records));memset(uses,0,sizeof(uses));
    for (unsigned int i=0;i<3;++i) { parameters[i].Construct();D3DXConstructGroup64(groups+i); }
    for (unsigned int i=0;i<4;++i) { D3DXConstructRecord64(records+i);records[i].value30=i==0 ? 2 : 0; }
    groups[0].array04=records;groups[1].array04=records+2;
    for (unsigned int i=0;i<4;++i) {
        unsigned int k=i==3 ? 0 : i;
        uses[i].value08=Address(records+(i+1)%4);uses[i].value14=k/2;uses[i].value16=k%2;
        uses[i].previous18=uses+(i+3)%4;uses[i].next1C=uses+(i+1)%4;
    }
    D3DXEffectParameter *table[3]={parameters,parameters+1,parameters+2};
    parameters[0].value14=1;parameters[1].value34=Address(uses);
    D3DXEffectUseLink *row[5]={uses+2,uses,uses+3,uses+1,0};
    D3DXEffectUseLink **grid[3]={0,row,0};unsigned char dirty[16]={0};
    effect.value2C=Address(groups);effect.valueB0=Address(table);effect.valueB4=mode&1 ? Address(grid) : 0;
    effect.valueB8=Address(grid);effect.valueBC=Address(dirty);effect.valueAC=mode&4 ? 1 : 2;effect.value118=mode&2 ? 0x100 : 0;
    std::vector<unsigned int> words;
    words.push_back(effect.ResetDirtyQueue());words.push_back(effect.value10C);words.push_back(effect.value110);
    if (!effect.value108) return false;
    effect.value10C=1;effect.value110=1;*reinterpret_cast<unsigned int *>(effect.value108)=0x12345678;
    words.push_back(effect.MarkParameterDirty(parameters));words.push_back(effect.MarkParameterDirty(parameters));
    words.push_back(effect.value10C);words.push_back(effect.value110);
    for (unsigned int i=0;i<effect.value10C;++i) words.push_back(reinterpret_cast<unsigned int *>(effect.value108)[i]);
    for (unsigned int i=0;i<4;++i) words.push_back(uses[i].value00);
    for (unsigned int i=0;i<4;++i) words.push_back(records[i].value00);
    for (unsigned int i=0;i<4;++i) words.push_back(records[i].value30&3);
    words.push_back(dirty[1]);
    memset(&storage,0xa5,sizeof(storage));D3DXConstructGroup64(&storage);
    storage.data00=UG2FxMalloc(4);storage.value0C=7;storage.value10=4;storage.value34=3;
    words.push_back(storage.AllocateData());words.push_back(storage.value0C);words.push_back(storage.value10);words.push_back(storage.value14);
    storage.value0C=3;storage.value34=0;
    words.push_back(storage.AllocateData());words.push_back(storage.value0C);words.push_back(storage.value10);words.push_back(storage.value14);
    // A known recovered buffer class exercises the COM bridge. This fixture
    // makes no claim about a native GPU device implementation.
    D3DXEffectBuffer buffer={0x7b711c,4,0,0};effect.value08=0;effect.value0C=0;
    D3DXEffectSecondaryPrefix *secondary=reinterpret_cast<D3DXEffectSecondaryPrefix *>(&effect.interface04);
    words.push_back(secondary->SetDevice(Address(&buffer)));words.push_back(buffer.refs04);
    words.push_back(secondary->SetDevice(Address(&buffer)));words.push_back(buffer.refs04);
    words.push_back(secondary->SetDevice(0));words.push_back(buffer.refs04);
    words.push_back(effect.value08);words.push_back(effect.value0C);
    UG2FxFree(storage.data00);UG2FxFree(reinterpret_cast<void *>(effect.value108));
    // Graph nodes and grid rows above are diagnostic stack fixtures. Only the
    // recovered helpers' owned allocations are freed; do not destroy the graph
    // as a fully deserialized effect.
    bool ok=!NativeFxAllocationCount();
    if (capture) {
        unsigned int header[2]={mode,static_cast<unsigned int>(words.size())};
        ok=ok && fwrite(header,1,sizeof(header),capture)==sizeof(header) && fwrite(words.data(),4,words.size(),capture)==words.size();
    }
    printf("effect storage %u: queue %u capacity %u dirty %u, 0 live allocations\n",mode,effect.value10C,effect.value110,dirty[1]);return ok;
}
}
int main(int argc,char **argv) {
    if (argc!=2 && argc!=3) { fprintf(stderr,"usage: ug2effectstorage_probe speed2.exe [capture.bin]\n");return 2; }
    char error[512];if (!MapOriginalImage(argv[1],error,sizeof(error))) { fprintf(stderr,"%s\n",error);return 1; }
    FILE *capture=argc==3 ? fopen(argv[2],"wb") : 0;
    if (argc==3&&!capture) return 3;
    if (capture&&fwrite("UG2FXST1",1,8,capture)!=8) return 3;
    for (unsigned int mode=0;mode<8;++mode) if (!Probe(mode,capture)) return 4;
    if (capture&&fclose(capture)) return 3;
    printf("effect storage: 8 synthetic graphs, 0 live allocations\n");return 0;
}
