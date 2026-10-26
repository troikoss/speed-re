// Asset-backed, display-free integration of the actual native record owner,
// recovered pack manager, original record indices and recovered bWare node pool.
#include "NativeSolidRecords.h"
#include "NativeTextureRecords.h"
#include "NativeTextureUpload.h"
#include "OriginalImage.h"
#include "../../Speed/Indep/Src/World/CarRenderSolidManager.h"
#include "../../Speed/Indep/Src/World/CarRenderTexturePack.h"
#include "../../Speed/Indep/Src/World/CarRenderTextureUpload.h"
#include "../../Speed/Indep/Libs/bWare/bSlotPool.h"
#include <assert.h>
#include <stdio.h>
#include <string>
#include <vector>

extern bSlotPool *nodeSlotPool8284F0;
static bool Read(const std::string &path,std::vector<unsigned char> &data) {
    FILE *file=fopen(path.c_str(),"rb"); if (!file) return false;
    fseek(file,0,SEEK_END); long size=ftell(file); rewind(file);
    if (size<0) { fclose(file); return false; }
    data.resize(static_cast<size_t>(size));
    bool ok=data.empty() || fread(&data[0],1,data.size(),file)==data.size();
    fclose(file); return ok;
}
int main(int argc,char **argv) {
    if (argc!=3) { fprintf(stderr,"usage: %s speed2.exe GEOMETRY.BIN\n",argv[0]); return 2; }
    char error[256];
    if (!MapOriginalImage(argv[1],error,sizeof(error))) { fprintf(stderr,"%s\n",error); return 1; }
    ug2LightProfiles8358C0.Construct();
    std::vector<unsigned char> data;
    if (!Read(argv[2],data) || data.empty()) return 1;
    std::string exe(argv[1]),geometry(argv[2]);
    size_t exeSlash=exe.find_last_of('/'), carSlash=geometry.find_last_of('/');
    std::string game=exeSlash==std::string::npos?".":exe.substr(0,exeSlash);
    std::string car=carSlash==std::string::npos?".":geometry.substr(0,carSlash);
    std::string paths[]={car+"/TEXTURES.BIN",game+"/CARS/TEXTURES.BIN",game+"/CARS/BRAKES/TEXTURES.BIN",game+"/GLOBAL/GLOBALB.BUN"};
    unsigned int expected=0,expectedTextures=0,textureSlots=0;
    for (int pass=0;pass<3;++pass) {
        NativeTextureRecords textures;
        for (int i=0;i<4;++i) {
            std::vector<unsigned char> bytes;
            assert(Read(paths[i],bytes));
            assert(textures.Add(bytes.empty()?0:&bytes[0],bytes.size(),paths[i]));
            const size_t count=textures.TextureCount();
            assert(textures.Add(bytes.empty()?0:&bytes[0],bytes.size(),paths[i]));
            assert(textures.TextureCount()==count); // Same file must not register twice.
        }
        assert(textures.PackCount()>0 && textures.TextureCount()==ug2TextureCount833780);
        assert(ug2TextureDefault82DB08 && ug2TextureDefault82DB08->pixels70);
        if (!pass) expectedTextures=textures.TextureCount(); else assert(textures.TextureCount()==expectedTextures);
        NativeSolidRecords records;
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
                for (int j=0;j<solid->textureCount19;++j) {
                    CarRenderTexture *texture=solid->textures2C[j].texture04;
                    assert(texture && texture->state00 && texture->pixels70);
                    assert(!texture->paletteBytes3C || texture->palette74);
                    assert(texture->state00->texture18 && NativeTextureBufferExists(texture->state00->texture18));
                    ++textureSlots;
                }
                ++count;
            }
        }
        assert(count==ug2SolidLoadedCount830DA0);
        if (!pass) expected=count;else assert(count==expected);
    }
    assert(ug2TextureCount833780==0 && !ug2TextureDefault82DB08 && !ug2TexturePackPool82DB0C);
    assert(NativeTextureBufferCount()==0);
    assert(ug2TextureStates872B4C.next00==&ug2TextureStates872B4C && ug2TextureStates872B4C.previous04==&ug2TextureStates872B4C);
    assert(ug2TextureReleases8763B0.head00.next00==&ug2TextureReleases8763B0.head00 &&
        ug2TextureReleases8763B0.head00.previous04==&ug2TextureReleases8763B0.head00);
    assert(ug2TexturePacks830AF0.next00==&ug2TexturePacks830AF0 && ug2TexturePixelQueue8358C8.next00==&ug2TexturePixelQueue8358C8);
    for (int i=0;i<8192;++i) assert(ug2TextureBuckets833784[i]==0);
    assert(ug2SolidPackCount82D380==0 && ug2SolidLoadedCount830DA0==0 && ug2SolidLoads82D384==0);
    assert(ug2SolidPacks830D1C.next00==&ug2SolidPacks830D1C && ug2PlatformSolids872ED0.next00==&ug2PlatformSolids872ED0);
    assert(ug2LoadedSolids835794.next00==&ug2LoadedSolids835794 && ug2FailedSolids835858.next00==&ug2FailedSolids835858);
    assert(ug2VertexReleases870D28.head00.next00==&ug2VertexReleases870D28.head00);
    for (int i=0;i<8192;++i) assert(ug2SolidBuckets830DA4[i]==0);
    assert(!nodeSlotPool8284F0 || nodeSlotPool8284F0->used18==0);
    printf("native solid/texture lifecycle: 3 loads, %u solid and %u texture records per load, %u bound texture slots, clean unload and pools\n",
        expected,expectedTextures,textureSlots);
    return 0;
}
