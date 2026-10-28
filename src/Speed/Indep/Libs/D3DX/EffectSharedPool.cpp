#include "EffectSharedPool.h"
#include "EffectHandles.h"

// Original 0x767730. Equal raw bytes bypass folding; mismatches fold ASCII
// uppercase bytes. Return is exactly -1/0/1, including bytes above 0x7f.
extern "C" int __cdecl D3DXEffectAsciiCompare(const char *left,const char *right) {
    unsigned int previous=0xff;
    for (;;) {
        if (!previous) return 0;
        unsigned char b=static_cast<unsigned char>(*right++),a=static_cast<unsigned char>(*left++);
        previous=b;
        if (a==b) continue;
        unsigned char offset=static_cast<unsigned char>(b-0x41);
        b=static_cast<unsigned char>(offset+(offset<0x1a ? 0x20 : 0)+0x41);
        offset=static_cast<unsigned char>(a-0x41);
        a=static_cast<unsigned char>(offset+(offset<0x1a ? 0x20 : 0)+0x41);
        previous=a;
        if (a!=b) return a<b ? -1 : 1;
    }
}
// Original 0x75dd6f. The non-C locale lowercase service is a separate boundary.
extern "C" int __cdecl D3DXEffectCaseCompare(const char *left,const char *right) {
    const unsigned char *thread=static_cast<const unsigned char *>(UG2FxCrtGetThreadData());
    const unsigned char *locale=*reinterpret_cast<const unsigned char *const *>(thread+0x64);
    if (reinterpret_cast<unsigned int>(locale)!=ug2FxCrtLocale819D24) locale=static_cast<const unsigned char *>(UG2FxCrtUpdateLocale());
    if (!*reinterpret_cast<const unsigned int *>(locale+0x14)) return D3DXEffectAsciiCompare(left,right);
    unsigned int a,b;
    do {
        a=UG2FxCrtLowerLocale(locale,static_cast<unsigned char>(*left++));
        b=UG2FxCrtLowerLocale(locale,static_cast<unsigned char>(*right++));
    } while (a && a==b);
    return static_cast<int>(a-b);
}
// Original 0x6418a2. Names compare bytewise; semantics use the CRT comparator.
extern "C" int __stdcall D3DXEffectCompareSharedType(unsigned int base,unsigned int **type,unsigned int otherBase,unsigned int **otherType,unsigned int *resources) {
    unsigned int *description=*type,*other=*otherType;
    if (description[1]!=other[1] || description[0]!=other[0] || description[4]!=other[4]) return static_cast<int>(0x80004005);
    const unsigned int *name=reinterpret_cast<const unsigned int *>(base+description[2]),*otherName=reinterpret_cast<const unsigned int *>(otherBase+other[2]);
    const unsigned char *a=*name ? reinterpret_cast<const unsigned char *>(name+1) : 0,*b=*otherName ? reinterpret_cast<const unsigned char *>(otherName+1) : 0;
    const unsigned int *semantic=reinterpret_cast<const unsigned int *>(base+description[3]),*otherSemantic=reinterpret_cast<const unsigned int *>(otherBase+other[3]);
    const char *sa=*semantic ? reinterpret_cast<const char *>(semantic+1) : 0,*sb=*otherSemantic ? reinterpret_cast<const char *>(otherSemantic+1) : 0;
    if (a) {
        if (!b) return static_cast<int>(0x80004005);
        while (*a==*b && *a) { ++a;++b; }
        if (*a!=*b) return static_cast<int>(0x80004005);
    } else if (b) return static_cast<int>(0x80004005);
    if (sa) { if (!sb || D3DXEffectCaseCompare(sa,sb)) return static_cast<int>(0x80004005); }
    else if (sb) return static_cast<int>(0x80004005);
    int shape=static_cast<int>(description[1]);
    if (shape>=0 && shape<4) {
        if (description[5]!=(*otherType)[5] || description[6]!=(*otherType)[6]) return static_cast<int>(0x80004005);
        *type=description+7;*otherType+=7;return 0;
    }
    if (shape==4) {
        int kind=static_cast<int>(description[0]);
        if ((kind>4 && kind<10) || (kind>14 && kind<17)) *resources=1;
        *type+=5;*otherType+=5;return 0;
    }
    if (shape==5 && description[5]==(*otherType)[5]) {
        unsigned int count=description[5];*type=description+6;*otherType+=6;
        for (unsigned int i=0;i<count;++i) { int result=D3DXEffectCompareSharedType(base,type,otherBase,otherType,resources);if (result<0) return result; }
        return 0;
    }
    return static_cast<int>(0x80004005);
}
// Original 0x641a3d, RET 20; HRESULT is returned from the recursive body.
extern "C" int __stdcall D3DXEffectMatchSharedType(unsigned int base,unsigned int *type,unsigned int otherBase,unsigned int *otherType,unsigned int *resources) {
    *resources=0;return D3DXEffectCompareSharedType(base,&type,otherBase,&otherType,resources);
}
// Original 0x641a60. Name is required here. Both observed strlen passes are
// retained; missing semantics still reserve a four-byte zero prefix.
extern "C" int __stdcall D3DXEffectSharedSize(unsigned int base,unsigned int **type,unsigned int *types,unsigned int *data,unsigned int *names) {
    unsigned int *description=*type,count=description[4];if (!count) count=1;
    int kind=static_cast<int>(description[0]),shape=static_cast<int>(description[1]);
    unsigned int *prefix=reinterpret_cast<unsigned int *>(base+description[2]);const char *name=*prefix ? reinterpret_cast<const char *>(prefix+1) : 0;
    prefix=reinterpret_cast<unsigned int *>(base+description[3]);const char *semantic=*prefix ? reinterpret_cast<const char *>(prefix+1) : 0;
    const char *end=name;do { ++end; } while (end[-1]);unsigned int first=static_cast<unsigned int>(end-name);
    end=name;do { ++end; } while (end[-1]);unsigned int second=static_cast<unsigned int>(end-name);
    unsigned int extra=0;
    if (semantic) {
        end=semantic;do { ++end; } while (end[-1]);unsigned int a=static_cast<unsigned int>(end-semantic);
        end=semantic;do { ++end; } while (end[-1]);unsigned int b=static_cast<unsigned int>(end-semantic);
        extra=(a>>2)+((b&3)!=0);
    }
    *names=((first>>2)+((second&3)!=0)+extra)*4+8;*data=0;*types=0;
    if (shape<0) return static_cast<int>(0x80004005);
    if (shape<4) {
        *data=shape<2 ? count*4 : count*16;*types=0x1c;
        if (kind!=1 && kind!=2 && kind!=3) return static_cast<int>(0x80004005);
        *data<<=2;*type+=7;return 0;
    }
    if (shape==4) {
        *types=0x14;
        if (kind<4 || (kind>9 && (kind<15 || kind>16))) return static_cast<int>(0x80004005);
        *data=count*4;*type+=5;return 0;
    }
    if (shape==5) {
        unsigned int members=description[5];*type=description+6;*types=0x18;
        for (unsigned int i=0;i<members;++i) {
            unsigned int childType,childData,childNames;
            int result=D3DXEffectSharedSize(base,type,&childType,&childData,&childNames);if (result<0) return result;
            *data+=childData;*types+=childType;*names+=childNames;
        }
        *data*=count;return 0;
    }
    return static_cast<int>(0x80004005);
}
// Original 0x641c16. Pooled descriptors and names are emitted once for struct
// arrays; numeric/resource data is emitted for each element in original order.
extern "C" int __stdcall D3DXEffectCopySharedType(unsigned int base,unsigned int **type,unsigned int **input,unsigned int outputBase,unsigned int *names,unsigned int **outputType,unsigned int **outputData,unsigned int *resources) {
    (*outputType)[1]=(*type)[1];(*outputType)[0]=(*type)[0];(*outputType)[4]=(*type)[4];(*outputType)[2]=*names;
    unsigned int *description=*type,count=description[4];if (!count) count=1;
    int shape=static_cast<int>(description[1]),kind=static_cast<int>(description[0]);
    unsigned int *prefix=reinterpret_cast<unsigned int *>(base+description[2]);const char *name=*prefix ? reinterpret_cast<const char *>(prefix+1) : 0;
    prefix=reinterpret_cast<unsigned int *>(base+description[3]);const char *semantic=*prefix ? reinterpret_cast<const char *>(prefix+1) : 0;
    const char *end=name;do { ++end; } while (end[-1]);unsigned int size=static_cast<unsigned int>(end-name);
    *reinterpret_cast<unsigned int *>(outputBase+*names)=size;*names+=4;
    unsigned char *destination=reinterpret_cast<unsigned char *>(outputBase+*names);
    unsigned int i;for (i=0;i<(size>>2);++i) reinterpret_cast<unsigned int *>(destination)[i]=reinterpret_cast<const unsigned int *>(name)[i];
    for (i=size&0xfffffffc;i<size;++i) destination[i]=static_cast<unsigned char>(name[i]);
    *names+=((size>>2)+((size&3)!=0))*4;
    (*outputType)[3]=*names;
    if (!semantic) { *reinterpret_cast<unsigned int *>(outputBase+*names)=0;*names+=4; }
    else {
        end=semantic;do { ++end; } while (end[-1]);size=static_cast<unsigned int>(end-semantic);
        *reinterpret_cast<unsigned int *>(outputBase+*names)=size;*names+=4;destination=reinterpret_cast<unsigned char *>(outputBase+*names);
        for (i=0;i<(size>>2);++i) reinterpret_cast<unsigned int *>(destination)[i]=reinterpret_cast<const unsigned int *>(semantic)[i];
        for (i=size&0xfffffffc;i<size;++i) destination[i]=static_cast<unsigned char>(semantic[i]);
        *names+=((size>>2)+((size&3)!=0))*4;
    }
    if (shape>=0 && shape<4) {
        (*outputType)[5]=(*type)[5];(*outputType)[6]=(*type)[6];
        if (kind!=1 && kind!=2 && kind!=3) return static_cast<int>(0x80004005);
        unsigned int bytes=shape<2 ? count*16 : count*64;
        unsigned int *from=*input,*to=*outputData;
        for (i=0;i<(bytes>>2);++i) to[i]=from[i];
        *input+=bytes/4;*outputData+=bytes/4;*type+=7;*outputType+=7;return 0;
    }
    if (shape==4) {
        if (kind<4 || (kind>9 && (kind<15 || kind>16))) return static_cast<int>(0x80004005);
        for (i=count;i;--i) { **outputData=*resources;++*resources;++*input;++*outputData; }
        *type+=5;*outputType+=5;return 0;
    }
    if (shape==5) {
        (*outputType)[5]=(*type)[5];unsigned int members=(*type)[5];*type+=6;*outputType+=6;
        unsigned int firstNames=*names,*firstType=*type,*firstOutput=*outputType;
        for (unsigned int element=0;element<count;++element) {
            *type=firstType;*outputType=firstOutput;*names=firstNames;
            for (i=0;i<members;++i) { int result=D3DXEffectCopySharedType(base,type,input,outputBase,names,outputType,outputData,resources);if (result<0) return result; }
        }
        return 0;
    }
    return static_cast<int>(0x80004005);
}
// Original 0x641ec8; names precede descriptor bytes, then padded data.
extern "C" int __stdcall D3DXEffectCreateSharedType(unsigned int base,unsigned int *type,unsigned int *input,unsigned int outputBase,unsigned int names,unsigned int namesSize,unsigned int typeSize,unsigned int *resources) {
    unsigned int *outputType=reinterpret_cast<unsigned int *>(outputBase+names+namesSize),*outputData=reinterpret_cast<unsigned int *>(outputBase+names+namesSize+typeSize);
    *resources=0;return D3DXEffectCopySharedType(base,&type,&input,outputBase,&names,&outputType,&outputData,resources);
}
// Original 0x641f0d. Numeric skipping uses compact rows*columns, even though
// the pooled data copy above uses sixteen-/sixty-four-byte elements.
extern "C" int __stdcall D3DXEffectWalkSharedIndices(unsigned int **type,unsigned int **input,unsigned int **output) {
    unsigned int *description=*type,count=description[4];if (!count) count=1;
    int shape=static_cast<int>(description[1]),kind=static_cast<int>(description[0]);
    if (shape>=0 && shape<4) {
        if (kind!=1 && kind!=2 && kind!=3) return static_cast<int>(0x80004005);
        *input+=description[6]*description[5]*count;*type+=7;return 0;
    }
    if (shape==4) {
        if (kind<4 || (kind>9 && (kind<15 || kind>16))) return static_cast<int>(0x80004005);
        for (unsigned int i=count;i;--i) { **output=**input;++*output;++*input; }
        *type+=5;return 0;
    }
    if (shape==5) {
        unsigned int members=description[5];*type=description+6;
        for (unsigned int e=0;e<count;++e) { *type=description+6;for (unsigned int i=0;i<members;++i) { int result=D3DXEffectWalkSharedIndices(type,input,output);if (result<0) return result; } }
        return 0;
    }
    return static_cast<int>(0x80004005);
}
extern "C" int __stdcall D3DXEffectSharedIndices(unsigned int *type,unsigned int *input,unsigned int *output) { return D3DXEffectWalkSharedIndices(&type,&input,&output); } // 0x641ffd
// Original 0x642017. Refresh compact numeric data but skip resources.
extern "C" int __stdcall D3DXEffectCopySharedNumeric(unsigned int **type,unsigned int **output,unsigned int **input) {
    unsigned int *description=*type,count=description[4];if (!count) count=1;
    int shape=static_cast<int>(description[1]),kind=static_cast<int>(description[0]);
    if (shape>=0 && shape<4) {
        if (kind!=1 && kind!=2 && kind!=3) return static_cast<int>(0x80004005);
        unsigned int bytes=description[6]*description[5]*count*4,*to=*output,*from=*input;
        for (unsigned int i=0;i<(bytes>>2);++i) to[i]=from[i];
        *output+=bytes/4;*input+=bytes/4;*type+=7;return 0;
    }
    if (shape==4) {
        if ((kind>=4 && kind<=9) || kind==15 || kind==16) { *output+=count;*input+=count; }
        else if (!((kind>=10 && kind<=14) || kind==17 || kind==18)) return static_cast<int>(0x80004005);
        *type+=5;return 0;
    }
    if (shape==5) {
        unsigned int members=description[5];*type=description+6;
        for (unsigned int e=0;e<count;++e) { *type=description+6;for (unsigned int i=0;i<members;++i) { int result=D3DXEffectCopySharedNumeric(type,output,input);if (result<0) return result; } }
        return 0;
    }
    return static_cast<int>(0x80004005);
}
extern "C" int __stdcall D3DXEffectRefreshSharedNumeric(unsigned int *type,unsigned int *output,unsigned int *input) { return D3DXEffectCopySharedNumeric(&type,&output,&input); } // 0x642127
// Original 0x642141. Retain source slots before releasing destination slots;
// shaders copy both owned slots, bit 1 and value0C in the observed order.
extern "C" int __stdcall D3DXEffectCopySharedResources(unsigned int **type,unsigned int **data,D3DXEffectPoolLink *source,D3DXEffectPoolLink *destination) {
    unsigned int *description=*type,count=description[4];if (!count) count=1;
    int shape=static_cast<int>(description[1]),kind=static_cast<int>(description[0]);
    if (shape>=0 && shape<4) {
        if (kind!=1 && kind!=2 && kind!=3) return static_cast<int>(0x80004005);
        *data+=description[6]*description[5]*count;*type+=7;return 0;
    }
    if (shape==4) {
        if (kind==4 || (kind>=5 && kind<=9) || kind==15 || kind==16) {
            for (unsigned int i=count;i;--i) {
                D3DXEffectObject *toEffect=reinterpret_cast<D3DXEffectObject *>(destination->value00),*fromEffect=reinterpret_cast<D3DXEffectObject *>(source->value00);
                D3DXEffectReference18 *to=reinterpret_cast<D3DXEffectReference18 *>(toEffect->value18)+static_cast<unsigned int *>(destination->data08)[**data];
                D3DXEffectReference18 *from=reinterpret_cast<D3DXEffectReference18 *>(fromEffect->value18)+static_cast<unsigned int *>(source->data08)[**data];
                if (from->value04) UG2FxComAddRef(from->value04);
                if ((kind==15 || kind==16) && from->value08) UG2FxComAddRef(from->value08);
                if (to->value04) { UG2FxComRelease(to->value04);to->value04=0; }
                if ((kind==15 || kind==16) && to->value08) { UG2FxComRelease(to->value08);to->value08=0; }
                to->value04=from->value04;
                if (kind==15 || kind==16) {
                    to->value08=from->value08;to->value14=(to->value14&0xfffffffd)|(from->value14&2);to->value0C=from->value0C;
                }
                ++*data;
            }
        } else if (!((kind>=10 && kind<=14) || kind==17 || kind==18)) return static_cast<int>(0x80004005);
        *type+=5;return 0;
    }
    if (shape==5) {
        unsigned int members=description[5];*type=description+6;
        for (unsigned int e=0;e<count;++e) { *type=description+6;for (unsigned int i=0;i<members;++i) { int result=D3DXEffectCopySharedResources(type,data,source,destination);if (result<0) return result; } }
        return 0;
    }
    return static_cast<int>(0x80004005);
}
extern "C" int __stdcall D3DXEffectRefreshSharedResources(unsigned int *type,unsigned int *data,D3DXEffectPoolLink *destination,D3DXEffectPoolLink *source) {
    if (destination==source || !destination->value00) return 0;
    return D3DXEffectCopySharedResources(&type,&data,source,destination);
} // 0x642487
// Original 0x641862. Unspecified node prefix/link fields remain untouched.
D3DXEffectPoolNode *D3DXEffectPoolNode::Construct() {
    link0C.data08=0;link0C.value00=0;link0C.next10=&link0C;link0C.previous0C=&link0C;linked20=0;return this;
}
// Original 0x6424b2. Only parameter+0 is rebound here, not its type pointer.
int D3DXEffectPool::RefreshPointers() {
    for (D3DXEffectPoolNode *node=node1C;node;node=node->linked20) {
        for (D3DXEffectPoolLink *link=node->link0C.next10;link!=&node->link0C;link=link->next10) {
            D3DXEffectParameter *parameter=reinterpret_cast<D3DXEffectParameter *>(link->value04);
            parameter->value00=reinterpret_cast<unsigned int>(data10)+parameter->value30;
        }
    }
    return 0;
}
// Original 0x64259c. Preserve the original capacity publication, insertion and
// partial state on failures. Growth chooses max(64KiB, capacity*2), not required.
int D3DXEffectPool::Bind(D3DXEffectObject *effect,D3DXEffectParameter *parameter) {
    unsigned int *type=reinterpret_cast<unsigned int *>(effect->value20+parameter->value2C),*input=reinterpret_cast<unsigned int *>(effect->value20+parameter->value30);
    D3DXEffectPoolNode *node=node1C;
    for (;node;node=node->linked20) {
        unsigned int resources;
        int result=D3DXEffectMatchSharedType(effect->value20,type,reinterpret_cast<unsigned int>(data10),reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned int>(data10)+node->value00),&resources);
        if (result>=0) {
            if (resources && node->link0C.next10->value00 && effect->value0C!=reinterpret_cast<D3DXEffectObject *>(node->link0C.next10->value00)->value0C) return static_cast<int>(0x80004005);
            if (node->link0C.next10==&node->link0C) {
                result=D3DXEffectRefreshSharedNumeric(type,reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned int>(data10)+node->value04),input);
                if (result<0) return result;
            }
            break;
        }
    }
    if (!node) {
        unsigned int *cursor=type,types,values,names;
        int result=D3DXEffectSharedSize(effect->value20,&cursor,&types,&values,&names);if (result<0) return result;
        unsigned int required=value14+types+values+names;
        if (value18<required) {
            unsigned int doubled=value18*2;value18=doubled<0x10000 ? 0x10000 : doubled;
            unsigned char *grown=static_cast<unsigned char *>(UG2FxMalloc(value18));if (!grown) return static_cast<int>(0x8007000e);
            unsigned int i;for (i=0;i<(value14>>2);++i) reinterpret_cast<unsigned int *>(grown)[i]=static_cast<unsigned int *>(data10)[i];
            for (i=value14&0xfffffffc;i<value14;++i) grown[i]=static_cast<unsigned char *>(data10)[i];
            unsigned int bytes=value18-value14;unsigned char *zero=grown+value14;
            for (i=0;i<(bytes>>2);++i) reinterpret_cast<unsigned int *>(zero)[i]=0;
            for (i=bytes&0xfffffffc;i<bytes;++i) zero[i]=0;
            if (data10) { UG2FxFree(data10);data10=0; }
            data10=grown;RefreshPointers();
        }
        unsigned int resources;
        result=D3DXEffectCreateSharedType(effect->value20,type,input,reinterpret_cast<unsigned int>(data10),value14,names,types,&resources);if (result<0) return result;
        node=static_cast<D3DXEffectPoolNode *>(UG2FxMalloc(0x24));if (node) node=node->Construct();
        if (!node) return static_cast<int>(0x8007000e);
        node->value00=value14+names;node->value04=value14+types+names;node->value08=resources;node->linked20=node1C;node1C=node;value14=required;
    }
    D3DXEffectPoolLink *link=static_cast<D3DXEffectPoolLink *>(UG2FxMalloc(0x14));
    if (!link) return static_cast<int>(0x8007000e);
    link->data08=0;link->value00=0;link->next10=link;link->previous0C=link;
    int result=0;
    if (node->value08) {
        link->data08=UG2FxMalloc(node->value08*4);
        if (!link->data08) { result=static_cast<int>(0x8007000e);goto finish; }
        result=D3DXEffectSharedIndices(type,input,static_cast<unsigned int *>(link->data08));if (result<0) goto finish;
    }
    link->value00=reinterpret_cast<unsigned int>(effect);link->value04=reinterpret_cast<unsigned int>(parameter);
    node->link0C.next10->previous0C=link;link->previous0C=&node->link0C;link->next10=node->link0C.next10;node->link0C.next10=link;
    parameter->value40=reinterpret_cast<unsigned int>(link);parameter->value2C=node->value00;parameter->value30=node->value04;
    parameter->value04=reinterpret_cast<unsigned int>(data10)+node->value00;parameter->value00=reinterpret_cast<unsigned int>(data10)+node->value04;
    link=0;
    result=D3DXEffectRefreshSharedResources(reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned int>(data10)+parameter->value2C),reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned int>(data10)+parameter->value30),reinterpret_cast<D3DXEffectPoolLink *>(parameter->value40),node->link0C.previous0C);
finish:
    if (link) link->Delete(1);
    return result;
}
