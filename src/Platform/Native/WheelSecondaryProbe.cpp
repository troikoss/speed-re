#include "OriginalImage.h"
#include "../../Speed/Indep/Src/World/CarWheelSecondary.h"
#include "../../Speed/Indep/Src/World/CarRenderSolid.h"
#include <stdio.h>
#include <string.h>
namespace {
struct Row { unsigned int id,control,mode,payload,gate,texture,flags;unsigned char quad[100]; };
typedef char FixtureSize[(sizeof(Row)==128)?1:-1];
CarRenderPartTable table;short indices[]={2,0,1};unsigned int records[4];
CarRenderQuad captured;CarRenderTexture *textureToken=reinterpret_cast<CarRenderTexture *>(0x12345678);WheelMatrix *expectedWorld;
unsigned int calls,lookups;const Row *current;
void Bytes(unsigned char *out,const CarRenderQuad &quad) {
    unsigned int n=0;const unsigned char *p=reinterpret_cast<const unsigned char *>(&quad);
    for (unsigned int j=0;j<0x71;++j) if(j>=0x40||j%16<12) out[n++]=p[j];
}
}
extern "C" { CarRenderPartTable *ug2CarPartTable8A1CC8=&table;extern int ug2WheelLodGate; }
extern "C" CarRenderTexture *UG2SolidFindTexture(unsigned int hash,int fallback,int mode) {
    // Original cshash(TIREMASK), independently pinned in fixture generation.
    if (hash!=0xc8cbb2df||fallback!=1||mode) __builtin_trap();++lookups;return current->texture ? textureToken : 0;
}
void CarRenderSubmitQuad(CarRenderQuad *quad,CarRenderTexture *texture,WheelMatrix *world,unsigned int fourth) {
    if (world!=expectedWorld||texture!=(current->texture ? textureToken : 0)||fourth) __builtin_trap();++calls;memcpy(&captured,quad,sizeof(captured));
}
int main(int argc,char **argv) {
    char error[256];if (argc!=3||!MapOriginalImage(argv[1],error,sizeof(error))) return 2;
    FILE *file=fopen(argv[2],"rb");char magic[8];unsigned int count;
    if(!file||fread(magic,1,8,file)!=8||memcmp(magic,"UG2WHQS1",8)||fread(&count,4,1,file)!=1) return 2;
    table.indices14=indices;table.records1C=reinterpret_cast<unsigned char *>(records);
    for(unsigned int i=0;i<count;++i) {
        Row row;if(fread(&row,1,sizeof(row),file)!=sizeof(row)||row.id!=i) return 2;current=&row;
        CarRenderWheelInfo info;unsigned char car[0x610];CarRenderPart part;CarRenderView view;WheelMatrix world;
        memset(&info,0,sizeof(info));memset(car,0,sizeof(car));memset(&part,0,sizeof(part));memset(&view,0,sizeof(view));memset(&world,0,sizeof(world));
        info.carObject=car;CarRenderPart *pointer=row.mode ? &part : 0;memcpy(car+0x604,&pointer,4);part.index0A=row.mode==2 ? 0xffff : 0;
        records[0]=0x12345678;records[1]=0xabcdef01;records[2]=row.mode==3 ? 0x11111111 : 0xeb0101e2;records[3]=row.payload;
        expectedWorld=&world;calls=lookups=0;unsigned short cw=row.control;
        ug2WheelLodGate=static_cast<int>(row.gate);
        if(*reinterpret_cast<volatile unsigned int *>(0x8654a4)!=row.gate) return 3;
        asm volatile("fninit; fldcw %0" : : "m"(cw));info.Fn620600(&view,&world);
        unsigned short env[14];asm volatile("fnstenv %0" : "=m"(env));unsigned char actual[97];Bytes(actual,captured);
        if(calls!=1||lookups!=1||memcmp(actual,row.quad,97)||CarWheelCallerFlags(ug2WheelLodGate)!=row.flags||env[0]!=row.control||env[4]!=0xffff||(env[2]&0x3800)) {
            fprintf(stderr,"native wheel secondary mismatch row %u mode %u CW %x/%x calls %u/%u payload %08x\n",i,row.mode,env[0],row.control,calls,lookups,row.payload);return 1;
        }
    }
    asm volatile("fninit");if(fgetc(file)!=EOF) return 2;fclose(file);printf("wheel secondary native: %u exact quad/flag/call/FP-stack checks, zero mismatches; packing/GPU submission controlled\n",count);return 0;
}
