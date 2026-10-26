// Asset-backed, display-free integration of the actual native record owner,
// recovered pack manager, original record indices and recovered bWare node pool.
#include "NativeSolidRecords.h"
#include "NativeTextureRecords.h"
#include "NativeTextureUpload.h"
#include "OriginalImage.h"
#include "../../Speed/Indep/Src/World/CarRenderSolidManager.h"
#include "../../Speed/Indep/Src/World/CarRenderTexturePack.h"
#include "../../Speed/Indep/Src/World/CarRenderTextureUpload.h"
#include "../../Speed/Indep/Src/World/CarRenderTextureReferences.h"
#include "../../Speed/Indep/Libs/bWare/bSlotPool.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
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
static void LoadPack(NativeTextureRecords &textures,const std::string &path) {
    std::vector<unsigned char> bytes;
    assert(Read(path,bytes));
    assert(textures.Add(bytes.empty()?0:&bytes[0],bytes.size(),path));
    const size_t count=textures.TextureCount();
    assert(textures.Add(bytes.empty()?0:&bytes[0],bytes.size(),path));
    assert(textures.TextureCount()==count);
}
static std::vector<CarRenderTexture *> ActiveTextures() {
    std::vector<CarRenderTexture *> active;
    for (CarRenderQueueLink *node=ug2TexturePacks830AF0.next00;node!=&ug2TexturePacks830AF0;node=node->next00) {
        CarRenderTexturePack *pack=reinterpret_cast<CarRenderTexturePack *>(node);
        for (int i=0;i<pack->count74;++i) if (pack->header70->indices74[i].texture04)
            active.push_back(pack->header70->indices74[i].texture04);
    }
    return active;
}
static unsigned int CheckBindings(unsigned int *matched,unsigned int *defaults) {
    const std::vector<CarRenderTexture *> active=ActiveTextures();
    unsigned int slots=0;*matched=*defaults=0;
    for (CarRenderQueueLink *node=ug2SolidPacks830D1C.next00;node!=&ug2SolidPacks830D1C;node=node->next00) {
        CarRenderSolidPack *pack=reinterpret_cast<CarRenderSolidPack *>(node);
        for (int i=0;i<pack->count0C;++i) if (pack->resources74[i].solid04) {
            CarRenderSlotTable *solid=pack->resources74[i].solid04;
            for (int j=0;j<solid->textureCount19;++j) {
                CarRenderTexture *texture=solid->textures2C[j].texture04;
                bool registered=false;
                for (size_t k=0;k<active.size();++k) if (active[k]==texture) registered=true;
                assert(registered); // Check ownership before dereferencing a potentially stale slot.
                assert(texture->state00 && texture->pixels70 && (!texture->paletteBytes3C || texture->palette74));
                assert(texture->state00->texture18 && NativeTextureBufferExists(texture->state00->texture18));
                if (texture==ug2TextureDefault82DB08) ++*defaults;
                else { assert(texture->hash24==solid->textures2C[j].hash00); ++*matched; }
                ++slots;
            }
        }
    }
    return slots;
}
static void LinkFixture(CarRenderQueueLink *head,CarRenderQueueLink *node) {
    assert(head->next00==head);
    head->next00=head->previous04=node;
    node->next00=node->previous04=head;
}
int main(int argc,char **argv) {
    bool late=argc==4 && strcmp(argv[3],"--late-textures")==0;
    if (argc!=3 && !late) { fprintf(stderr,"usage: %s speed2.exe GEOMETRY.BIN [--late-textures]\n",argv[0]); return 2; }
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
        if (late) LoadPack(textures,paths[3]);
        else for (int i=0;i<4;++i) LoadPack(textures,paths[i]);
        assert(textures.PackCount()>0 && textures.TextureCount()==ug2TextureCount833780);
        assert(ug2TextureDefault82DB08 && ug2TextureDefault82DB08->pixels70);
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
                ++count;
            }
        }
        assert(count==ug2SolidLoadedCount830DA0);
        if (!pass) expected=count;else assert(count==expected);
        unsigned int matched=0,defaults=0;
        if (late) {
            CheckBindings(&matched,&defaults);
            unsigned int before=matched,beforeDefaults=defaults;
            const std::vector<CarRenderTexture *> base=ActiveTextures();
            NativeTextureRecords additions;
            for (int i=0;i<3;++i) LoadPack(additions,paths[i]);
            textureSlots+=CheckBindings(&matched,&defaults);
            assert(matched>before && defaults<beforeDefaults);
            if (!pass) expectedTextures=ug2TextureCount833780;else assert(ug2TextureCount833780==expectedTextures);
            // Native fixture prefixes exercise both standalone notification lists.
            CarRenderTexture *selected=0;
            const std::vector<CarRenderTexture *> loaded=ActiveTextures();
            for (size_t i=0;i<loaded.size() && !selected;++i) {
                bool inBase=false;
                for (size_t j=0;j<base.size();++j) if (base[j]->hash24==loaded[i]->hash24) inBase=true;
                if (!inBase) selected=loaded[i];
            }
            assert(selected);
            CarRenderTextureReference5C reference={};reference.hash0C=selected->hash24;reference.texture14=selected;
            CarRenderFontTextureReference font={};font.hash18=selected->hash24;font.texture08=selected;
            LinkFixture(&ug2TextureReferences830D08,&reference.link00);LinkFixture(&ug2FontTextures863768,&font.link00);
            for (int i=0;i<6;++i) for (int j=0;j<0x72;++j) ug2TextureCache89F050[i].value00[j]=0xc0ffee00+i+j;
            additions.Clear(); // Real detach notifications while solids/reference prefixes remain alive.
            assert(!reference.texture14 && !font.texture08);
            ug2TextureReferences830D08.next00=ug2TextureReferences830D08.previous04=&ug2TextureReferences830D08;
            ug2FontTextures863768.next00=ug2FontTextures863768.previous04=&ug2FontTextures863768;
            for (int i=0;i<6;++i) for (int j=0;j<0x72;++j) assert(!ug2TextureCache89F050[i].value00[j]);
            unsigned int after=0,afterDefaults=0;CheckBindings(&after,&afterDefaults);
            assert(after==before && afterDefaults==beforeDefaults);
            printf("late texture pass %d: %u -> %u -> %u matching slots, standalone/font refs cleared, six caches invalidated\n",
                pass+1,before,matched,after);
        } else {
            textureSlots+=CheckBindings(&matched,&defaults);
            if (!pass) expectedTextures=textures.TextureCount();else assert(textures.TextureCount()==expectedTextures);
        }
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
