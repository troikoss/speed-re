// Four complete 305-byte bodies. Slot is captured before callbacks; manager
// and old block are reread at their original boundaries. Both temporary blocks
// are released on failure; only the second transfers to the technique on success.
int D3DXEffectObject::FX_CAPTURE(int enabled,unsigned short *sizes) {
    unsigned int *slot=reinterpret_cast<unsigned int *>(valueA4+
        reinterpret_cast<unsigned short *>(&value40)[1]*0xb4+FX_SLOT);
    unsigned int blocks[2]={0,0};
    int result;
    if (!(reinterpret_cast<unsigned char *>(&value118)[1]&2)) {
        for (unsigned int i=0;i<2;++i) {
            result=UG2FxStateBlockBegin(value0C);
            if (result<0) goto finish;
            D3DXEffectDebugMute(1);
            result=FX_RESET(enabled,sizes);
            D3DXEffectDebugMute(0);
            if (result<0) { UG2FxStateBlockEnd(value0C,blocks+i);goto finish; }
            result=UG2FxStateBlockEnd(value0C,blocks+i);
            if (result<0) goto finish;
        }
    }
    result=UG2FxStateBlockCapture(blocks[0]);
    if (result<0) goto finish;
    result=*slot ? UG2FxStateBlockApply(*slot) : 0;
    if (result<0) goto finish;
    result=UG2FxStateBlockCapture(blocks[1]);
    if (result<0) goto finish;
    result=UG2FxStateBlockApply(blocks[0]);
    if (result<0) goto finish;
    if (*slot) { UG2FxStateBlockRelease(*slot);*slot=0; }
    *slot=blocks[1];blocks[1]=0;
finish:
    if (blocks[0]) { UG2FxStateBlockRelease(blocks[0]);blocks[0]=0; }
    if (blocks[1]) UG2FxStateBlockRelease(blocks[1]);
    return result;
}
