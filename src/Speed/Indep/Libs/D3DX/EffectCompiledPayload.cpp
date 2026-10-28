#include "EffectCompiledPayload.h"

// Original 0x671db5. Walk the token stream's own lengths. There is no added
// byte-bound check or instruction decoder beyond this original format walk.
extern "C" int __stdcall D3DXEffectFindComment(const unsigned int *tokens,unsigned int tag,unsigned int **data,unsigned int *bytes) {
    if (data) *data=0;
    if (bytes) *bytes=0;
    if (!tokens) return static_cast<int>(0x8876086c);
    unsigned int version=*tokens,kind=version&0xffff0000;
    if (kind!=0x46580000 && kind!=0x54580000 && kind!=0x7ffe0000 && kind!=0x7fff0000 && kind!=0xfffe0000 && kind!=0xffff0000)
        return static_cast<int>(0x88760b59);
    const unsigned int *cursor=tokens+1;
    for (;;) {
        unsigned int token=*cursor++;
        if (static_cast<int>(token)<0) continue;
        unsigned int opcode=token&0xffff;
        if (opcode==0xffff) return 1;
        if (opcode==0xfffe) {
            unsigned int count=(token>>16)&0x7fff;
            if (count>1 && tag==cursor[0]) {
                if (data) *data=const_cast<unsigned int *>(cursor+1);
                if (bytes) *bytes=count*4-4;
                return 0;
            }
            cursor+=count;
        } else if ((version&0xffff)>=0x200) cursor+=(token>>24)&15;
        else if (opcode==0x51) cursor+=5;
    }
}

// Original 0x65b3f7. The four register banks retain the original remapping,
// word overflow, minimum/maximum comparisons and strict default-data bound.
extern "C" int __stdcall D3DXEffectReadConstantTable(const unsigned int *tokens,unsigned int **table,unsigned short *counts,unsigned short *minimum,unsigned short *maximum) {
    for (unsigned int i=0;i<4;++i) counts[i]=0;
    for (unsigned int i=0;i<4;++i) minimum[i]=0xffff;
    for (unsigned int i=0;i<4;++i) maximum[i]=0;
    *table=0;
    if (tokens) {
        unsigned int *header=0,span;
        int result=D3DXEffectFindComment(tokens,0x42415443,&header,&span);
        if (result>=0 && header) {
            if (span<0x1c || span<header[3]*4+0x1c) return static_cast<int>(0x80004005);
            unsigned int *entry=reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned int>(header)+header[4]);
            for (unsigned int i=0;i<header[3];++i,entry+=5) {
                unsigned int length=0,offset=entry[0];
                while (offset<span && reinterpret_cast<unsigned char *>(header)[offset]) { ++offset; ++length; }
                if (span<=entry[0]+length) return static_cast<int>(0x80004005);
                unsigned int bank,unit;
                switch (static_cast<unsigned short>(entry[1])) {
                case 0: bank=1;unit=4;break;
                case 1: bank=2;unit=16;break;
                case 2: bank=0;unit=16;break;
                case 3: bank=3;unit=0;break;
                default: return static_cast<int>(0x80004005);
                }
                unsigned short count=static_cast<unsigned short>(entry[2]),start=static_cast<unsigned short>(entry[1]>>16);
                counts[bank]=static_cast<unsigned short>(counts[bank]+count);
                unsigned int end=static_cast<unsigned int>(count)+start;
                if (maximum[bank]<=end) maximum[bank]=static_cast<unsigned short>(end);
                if (start<=minimum[bank]) minimum[bank]=start;
                unsigned int bytes=static_cast<unsigned int>(count)*unit;
                if (bytes && span<=entry[4]+bytes) return static_cast<int>(0x80004005);
            }
            *table=header;
        }
    }
    return 0;
}

// Original 0x688d2c. Cursor stores precede each tag check, including failures.
extern "C" int __stdcall D3DXEffectWalkOperands(unsigned int count,unsigned int **cursor) {
    for (unsigned int i=0;i<count;++i) {
        unsigned int terms=**cursor;
        ++*cursor;
        for (unsigned int j=0;j<terms;++j) {
            unsigned int kind=**cursor;
            *cursor+=2;
            if (kind==9) return static_cast<int>(0x80004005);
        }
        unsigned int kind=**cursor;
        *cursor+=2;
        if (!kind || kind>9) return static_cast<int>(0x80004005);
    }
    return 0;
}
// Original 0x688d8c. A nonzero first word advances only one DWORD.
extern "C" int __stdcall D3DXEffectWalkResult(unsigned int **cursor) {
    unsigned int *start=*cursor;
    unsigned int first=start[0];
    *cursor=start+1;
    if (!first) {
        unsigned int kind=start[1];
        *cursor=start+3;
        if (kind==7 || kind==4 || kind==5 || kind==6) return 0;
    }
    return static_cast<int>(0x80004005);
}
// Original 0x688dc9. Both section pointers are published before operand walks.
extern "C" int __stdcall D3DXEffectReadCompiledSections(const unsigned int *tokens,unsigned int **instructions,unsigned int **literals) {
    unsigned int version=*tokens;
    if (version<0x46580200 || (version>0x46580201 && version!=0x54580100)) return static_cast<int>(0x80004005);
    int result=D3DXEffectFindComment(tokens,0x434c5846,instructions,0);
    if (result<0 || result==1) return static_cast<int>(0x80004005);
    result=D3DXEffectFindComment(tokens,0x54494c43,literals,0);
    if (result<0 || result==1 || !*instructions || !*literals) return static_cast<int>(0x80004005);
    unsigned int *cursor=*instructions;
    unsigned int count=*cursor++;
    for (unsigned int i=0;i<count;++i) {
        ++cursor;
        unsigned int operands=*cursor++;
        result=D3DXEffectWalkOperands(operands,&cursor);
        if (result<0) return result;
        result=D3DXEffectWalkResult(&cursor);
        if (result<0) return result;
    }
    return 0;
}

// Original 0x65be0d. The depth argument is unused but occupies the second
// stack slot. Encoded handles are complements of observed record addresses.
D3DXEffectParameter *D3DXEffectObject::FindRootParameter(unsigned int handle,unsigned int depth) {
    (void)depth;
    if ((handle&0xffff)==handle) return 0;
    if (static_cast<int>(handle)<1) {
        D3DXEffectBaseRecord *record=reinterpret_cast<D3DXEffectBaseRecord *>(~handle);
        if (record->value10==3 && record->value14<valueA8 && !record->value18 && !record->value1C && record->value08==0xffffffff) {
            D3DXEffectParameter *parameter=reinterpret_cast<D3DXEffectParameter **>(valueB0)[record->value14];
            if (!(parameter->value28&4)) return parameter;
        }
    } else {
        const unsigned char *query=reinterpret_cast<const unsigned char *>(handle);
        for (unsigned int i=0;i<value98;++i) {
            D3DXEffectParameter *parameter=reinterpret_cast<D3DXEffectParameter *>(value9C)+i;
            unsigned int base=parameter->value40 ? reinterpret_cast<unsigned int>(reinterpret_cast<D3DXEffectPool *>(value38)->data10) : value20;
            unsigned int nameOffset=*reinterpret_cast<unsigned int *>(base+parameter->value2C+8);
            unsigned int *prefix=reinterpret_cast<unsigned int *>(base+nameOffset);
            const unsigned char *name=*prefix ? reinterpret_cast<unsigned char *>(prefix+1) : 0;
            if (name) {
                const unsigned char *a=name,*b=query;
                while (*a==*b && *a) { ++a; ++b; }
                if (*a==*b) return parameter;
            }
        }
    }
    return 0;
}

// Original 0x660d4e. Link each CTAB entry before reserving its register storage.
int D3DXEffectObject::BindCompiledRecord(unsigned int groupIndex,unsigned int recordIndex,unsigned int *data) {
    D3DXEffectGroup64 *group=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+groupIndex;
    D3DXEffectRecord64 *record=group->array04+recordIndex;
    if (record->value50 || record->array34) return static_cast<int>(0x80004005);
    record->value30&=0xfffffffd;record->value4E=0;record->value50=0;record->value00=1;
    unsigned short counts[4],minimum[4],maximum[4];
    int result=D3DXEffectReadConstantTable(data+8,reinterpret_cast<unsigned int **>(&record->value3C),counts,minimum,maximum);
    if (result>=0 && record->value3C) {
        unsigned int count=reinterpret_cast<unsigned int *>(record->value3C)[3];
        unsigned int *cookie=static_cast<unsigned int *>(UG2FxMalloc(count*0x20+4));
        D3DXEffectUseLink *uses=0;
        if (cookie) { *cookie=count;uses=reinterpret_cast<D3DXEffectUseLink *>(cookie+1);D3DXEffectConstructArray(uses,0x20,count,D3DXConstructUse); }
        record->array34=reinterpret_cast<D3DXEffectRecord20 *>(uses);
        if (!uses) return static_cast<int>(0x8007000e);
        record->value50=static_cast<unsigned short>(reinterpret_cast<unsigned int *>(record->value3C)[3]);
        unsigned int *entry=reinterpret_cast<unsigned int *>(record->value3C+reinterpret_cast<unsigned int *>(record->value3C)[4]);
        for (unsigned int i=0;i<reinterpret_cast<unsigned int *>(record->value3C)[3];++i,entry+=5) {
            D3DXEffectParameter *parameter=FindRootParameter(record->value3C+entry[0],10);
            if (!parameter) return static_cast<int>(0x80004005);
            unsigned int index=record->value4E++;
            result=LinkParameter(groupIndex,recordIndex,index,parameter,reinterpret_cast<unsigned int>(entry));
            if (result<0) return result;
        }
        unsigned int mask=static_cast<unsigned int>(maximum[1])+(static_cast<unsigned int>(maximum[0])+maximum[2])*4;
        data[2]=mask;
        if (mask) --mask;
        mask|=mask>>1;mask|=mask>>2;mask|=mask>>4;mask|=mask>>8;mask|=mask>>16;
        data[2]=mask;
        unsigned int offset=group->value0C;
        unsigned int limit=(offset&7)+8+((offset>>3)+mask)*8;
        offset=(offset&7)+(offset>>3)*8;group->value0C=offset;data[4]=offset;
        offset=group->value0C+static_cast<unsigned int>(maximum[0])*0x20;
        offset=(offset&7)+(offset>>3)*8;group->value0C=offset;data[5]=offset;
        offset=group->value0C+static_cast<unsigned int>(maximum[1])*8;
        offset=(offset&7)+(offset>>3)*8;group->value0C=offset;data[6]=offset;
        group->value0C+=static_cast<unsigned int>(maximum[2])*0x20;
        if (group->value0C<=limit) group->value0C=limit;
    }
    return result;
}

// Original 0x661a40. Parse sections twice around the final length check, then
// link the CTAB and allocate the original mask-sized group storage.
int D3DXEffectObject::ReadCompiledRecord(unsigned int groupIndex,unsigned int recordIndex,D3DXEffectBuffer *buffer,unsigned int *position) {
    D3DXEffectGroup64 *group=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+groupIndex;
    D3DXEffectRecord64 *record=group->array04+recordIndex;
    unsigned int size;
    if (buffer) {
        unsigned int limit=UG2FxBufferLengthVirtual(buffer);
        if (*position+1<=limit) {
            unsigned char *bytes=UG2FxBufferPointerVirtual(buffer);
            size=*reinterpret_cast<unsigned int *>(bytes+*position*4);
        }
    }
    ++*position;
    unsigned int limit=0;
    if (buffer) limit=UG2FxBufferLengthVirtual(buffer);
    if (buffer && *position<=limit) {
        int result=D3DXEffectBufferCreateForward(size+0x20,reinterpret_cast<D3DXEffectBuffer **>(&record->value38));
        if (result<0) return result;
        unsigned int *data=reinterpret_cast<unsigned int *>(UG2FxBufferPointerVirtual(reinterpret_cast<D3DXEffectBuffer *>(record->value38)));
        data[2]=0;
        if (!size) return result;
        unsigned int words=(size>>2)+((size&3)!=0);
        limit=UG2FxBufferLengthVirtual(buffer);
        if (*position+words<=limit) {
            unsigned char *bytes=UG2FxBufferPointerVirtual(buffer);bytes+=*position*4;
            unsigned int i;
            for (i=0;i<(size>>2);++i) data[8+i]=reinterpret_cast<unsigned int *>(bytes)[i];
            for (i=size&0xfffffffc;i<size;++i) reinterpret_cast<unsigned char *>(data+8)[i]=bytes[i];
        }
        *position+=words;
        result=D3DXEffectReadCompiledSections(data+8,reinterpret_cast<unsigned int **>(&record->value40),reinterpret_cast<unsigned int **>(&record->value44));
        if (result<0) return result;
        limit=UG2FxBufferLengthVirtual(buffer);
        if (*position<=limit) {
            result=D3DXEffectReadCompiledSections(data+8,reinterpret_cast<unsigned int **>(&record->value40),reinterpret_cast<unsigned int **>(&record->value44));
            if (result<0) return result;
            result=BindCompiledRecord(groupIndex,recordIndex,data);
            if (result<0) return result;
            unsigned int type=record->value08.value18+value20;
            unsigned int units=D3DXEffectUnitCount(&type,4);data[0]=units;
            unsigned int mask=units*4;data[1]=mask;
            if (mask) --mask;
            mask|=mask>>1;mask|=mask>>2;mask|=mask>>4;mask|=mask>>8;mask|=mask>>16;data[1]=mask;
            unsigned int offset=(group->value0C&7)+(group->value0C>>3)*8;
            group->value0C=offset;data[3]=offset;group->value0C+=mask*8+8;
            record->value00=1;
            return result;
        }
    }
    return static_cast<int>(0x80004005);
}
