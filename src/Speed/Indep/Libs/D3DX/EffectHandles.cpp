#include "EffectHandles.h"

// Original 0x75d5d3. Retain locale access, byte classification and wrapping
// decimal arithmetic; do not substitute a saturating host strtol/atol call.
extern "C" int __cdecl D3DXEffectAtolBody(const char *text) {
    const unsigned char *thread=static_cast<const unsigned char *>(UG2FxCrtGetThreadData());
    const unsigned char *locale=*reinterpret_cast<const unsigned char *const *>(thread+0x64);
    if (reinterpret_cast<unsigned int>(locale)!=ug2FxCrtLocale819D24) locale=static_cast<const unsigned char *>(UG2FxCrtUpdateLocale());
    const unsigned char *cursor=reinterpret_cast<const unsigned char *>(text);
    for (;;) {
        unsigned int space;
        if (*reinterpret_cast<const int *>(locale+0x28)>1) space=UG2FxCrtClassify(locale,*cursor,8);
        else space=(*reinterpret_cast<const unsigned char *const *>(locale+0x48))[*cursor*2]&8;
        if (!space) break;
        ++cursor;
    }
    unsigned int sign=*cursor++,character=sign;
    if (character=='-' || character=='+') character=*cursor++;
    unsigned int value=0;
    for (;;) {
        unsigned int digit=(character<'0' || character>'9') ? 0xffffffff : character-'0';
        if (digit==0xffffffff) break;
        value=digit+value*10;
        character=*cursor++;
    }
    if (sign=='-') value=0-value;
    return static_cast<int>(value);
}
// Original 0x75d65b is a jump to 0x75d5d3.
extern "C" int __cdecl D3DXEffectAtol(const char *text) { return D3DXEffectAtolBody(text); }

// Original 0x65c61f. The unused depth argument still occupies a stack slot.
D3DXEffectRecordB4 *D3DXEffectObject::FindTechnique(unsigned int handle,unsigned int depth) {
    (void)depth;
    if ((handle&0xffff)==handle) return 0;
    unsigned int index=0;
    if (static_cast<int>(handle)<1) {
        D3DXEffectBaseRecord *record=reinterpret_cast<D3DXEffectBaseRecord *>(~handle);
        index=static_cast<unsigned short>(record->value14);
        if (record->value10!=0 || valueA0<=index) return 0;
    } else {
        for (;index<valueA0;++index) {
            D3DXEffectRecordB4 *record=reinterpret_cast<D3DXEffectRecordB4 *>(valueA4)+index;
            unsigned int *prefix=reinterpret_cast<unsigned int *>(value20+record->value28);
            if (*prefix) {
                const unsigned char *a=reinterpret_cast<unsigned char *>(prefix+1),*b=reinterpret_cast<unsigned char *>(handle);
                while (*a==*b && *a) { ++a; ++b; }
                if (*a==*b) break;
            }
        }
        if (index==valueA0) return 0;
    }
    return reinterpret_cast<D3DXEffectRecordB4 *>(valueA4)+index;
}
// Original 0x65c5de. Unlike technique lookup, this receives a complemented
// record directly without a small-integer/name gate. Preserve both halfwords.
D3DXEffectRecord38 *D3DXEffectObject::FindPass(unsigned int handle,unsigned int depth) {
    (void)depth;
    D3DXEffectBaseRecord *record=reinterpret_cast<D3DXEffectBaseRecord *>(~handle);
    unsigned int index=static_cast<unsigned short>(record->value14);
    if (record->value10==1 && index<valueA0) {
        D3DXEffectRecordB4 *technique=reinterpret_cast<D3DXEffectRecordB4 *>(valueA4)+index;
        unsigned int pass=static_cast<unsigned short>(record->value18);
        if (pass<technique->value34) return technique->array38+pass;
    }
    return 0;
}

// Original 0x661c3e. Rebuild the entire member array; old children are deleted
// before allocation, and both value and padded size walks retain their cursors.
int D3DXEffectObject::BuildMembers(D3DXEffectBaseRecord *record,unsigned int count) {
    if (record->array20) { record->array20->DeleteArray(3); record->array20=0; }
    unsigned int *cookie=static_cast<unsigned int *>(UG2FxMalloc(count*0x28+4));
    D3DXEffectBaseRecord *members=0;
    if (cookie) { *cookie=count;members=reinterpret_cast<D3DXEffectBaseRecord *>(cookie+1);D3DXEffectConstructArray(members,0x28,count,D3DXConstructBase); }
    record->array20=members;
    if (!members) return static_cast<int>(0x8007000e);
    D3DXEffectParameter *owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[record->value14];
    unsigned int type=owner->value2C+0x18+record->value18,paddedType=type;
    unsigned int data=owner->value30+record->value1C,paddedData=data;
    for (unsigned int i=0;i<count;++i) {
        D3DXEffectBaseRecord *member=record->array20+i;
        member->value10=3;member->value14=record->value14;
        owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[record->value14];member->value18=type-owner->value2C;
        owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[record->value14];member->value1C=paddedData-owner->value30;
        unsigned int base=owner->value40 ? reinterpret_cast<unsigned int>(reinterpret_cast<D3DXEffectPool *>(value38)->data10) : value20;
        unsigned int size=D3DXEffectValueSize(base,&type,&data);(record->array20+i)->value0C=size;
        owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[record->value14];
        base=owner->value40 ? reinterpret_cast<unsigned int>(reinterpret_cast<D3DXEffectPool *>(value38)->data10) : value20;
        D3DXEffectPaddedSize(base,&paddedType,&paddedData);
        member=record->array20+i;owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[member->value14];
        base=owner->value40 ? reinterpret_cast<unsigned int>(reinterpret_cast<D3DXEffectPool *>(value38)->data10) : value20;
        member->value04=owner->value2C+member->value18+base;
        member=record->array20+i;owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[member->value14];
        base=owner->value40 ? reinterpret_cast<unsigned int>(reinterpret_cast<D3DXEffectPool *>(value38)->data10) : value20;
        member->value00=owner->value30+member->value1C+base;
    }
    return 0;
}

// Original 0x65c123. Indexed children form a sorted linked list. Return the
// complement of a cached/new child, with unsigned stride divisions unchanged.
unsigned int D3DXEffectObject::ResolveElement(unsigned int handle,unsigned int index,unsigned int depth) {
    if (!handle) return 0;
    D3DXEffectBaseRecord *record=static_cast<int>(handle)<0 ? reinterpret_cast<D3DXEffectBaseRecord *>(~handle) : ResolveParameter(handle,depth,0);
    if (!record) return 0;
    unsigned int count=reinterpret_cast<unsigned int *>(record->value04)[4];
    if (record->value08!=0xffffffff || !count || count<=index) return 0;
    D3DXEffectBaseRecord *previous=0,*child=record->linked24;
    while (child && child->value08<index) { previous=child;child=child->linked24; }
    if (child && child->value08<=index) return ~reinterpret_cast<unsigned int>(child);
    child=static_cast<D3DXEffectBaseRecord *>(UG2FxMalloc(0x28));
    if (!child) return 0;
    child->value08=0xffffffff;child->linked24=0;child->array20=0;child->value00=0;child->value04=0;
    D3DXEffectParameter *owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[record->value14];
    unsigned int paddedType=owner->value2C+record->value18,type=paddedType;
    unsigned int paddedData=owner->value30+record->value1C,data=paddedData;
    unsigned int base=owner->value40 ? reinterpret_cast<unsigned int>(reinterpret_cast<D3DXEffectPool *>(value38)->data10) : value20;
    unsigned int size=D3DXEffectValueSize(base,&type,&data);
    owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[record->value14];
    base=owner->value40 ? reinterpret_cast<unsigned int>(reinterpret_cast<D3DXEffectPool *>(value38)->data10) : value20;
    D3DXEffectPaddedSize(base,&paddedType,&paddedData);
    child->value10=3;child->value14=record->value14;child->value18=record->value18;
    owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[record->value14];
    child->value1C=((paddedData-owner->value30-record->value1C)/count)*index+record->value1C;
    child->value0C=size/count;child->value08=index;child->value04=record->value04;
    owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[child->value14];
    base=owner->value40 ? reinterpret_cast<unsigned int>(reinterpret_cast<D3DXEffectPool *>(value38)->data10) : value20;
    child->value00=owner->value30+base+child->value1C;
    if (previous) { child->linked24=previous->linked24;previous->linked24=child; }
    else { child->linked24=record->linked24;record->linked24=child; }
    return ~reinterpret_cast<unsigned int>(child);
}

// Original 0x66bbee. Raw pointers are returned internally; suffix helpers use
// complemented handles. The annotation-create bit is cleared after that call.
D3DXEffectBaseRecord *D3DXEffectObject::ResolveParameter(unsigned int handle,unsigned int depth,unsigned int create) {
    if ((handle&0xffff)==handle) return 0;
    if (static_cast<int>(handle)<=0) {
        D3DXEffectBaseRecord *record=reinterpret_cast<D3DXEffectBaseRecord *>(~handle);
        if (record->value10==3 && record->value14<valueA8) return record;
        return 0;
    }
    const char *query=reinterpret_cast<const char *>(handle);
    for (unsigned int i=0;i<value98;++i) {
        D3DXEffectBaseRecord *record=reinterpret_cast<D3DXEffectBaseRecord *>(reinterpret_cast<D3DXEffectParameter *>(value9C)+i);
        D3DXEffectParameter *owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[record->value14];
        unsigned int base=owner->value40 ? reinterpret_cast<unsigned int>(reinterpret_cast<D3DXEffectPool *>(value38)->data10) : value20;
        unsigned int offset=reinterpret_cast<unsigned int *>(record->value04)[2];
        const char *name=*reinterpret_cast<unsigned int *>(base+offset) ? reinterpret_cast<const char *>(base+offset+4) : 0;
        const char *cursor=query;
        while (*cursor==*name && *cursor) { ++cursor;++name; }
        char next=*cursor;
        if (next==*name) return record;
        unsigned int resolved=0;
        if (!*name) {
            if (next=='.') resolved=ResolveMember(~reinterpret_cast<unsigned int>(record),cursor+1,depth);
            else if (next=='@') {
                value118^=(create*2^value118)&2;
                resolved=ResolveAnnotation(~reinterpret_cast<unsigned int>(record),cursor+1,depth);
                value118&=0xfffffffd;
            } else if (next=='[') {
                ++cursor;if (*cursor<'0' || *cursor>'9') return 0;
                unsigned int index=static_cast<unsigned int>(D3DXEffectAtol(cursor));
                if (reinterpret_cast<unsigned int *>(record->value04)[4]<=index) return 0;
                while (*cursor>='0' && *cursor<='9') ++cursor;
                if (*cursor!=']') return 0;
                resolved=ResolveElement(~reinterpret_cast<unsigned int>(record),index,depth);
                if (!resolved) return 0;
                if (!cursor[1]) return reinterpret_cast<D3DXEffectBaseRecord *>(~resolved);
                if (cursor[1]!='.') return 0;
                resolved=ResolveMember(resolved,cursor+2,depth);
            } else continue;
            return resolved ? reinterpret_cast<D3DXEffectBaseRecord *>(~resolved) : 0;
        }
    }
    return 0;
}

// Original 0x66a83c. Dot traversal re-enters the same body without growing the
// call stack; annotation and indexed-member suffixes retain their own calls.
unsigned int D3DXEffectObject::ResolveMember(unsigned int handle,const char *name,unsigned int depth) {
    for (;;) {
        if (!handle) {
            D3DXEffectBaseRecord *record=name ? ResolveParameter(reinterpret_cast<unsigned int>(name),depth+1,0) : 0;
            return record ? ~reinterpret_cast<unsigned int>(record) : 0;
        }
        D3DXEffectBaseRecord *record=ResolveParameter(handle,depth+1,0);
        if (!record) return 0;
        if (!name) return ~reinterpret_cast<unsigned int>(record);
        unsigned int *type=reinterpret_cast<unsigned int *>(record->value04);
        if (type[1]!=5 || (record->value08==0xffffffff && type[4])) return 0;
        unsigned int count=type[5];
        if (!record->array20 && BuildMembers(record,count)<0) return 0;
        bool descend=false;
        for (unsigned int i=0;i<count;++i) {
            D3DXEffectBaseRecord *member=record->array20+i;
            D3DXEffectParameter *owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[record->value14];
            unsigned int base=owner->value40 ? reinterpret_cast<unsigned int>(reinterpret_cast<D3DXEffectPool *>(value38)->data10) : value20;
            unsigned int offset=reinterpret_cast<unsigned int *>(member->value04)[2];
            const char *candidate=*reinterpret_cast<unsigned int *>(base+offset) ? reinterpret_cast<const char *>(base+offset+4) : 0;
            const char *cursor=name;
            while (*cursor==*candidate && *cursor) { ++cursor;++candidate; }
            if (*cursor==*candidate) return ~reinterpret_cast<unsigned int>(member);
            if (!*candidate) {
                if (*cursor=='.') { handle=~reinterpret_cast<unsigned int>(member);name=cursor+1;descend=true;break; }
                if (*cursor=='@') return ResolveAnnotation(~reinterpret_cast<unsigned int>(member),cursor+1,depth);
                if (*cursor=='[') {
                    ++cursor;if (*cursor<'0' || *cursor>'9') return 0;
                    unsigned int index=static_cast<unsigned int>(D3DXEffectAtol(cursor));
                    if (reinterpret_cast<unsigned int *>(member->value04)[4]<=index) return 0;
                    while (*cursor>='0' && *cursor<='9') ++cursor;
                    if (*cursor!=']') return 0;
                    handle=ResolveElement(~reinterpret_cast<unsigned int>(member),index,depth);
                    if (!handle) return 0;
                    ++cursor;if (!*cursor) return handle;
                    if (*cursor!='.') return 0;
                    name=cursor+1;descend=true;break;
                }
            }
        }
        if (!descend) return 0;
    }
}

// Original 0x665a15. New annotation publication is deliberately not atomic:
// the extended blob table is installed before descriptor validation/allocation.
unsigned int D3DXEffectObject::ResolveAnnotation(unsigned int handle,const char *name,unsigned int depth) {
    if (!name || !*name) return 0;
    unsigned int *count,*head;
    D3DXEffectParameter *root=FindRootParameter(handle,10);
    if (root) { count=&root->value38;head=&root->value3C; }
    else {
        D3DXEffectRecordB4 *technique=FindTechnique(handle,10);
        if (technique) { count=&technique->value2C;head=reinterpret_cast<unsigned int *>(&technique->parameter30); }
        else {
            D3DXEffectRecord38 *pass=FindPass(handle,10);
            if (!pass) return 0;
            count=&pass->value2C;head=reinterpret_cast<unsigned int *>(&pass->parameter30);
        }
    }
    D3DXEffectParameter *tail=0;
    for (D3DXEffectParameter *annotation=reinterpret_cast<D3DXEffectParameter *>(*head);annotation;annotation=reinterpret_cast<D3DXEffectParameter *>(annotation->value3C)) {
        unsigned int base;
        if (annotation->value14<valueAC) {
            D3DXEffectParameter *owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[annotation->value14];
            base=owner->value40 ? reinterpret_cast<unsigned int>(reinterpret_cast<D3DXEffectPool *>(value38)->data10) : value20;
        } else base=reinterpret_cast<unsigned int *>(value11C)[annotation->value14-valueAC];
        unsigned int offset=reinterpret_cast<unsigned int *>(annotation->value04)[2];
        const char *candidate=*reinterpret_cast<unsigned int *>(base+offset) ? reinterpret_cast<const char *>(base+offset+4) : 0;
        const char *cursor=name;
        while (*cursor==*candidate && *cursor) { ++cursor;++candidate; }
        if (*cursor==*candidate) return ~reinterpret_cast<unsigned int>(annotation);
        if (!*candidate) {
            if (*cursor=='.') return ResolveMember(~reinterpret_cast<unsigned int>(annotation),cursor+1,depth);
            if (*cursor=='[') {
                ++cursor;if (*cursor<'0' || *cursor>'9') return 0;
                unsigned int index=static_cast<unsigned int>(D3DXEffectAtol(cursor));
                if (reinterpret_cast<unsigned int *>(annotation->value04)[4]<=index) return 0;
                while (*cursor>='0' && *cursor<='9') ++cursor;
                if (*cursor!=']') return 0;
                unsigned int child=ResolveElement(~reinterpret_cast<unsigned int>(annotation),index,depth);
                if (!child) return 0;
                ++cursor;if (!*cursor) return child;
                if (*cursor!='.') return 0;
                return ResolveMember(child,cursor+1,depth);
            }
        }
        tail=annotation;
    }
    if (!(value118&2)) return 0;
    unsigned int blobIndex=value120,result=0;
    unsigned int *newTable=0,*newBlobs=0;
    D3DXEffectReference18 *newReferences=0;
    D3DXEffectParameter *pending=0;
    unsigned char *blob=0;
    unsigned int length=0,padded=0;
    for (const char *cursor=name;*cursor;++cursor) {
        if (*cursor=='.' || *cursor=='@' || *cursor=='[' || *cursor==']') goto cleanup;
    }
    while (name[length]) ++length;
    ++length;padded=((length>>2)+((length&3)!=0))*4;
    newTable=static_cast<unsigned int *>(UG2FxMalloc(valueA8*4+4));
    if (!newTable) goto cleanup;
    newBlobs=static_cast<unsigned int *>(UG2FxMalloc(value120*4+4));
    if (!newBlobs) goto cleanup;
    pending=static_cast<D3DXEffectParameter *>(UG2FxMalloc(0x50));
    if (!pending) goto cleanup;
    pending->Construct();
    {
        unsigned int *old=reinterpret_cast<unsigned int *>(value11C);
        unsigned int words=(value120*4)>>2;
        for (unsigned int i=0;i<words;++i) newBlobs[i]=old[i];
        newBlobs[value120]=0;
        UG2FxFree(reinterpret_cast<void *>(value11C));value11C=reinterpret_cast<unsigned int>(newBlobs);++value120;newBlobs=0;
    }
    pending->value08=0xffffffff;pending->value14=static_cast<unsigned short>(valueA8);
    pending->value2C=padded+4;pending->value28=4;pending->value18=0;pending->value1C=0;
    {
        unsigned int *description=reinterpret_cast<unsigned int *>(valueC4);
        int shape=static_cast<int>(description[1]);
        unsigned int kind=description[0],elements=description[4];
        if (shape>=0 && shape<4 && (kind==1 || kind==2 || kind==3)) {
            unsigned int bytes=description[6]*description[5]*4;
            pending->value0C=4;pending->value0C=bytes;
            if (elements) { pending->value0C*=elements;bytes*=elements; }
            unsigned int size=bytes+0x20+padded;
            blob=static_cast<unsigned char *>(UG2FxMalloc(size));
            if (!blob) goto cleanup;
            unsigned int i;
            for (i=0;i<(size>>2);++i) reinterpret_cast<unsigned int *>(blob)[i]=0;
            for (i=size&0xfffffffc;i<size;++i) blob[i]=0;
            description=reinterpret_cast<unsigned int *>(valueC4);
            for (i=0;i<7;++i) reinterpret_cast<unsigned int *>(blob+pending->value2C)[i]=description[i];
            pending->value30=pending->value2C+0x1c;
        } else if (shape==4 && !elements && kind==4) {
            unsigned int size=padded+0x1c;
            blob=static_cast<unsigned char *>(UG2FxMalloc(size));
            if (!blob) goto cleanup;
            unsigned int total=value14+1;
            unsigned int *cookie=static_cast<unsigned int *>(UG2FxMalloc(total*0x18+4));
            if (cookie) { *cookie=total;newReferences=reinterpret_cast<D3DXEffectReference18 *>(cookie+1);D3DXEffectConstructArray(newReferences,0x18,total,D3DXConstructReference18); }
            if (!newReferences) goto cleanup;
            unsigned int i;
            for (i=0;i<(size>>2);++i) reinterpret_cast<unsigned int *>(blob)[i]=0;
            for (i=size&0xfffffffc;i<size;++i) blob[i]=0;
            for (i=0;i<6;++i) reinterpret_cast<unsigned int *>(newReferences+value14)[i]=0;
            newReferences[value14].value00=4;
            unsigned int *old=reinterpret_cast<unsigned int *>(value18),*copied=reinterpret_cast<unsigned int *>(newReferences);
            unsigned int words=(value14*0x18)>>2;
            for (i=0;i<words;++i) copied[i]=old[i];
            pending->value0C=4;pending->value30=pending->value2C+0x14;
            description=reinterpret_cast<unsigned int *>(valueC4);
            for (i=0;i<5;++i) reinterpret_cast<unsigned int *>(blob+pending->value2C)[i]=description[i];
            *reinterpret_cast<unsigned int *>(blob+pending->value30)=value14;
            for (i=0;i<words;++i) old[i]=0;
            if (value18) { reinterpret_cast<D3DXEffectReference18 *>(value18)->DeleteArray(3);value18=0; }
            ++value14;value18=reinterpret_cast<unsigned int>(newReferences);newReferences=0;
        } else goto cleanup;
    }
    {
        *reinterpret_cast<unsigned int *>(blob)=length;
        unsigned int i;
        for (i=0;i<(length>>2);++i) reinterpret_cast<unsigned int *>(blob+4)[i]=reinterpret_cast<const unsigned int *>(name)[i];
        for (i=length&0xfffffffc;i<length;++i) blob[4+i]=static_cast<unsigned char>(name[i]);
        *reinterpret_cast<unsigned int *>(blob+pending->value2C+8)=0;
        *reinterpret_cast<unsigned int *>(blob+pending->value2C+0x0c)=0;
        unsigned int *old=reinterpret_cast<unsigned int *>(valueB0);
        unsigned int words=(valueA8*4)>>2;
        for (i=0;i<words;++i) newTable[i]=old[i];
        newTable[valueA8]=reinterpret_cast<unsigned int>(pending);
        if (valueB0) { UG2FxFree(reinterpret_cast<void *>(valueB0));valueB0=0; }
        ++valueA8;valueB0=reinterpret_cast<unsigned int>(newTable);newTable=0;
        if (tail) tail->value3C=reinterpret_cast<unsigned int>(pending);
        else *head=reinterpret_cast<unsigned int>(pending);
        ++*count;
        pending->value04=reinterpret_cast<unsigned int>(blob)+pending->value2C;
        pending->value00=reinterpret_cast<unsigned int>(blob)+pending->value30;
        result=~reinterpret_cast<unsigned int>(pending);
        reinterpret_cast<unsigned int *>(value11C)[blobIndex]=reinterpret_cast<unsigned int>(blob);
        blob=0;pending=0;
    }
cleanup:
    if (pending) pending->Delete(1);
    if (newTable) UG2FxFree(newTable);
    if (newBlobs) UG2FxFree(newBlobs);
    if (newReferences) newReferences->DeleteArray(3);
    if (blob) UG2FxFree(blob);
    return result;
}

#define FX_SYMBOL_WORD(destination) do { \
    if (buffer) { unsigned int limit=UG2FxBufferLengthVirtual(buffer); \
        if (*position+1<=limit) { unsigned char *bytes=UG2FxBufferPointerVirtual(buffer); \
            destination=*reinterpret_cast<unsigned int *>(bytes+*position*4); } } \
    ++*position; \
} while (0)

// Original 0x66c37e. Resolve a staged symbol and convert compatible resource
// records, or attach a single parameter-use link for kinds 5 and 10.
int D3DXEffectObject::ReadSymbol(unsigned int groupIndex,unsigned int recordIndex,D3DXEffectBuffer *buffer,unsigned int *position) {
    D3DXEffectGroup64 *group=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+groupIndex;
    D3DXEffectRecord64 *record=group->array04+recordIndex;
    unsigned int kind=ug2FxTypeTable7B582C[record->value52][0];
    if (kind!=16 && kind!=15 && kind!=5 && kind!=10) return static_cast<int>(0x80004005);
    unsigned int size;
    FX_SYMBOL_WORD(size);
    if (!buffer) return static_cast<int>(0x80004005);
    unsigned int limit=UG2FxBufferLengthVirtual(buffer);
    if (limit<*position || size<2) return static_cast<int>(0x80004005);
    unsigned char *temporary=static_cast<unsigned char *>(UG2FxMalloc(size));
    int result=static_cast<int>(0x8007000e);
    if (temporary) {
        unsigned int words=(size>>2)+((size&3)!=0);
        limit=UG2FxBufferLengthVirtual(buffer);
        if (*position+words<=limit) {
            unsigned char *source=UG2FxBufferPointerVirtual(buffer);source+=*position*4;
            unsigned int i;
            for (i=0;i<(size>>2);++i) reinterpret_cast<unsigned int *>(temporary)[i]=reinterpret_cast<unsigned int *>(source)[i];
            for (i=size&0xfffffffc;i<size;++i) temporary[i]=source[i];
        }
        *position+=words;
        limit=UG2FxBufferLengthVirtual(buffer);
        result=static_cast<int>(0x80004005);
        if (*position<=limit && !temporary[size-1]) {
            D3DXEffectBaseRecord *parameter=ResolveParameter(reinterpret_cast<unsigned int>(temporary),2,0);
            if (parameter) {
                D3DXEffectParameter *owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[parameter->value14];
                unsigned int base=owner->value40 ? reinterpret_cast<unsigned int>(reinterpret_cast<D3DXEffectPool *>(value38)->data10) : value20;
                unsigned int type=*reinterpret_cast<unsigned int *>(owner->value2C+parameter->value18+base);
                record->value30&=0xfffffffd;kind=ug2FxTypeTable7B582C[record->value52][0];
                result=0;
                if (kind==5) {
                    if (type!=5 && type!=6 && type!=7 && type!=8 && type!=9) result=static_cast<int>(0x80004005);
                    record->value00=1;
                } else if (kind==10) {
                    if (type!=10 && type!=11 && type!=12 && type!=13 && type!=14) result=static_cast<int>(0x80004005);
                    record->value00=0;
                } else if (kind>14 && kind<17) {
                    if (kind!=type) goto finish;
                    unsigned int *source=reinterpret_cast<unsigned int *>(parameter),*destination=reinterpret_cast<unsigned int *>(&record->value08);
                    for (unsigned int i=0;i<10;++i) destination[i]=source[i];
                    record->value54=0xffff;record->value08.array20=0;record->value08.linked24=0;
                    record->value00=0;record->value52=0xb3;record->value58=0x0e000000;record->value56=1;
                    ++group->value34;
                    unsigned int offset=(group->value0C&3)+(group->value0C>>2)*4;
                    group->value0C=offset;record->value48=offset;group->value0C+=4;
                    goto cleanup;
                }
                if (result<0) goto cleanup;
                {
                    unsigned int *source=reinterpret_cast<unsigned int *>(parameter),*destination=reinterpret_cast<unsigned int *>(&record->value08);
                    for (unsigned int i=0;i<10;++i) destination[i]=source[i];
                    record->value08.array20=0;record->value08.linked24=0;
                    unsigned int *cookie=static_cast<unsigned int *>(UG2FxMalloc(0x24));
                    D3DXEffectUseLink *use=0;
                    if (cookie) { *cookie=1;use=reinterpret_cast<D3DXEffectUseLink *>(cookie+1);D3DXEffectConstructArray(use,0x20,1,D3DXConstructUse); }
                    record->array34=reinterpret_cast<D3DXEffectRecord20 *>(use);
                    if (!use) { result=static_cast<int>(0x8007000e);goto cleanup; }
                    record->value50=1;record->value4E=1;
                    result=LinkParameter(groupIndex,recordIndex,0,reinterpret_cast<D3DXEffectParameter *>(parameter),0);
                    goto cleanup;
                }
            }
        }
    finish:
        result=static_cast<int>(0x80004005);
    }
cleanup:
    if (temporary) UG2FxFree(temporary);
    return result;
}

// Original 0x66c645. Copy a resolved symbol into a compiled indirect record,
// then parse its trailing FXLC/CLIT payload twice and bind the CTAB.
int D3DXEffectObject::ReadSymbolRecord(unsigned int groupIndex,unsigned int recordIndex,D3DXEffectBuffer *buffer,unsigned int *position) {
    D3DXEffectGroup64 *group=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+groupIndex;
    D3DXEffectRecord64 *record=group->array04+recordIndex;
    unsigned int total,size;
    FX_SYMBOL_WORD(total);FX_SYMBOL_WORD(size);
    if (!buffer) return static_cast<int>(0x80004005);
    unsigned int limit=UG2FxBufferLengthVirtual(buffer);
    if (limit<*position || size<2) return static_cast<int>(0x80004005);
    unsigned char *temporary=static_cast<unsigned char *>(UG2FxMalloc(size));
    int result=static_cast<int>(0x8007000e);
    if (!temporary) goto cleanup;
    {
        unsigned int words=(size>>2)+((size&3)!=0);
        limit=UG2FxBufferLengthVirtual(buffer);
        if (*position+words<=limit) {
            unsigned char *source=UG2FxBufferPointerVirtual(buffer);source+=*position*4;
            unsigned int i;
            for (i=0;i<(size>>2);++i) reinterpret_cast<unsigned int *>(temporary)[i]=reinterpret_cast<unsigned int *>(source)[i];
            for (i=size&0xfffffffc;i<size;++i) temporary[i]=source[i];
        }
        *position+=words;
        limit=UG2FxBufferLengthVirtual(buffer);
        if (*position<=limit && !temporary[size-1]) {
            D3DXEffectBaseRecord *parameter=ResolveParameter(reinterpret_cast<unsigned int>(temporary),2,0);
            if (parameter) {
                unsigned int *source=reinterpret_cast<unsigned int *>(parameter),*destination=reinterpret_cast<unsigned int *>(&record->value08);
                for (unsigned int i=0;i<10;++i) destination[i]=source[i];
                record->value08.array20=0;record->value08.linked24=0;record->value54=0xffff;
                record->value30&=0xfffffffd;record->value56=1;record->value00=1;
                unsigned int bytes=total-4-size;
                record->value52=0xb3;record->value58=0x0e000000;++group->value34;
                result=D3DXEffectBufferCreateForward(bytes+0x20,reinterpret_cast<D3DXEffectBuffer **>(&record->value38));
                if (result<0) goto cleanup;
                unsigned int *data=reinterpret_cast<unsigned int *>(UG2FxBufferPointerVirtual(reinterpret_cast<D3DXEffectBuffer *>(record->value38)));
                data[2]=0;data[0]=1;data[1]=3;
                unsigned int offset=(group->value0C&7)+(group->value0C>>3)*8;
                group->value0C=offset;data[3]=offset;
                offset=group->value0C+0x20;offset=(offset&3)+(offset>>2)*4;
                group->value0C=offset;record->value48=offset;group->value0C+=4;
                if (!bytes) goto cleanup;
                words=(bytes>>2)+((bytes&3)!=0);
                limit=UG2FxBufferLengthVirtual(buffer);
                if (*position+words<=limit) {
                    unsigned char *input=UG2FxBufferPointerVirtual(buffer);input+=*position*4;
                    unsigned int i;
                    for (i=0;i<(bytes>>2);++i) data[8+i]=reinterpret_cast<unsigned int *>(input)[i];
                    for (i=bytes&0xfffffffc;i<bytes;++i) reinterpret_cast<unsigned char *>(data+8)[i]=input[i];
                }
                *position+=words;
                result=D3DXEffectReadCompiledSections(data+8,reinterpret_cast<unsigned int **>(&record->value40),reinterpret_cast<unsigned int **>(&record->value44));
                if (result<0) goto cleanup;
                limit=UG2FxBufferLengthVirtual(buffer);
                if (*position<=limit) {
                    result=D3DXEffectReadCompiledSections(data+8,reinterpret_cast<unsigned int **>(&record->value40),reinterpret_cast<unsigned int **>(&record->value44));
                    if (result>=0) result=BindCompiledRecord(groupIndex,recordIndex,data);
                    goto cleanup;
                }
            }
        }
    }
    result=static_cast<int>(0x80004005);
cleanup:
    if (temporary) UG2FxFree(temporary);
    return result;
}
#undef FX_SYMBOL_WORD
