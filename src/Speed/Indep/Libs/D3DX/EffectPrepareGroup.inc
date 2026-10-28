// Shared transcription expanded into the two original thiscall / RET 4 bodies.
// All offsets, stage constants and odd word-capacity/storage rules are retained.
int D3DXEffectObject::FX_PREPARE_METHOD(unsigned int groupIndex) {
    int result=0;unsigned int **sorted=0;
    {
        D3DXEffectGroup64 *group=reinterpret_cast<D3DXEffectGroup64 *>(value2C)+groupIndex;
        D3DXEffectRecord64 *primary=group->array04;
        D3DXEffectParameter *owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[primary->value08.value14];
        unsigned int base=owner->value40 ? reinterpret_cast<unsigned int>(reinterpret_cast<D3DXEffectPool *>(value38)->data10) : value20;
        primary->value08.value00=base+owner->value30+primary->value08.value1C;
        primary=group->array04;owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[primary->value08.value14];
        base=owner->value40 ? reinterpret_cast<unsigned int>(reinterpret_cast<D3DXEffectPool *>(value38)->data10) : value20;
        primary->value08.value04=base+owner->value2C+primary->value08.value18;
        primary=group->array04;owner=reinterpret_cast<D3DXEffectParameter **>(valueB0)[primary->value08.value14];
        base=owner->value40 ? reinterpret_cast<unsigned int>(reinterpret_cast<D3DXEffectPool *>(value38)->data10) : value20;
        unsigned int index=*reinterpret_cast<unsigned int *>(base+owner->value30+primary->value08.value1C);
        if (owner->value40) index=static_cast<unsigned int *>(reinterpret_cast<D3DXEffectPoolLink *>(owner->value40)->data08)[index];
        D3DXEffectReference18 *reference=reinterpret_cast<D3DXEffectReference18 *>(value18)+index;
        if (!(reference->value14&1)) {
            if ((group->value08&8) && valueB4 && !(reinterpret_cast<unsigned char *>(&value118)[1]&8)) {
                unsigned int rows=valueAC*valueA0;
                for (unsigned int row=0;row<rows;++row) {
                    D3DXEffectUseLink **second=reinterpret_cast<D3DXEffectUseLink ***>(valueC0)[row];
                    if (second) {
                        D3DXEffectUseLink **first=reinterpret_cast<D3DXEffectUseLink ***>(valueB4)[row],**end=0;
                        for (;*second;++second) if ((*second)->value14==groupIndex) {
                            if (!end) { end=first;while (*end) ++end; }
                            *end++=*second;*end=0;
                        }
                    }
                }
                group->value08&=0xfffffff7;
            }
            result=0;goto finish;
        }
        if (!reference->value04) { reference->value14|=2;reference->value0C=0; }
        else { result=FX_PREPARE_CAPTURE(reference);if (result<0) goto finish; }
        reference->value14&=0xfffffffe;
        for (unsigned int recordIndex=1;recordIndex<group->value36;++recordIndex) {
            D3DXEffectRecord64 *record=group->array04+recordIndex;
            if (record->value52==0xb4 || record->value52==0xb5) {
                if (record->value38) { UG2FxComRelease(record->value38);record->value38=0; }
                for (unsigned int use=0;use<record->value4E;++use) { result=UnlinkParameterUse(groupIndex,recordIndex,use);if (result<0) goto finish; }
            } else if (record->value4E) { result=UnlinkParameterUse(groupIndex,recordIndex,0);if (result<0) goto finish; }
            record->value4E=0;
        }
        D3DXEffectRecord64 *last[4]={0,0,0,0};
        group->value36=1;group->value34=0;group->FX_PREPARE_SELECTOR=0;
        unsigned short *banks=reinterpret_cast<unsigned short *>(reinterpret_cast<unsigned char *>(group)+FX_PREPARE_BANK_OFFSET);
        *reinterpret_cast<unsigned int *>(banks)=0;*reinterpret_cast<unsigned int *>(banks+2)=0;
        if (!(reference->value14&2)) goto finish;
        unsigned int *pres=0,*rsi=0,presBytes,prefix=1,fast=1,*table=reinterpret_cast<unsigned int *>(reference->value0C);
        unsigned short counts[4],minimum[4],maximum[4],presLimit[4];
        unsigned char *bulk=reinterpret_cast<unsigned char *>(group)+FX_PREPARE_BULK_OFFSET;
        if (!reference->value04) {
            reference->value0C=0;table=0;fast=0;
            for (unsigned int j=0;j<4;++j) { bulk[j]=0;banks[j]=0; }
            presLimit[0]=presLimit[1]=presLimit[2]=0;
        } else {
            unsigned char *tokens=UG2FxBufferPointerVirtual(reinterpret_cast<D3DXEffectBuffer *>(reference->value08));
            result=D3DXEffectReadConstantTable(reinterpret_cast<unsigned int *>(tokens),reinterpret_cast<unsigned int **>(&reference->value0C),counts,minimum,maximum);
            if (result<0) goto finish;
            *reinterpret_cast<unsigned int *>(banks)=*reinterpret_cast<unsigned int *>(maximum);
            *reinterpret_cast<unsigned int *>(banks+2)=*reinterpret_cast<unsigned int *>(maximum+2);
            tokens=UG2FxBufferPointerVirtual(reinterpret_cast<D3DXEffectBuffer *>(reference->value08));
            result=D3DXEffectFindComment(reinterpret_cast<unsigned int *>(tokens),0x53455250,&pres,&presBytes);
            if (result<0) return result;
            if (!result) {
                prefix=2;unsigned int ignoredBytes,*ignoredTable;unsigned short ignoredCounts[4],ignoredMinimum[4];
                int found=D3DXEffectFindComment(pres,0x49535250,&rsi,&ignoredBytes);if (found<0) return found;
                result=D3DXEffectReadConstantTable(pres,&ignoredTable,ignoredCounts,ignoredMinimum,presLimit);if (result<0) return result;
                for (unsigned int j=0;j<3;++j) presLimit[j]=static_cast<unsigned short>(rsi[j]+rsi[j+3]);
            } else presLimit[0]=presLimit[1]=presLimit[2]=0;
            for (unsigned int j=0;j<4;++j) { bulk[j]=minimum[j]==0 && maximum[j]==counts[j];if (counts[j] && !bulk[j]) fast=0; }
            table=reinterpret_cast<unsigned int *>(reference->value0C);
        }
        presLimit[3]=0;
        unsigned short *ranges=reinterpret_cast<unsigned short *>(reinterpret_cast<unsigned char *>(group)+0x4c);
        ranges[0]=ranges[1]=ranges[3]=ranges[4]=ranges[6]=ranges[7]=ranges[9]=ranges[10]=1;
        ranges[2]=banks[0];ranges[5]=banks[2];ranges[8]=banks[1];
        unsigned int flags=(group->value08&0xfffffffa)|(fast&1);
        group->value08=flags|FX_PREPARE_STAGE_FLAG;
        if (!table) goto finish;
        group->value08=flags|FX_PREPARE_STAGE_FLAG|8;
        unsigned int required=table[3]+prefix;
        if (group->value38<required) {
            unsigned int *cookie=static_cast<unsigned int *>(UG2FxMalloc(required*0x64+4));D3DXEffectRecord64 *records=0;
            if (cookie) { *cookie=required;records=reinterpret_cast<D3DXEffectRecord64 *>(cookie+1);D3DXEffectConstructArray(records,0x64,required,D3DXConstructRecord64); }
            if (!records) goto oom;
            primary=group->array04;records->value52=primary->value52;records->value54=0xffff;records->value58=ug2FxTypeTable7B582C[records->value52][4];records->value56=1;
            for (unsigned int word=0;word<10;++word) reinterpret_cast<unsigned int *>(&records->value08)[word]=reinterpret_cast<unsigned int *>(&primary->value08)[word];
            records->value00=primary->value00;records->value30=(records->value30&0xfffffffe)|(primary->value30&1);records->value30=(records->value30&0xfffffffd)|(primary->value30&2);
            records->value3C=primary->value3C;records->array34=primary->array34;records->value4E=primary->value4E;records->value50=primary->value50;records->value38=primary->value38;
            primary->array34=0;primary->value4E=0;primary->value50=0;primary->value38=0;
            if (group->array04) { group->array04->DeleteArray(3);group->array04=0; }
            group->array04=records;group->value38=static_cast<unsigned short>(table[3]+1);
        }
        group->value0C=0;group->value40=0;
        unsigned int amount=banks[0];if (amount<=presLimit[0]) amount=presLimit[0];
        unsigned int boolBase=amount*16;group->value0C=boolBase;group->value48=boolBase;
        amount=banks[1];if (amount<=presLimit[1]) amount=presLimit[1];
        unsigned int intBase=boolBase+amount*4;group->value0C=intBase;group->value44=intBase;
        amount=banks[2];if (amount<=presLimit[2]) amount=presLimit[2];
        unsigned int samplerBase=intBase+amount*16;group->value0C=samplerBase+banks[3]*4;group->value34=banks[3];
        if (pres) {
            unsigned int presIndex=group->value36++;D3DXEffectRecord64 *record=group->array04+presIndex;
            record->value00=1;record->value30=(record->value30&0xfffffff5)|1;group->value08|=4;
            result=D3DXEffectBufferCreateForward(presBytes+0x68,reinterpret_cast<D3DXEffectBuffer **>(&record->value38));if (result<0) goto finish;
            unsigned int *data=reinterpret_cast<unsigned int *>(UG2FxBufferPointerVirtual(reinterpret_cast<D3DXEffectBuffer *>(record->value38)));
            for (unsigned int bank=0;bank<3;++bank) {
                unsigned int count=rsi[bank]+rsi[bank+3],mask=count*4|((count&0x3fffffff)<<1);
                mask|=mask>>2;mask|=mask>>4;mask|=mask>>8;mask|=mask>>16;data[bank]=mask;
                mask=count*4;data[bank+3]=mask;if (mask) --mask;
                mask|=mask>>1;mask|=mask>>2;mask|=mask>>4;mask|=mask>>8;mask|=mask>>16;data[bank+3]=mask;
            }
            data[6]=0;
            unsigned int offset=(group->value0C&7)+(group->value0C&0xfffffff8);group->value0C=offset;data[7]=offset;
            offset=group->value0C+data[0]*8;offset=(offset&7)+(offset&0xfffffff8);group->value0C=offset;data[8]=offset;
            offset=group->value0C+data[1]*8;offset=(offset&7)+(offset&0xfffffff8);group->value0C=offset;data[9]=offset;
            group->value0C+=data[2]*8;data[10]=0;data[11]=boolBase;data[12]=intBase;record->value56=0;record->value54=0;
            for (unsigned int bank=0;bank<3;++bank) { data[16+bank]=rsi[bank];data[19+bank]=rsi[bank+3]; }
            data[23]=reinterpret_cast<unsigned int>(rsi)+0x80-reinterpret_cast<unsigned int>(pres);
            data[24]=reinterpret_cast<unsigned int>(rsi)+rsi[6]*8+0x84-reinterpret_cast<unsigned int>(pres);
            data[25]=data[24]+4+*reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned int>(pres)+data[24]-0x68)*8;
            unsigned int words=presBytes>>2,byte;
            for (byte=0;byte<words;++byte) data[26+byte]=pres[byte];
            for (byte=presBytes&0xfffffffc;byte<presBytes;++byte) reinterpret_cast<unsigned char *>(data+26)[byte]=reinterpret_cast<unsigned char *>(pres)[byte];
            result=D3DXEffectReadCompiledSections(data+26,reinterpret_cast<unsigned int **>(&record->value40),reinterpret_cast<unsigned int **>(&record->value44));if (result<0) goto finish;
            result=BindPreparationConstants(groupIndex,static_cast<unsigned int>(group->value36)-1,data);if (result<0) goto finish;
            record->value52=FX_PREPARE_PRES_CODE;record->value58=FX_PREPARE_PRES_TAG;group->value08|=1;
            for (unsigned int bank=0;bank<3;++bank) if (presLimit[bank]) {
                unsigned int *list=reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned int>(data)+data[23+bank]),sum=0;
                for (unsigned int range=list[0];range;--range) { list+=2;sum+=*list; }
                unsigned int maximum=banks[bank];if (maximum<=presLimit[bank]) maximum=presLimit[bank];
                if (counts[bank]+sum!=maximum) { group->value08&=0xfffffffe;break; }
                if (sum && !counts[bank]) group->value08|=0x10;
            }
            if (group->value08&1) {
                ranges[2]=banks[0]<=presLimit[0] ? presLimit[0] : banks[0];
                ranges[5]=banks[2]<=presLimit[2] ? presLimit[2] : banks[2];
                ranges[8]=banks[1]<=presLimit[1] ? presLimit[1] : banks[1];
            }
        }
        result=group->AllocateData();if (result<0) goto finish;
        group->value34=0;
        sorted=static_cast<unsigned int **>(UG2FxMalloc(table[3]*4));if (!sorted) goto oom;
        unsigned int *entry=reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned int>(table)+table[4]);
        for (unsigned int ordinal=0;ordinal<table[3];++ordinal,entry+=5) sorted[ordinal]=entry;
        D3DXEffectSort(sorted,table[3],4,D3DXEffectCompareConstants);
        unsigned int floatCount=0,intCount=0,boolCount=0;
        entry=reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned int>(table)+table[4]);
        for (unsigned int ordinal=0;ordinal<table[3];++ordinal,entry+=5) {
            unsigned short bank=static_cast<unsigned short>(entry[1]);if (!bank) ++boolCount;else if (bank==1) ++intCount;else if (bank==2) ++floatCount;
        }
        unsigned int first=group->value36,floatCursor=first,intCursor=first+floatCount,boolCursor=intCursor+intCount,samplerCursor=boolCursor+boolCount;
        group->value36=static_cast<unsigned short>(table[3]+first);ranges[0]=static_cast<unsigned short>(first);ranges[3]=static_cast<unsigned short>(intCursor);ranges[6]=static_cast<unsigned short>(boolCursor);ranges[9]=static_cast<unsigned short>(samplerCursor);
        for (unsigned int ordinal=0;ordinal<table[3];++ordinal) {
            entry=sorted[ordinal];unsigned short bank=static_cast<unsigned short>(entry[1]),start=static_cast<unsigned short>(entry[1]>>16),count=static_cast<unsigned short>(entry[2]);
            unsigned int recordIndex=0;
            if (!bank) recordIndex=static_cast<unsigned short>(boolCursor++);else if (bank==1) recordIndex=static_cast<unsigned short>(intCursor++);else if (bank==2) recordIndex=static_cast<unsigned short>(floatCursor++);else if (bank==3) recordIndex=static_cast<unsigned short>(samplerCursor++);
            D3DXEffectRecord64 *record=group->array04+recordIndex;record->value54=start;record->value30|=9;record->value56=count;
            D3DXEffectParameter *parameter=FindRootParameter(reinterpret_cast<unsigned int>(table)+entry[0],10);
            if (!parameter) record->value00=0;
            else {
                if (!record->value50) {
                    unsigned int *cookie=static_cast<unsigned int *>(UG2FxMalloc(0x24));D3DXEffectUseLink *use=0;
                    if (cookie) { *cookie=1;use=reinterpret_cast<D3DXEffectUseLink *>(cookie+1);D3DXEffectConstructArray(use,0x20,1,D3DXConstructUse); }
                    record->array34=reinterpret_cast<D3DXEffectRecord20 *>(use);if (!use) goto oom;record->value50=1;
                }
                result=LinkParameter(groupIndex,recordIndex,0,parameter,reinterpret_cast<unsigned int>(entry));if (result<0) goto finish;record->value4E=1;
                for (unsigned int word=0;word<10;++word) reinterpret_cast<unsigned int *>(&record->value08)[word]=reinterpret_cast<unsigned int *>(parameter)[word];
                record->value08.array20=0;record->value08.linked24=0;record->value30&=0xfffffffd;record->value00=1;
            }
            record->value3C=reinterpret_cast<unsigned int>(table);
            if (!bank) {
                record->value52=FX_PREPARE_BASE_CODE+1;record->value58=FX_PREPARE_BASE_TAG|0x10000;record->value48=boolBase+start*4;
                unsigned int *output=reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned int>(group->data00)+record->value48),*input=entry[4] ? reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned int>(table)+entry[4]) : 0;
                for (unsigned int word=0;word<count;++word) output[word]=input ? input[word] : 0;
                if (bulk[1]) { if (last[1]) last[1]->value30&=0xfffffffe;last[1]=record; }
            } else if (bank==1) {
                record->value52=FX_PREPARE_BASE_CODE+2;record->value58=FX_PREPARE_BASE_TAG|0x20000;record->value48=intBase+start*16;
                for (unsigned int unit=0;unit<count;++unit) { unsigned int *output=reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned int>(group->data00)+record->value48+unit*16);output[0]=0;output[1]=0;output[2]=1;output[3]=0; }
                if (entry[4]) { unsigned int *input=reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned int>(table)+entry[4]),*output=reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned int>(group->data00)+record->value48);for (unsigned int word=0;word<static_cast<unsigned int>(count)*4;++word) output[word]=input[word]; }
                if (bulk[2]) { if (last[2]) last[2]->value30&=0xfffffffe;last[2]=record; }
            } else if (bank==2) {
                if (parameter) {
                    unsigned int *description=reinterpret_cast<unsigned int *>(parameter->value04),elements=description[4] ? description[4] : 1,columns=description[6],units=0;bool cache=false;int shape=static_cast<int>(description[1]);
                    if (shape>=0 && shape<2 && description[0]==3) { cache=true;units=elements; }
                    else if (shape==2 && description[0]==3 && *reinterpret_cast<unsigned short *>(reinterpret_cast<unsigned int>(table)+entry[3])==3) {
                        if (elements==1) { cache=true;units=columns; } else if (columns==4) { cache=true;units=elements<<2; }
                    } else if (shape==5) { unsigned int type=parameter->value04;if (D3DXEffectVectorOnly(reinterpret_cast<unsigned int **>(&type))) { type=parameter->value04;units=D3DXEffectUnitCount(&type,4);cache=true; } }
                    if (cache) { record->value30|=0x10;record->value5C=parameter->value00;record->value60=(units<count ? units : static_cast<unsigned int>(count))*16; }
                }
                record->value52=FX_PREPARE_BASE_CODE;record->value58=FX_PREPARE_BASE_TAG;record->value48=start*16;
                unsigned int *output=reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned int>(group->data00)+record->value48),*input=entry[4] ? reinterpret_cast<unsigned int *>(reinterpret_cast<unsigned int>(table)+entry[4]) : 0;
                for (unsigned int word=0;word<static_cast<unsigned int>(count)*4;++word) output[word]=input ? input[word] : 0;
                if (bulk[0]) { if (last[0]) last[0]->value30&=0xfffffffe;last[0]=record; }
            } else if (bank==3) {
                record->value52=0xb2;record->value58=0x0f000000;record->value54=static_cast<unsigned short>(record->value54+FX_PREPARE_SAMPLER_BIAS);record->value30&=0xfffffffd;record->value48=samplerBase+start*4;record->value00=0;
                unsigned short *storage=reinterpret_cast<unsigned short *>(reinterpret_cast<unsigned int>(group->data00)+record->value48);
                if (!parameter) { storage[0]=0;storage[1]=0; }
                else { storage[0]=parameter->value44+1;storage[1]=0;record->value56=record->value56<parameter->value46 ? record->value56 : parameter->value46; }
                reinterpret_cast<unsigned short *>(reinterpret_cast<unsigned int>(group->data00)+group->value14)[group->value34++]=static_cast<unsigned short>(recordIndex);
            }
        }
        ranges[1]=static_cast<unsigned short>(floatCursor);ranges[4]=static_cast<unsigned short>(intCursor);ranges[7]=static_cast<unsigned short>(boolCursor);ranges[10]=static_cast<unsigned short>(samplerCursor);
        goto finish;
    }
oom:
    result=static_cast<int>(0x8007000e);
finish:
    UG2FxFree(sorted);return result;
}
