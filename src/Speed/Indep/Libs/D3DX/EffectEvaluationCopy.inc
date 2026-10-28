#if FX_COPY_RECURSIVE
extern "C" void __stdcall FX_COPY_NAME(unsigned int **typeCursor,unsigned int **sourceCursor,double **destinationCursor,
    unsigned int tableBase,unsigned int typeOffset,unsigned int *remaining,unsigned int elements) {
    unsigned int *type=*typeCursor;
    unsigned short *reflection=tableBase ? reinterpret_cast<unsigned short *>(tableBase+typeOffset) : 0;
#else
extern "C" void __stdcall FX_COPY_NAME(D3DXEffectBaseRecord *parameter,double *destination,unsigned int count,
    unsigned int tableBase,unsigned int entry,unsigned int *written,unsigned int elements) {
    unsigned int *type=reinterpret_cast<unsigned int *>(parameter->value04);
    unsigned short *reflection=tableBase ? reinterpret_cast<unsigned short *>(tableBase+reinterpret_cast<unsigned int *>(entry)[3]) : 0;
    unsigned int *source=reinterpret_cast<unsigned int *>(parameter->value00);
    double *initial=destination;
#endif
    int shape=tableBase ? static_cast<int>(*reflection) : static_cast<int>(type[1]);
    unsigned int arrays=elements;
    if (elements==0xffffffff) { arrays=type[4];if (!arrays) arrays=1; }
    if (shape<0) goto finish;
    if (shape==5) {
        unsigned int members=type[5],*children=type+6,memberOffset=0;
        unsigned int *list=tableBase ? reinterpret_cast<unsigned int *>(tableBase+reinterpret_cast<unsigned int *>(reflection)[3]) : 0;
#if FX_COPY_RECURSIVE
        *typeCursor=children;
        for (unsigned int array=0;array<arrays;++array) {
            if (!*remaining) return;
            *typeCursor=children;
            for (unsigned int member=0;member<members;++member) {
                if (!*remaining) break;
                if (tableBase) memberOffset=list[member*2+1];
                FX_COPY_WALK(typeCursor,sourceCursor,destinationCursor,tableBase,memberOffset,remaining,0xffffffff);
            }
        }
#else
        for (unsigned int array=0;array<arrays;++array) {
            if (!count) break;
            unsigned int *child=children;
            for (unsigned int member=0;member<members;++member) {
                if (!count) break;
                if (tableBase) memberOffset=list[member*2+1];
                FX_COPY_WALK(&child,&source,&destination,tableBase,memberOffset,&count,0xffffffff);
            }
        }
#endif
        goto finish;
    }
    if (shape>3) goto finish;
    {
        unsigned int kind=type[0];if (kind<1 || kind>3) goto finish;
#if FX_COPY_RECURSIVE
        unsigned int *source=*sourceCursor,count=*remaining;
        double *destination=*destinationCursor;
#endif
        if (shape==2) {
            unsigned int rows=type[5];
#if !FX_COPY_SCALAR
            unsigned int columns=type[6];
#endif
            unsigned int *matrix=source;
            for (unsigned int array=0;array<arrays;++array) {
                if (!count) break;
                for (unsigned int row=0;row<rows;++row) {
                    if (!count) break;
#if FX_COPY_SCALAR
                    if (kind==3) *destination=static_cast<double>(reinterpret_cast<float *>(matrix)[row]);
                    else *destination=static_cast<double>(reinterpret_cast<int *>(source)[row]);
                    ++destination;
#else
                    for (unsigned int column=0;column<columns;++column) {
                        if (kind==3) destination[column]=static_cast<double>(reinterpret_cast<float *>(matrix)[row+column*4]);
                        else destination[column]=static_cast<double>(reinterpret_cast<int *>(source)[row+column*4]);
                    }
                    destination+=4;
#endif
                    --count;
                }
                matrix+=16;
            }
#if FX_COPY_RECURSIVE
            // Integer matrices read the original source for every array but
            // still advance the published input cursor by 0x40 per array.
            source=matrix;
#endif
        } else {
            unsigned int columns=type[6],padding=shape==3 ? (4-columns)*4 : 0;
#if !FX_COPY_SCALAR
            unsigned int rows=type[5];
#endif
            for (unsigned int array=0;array<arrays;++array) {
                if (!count) break;
                for (unsigned int column=0;column<columns;++column) {
                    if (!count) break;
                    if (kind==3) {
                        // Scalar copies also write four doubles, while moving
                        // their output by one double: later writes overlap.
                        destination[0]=static_cast<double>(reinterpret_cast<float *>(source)[0]);
                        destination[1]=static_cast<double>(reinterpret_cast<float *>(source)[1]);
                        destination[2]=static_cast<double>(reinterpret_cast<float *>(source)[2]);
                        destination[3]=static_cast<double>(reinterpret_cast<float *>(source)[3]);
                    } else {
#if FX_COPY_SCALAR
                        *destination=static_cast<double>(*reinterpret_cast<int *>(source));
#else
                        for (unsigned int row=0;row<rows;++row) destination[row]=static_cast<double>(reinterpret_cast<int *>(source)[row]);
#endif
                    }
                    source+=4;
                    destination+=FX_COPY_SCALAR ? 1 : 4;
                    --count;
                }
                source+=padding;
            }
        }
#if FX_COPY_RECURSIVE
        *destinationCursor=destination;*remaining=count;*sourceCursor=source;*typeCursor+=7;
#endif
    }
finish:
#if !FX_COPY_RECURSIVE
    *written=(reinterpret_cast<unsigned int>(destination)-reinterpret_cast<unsigned int>(initial))>>(FX_COPY_SCALAR ? 3 : 5);
#endif
    return;
}
