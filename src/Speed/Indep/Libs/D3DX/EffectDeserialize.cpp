#include "EffectSharedPool.h"

// These macros retain the original distinct cursor lifetimes across virtual
// getters. Header fields reread the cursor after the pointer getter; the magic
// and payload-selector words instead use a cached position.
#define FX_DESERIAL_WORD(destination) do { \
    unsigned int limit=UG2FxBufferLengthVirtual(buffer); \
    if (*position+1<=limit) { \
        unsigned char *data=UG2FxBufferPointerVirtual(buffer); \
        destination=*reinterpret_cast<unsigned int *>(data+*position*4); \
    } \
    ++*position; \
} while (0)
#define FX_DESERIAL_CACHED(destination) do { \
    unsigned int limit=UG2FxBufferLengthVirtual(buffer); \
    if (cached+1<=limit) { \
        cached=*position; \
        unsigned char *data=UG2FxBufferPointerVirtual(buffer); \
        destination=*reinterpret_cast<unsigned int *>(data+cached*4); \
    } \
    *position=++cached; \
} while (0)
#define FX_DESERIAL_OWNER(p) (reinterpret_cast<D3DXEffectParameter **>(valueB0)[(p)->value14])
#define FX_DESERIAL_BASE(owner) ((owner)->value40 ? reinterpret_cast<unsigned int>(reinterpret_cast<D3DXEffectPool *>(value38)->data10) : value20)
#define FX_DESERIAL_PARAMETER_PREFIX(p) do { \
    (p)->value00=value20+(p)->value30; \
    (p)->value04=value20+(p)->value2C; \
} while (0)
// Annotation relocation precedes validation; root parameters use the reverse
// order below. Preserve old compact type/data cursors for validation and size.
#define FX_DESERIAL_ANNOTATION(p,index) do { \
    reinterpret_cast<D3DXEffectParameter **>(valueB0)[index]=p; \
    (p)->value14=index; \
    D3DXEffectParameter *owner=FX_DESERIAL_OWNER(p); \
    unsigned int type=FX_DESERIAL_BASE(owner)+owner->value2C+(p)->value18; \
    owner=FX_DESERIAL_OWNER(p); \
    unsigned int data=FX_DESERIAL_BASE(owner)+owner->value30+(p)->value1C; \
    unsigned int oldType=(p)->value2C,oldData=(p)->value30; \
    result=RelocateParameter(p,&scalar,&matrix,&structure);if (result<0) goto finish; \
    FX_DESERIAL_PARAMETER_PREFIX(p); \
    (p)->value46=0;(p)->value44=value30; \
    result=ValidateType(&type,&data,index);if (result<0) goto finish; \
    owner=FX_DESERIAL_OWNER(p); \
    (p)->value0C=D3DXEffectValueSize(FX_DESERIAL_BASE(owner),&oldType,&oldData); \
} while (0)

// Original UG2 PC v1.2 0x66f69c..0x67064a, thiscall / RET 16. All readers,
// metadata, relocation, shared pools and storage callees are recovered bodies.
int D3DXEffectObject::Deserialize(unsigned int device,D3DXEffectPool *pool,D3DXEffectBuffer *buffer,unsigned int *position) {
    int result;
    if (!device || !buffer) { result=static_cast<int>(0x8876086c);goto finish; }
    {
        valueC4=reinterpret_cast<unsigned int>(UG2FxMalloc(0x1c));if (!valueC4) goto oom;
        result=ResetDirtyQueue();if (result<0) goto finish;
        for (unsigned int i=0;i<7;++i) reinterpret_cast<unsigned int *>(valueC4)[i]=0;
        value38=reinterpret_cast<unsigned int>(pool);
        if (pool) UG2FxComAddRef(reinterpret_cast<unsigned int>(pool));
        reinterpret_cast<D3DXEffectSecondaryPrefix *>(&interface04)->SetDevice(device);
        value28=reinterpret_cast<unsigned int>(buffer);UG2FxComAddRef(reinterpret_cast<unsigned int>(buffer));
        unsigned int cached=*position,magic=reinterpret_cast<unsigned int>(pool);
        FX_DESERIAL_CACHED(magic);
        FX_DESERIAL_WORD(value1C);
        if (magic!=0xfeff0901) goto invalid;
        if (UG2FxBufferLengthVirtual(buffer)<*position) goto invalid;
        if (value20) { UG2FxFree(reinterpret_cast<void *>(value20));value20=0; }
        value20=reinterpret_cast<unsigned int>(UG2FxMalloc(value1C));if (!value20) goto oom;
        if (value1C) {
            unsigned int bytes=value1C;
            unsigned int limit=UG2FxBufferLengthVirtual(buffer);
            if (*position+(bytes>>2)+((bytes&3)!=0)<=limit) {
                unsigned char *input=UG2FxBufferPointerVirtual(buffer);
                input+=*position*4;
                unsigned char *output=reinterpret_cast<unsigned char *>(value20);
                unsigned int i;for (i=0;i<(bytes>>2);++i) reinterpret_cast<unsigned int *>(output)[i]=reinterpret_cast<unsigned int *>(input)[i];
                for (i=bytes&0xfffffffc;i<bytes;++i) output[i]=input[i];
            }
            *position+=(value1C>>2)+((value1C&3)!=0);
        }
        FX_DESERIAL_WORD(value98);FX_DESERIAL_WORD(valueA0);FX_DESERIAL_WORD(value34);FX_DESERIAL_WORD(value14);
        if (UG2FxBufferLengthVirtual(buffer)<*position || value1C<4 || *reinterpret_cast<unsigned int *>(value20)) goto invalid;
        if (value34) {
            unsigned int count=value34,*cookie=static_cast<unsigned int *>(UG2FxMalloc(count*0x64+4));
            D3DXEffectGroup64 *groups=0;
            if (cookie) { *cookie=count;groups=reinterpret_cast<D3DXEffectGroup64 *>(cookie+1);D3DXEffectConstructArray(groups,0x64,count,D3DXConstructGroup64); }
            value2C=reinterpret_cast<unsigned int>(groups);if (!groups) goto oom;
        }
        value30=0;
        if (value98) {
            unsigned int count=value98,*cookie=static_cast<unsigned int *>(UG2FxMalloc(count*0x50+4));
            D3DXEffectParameter *parameters=0;
            if (cookie) {
                *cookie=count;parameters=reinterpret_cast<D3DXEffectParameter *>(cookie+1);
                D3DXEffectConstructArray(parameters,0x50,count,UG2FxConstructParameter);
            }
            value9C=reinterpret_cast<unsigned int>(parameters);if (!parameters) goto oom;
        }
        if (!valueA0) goto invalid; // 0x66f957 rejects a zero technique count.
        {
            unsigned int count=valueA0,*cookie=static_cast<unsigned int *>(UG2FxMalloc(count*0xb4+4));
            D3DXEffectRecordB4 *techniques=0;
            if (cookie) { *cookie=count;techniques=reinterpret_cast<D3DXEffectRecordB4 *>(cookie+1);D3DXEffectConstructArray(techniques,0xb4,count,D3DXConstructRecordB4); }
            valueA4=reinterpret_cast<unsigned int>(techniques);if (!techniques) goto oom;
        }
        if (value14) {
            unsigned int count=value14,*cookie=static_cast<unsigned int *>(UG2FxMalloc(count*0x18+4));
            D3DXEffectReference18 *references=0;
            if (cookie) { *cookie=count;references=reinterpret_cast<D3DXEffectReference18 *>(cookie+1);D3DXEffectConstructArray(references,0x18,count,D3DXConstructReference18); }
            value18=reinterpret_cast<unsigned int>(references);if (!references) goto oom;
        }
        if (UG2FxBufferLengthVirtual(buffer)<*position) goto invalid;
        valueA8=1;
        for (unsigned int root=0;static_cast<unsigned short>(root)<value98;++root) {
            result=ReadParameter(reinterpret_cast<D3DXEffectParameter *>(value9C)+static_cast<unsigned short>(root),buffer,position);if (result<0) goto finish;
        }
        for (unsigned short t=0;t<valueA0;++t) {
            D3DXEffectRecordB4 *technique=reinterpret_cast<D3DXEffectRecordB4 *>(valueA4)+t;
            *reinterpret_cast<unsigned short *>(&technique->value00.value14)=t;
            result=ReadTechnique(technique,buffer,position);if (result<0) goto finish;
        }
        unsigned int allocation=value12C+value1C+value128+0x20+value124;
        unsigned char *grown=static_cast<unsigned char *>(UG2FxMalloc(allocation));if (!grown) goto oom;
        unsigned int i;for (i=0;i<(value1C>>2);++i) reinterpret_cast<unsigned int *>(grown)[i]=reinterpret_cast<unsigned int *>(value20)[i];
        for (i=value1C&0xfffffffc;i<value1C;++i) grown[i]=reinterpret_cast<unsigned char *>(value20)[i];
        unsigned int bytes=allocation-value1C;unsigned char *zero=grown+value1C;
        for (i=0;i<(bytes>>2);++i) reinterpret_cast<unsigned int *>(zero)[i]=0;
        for (i=bytes&0xfffffffc;i<bytes;++i) zero[i]=0;
        unsigned int matrix=((reinterpret_cast<unsigned int>(grown)+value1C)&0xffffffe0)-reinterpret_cast<unsigned int>(grown)+0x20;
        unsigned int scalar=value124+matrix,structure=value128+scalar;
        if (value20) { UG2FxFree(reinterpret_cast<void *>(value20));value20=0; }
        value20=reinterpret_cast<unsigned int>(grown);
        valueB0=reinterpret_cast<unsigned int>(UG2FxMalloc(valueA8*4));if (!valueB0) goto oom;
        reinterpret_cast<D3DXEffectParameter **>(valueB0)[0]=&value48;
        unsigned int identifier=1;
        for (unsigned int root=0;static_cast<unsigned short>(root)<value98;++root) {
            unsigned int index=static_cast<unsigned short>(identifier);if (index>=valueA8) break;
            D3DXEffectParameter *parameter=reinterpret_cast<D3DXEffectParameter *>(value9C)+static_cast<unsigned short>(root);
            reinterpret_cast<D3DXEffectParameter **>(valueB0)[index]=parameter;parameter->value14=index;
            D3DXEffectParameter *owner=FX_DESERIAL_OWNER(parameter);
            unsigned int type=FX_DESERIAL_BASE(owner)+owner->value2C+parameter->value18;
            owner=FX_DESERIAL_OWNER(parameter);
            unsigned int data=FX_DESERIAL_BASE(owner)+owner->value30+parameter->value1C;
            unsigned int oldType=parameter->value2C,oldData=parameter->value30;
            parameter->value44=value30;parameter->value46=0;
            result=ValidateType(&type,&data,index);if (result<0) goto finish;
            owner=FX_DESERIAL_OWNER(parameter);
            parameter->value0C=D3DXEffectValueSize(FX_DESERIAL_BASE(owner),&oldType,&oldData);parameter->value48=0;
            owner=FX_DESERIAL_OWNER(parameter);
            unsigned int *description=reinterpret_cast<unsigned int *>(FX_DESERIAL_BASE(owner)+owner->value2C+parameter->value18);
            parameter->value4C=description[4];if (!parameter->value4C) parameter->value4C=1;
            ++identifier;
            result=RelocateParameter(parameter,&scalar,&matrix,&structure);if (result<0) goto finish;
            FX_DESERIAL_PARAMETER_PREFIX(parameter);
            D3DXEffectParameter *annotation=reinterpret_cast<D3DXEffectParameter *>(parameter->value3C);
            while (annotation && static_cast<unsigned short>(identifier)<valueA8) {
                index=static_cast<unsigned short>(identifier);
                FX_DESERIAL_ANNOTATION(annotation,index);
                ++identifier;annotation=reinterpret_cast<D3DXEffectParameter *>(annotation->value3C);
            }
        }
        for (unsigned int t=0;static_cast<unsigned short>(t)<valueA0;++t) {
            D3DXEffectRecordB4 *technique=reinterpret_cast<D3DXEffectRecordB4 *>(valueA4)+static_cast<unsigned short>(t);
            D3DXEffectParameter *annotation=technique->parameter30;
            while (annotation && static_cast<unsigned short>(identifier)<valueA8) {
                unsigned int index=static_cast<unsigned short>(identifier);FX_DESERIAL_ANNOTATION(annotation,index);
                ++identifier;annotation=reinterpret_cast<D3DXEffectParameter *>(annotation->value3C);
            }
            for (unsigned int pass=0;pass<technique->value34;++pass) {
                annotation=technique->array38[pass].parameter30;
                while (annotation && static_cast<unsigned short>(identifier)<valueA8) {
                    unsigned int index=static_cast<unsigned short>(identifier);FX_DESERIAL_ANNOTATION(annotation,index);
                    ++identifier;annotation=reinterpret_cast<D3DXEffectParameter *>(annotation->value3C);
                }
            }
        }
        if (static_cast<unsigned short>(identifier)!=valueA8) goto invalid;
        cached=*position;
        unsigned int resourceEntries=reinterpret_cast<unsigned int>(position),payloadEntries;
        FX_DESERIAL_CACHED(resourceEntries);FX_DESERIAL_CACHED(payloadEntries);
        if (UG2FxBufferLengthVirtual(buffer)<*position) goto invalid;
        result=0;
        for (unsigned int entry=0;entry<resourceEntries;++entry) { result=ReadResource(buffer,position);if (result<0) goto finish; }
        for (unsigned int entry=0;entry<payloadEntries;++entry) {
            cached=*position;
            unsigned int techniqueIndex,parameterIndex,element,recordIndex,selector;
            FX_DESERIAL_CACHED(techniqueIndex);FX_DESERIAL_CACHED(parameterIndex);FX_DESERIAL_CACHED(element);FX_DESERIAL_CACHED(recordIndex);FX_DESERIAL_CACHED(selector);
            if (UG2FxBufferLengthVirtual(buffer)<*position) goto invalid;
            unsigned int group;
            if (techniqueIndex==0xffffffff) {
                if (parameterIndex>=value98) goto invalid;
                D3DXEffectParameter *parameter=reinterpret_cast<D3DXEffectParameter *>(value9C)+parameterIndex;
                if (element>=parameter->value46) goto invalid;
                group=element+parameter->value44;
            } else {
                if (techniqueIndex>=valueA0) goto invalid;
                D3DXEffectRecordB4 *technique=reinterpret_cast<D3DXEffectRecordB4 *>(valueA4)+techniqueIndex;
                if (parameterIndex>=technique->value34 || element!=0xffffffff) goto invalid;
                group=static_cast<unsigned short>(technique->array38[parameterIndex].value34);
            }
            D3DXEffectGroup64 *selected=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+group;
            if (recordIndex>=selected->value36) goto invalid;
            unsigned int kind=ug2FxTypeTable7B582C[selected->array04[recordIndex].value52][0];
            if (kind==5 || kind==10) {
                if (selector==1) result=ReadSymbol(group,recordIndex,buffer,position);
                else result=static_cast<int>(0x80004005);
            } else if (kind<15 || kind>16) result=ReadCompiledRecord(group,recordIndex,buffer,position);
            else if (!selector) result=ReadShaderRecord(group,recordIndex,buffer,position);
            else if (selector==1) result=ReadSymbol(group,recordIndex,buffer,position);
            else result=ReadSymbolRecord(group,recordIndex,buffer,position);
            if (result<0) goto finish;
        }
        if (value38) for (unsigned int root=0;static_cast<unsigned short>(root)<value98;++root) {
            D3DXEffectParameter *parameter=reinterpret_cast<D3DXEffectParameter *>(value9C)+static_cast<unsigned short>(root);
            if (FX_DESERIAL_OWNER(parameter)->value28&1) { result=reinterpret_cast<D3DXEffectPool *>(value38)->Bind(this,parameter);if (result<0) goto finish; }
        }
        for (unsigned short groupIndex=0;groupIndex<value30;++groupIndex) {
            D3DXEffectGroup64 *group=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+groupIndex;
            result=group->AllocateData();if (result<0) goto finish;
            unsigned int count=group->value0C;unsigned char *data=static_cast<unsigned char *>(group->data00);
            for (i=0;i<(count>>2);++i) reinterpret_cast<unsigned int *>(data)[i]=0;
            for (i=count&0xfffffffc;i<count;++i) data[i]=0;
            unsigned int ordinal=0;
            for (unsigned short recordIndex=0;recordIndex<group->value36;++recordIndex) {
                D3DXEffectRecord64 *record=group->array04+recordIndex;
                D3DXEffectParameter *owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[record->value08.value14];
                record->value08.value00=owner->value30+record->value08.value1C+FX_DESERIAL_BASE(owner);
                owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[record->value08.value14];
                record->value08.value04=owner->value2C+record->value08.value18+FX_DESERIAL_BASE(owner);
                unsigned int tag=ug2FxTypeTable7B582C[record->value52][4];
                if (tag==0x0e000000 || tag==0x0f000000) {
                    reinterpret_cast<unsigned short *>(static_cast<unsigned char *>(group->data00)+group->value14)[static_cast<unsigned short>(ordinal++)]=recordIndex;
                    unsigned short *storage=reinterpret_cast<unsigned short *>(static_cast<unsigned char *>(group->data00)+record->value48);
                    if (!record->value08.value14) {
                        owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[0];
                        unsigned int reference=*reinterpret_cast<unsigned int *>(owner->value30+record->value08.value1C+FX_DESERIAL_BASE(owner));
                        if (owner->value40) reference=static_cast<unsigned int *>(reinterpret_cast<D3DXEffectPoolLink *>(owner->value40)->data08)[reference];
                        storage[1]=0;storage[0]=static_cast<unsigned short>(reinterpret_cast<D3DXEffectReference18 *>(value18)[reference].value10)+1;
                    } else {
                        storage[0]=reinterpret_cast<D3DXEffectParameter **>(valueB0)[record->value08.value14]->value44+1;
                        storage[1]=static_cast<unsigned short>(record->value08.value1C>>2);
                    }
                }
            }
        }
        *reinterpret_cast<unsigned short *>(&value44)=0xffff;*reinterpret_cast<unsigned short *>(&value40)=0xffff;
        value118|=0x40;value3C=0;reinterpret_cast<unsigned short *>(&value40)[1]=0;
        for (unsigned int root=0;static_cast<unsigned short>(root)<value98;++root) {
            D3DXEffectParameter *parameter=reinterpret_cast<D3DXEffectParameter *>(value9C)+static_cast<unsigned short>(root);
            int kind=*reinterpret_cast<int *>(parameter->value04);
            if (kind<10 || kind>14) { result=MarkParameterDirty(parameter);if (result<0) goto finish; }
        }
        for (unsigned short index=0;index<valueA8;++index) {
            D3DXEffectBaseRecord *record=reinterpret_cast<D3DXEffectBaseRecord **>(valueB0)[index];
            while (record) {
                D3DXEffectParameter *owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[record->value14];
                record->value00=record->value1C+owner->value30+FX_DESERIAL_BASE(owner);
                owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[record->value14];
                record->value04=record->value18+owner->value2C+FX_DESERIAL_BASE(owner);
                record=record->linked24;
            }
        }
    }
    goto finish;
oom:
    result=static_cast<int>(0x8007000e);goto finish;
invalid:
    result=static_cast<int>(0x80004005);
finish:
    valueAC=valueA8;return result;
}
#undef FX_DESERIAL_WORD
#undef FX_DESERIAL_CACHED
#undef FX_DESERIAL_OWNER
#undef FX_DESERIAL_BASE
#undef FX_DESERIAL_PARAMETER_PREFIX
#undef FX_DESERIAL_ANNOTATION
