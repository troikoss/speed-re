// Asset-backed, display-free integration of the actual native record owner,
// recovered pack manager, original record indices and recovered bWare node pool.
#include "NativeSolidRecords.h"
#include "OriginalImage.h"
#include "../../Speed/Indep/Src/World/CarRenderSolidManager.h"
#include "../../Speed/Indep/Libs/bWare/bSlotPool.h"
#include <assert.h>
#include <stdio.h>
#include <string>
#include <vector>

extern bSlotPool *nodeSlotPool8284F0;
int main(int argc,char **argv) {
    if (argc!=3) { fprintf(stderr,"usage: %s speed2.exe GEOMETRY.BIN\n",argv[0]); return 2; }
    char error[256];
    if (!MapOriginalImage(argv[1],error,sizeof(error))) { fprintf(stderr,"%s\n",error); return 1; }
    ug2LightProfiles8358C0.Construct();
    FILE *file=fopen(argv[2],"rb");if (!file) return 1;
    fseek(file,0,SEEK_END);long size=ftell(file);rewind(file);
    if (size<=0) { fclose(file); return 1; }
    std::vector<unsigned char> data(static_cast<size_t>(size));
    if (fread(&data[0],1,data.size(),file)!=data.size()) { fclose(file); return 1; }
    fclose(file);
    unsigned int expected=0;
    {
        NativeSolidRecords records;
        for (int pass=0;pass<3;++pass) {
            assert(records.Load(&data[0],data.size()));
            assert(ug2SolidPackCount82D380>0);
            unsigned int count=0;
            for (CarRenderQueueLink *link=ug2SolidPacks830D1C.next00;link!=&ug2SolidPacks830D1C;link=link->next00) {
                CarRenderSolidPack *pack=reinterpret_cast<CarRenderSolidPack *>(link);
                for (int i=0;i<pack->count0C;++i) {
                    CarRenderSolidResource &entry=pack->resources74[i];
                    CarRenderSlotTable *solid=records.Find(entry.hash00);
                    assert(solid && solid==entry.solid04 && solid->hash10==entry.hash00);
                    assert(solid->loaded0D==1 && solid->platform00);
                    ++count;
                }
            }
            assert(count==ug2SolidLoadedCount830DA0);
            if (!pass) expected=count;else assert(count==expected);
        }
    }
    assert(ug2SolidPackCount82D380==0 && ug2SolidLoadedCount830DA0==0 && ug2SolidLoads82D384==0);
    assert(ug2SolidPacks830D1C.next00==&ug2SolidPacks830D1C && ug2PlatformSolids872ED0.next00==&ug2PlatformSolids872ED0);
    assert(ug2LoadedSolids835794.next00==&ug2LoadedSolids835794 && ug2FailedSolids835858.next00==&ug2FailedSolids835858);
    assert(ug2VertexReleases870D28.head00.next00==&ug2VertexReleases870D28.head00);
    for (int i=0;i<8192;++i) assert(ug2SolidBuckets830DA4[i]==0);
    assert(!nodeSlotPool8284F0 || nodeSlotPool8284F0->used18==0);
    printf("native solid lifecycle: 3 loads, %u resource bindings per load, clean unload and node pool\n",expected);
    return 0;
}
