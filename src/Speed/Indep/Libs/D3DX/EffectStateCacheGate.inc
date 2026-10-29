// Two complete 347-byte bodies differ only in the technique offsets and
// selected capture helpers. FX_* constants are verified per instantiation.
int D3DXEffectObject::FX_GATE(int enabled,unsigned short *requested) {
    unsigned char *technique=reinterpret_cast<unsigned char *>(valueA4)+
        reinterpret_cast<unsigned short *>(&value40)[1]*0xb4;
    int result=0;
    if (!(value118&8)) return result;
    unsigned short sizes[4]; // first three stay indeterminate on sampler-only paths
    if (value118&0x10) {
        bool skip=false;
        unsigned int *ready=reinterpret_cast<unsigned int *>(technique+FX_READY);
        unsigned short *old=reinterpret_cast<unsigned short *>(technique+FX_SIZES);
        if (*reinterpret_cast<unsigned int *>(technique+FX_CONSTANT_SLOT) && (!enabled || *ready)) {
            if (requested[0]<=old[0] && requested[1]<=old[1] && requested[2]<=old[2]) skip=true;
        }
        if (!skip) {
            unsigned int captureShader=(*ready || enabled) ? 1 : 0;
            sizes[0]=old[0]>requested[0] ? old[0] : requested[0];
            sizes[1]=old[1]>requested[1] ? old[1] : requested[1];
            sizes[2]=old[2]>requested[2] ? old[2] : requested[2];
            result=FX_CONSTANT_CAPTURE(captureShader,sizes);
            if (result<0) return result;
            old[0]=sizes[0];old[1]=sizes[1];*ready=captureShader;old[2]=sizes[2];
        }
    }
    unsigned short *oldSampler=reinterpret_cast<unsigned short *>(technique+FX_SIZES+6);
    if (!*reinterpret_cast<unsigned int *>(technique+FX_SAMPLER_SLOT) || *oldSampler<requested[3]) {
        unsigned short count=requested[3]<*oldSampler ? *oldSampler : requested[3];
        sizes[3]=count;
        result=FX_SAMPLER_CAPTURE(0,sizes);
        if (result>=0) *oldSampler=count;
    }
    return result;
}
