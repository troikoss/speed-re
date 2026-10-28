#include "EffectValues.h"
#include "../../Src/Physics/D3DXMatrixScalar.h"

// Original 0x65cc10. Shared maps contain reference indices, not object pointers.
int D3DXEffectObject::WriteShaderResource(unsigned int object,unsigned int index,D3DXEffectPoolLink *shared) {
    unsigned int mapped=shared ? static_cast<unsigned int *>(shared->data08)[index] : index;
    D3DXEffectReference18 *reference=reinterpret_cast<D3DXEffectReference18 *>(value18)+mapped;
    if (object!=reference->value04) {
        if (object) UG2FxComAddRef(object);
        if (reference->value04) { UG2FxComRelease(reference->value04); reference->value04=0; }
        reference->value14|=1;
        reference->value04=object;
        value118|=0x40;
        if (shared) for (D3DXEffectPoolLink *node=shared->next10;node!=shared;node=node->next10) {
            if (node->value00) {
                D3DXEffectObject *effect=reinterpret_cast<D3DXEffectObject *>(node->value00);
                D3DXEffectReference18 *other=reinterpret_cast<D3DXEffectReference18 *>(effect->value18)+static_cast<unsigned int *>(node->data08)[index];
                if (reference->value04) UG2FxComAddRef(reference->value04);
                if (other->value04) { UG2FxComRelease(other->value04); other->value04=0; }
                other->value14|=1;
                other->value04=reference->value04;
            }
        }
    }
    return 0;
}
// Original 0x65ccc8. Unlike shader writes, no shader flags are changed.
int D3DXEffectObject::WriteObjectResource(unsigned int object,unsigned int index,D3DXEffectPoolLink *shared) {
    unsigned int mapped=shared ? static_cast<unsigned int *>(shared->data08)[index] : index;
    D3DXEffectReference18 *reference=reinterpret_cast<D3DXEffectReference18 *>(value18)+mapped;
    if (object!=reference->value04) {
        if (object) UG2FxComAddRef(object);
        if (reference->value04) { UG2FxComRelease(reference->value04); reference->value04=0; }
        reference->value04=object;
        if (shared) for (D3DXEffectPoolLink *node=shared->next10;node!=shared;node=node->next10) {
            if (node->value00) {
                D3DXEffectObject *effect=reinterpret_cast<D3DXEffectObject *>(node->value00);
                D3DXEffectReference18 *other=reinterpret_cast<D3DXEffectReference18 *>(effect->value18)+static_cast<unsigned int *>(node->data08)[index];
                if (reference->value04) UG2FxComAddRef(reference->value04);
                if (other->value04) { UG2FxComRelease(other->value04); other->value04=0; }
                other->value04=reference->value04;
            }
        }
    }
    return 0;
}
// Original 0x65cd6c. A sufficiently large buffer is reused, including its tail.
// Retain the local reference until after publication to all shared effects.
int D3DXEffectObject::WriteStringResource(const char *text,unsigned int index,D3DXEffectPoolLink *shared) {
    int result=0;
    D3DXEffectBuffer *buffer=0;
    unsigned int mapped=shared ? static_cast<unsigned int *>(shared->data08)[index] : index;
    D3DXEffectReference18 *reference=reinterpret_cast<D3DXEffectReference18 *>(value18)+mapped;
    if (text) {
        const char *end=text;
        do { ++end; } while (end[-1]);
        unsigned int size=static_cast<unsigned int>(end-text);
        if (!reference->value04 || UG2FxBufferLengthVirtual(reinterpret_cast<D3DXEffectBuffer *>(reference->value04))<size) {
            result=D3DXEffectBufferCreateForward(size,&buffer);
            if (result<0) goto finish;
        } else {
            buffer=reinterpret_cast<D3DXEffectBuffer *>(reference->value04);
            if (buffer) UG2FxComAddRef(reinterpret_cast<unsigned int>(buffer));
        }
        unsigned char *output=UG2FxBufferPointerVirtual(buffer);
        unsigned int i;
        for (i=0;i<(size>>2);++i) reinterpret_cast<unsigned int *>(output)[i]=reinterpret_cast<const unsigned int *>(text)[i];
        for (i=size&0xfffffffc;i<size;++i) output[i]=static_cast<unsigned char>(text[i]);
        if (buffer) UG2FxComAddRef(reinterpret_cast<unsigned int>(buffer));
    }
    if (reference->value04) { UG2FxComRelease(reference->value04); reference->value04=0; }
    reference->value04=reinterpret_cast<unsigned int>(buffer);
    if (shared) for (D3DXEffectPoolLink *node=shared->next10;node!=shared;node=node->next10) {
        if (node->value00) {
            D3DXEffectObject *effect=reinterpret_cast<D3DXEffectObject *>(node->value00);
            D3DXEffectReference18 *other=reinterpret_cast<D3DXEffectReference18 *>(effect->value18)+static_cast<unsigned int *>(node->data08)[index];
            if (reference->value04) UG2FxComAddRef(reference->value04);
            if (other->value04) { UG2FxComRelease(other->value04); other->value04=0; }
            other->value04=reference->value04;
        }
    }
finish:
    if (buffer) UG2FxComRelease(reinterpret_cast<unsigned int>(buffer));
    return result;
}

// Original 0x65ddc8. Cursors are published only at the original branch points.
int D3DXEffectObject::CopyTypedValue(unsigned int **type,unsigned int **output,const unsigned int **input,
    unsigned int count,D3DXEffectBaseRecord *record) {
    unsigned int *description=*type;
    int kind=static_cast<int>(description[0]),shape=static_cast<int>(description[1]);
    if (shape<0) return static_cast<int>(0x80004005);
    if (shape<2) {
        unsigned int *to=*output;
        const unsigned int *from=*input;
        unsigned int rows=description[5];
        unsigned int end=reinterpret_cast<unsigned int>(to)+count*16;
        for (;reinterpret_cast<unsigned int>(to)<end;to+=4-rows) {
            for (unsigned int i=rows;i;--i) *to++=*from++;
        }
        *output=to;*input=from;
    } else if (shape<4) {
        unsigned int rows=description[5],columns=description[6];
        if (kind==1 || kind==2 || kind==3) {
            for (;count;--count) {
                for (unsigned int column=0;column<columns;++column) {
                    for (unsigned int row=0;row<rows;++row) {
                        unsigned int word=(*input)[row*columns+column];
                        (*output)[column*4+row]=kind==1 ? (word!=0) : word;
                    }
                }
                *output+=16;
                *input+=columns*rows;
            }
        }
    } else if (shape==4) {
        if (kind==4 || (kind>4 && kind<10) || kind==15 || kind==16) {
            for (;count;--count) {
                D3DXEffectPoolLink *shared=reinterpret_cast<D3DXEffectPoolLink *>(reinterpret_cast<D3DXEffectParameter **>(valueB0)[record->value14]->value40);
                int result;
                if (kind==4) result=WriteStringResource(reinterpret_cast<const char *>(**input),**output,shared);
                else if (kind<10) result=WriteObjectResource(**input,**output,shared);
                else result=WriteShaderResource(**input,**output,shared);
                if (result<0) return result;
                ++*output;++*input;
            }
        }
        *type+=5;
        return 0;
    } else if (shape==5) {
        unsigned int members=description[5];
        *type=description+6;
        for (;count;--count) {
            *type=description+6;
            for (unsigned int i=0;i<members;++i) {
                unsigned int elements=(*type)[4];
                if (!elements) elements=1;
                int result=CopyTypedValue(type,output,input,elements,record);
                if (result<0) return result;
            }
        }
        return 0;
    } else return static_cast<int>(0x80004005);
    *type+=7;
    return 0;
}

// Original 0x661fc6. Indexed handles write one element. Matrix boolean lanes
// normalize DWORD truth values; scalar/vector lanes retain raw DWORDs.
int D3DXEffectObject::CopyValue(D3DXEffectBaseRecord *record,const unsigned int *input) {
    unsigned int *output=reinterpret_cast<unsigned int *>(record->value00);
    unsigned int *type=reinterpret_cast<unsigned int *>(record->value04);
    unsigned int count=type[4];
    if (record->value08!=0xffffffff || !count) count=1;
    int shape=static_cast<int>(type[1]),kind=static_cast<int>(type[0]);
    if (shape<0) return static_cast<int>(0x80004005);
    if (shape<2) {
        unsigned int rows=type[5];
        unsigned int end=reinterpret_cast<unsigned int>(output)+count*16;
        for (;reinterpret_cast<unsigned int>(output)<end;output+=4-rows) {
            for (unsigned int i=rows;i;--i) *output++=*input++;
        }
    } else if (shape<4) {
        unsigned int rows=type[5],columns=type[6];
        if (kind==1 || kind==2 || kind==3) for (;count;--count) {
            for (unsigned int column=0;column<columns;++column) for (unsigned int row=0;row<rows;++row) {
                unsigned int word=input[row*columns+column];
                output[column*4+row]=kind==1 ? (word!=0) : word;
            }
            output+=16;input+=columns*rows;
        }
    } else if (shape==4) {
        if (kind==4 || (kind>4 && kind<10) || kind==15 || kind==16) for (;count;--count) {
            D3DXEffectPoolLink *shared=reinterpret_cast<D3DXEffectPoolLink *>(reinterpret_cast<D3DXEffectParameter **>(valueB0)[record->value14]->value40);
            int result;
            if (kind==4) result=WriteStringResource(reinterpret_cast<const char *>(*input),*output,shared);
            else if (kind<10) result=WriteObjectResource(*input,*output,shared);
            else result=WriteShaderResource(*input,*output,shared);
            if (result<0) return result;
            ++input;++output;
        }
    } else if (shape==5) {
        unsigned int members=type[5],*first=type+6;
        for (;count;--count) {
            unsigned int *member=first;
            for (unsigned int i=0;i<members;++i) {
                unsigned int elements=member[4];if (!elements) elements=1;
                int result=CopyTypedValue(&member,&output,&input,elements,record);
                if (result<0) return result;
            }
        }
    } else return static_cast<int>(0x80004005);
    return 0;
}

// Original 0x66055c. A selected shared effect receives its own linked record.
int D3DXEffectObject::NotifyValue(D3DXEffectBaseRecord *record) {
    D3DXEffectPoolLink *shared=reinterpret_cast<D3DXEffectPoolLink *>(reinterpret_cast<D3DXEffectParameter **>(valueB0)[record->value14]->value40);
    if (!shared) return MarkParameterDirty(reinterpret_cast<D3DXEffectParameter *>(record));
    D3DXEffectPool *pool=reinterpret_cast<D3DXEffectPool *>(value38);
    if (pool->value08<2) {
        if (!pool->value08) return 0;
        if (pool->value0C==reinterpret_cast<unsigned int>(this)) return MarkParameterDirty(reinterpret_cast<D3DXEffectParameter *>(record));
        if (pool->value0C) for (D3DXEffectPoolLink *node=shared->next10;node!=shared;node=node->next10) {
            if (pool->value0C==node->value00) return reinterpret_cast<D3DXEffectObject *>(node->value00)->MarkParameterDirty(reinterpret_cast<D3DXEffectParameter *>(node->value04));
        }
    }
    for (D3DXEffectPoolLink *node=shared->next10;node!=shared;node=node->next10) {
        if (node->value00 && (reinterpret_cast<D3DXEffectObject *>(node->value00)->value118&0x100)) {
            int result=reinterpret_cast<D3DXEffectObject *>(node->value00)->MarkParameterDirty(reinterpret_cast<D3DXEffectParameter *>(node->value04));
            if (result<0) return result;
        }
    }
    if (value118&0x100) return MarkParameterDirty(reinterpret_cast<D3DXEffectParameter *>(record));
    return 0;
}

// Original 0x65ee33, constructor callback (ECX receiver; return ignored).
extern "C" void __fastcall D3DXConstructReference08(void *object) {
    D3DXEffectReference08 *reference=static_cast<D3DXEffectReference08 *>(object);
    reference->value04=0;reference->value00=4;
}
// Original 0x65eb8f. Count-only and ownership passes share the same descriptor
// walk. Resource pointers in the input copy are never replaced by buffers.
extern "C" int __stdcall D3DXEffectWalkValueReferences(unsigned int **type,unsigned int **data,
    D3DXEffectReference08 **references,unsigned int *count,int indexed) {
    unsigned int *description=*type;
    unsigned int elements=description[4];
    int shape=static_cast<int>(description[1]),kind=static_cast<int>(description[0]);
    if (indexed || !elements) elements=1;
    if (shape>=0 && shape<4) {
        if (kind!=1 && kind!=2 && kind!=3) return static_cast<int>(0x80004005);
        *data+=description[6]*description[5]*elements;
        *type+=7;
        return 0;
    }
    if (shape==4) {
        if (kind!=4 && !(kind>4 && kind<10) && kind!=15 && kind!=16) return static_cast<int>(0x80004005);
        for (;elements;--elements) {
            if (*references) {
                unsigned int object=**data;
                if (kind==4) {
                    D3DXEffectBuffer *buffer=0;
                    if (object) {
                        const char *text=reinterpret_cast<const char *>(object),*end=text;
                        do { ++end; } while (end[-1]);
                        unsigned int size=static_cast<unsigned int>(end-text);
                        int result=D3DXEffectBufferCreateForward(size,&buffer);
                        if (result<0) return result;
                        unsigned char *output=UG2FxBufferPointerVirtual(buffer);
                        unsigned int i;
                        for (i=0;i<(size>>2);++i) reinterpret_cast<unsigned int *>(output)[i]=reinterpret_cast<const unsigned int *>(text)[i];
                        for (i=size&0xfffffffc;i<size;++i) output[i]=static_cast<unsigned char>(text[i]);
                    }
                    (*references)->value00=4;(*references)->value04=reinterpret_cast<unsigned int>(buffer);
                } else {
                    if (object) UG2FxComAddRef(object);
                    (*references)->value00=kind<10 ? 5 : kind;
                    (*references)->value04=object;
                }
                ++*references;
            }
            if (count) ++*count;
            ++*data;
        }
        *type+=5;
        return 0;
    }
    if (shape==5) {
        unsigned int members=description[5];*type=description+6;
        for (;elements;--elements) {
            *type=description+6;
            for (unsigned int i=0;i<members;++i) {
                int result=D3DXEffectWalkValueReferences(type,data,references,count,0);
                if (result<0) return result;
            }
        }
        return 0;
    }
    return static_cast<int>(0x80004005);
}
// Original 0x65ee13. Only the three local pointer cursors are discarded.
extern "C" int __stdcall D3DXEffectValueReferences(unsigned int *type,unsigned int *data,
    D3DXEffectReference08 *references,unsigned int *count,int indexed) {
    return D3DXEffectWalkValueReferences(&type,&data,&references,count,indexed);
}
// Original 0x666c81. Publish to the tail only after both reference walks pass;
// cleanup preserves partially retained references when the second walk fails.
int D3DXEffectObject::RecordValue(unsigned int handle,const void *data,unsigned int bytes) {
    D3DXEffectReference08 *references=0;
    unsigned char *copy=0;
    D3DXEffectValuePrefix *value=static_cast<D3DXEffectValuePrefix *>(UG2FxMalloc(0x1c));
    if (value) value->Construct();
    if (!value) return static_cast<int>(0x8007000e);
    int result;
    if (bytes) {
        copy=static_cast<unsigned char *>(UG2FxMalloc(bytes));
        if (!copy) { result=static_cast<int>(0x8007000e);goto finish; }
        unsigned int i;
        for (i=0;i<(bytes>>2);++i) reinterpret_cast<unsigned int *>(copy)[i]=static_cast<const unsigned int *>(data)[i];
        for (i=bytes&0xfffffffc;i<bytes;++i) copy[i]=static_cast<const unsigned char *>(data)[i];
    }
    {
        D3DXEffectBaseRecord *record=reinterpret_cast<D3DXEffectBaseRecord *>(~handle);
        unsigned int *type=reinterpret_cast<unsigned int *>(record->value04),count=0;
        result=D3DXEffectValueReferences(type,reinterpret_cast<unsigned int *>(copy),0,&count,record->value08!=0xffffffff);
        if (result<0) goto finish;
        unsigned int *cookie=static_cast<unsigned int *>(UG2FxMalloc(count*8+4));
        if (cookie) {
            *cookie=count;references=reinterpret_cast<D3DXEffectReference08 *>(cookie+1);
            D3DXEffectConstructArray(references,8,count,D3DXConstructReference08);
        }
        if (!references) { result=static_cast<int>(0x8007000e);goto finish; }
        result=D3DXEffectValueReferences(type,reinterpret_cast<unsigned int *>(copy),references,0,record->value08!=0xffffffff);
        if (result>=0) {
            value->value04=handle;value->value08=bytes;value->value00=0;
            value->value0C=reinterpret_cast<unsigned int>(copy);value->value10=count;value->value14=reinterpret_cast<unsigned int>(references);
            reinterpret_cast<D3DXEffectValuePrefix *>(valueFC)->value18=reinterpret_cast<unsigned int>(value);
            valueFC=reinterpret_cast<unsigned int>(value);
            copy=0;references=0;value=0;
        }
    }
finish:
    if (copy) UG2FxFree(copy);
    if (references) references->DeleteArray(3);
    if (value) value->Delete(1);
    return result;
}
// Original 0x66c8f5, effect vtable+0x50 (stdcall, RET 16).
extern "C" int __stdcall D3DXEffectSetValue(D3DXEffectObject *effect,unsigned int handle,const void *data,unsigned int bytes) {
    D3DXEffectBaseRecord *record;
    if (static_cast<int>(handle)<0) record=reinterpret_cast<D3DXEffectBaseRecord *>(~handle);
    else { record=effect->ResolveParameter(handle,1,0);if (!record) return static_cast<int>(0x8876086c); }
    if (bytes!=0xffffffff && bytes<record->value0C) return static_cast<int>(0x8876086c);
    if (effect->value3C==3) return effect->RecordValue(~reinterpret_cast<unsigned int>(record),data,bytes);
    int result=effect->CopyValue(record,static_cast<const unsigned int *>(data));
    if (result>=0) result=effect->NotifyValue(record);
    return result;
}

// Original 0x6614a1. Scalar/matrix source data is local even when the type is
// shared. Struct relocation rebinds the prefix before calling vtable+0x50.
int D3DXEffectObject::RelocateParameter(D3DXEffectParameter *parameter,unsigned int *scalar,unsigned int *matrix,unsigned int *structure) {
    D3DXEffectParameter *owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[parameter->value14];
    unsigned int base=owner->value40 ? reinterpret_cast<unsigned int>(reinterpret_cast<D3DXEffectPool *>(value38)->data10) : value20;
    unsigned int *type=reinterpret_cast<unsigned int *>(base+owner->value2C+parameter->value18);
    unsigned int count=type[4];if (count<2) count=1;
    unsigned int shape=type[1],rows=type[5],columns=type[6];
    if (shape<2) {
        unsigned int *output=reinterpret_cast<unsigned int *>(value20+*scalar);
        const unsigned int *input=reinterpret_cast<const unsigned int *>(value20+parameter->value30);
        for (unsigned int i=count;i;--i) {
            for (unsigned int j=0;j<(rows&0x3fffffff);++j) output[j]=input[j];
            input+=rows;output+=4;
        }
        parameter->value30=*scalar;*scalar+=count*16;
    } else if (shape<4) {
        const unsigned int *input=reinterpret_cast<const unsigned int *>(value20+parameter->value30);
        unsigned int *output=reinterpret_cast<unsigned int *>(value20+*matrix);
        for (unsigned int i=count;i;--i) {
            unsigned int *first=output;
            for (unsigned int row=0;row<rows;++row) {
                for (unsigned int j=0;j<(columns&0x3fffffff);++j) output[j]=input[j];
                input+=columns;output+=4;
            }
            output+=(4-rows)*4;
            D3DXMatrixTransposeScalar(reinterpret_cast<WheelMatrix *>(first),reinterpret_cast<const WheelMatrix *>(first));
        }
        parameter->value30=*matrix;*matrix+=count*64;
    } else if (shape==5) {
        unsigned int typeOffset=parameter->value2C,dataOffset=parameter->value30;
        const void *oldData=reinterpret_cast<const void *>(value20+dataOffset);
        unsigned int bytes=D3DXEffectPaddedSize(value20,&typeOffset,&dataOffset);
        parameter->value30=*structure;*structure+=bytes;
        parameter->value00=value20+parameter->value30;parameter->value04=value20+parameter->value2C;
        int result=UG2FxSetValueVirtual(this,~reinterpret_cast<unsigned int>(parameter),oldData,parameter->value0C);
        if (result<0) return result;
    }
    return 0;
}
