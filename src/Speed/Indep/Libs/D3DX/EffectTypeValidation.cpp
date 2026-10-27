#include "EffectMetadata.h"

// Original 0x66548b. Type/data cursors are absolute x86 addresses. This routine
// both validates the stream and creates its parameter/group/use metadata.
int D3DXEffectObject::ValidateType(unsigned int *type,unsigned int *data,unsigned int parameter) {
    unsigned int end=value1C+value20;
    unsigned int start=*type;
    unsigned int *description=reinterpret_cast<unsigned int *>(start);
    if (end<start+0x14) return static_cast<int>(0x80004005);
    int kind=static_cast<int>(description[0]);
    unsigned int count=description[4];
    int result=ValidateName(description[2]); if (result<0) return result;
    result=ValidateString(description[3]); if (result<0) return result;
    if (!count) count=1;
    int shape=static_cast<int>(description[1]);
    if (shape<0) return static_cast<int>(0x80004005);
    if (shape<4) {
        if (kind<1 || kind>3) return static_cast<int>(0x80004005);
        if (end<start+0x1c) return static_cast<int>(0x80004005);
        unsigned int bytes=description[6]*description[5]*count*4;
        // The original compares against the descriptor start, not data+bytes.
        if (end<start+bytes) return static_cast<int>(0x80004005);
        *data+=bytes; *type+=0x1c;
        return 0;
    }
    if (shape==5) {
        if (kind) return static_cast<int>(0x80004005);
        unsigned int children=start+0x18;
        if (end<children) return static_cast<int>(0x80004005);
        unsigned int members=description[5];
        *type=children;
        result=0;
        for (unsigned int i=0;i<count;++i) {
            *type=children;
            for (unsigned int j=0;j<members;++j) {
                result=ValidateType(type,data,parameter); if (result<0) return result;
            }
        }
        return result;
    }
    if (shape!=4 || kind<4 || kind>16) return static_cast<int>(0x80004005);
    result=0;
    if (kind>=10 && kind<=14) {
        D3DXEffectParameter *p=reinterpret_cast<D3DXEffectParameter **>(valueB0)[parameter];
        if (p->value46) return static_cast<int>(0x80004005);
        p->value46=static_cast<unsigned short>(p->value46+count);
        value30=static_cast<unsigned short>(value30+count);
        for (unsigned int i=0;i<count;++i) {
            D3DXEffectGroup64 *group=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+p->value44+i;
            group->value3E=static_cast<unsigned short>(parameter);
            if (value1C+value20<*data+4) return static_cast<int>(0x80004005);
            unsigned int records=*reinterpret_cast<unsigned int *>(*data); *data+=4;
            if (group->ResizeRecords(static_cast<unsigned short>(records))<0) return static_cast<int>(0x8007000e);
            if (value1C+value20<records*0x10+*data) return static_cast<int>(0x80004005);
            for (unsigned int j=0;j<group->value36;++j) {
                D3DXEffectRecord64 *record=group->array04+j;
                record->value30&=0xfffffffd; record->value00=1;
                record->value52=*reinterpret_cast<unsigned short *>(*data); *data+=4;
                record->value58=ug2FxTypeTable7B582C[record->value52][4];
                record->value54=*reinterpret_cast<unsigned short *>(*data); *data+=4;
                record->value54=0xffff;
                record->value08.value18=*reinterpret_cast<unsigned int *>(*data); *data+=4;
                record->value08.value1C=*reinterpret_cast<unsigned int *>(*data); *data+=4;
                record->value08.value14=0;
                record->value08.value04=record->value08.value18+value20;
                record->value08.value00=value20+record->value08.value1C;
                unsigned int code=record->value52;
                // The lookup above precedes this check in the original.
                if (code>=0xb6) return static_cast<int>(0x80004005);
                if (record->value58==0x09000000) {
                    unsigned int index=*reinterpret_cast<unsigned int *>(record->value08.value1C+value20);
                    reinterpret_cast<D3DXEffectReference18 *>(value18)[index].value00=ug2FxTypeTable7B582C[code][0];
                } else {
                    if ((record->value58&0xff000000)!=0x03000000) return static_cast<int>(0x80004005);
                    unsigned int t=ug2FxTypeTable7B582C[code][0];
                    if (t!=1 && t!=2 && t!=3 && t!=0xf20000) return static_cast<int>(0x80004005);
                    unsigned int bytes=ug2FxTypeTable7B582C[code][2]*ug2FxTypeTable7B582C[code][1]*4;
                    unsigned int offset=group->value0C;
                    offset=(offset&3)+(offset>>2)*4;
                    offset=(offset&1)+(offset>>1)*2;
                    offset=(offset&3)+(offset>>2)*4;
                    offset=(offset&3)+(offset>>2)*4;
                    offset=(offset&3)+(offset>>2)*4;
                    group->value0C=offset; record->value48=offset;
                    group->value0C+=bytes;
                    record->value30&=0xfffffffd; record->value00=1;
                }
            }
        }
    } else {
        if (end<*data+count*4) return static_cast<int>(0x80004005);
        for (unsigned int i=0;i<count;++i) {
            unsigned int index=*reinterpret_cast<unsigned int *>(*data);
            if (index>=value14 || !index) return static_cast<int>(0x80004005);
            D3DXEffectReference18 *reference=reinterpret_cast<D3DXEffectReference18 *>(value18)+index;
            if (reference->value00!=0x7fffffff) return static_cast<int>(0x80004005);
            reference->value00=kind;
            reinterpret_cast<D3DXEffectReference18 *>(value18)[index].value14|=1;
            if (kind==16 || kind==15) {
                D3DXEffectParameter *p=reinterpret_cast<D3DXEffectParameter **>(valueB0)[parameter];
                if (value34<=value30) return static_cast<int>(0x80004005);
                *reinterpret_cast<unsigned short *>(&reinterpret_cast<D3DXEffectReference18 *>(value18)[index].value10)=value30;
                ++p->value46; ++value30;
                unsigned int groupIndex=(p->value44-1)+static_cast<unsigned int>(p->value46);
                D3DXEffectGroup64 *group=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+groupIndex;
                group->value3E=static_cast<unsigned short>(parameter);
                if (group->value38) return static_cast<int>(0x80004005);
                result=group->ResizeRecords(1); if (result<0) return result;
                if (kind==16) {
                    group->value18=0; group->array04->value52=0x92; group->array04->value58=0x0a000000;
                } else {
                    group->value1A=0; group->array04->value52=0x93; group->array04->value58=0x0b000000;
                }
                group->array04->value54=0xffff; group->array04->value56=1;
                group->array04->value00=1; group->array04->value30|=1; group->array04->value3C=0;
                unsigned int *destination=reinterpret_cast<unsigned int *>(group->array04)+2;
                unsigned int *source=reinterpret_cast<unsigned int *>(p);
                for (unsigned int j=0;j<10;++j) destination[j]=source[j];
                group->array04->value08.array20=0; group->array04->value08.linked24=0;
                group->array04->value08.value1C=i<<2;
                unsigned int element=i;
                if (!reinterpret_cast<unsigned int *>(*type)[4]) element=0xffffffff;
                group->array04->value08.value08=element;
                group->array04->value08.value0C=4;
                unsigned int *cookie=static_cast<unsigned int *>(UG2FxMalloc(0x24));
                D3DXEffectRecord20 *uses=0;
                if (cookie) {
                    *cookie=1; uses=reinterpret_cast<D3DXEffectRecord20 *>(cookie+1);
                    D3DXEffectConstructArray(uses,0x20,1,D3DXConstructUse);
                }
                group->array04->array34=uses;
                if (!uses) return static_cast<int>(0x8007000e);
                group->array04->value50=1; group->array04->value4E=group->array04->value50;
                result=LinkParameter(groupIndex,0,0,p,0); if (result<0) return result;
            }
            *data+=4;
        }
    }
    *type+=0x14;
    return result;
}
