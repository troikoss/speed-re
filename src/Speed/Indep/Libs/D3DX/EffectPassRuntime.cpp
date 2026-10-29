#include "EffectPassRuntime.h"
#include "EffectValues.h"

// Original complete 0x65adcb..0x65ae24. The queue count and base pointers are
// reread for each entry, including aliases into the effect or queue itself.
extern "C" int __stdcall D3DXEffectEndPass(D3DXEffectObject *effect) {
    reinterpret_cast<unsigned char *>(&effect->value118)[1]&=0xfe;
    if (effect->value10C) {
        for (unsigned int i=0;i<effect->value10C;++i) {
            unsigned short *entry=reinterpret_cast<unsigned short *>(effect->value108+i*4);
            D3DXEffectGroup64 *group=reinterpret_cast<D3DXEffectGroup64 *>(effect->value2C)+entry[0];
            group->array04[entry[1]].value30&=~2u;
        }
        effect->value10C=0;
    }
    return 0;
}

// Original complete 0x66ac17..0x66acd1. Failed record applications do not
// stop draining this queue or replace the final zero return value.
extern "C" int __stdcall D3DXEffectCommit(D3DXEffectObject *effect) {
    effect->value118|=4;
    for (unsigned int i=0;i<effect->value10C;++i) {
        unsigned short *entry=reinterpret_cast<unsigned short *>(effect->value108+i*4);
        D3DXEffectGroup64 *group=reinterpret_cast<D3DXEffectGroup64 *>(effect->value2C)+entry[0];
        D3DXEffectRecord64 *record=group->array04+entry[1];
        record->value30&=~2u;
        if (group->value3C && record->value00) {
            if (record->value30&8) UG2FxPassCachedRecord66A25F(effect,group,record);
            else {
                entry=reinterpret_cast<unsigned short *>(effect->value108+i*4);
                UG2FxPassRecord669286(effect,entry[0],entry[1],0xffff);
            }
        }
    }
    if (effect->value118&1) {
        D3DXEffectControlFp(effect->value114,0x30000);
        effect->value118&=~1u;
    }
    effect->value10C=0;
    effect->value118&=~4u;
    return 0;
}

// Original complete 0x66acd2..0x66adeb. Pass validation uses the full unsigned
// argument, reference selection its low word, final application the full index.
extern "C" int __stdcall D3DXEffectBeginPass(D3DXEffectObject *effect,unsigned int pass) {
    int result=static_cast<int>(0x8876086c);
    if (effect->value3C) {
        unsigned short *selected=reinterpret_cast<unsigned short *>(&effect->value40);
        D3DXEffectRecordB4 *technique=reinterpret_cast<D3DXEffectRecordB4 *>(effect->valueA4)+selected[1];
        if (pass<technique->value34) {
            unsigned short previous=static_cast<unsigned short>(effect->value44);
            if (static_cast<unsigned short>(pass)!=previous) {
                if (previous!=0xffff) {
                    result=effect->AdjustReferences(static_cast<unsigned short>(technique->array38[previous].value34),-1,selected[0]==selected[1] ? -1 : 0);
                    if (result<0) goto finish;
                }
                reinterpret_cast<unsigned short *>(&effect->value44)[0]=static_cast<unsigned short>(pass);
                result=effect->AdjustReferences(static_cast<unsigned short>(technique->array38[static_cast<unsigned short>(pass)].value34),1,selected[0]==selected[1]);
                if (result<0) goto finish;
            }
            reinterpret_cast<unsigned char *>(&effect->value118)[0]&=0x7f;
            result=static_cast<int>(0x8876086c);
            if (!(effect->value118&0x100)) {
                result=effect->ApplyPassGroup(static_cast<unsigned short>(technique->array38[pass].value34),0xffff);
                if (result>=0) effect->value118|=0x100;
            }
        }
    }
finish:
    if (effect->value118&1) {
        D3DXEffectControlFp(effect->value114,0x30000);
        effect->value118&=~1u;
    }
    return result;
}

// Original complete 0x6691a1..0x669285. Group selection narrows; record calls
// retain the original group argument. State 1 alone suppresses record errors.
int D3DXEffectObject::ApplyPassGroup(unsigned int group,unsigned int index) {
    D3DXEffectGroup64 *p=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+static_cast<unsigned short>(group);
    int result;
    if (value118&8) {
        result=UG2FxPassCache66403C(this,p->value18!=0xffff,&p->value1C);
        if (result<0) return result;
        result=UG2FxPassCache664197(this,p->value1A!=0xffff,&p->value24);
        if (result<0) return result;
    }
    if (p->value08&8) {
        result=p->value18==0xffff ? UG2FxPrepareGroupB(this,static_cast<unsigned short>(group)) : UG2FxPrepareGroupA(this,static_cast<unsigned short>(group));
        if (result<0) return result;
    }
    if (value3C==4) {
        if (p->value08&1) return UG2FxPassFastGroup668DFD(this,p);
        for (unsigned int i=0;static_cast<unsigned short>(i)<p->value36;++i) {
            result=UG2FxPassRecord669286(this,group,i,index);
            if (result<0) return result;
        }
        return 0;
    }
    for (unsigned int i=0;static_cast<unsigned short>(i)<p->value36;++i) {
        result=UG2FxPassRecord669286(this,group,i,index);
        if (result<0 && value3C!=1) return result;
    }
    return 0;
}

// Original complete 0x666a68..0x666b1a. Deferred texture ownership uses a
// cookie-backed one-element reference array and publishes only after retain.
int D3DXEffectObject::RecordTexture(unsigned int handle,unsigned int texture) {
    D3DXEffectValuePrefix *value=static_cast<D3DXEffectValuePrefix *>(UG2FxMalloc(0x1c));
    if (value) value->Construct();
    if (!value) return static_cast<int>(0x8007000e);
    unsigned int *cookie=static_cast<unsigned int *>(UG2FxMalloc(12));
    D3DXEffectReference08 *references=0;
    if (cookie) { *cookie=1;references=reinterpret_cast<D3DXEffectReference08 *>(cookie+1);D3DXEffectConstructArray(references,8,1,D3DXConstructReference08); }
    if (!references) { value->Delete(1);return static_cast<int>(0x8007000e); }
    if (texture) UG2FxComAddRef(texture);
    references->value04=texture;references->value00=5;
    value->value04=handle;value->value00=12;value->value10=1;value->value14=reinterpret_cast<unsigned int>(references);
    reinterpret_cast<D3DXEffectValuePrefix *>(valueFC)->value18=reinterpret_cast<unsigned int>(value);
    valueFC=reinterpret_cast<unsigned int>(value);
    return 0;
}

// Original complete 0x66d7d9..0x66d8ac, effect vtable+0xd0.
extern "C" int __stdcall D3DXEffectSetTexture(D3DXEffectObject *effect,unsigned int handle,unsigned int texture) {
    D3DXEffectBaseRecord *record;
    if (static_cast<int>(handle)<0) record=reinterpret_cast<D3DXEffectBaseRecord *>(~handle);
    else {
        reinterpret_cast<unsigned int *>(effect->valueC4)[1]=4;
        reinterpret_cast<unsigned int *>(effect->valueC4)[0]=5;
        reinterpret_cast<unsigned int *>(effect->valueC4)[4]=0;
        record=effect->ResolveParameter(handle,1,1);
        if (!record) return static_cast<int>(0x8876086c);
    }
    unsigned int *type=reinterpret_cast<unsigned int *>(record->value04);
    if ((type[4] && record->value08==0xffffffff) || static_cast<int>(type[0])<5 || static_cast<int>(type[0])>9) return static_cast<int>(0x8876086c);
    if (effect->value3C==3) return effect->RecordTexture(~reinterpret_cast<unsigned int>(record),texture);
    D3DXEffectPoolLink *shared=reinterpret_cast<D3DXEffectPoolLink *>(reinterpret_cast<D3DXEffectParameter **>(effect->valueB0)[record->value14]->value40);
    unsigned int index=*reinterpret_cast<unsigned int *>(record->value00);
    unsigned int mapped=shared ? static_cast<unsigned int *>(shared->data08)[index] : index;
    if (texture==reinterpret_cast<D3DXEffectReference18 *>(effect->value18)[mapped].value04) return 0;
    int result=effect->WriteObjectResource(texture,index,shared);
    if (result>=0) result=effect->NotifyValue(record);
    return result;
}
