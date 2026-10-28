#include "EffectSetup.h"
#ifdef _MSC_VER
 typedef __int64 EffectInteger;
#else
 typedef long long EffectInteger;
#endif

// Original 0x65ae25. Deltas use only their signed low words. Nonnegative sums
// are stored as words, including positive overflow wrapping back to zero.
int D3DXEffectObject::AdjustReferences(unsigned int group,int directDelta,int allDelta) {
    D3DXEffectGroup64 *p=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+group;
    short direct=static_cast<short>(directDelta),all=static_cast<short>(allDelta);
    if (static_cast<int>(p->value3C)+direct>=0) p->value3C=static_cast<unsigned short>(p->value3C+directDelta);
    if (static_cast<int>(p->value3A)+all>=0) p->value3A=static_cast<unsigned short>(p->value3A+allDelta);
    unsigned short *indices=reinterpret_cast<unsigned short *>(reinterpret_cast<unsigned int>(p->data00)+p->value14);
    for (unsigned int i=0;i<p->value34;++i) {
        D3DXEffectRecord64 *record=p->array04+indices[i];
        unsigned short *reference=reinterpret_cast<unsigned short *>(record->value48+reinterpret_cast<unsigned int>(p->data00));
        if (*reference) {
            unsigned int count=reinterpret_cast<unsigned int *>(record->value08.value04)[4];
            unsigned int j=0;
            while ((j&0xffff)<(count ? count : 1)) {
                if (all) AdjustReferences((*reference-1)+(j&0xffff),0,allDelta);
                ++j;
            }
            if (direct) AdjustReferences((reference[1]-1)+static_cast<unsigned int>(*reference),directDelta,0);
        }
    }
    return 0;
}
// Original 0x66aa2f. Preparation/evaluation calls retain their ABI bridges;
// all selection, count adjustment and recursive traversal is here.
int D3DXEffectObject::ResolveReferences(unsigned int group,int direct) {
    int result=0;
    D3DXEffectGroup64 *p=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+group;
    D3DXEffectRecord64 *record;
    if (p->value18!=0xffff && (record=p->array04+p->value18)->value00 && record->value58==0x0a000000) {
        result=AdjustReferences(group,direct ? -1 : 0,-1); if (result<0) return result;
        result=UG2FxPrepareGroupA(this,group); if (result<0) return result;
        result=AdjustReferences(group,direct!=0,1); if (result<0) return result;
    }
    if (p->value1A!=0xffff && (record=p->array04+p->value1A)->value00 && record->value58==0x0b000000) {
        result=AdjustReferences(group,direct ? -1 : 0,-1); if (result<0) return result;
        result=UG2FxPrepareGroupB(this,group); if (result<0) return result;
        result=AdjustReferences(group,direct!=0,1); if (result<0) return result;
    }
    unsigned short *indices=reinterpret_cast<unsigned short *>(p->value14+reinterpret_cast<unsigned int>(p->data00));
    for (unsigned short i=0;i<p->value34;++i) {
        D3DXEffectRecord64 *current=p->array04+indices[i];
        unsigned short *reference=reinterpret_cast<unsigned short *>(current->value48+reinterpret_cast<unsigned int>(p->data00));
        if (*reference) {
            unsigned int count=reinterpret_cast<unsigned int *>(current->value08.value04)[4];
            if (!count) count=1;
            if (current->value00) {
                current->value00=current->value04;
                result=UG2FxEvaluateRecord(this,group,indices[i]); if (result<0) return result;
                unsigned char *description=UG2FxBufferPointerVirtual(reinterpret_cast<D3DXEffectBuffer *>(current->value38));
                unsigned int offset=*reinterpret_cast<unsigned int *>(description+0x0c);
                unsigned short index=static_cast<unsigned short>(static_cast<EffectInteger>(
                    *reinterpret_cast<double *>(offset+reinterpret_cast<unsigned int>(p->data00))+ug2FxIndexBias7A1E58));
                if (reference[1]!=index) {
                    if (count<=index) return static_cast<int>(0x80004005);
                    reference[1]=index;
                }
            }
            unsigned int j=0;
            unsigned short narrowed=0;
            while (narrowed<count) {
                result=ResolveReferences((*reference-1)+static_cast<unsigned int>(narrowed),direct);
                if (result<0) return result;
                narrowed=static_cast<unsigned short>(++j);
            }
        }
    }
    return result;
}
// Original 0x66afe6. Two growable temporary lists are shared across rows, but
// reset their lengths per parameter. Persist partially populated grids on OOM.
int D3DXEffectObject::Setup() {
    reinterpret_cast<unsigned char *>(&value118)[1]|=8;
    int result=0;
    D3DXEffectUseLink **first=0,**second=0;
    unsigned int firstCapacity=0,secondCapacity=0;
    valueB4=reinterpret_cast<unsigned int>(UG2FxMalloc(valueA0*valueAC*4));
    valueC0=reinterpret_cast<unsigned int>(UG2FxMalloc(valueA0*valueAC*4));
    valueBC=reinterpret_cast<unsigned int>(UG2FxMalloc(valueAC));
    if (!valueB4 || !valueC0 || !valueBC) { result=static_cast<int>(0x8007000e); goto finish; }
    {
        unsigned char *bytes=reinterpret_cast<unsigned char *>(valueBC);
        unsigned int size=valueAC;
        unsigned int *words=reinterpret_cast<unsigned int *>(bytes);
        unsigned int i;
        for (i=0;i<(size>>2);++i) words[i]=0;
        bytes+=size&0xfffffffc;
        for (i=0;i<(size&3);++i) bytes[i]=0;
    }
    *reinterpret_cast<unsigned short *>(&value40)=0;
    while (*reinterpret_cast<unsigned short *>(&value40)<valueA0) {
        unsigned int row=*reinterpret_cast<unsigned short *>(&value40);
        valueB8=valueB4+valueAC*row*4;
        unsigned int secondRow=valueC0+valueAC*row*4;
        unsigned int pass=0,passOffset=0;
        while (pass<reinterpret_cast<D3DXEffectRecordB4 *>(valueA4)[*reinterpret_cast<unsigned short *>(&value40)].value34) {
            D3DXEffectRecord38 *record=reinterpret_cast<D3DXEffectRecord38 *>(
                reinterpret_cast<unsigned int>(reinterpret_cast<D3DXEffectRecordB4 *>(valueA4)[*reinterpret_cast<unsigned short *>(&value40)].array38)+passOffset);
            AdjustReferences(static_cast<unsigned short>(record->value34),0,1);
            ResolveReferences(static_cast<unsigned short>(record->value34),0);
            ++pass;passOffset+=0x38;
        }
        for (unsigned int parameter=0;parameter<valueAC;++parameter) {
            D3DXEffectUseLink *head=reinterpret_cast<D3DXEffectUseLink *>(
                reinterpret_cast<D3DXEffectParameter **>(valueB0)[parameter]->value34);
            unsigned int firstCount=0,secondCount=0;
            if (head) {
                D3DXEffectUseLink *link=head->previous18;
                do {
                    link=link->next1C;
                    D3DXEffectGroup64 *group=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+link->value14;
                    if (group->value3A) {
                        if (!(group->value08&8)) {
                            if (firstCount>=firstCapacity) {
                                firstCapacity=firstCapacity*2+1;
                                D3DXEffectUseLink **grown=static_cast<D3DXEffectUseLink **>(UG2FxMalloc(firstCapacity*4));
                                if (!grown) goto failed_row;
                                for (unsigned int i=0;i<firstCount;++i) grown[i]=first[i];
                                UG2FxFree(first);first=grown;
                            }
                            first[firstCount++]=link;
                        } else {
                            if (secondCount>=secondCapacity) {
                                secondCapacity=secondCapacity*2+1;
                                D3DXEffectUseLink **grown=static_cast<D3DXEffectUseLink **>(UG2FxMalloc(secondCapacity*4));
                                if (!grown) goto failed_row;
                                for (unsigned int i=0;i<secondCount;++i) grown[i]=second[i];
                                UG2FxFree(second);second=grown;
                            }
                            second[secondCount++]=link;
                        }
                    }
                } while (link->next1C!=reinterpret_cast<D3DXEffectUseLink *>(
                    reinterpret_cast<D3DXEffectParameter **>(valueB0)[parameter]->value34));
            }
            if (firstCount+secondCount) {
                reinterpret_cast<unsigned int *>(valueB8)[parameter]=reinterpret_cast<unsigned int>(UG2FxMalloc((firstCount+secondCount)*4+4));
                unsigned int *output=reinterpret_cast<unsigned int **>(valueB8)[parameter];
                if (!output) goto failed_row;
                for (unsigned int i=0;i<firstCount;++i) output[i]=reinterpret_cast<unsigned int>(first[i]);
                output=reinterpret_cast<unsigned int **>(valueB8)[parameter]+firstCount;
                for (unsigned int i=0;i<(secondCount*4+4)/4;++i) output[i]=0;
                if (secondCount) {
                    unsigned int bytes=secondCount*4;
                    unsigned int **slot=reinterpret_cast<unsigned int **>(secondRow)+parameter;
                    *slot=static_cast<unsigned int *>(UG2FxMalloc(bytes+4));
                    if (!*slot) goto failed_row;
                    for (unsigned int i=0;i<(bytes>>2);++i) (*slot)[i]=reinterpret_cast<unsigned int>(second[i]);
                    *reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned int>(*slot)+bytes)=0;
                } else reinterpret_cast<unsigned int *>(secondRow)[parameter]=0;
            } else {
                reinterpret_cast<unsigned int *>(valueB8)[parameter]=0;
                reinterpret_cast<unsigned int *>(secondRow)[parameter]=0;
            }
        }
        pass=0;passOffset=0;
        while (pass<reinterpret_cast<D3DXEffectRecordB4 *>(valueA4)[*reinterpret_cast<unsigned short *>(&value40)].value34) {
            D3DXEffectRecord38 *record=reinterpret_cast<D3DXEffectRecord38 *>(
                reinterpret_cast<unsigned int>(reinterpret_cast<D3DXEffectRecordB4 *>(valueA4)[*reinterpret_cast<unsigned short *>(&value40)].array38)+passOffset);
            AdjustReferences(static_cast<unsigned short>(record->value34),0,-1);
            ++pass;passOffset+=0x38;
        }
        ++*reinterpret_cast<unsigned short *>(&value40);
        continue;
failed_row:
        result=static_cast<int>(0x8007000e);
        pass=0;passOffset=0;
        while (pass<reinterpret_cast<D3DXEffectRecordB4 *>(valueA4)[*reinterpret_cast<unsigned short *>(&value40)].value34) {
            D3DXEffectRecord38 *record=reinterpret_cast<D3DXEffectRecord38 *>(
                reinterpret_cast<unsigned int>(reinterpret_cast<D3DXEffectRecordB4 *>(valueA4)[*reinterpret_cast<unsigned short *>(&value40)].array38)+passOffset);
            AdjustReferences(static_cast<unsigned short>(record->value34),0,-1);
            ++pass;passOffset+=0x38;
        }
        break;
    }
    *reinterpret_cast<unsigned short *>(&value40)=0xffff;
finish:
    UG2FxFree(first);UG2FxFree(second);
    reinterpret_cast<unsigned char *>(&value118)[1]&=0xf7;
    return result;
}
