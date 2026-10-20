#include "EngineSoundMixer.h"
// UG2 PC v1.2 0x477a50. Fields not written by the binary remain untouched.
void EngineSoundMixerChannel::Setup477A50(unsigned int id, void *section) {
    section44 = section;
    value8 = id;
    count30 = (id >> 16) & 0xff;
    valueC = ((id & 0xff00) | 0x200000) << 8 | (id & 0x100000ff);
    value4 = 0; count2C = 0; value10 = 0;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x24) = 32767;
}
// 0x467400 / 0x467410: master channel output key.
void EngineSoundMixerChannel::SetOutput467400(unsigned int id) { value14 = id; }
unsigned int EngineSoundMixerChannel::Output467410() { return value14; }
// 0x479030: RET 0xc establishes the callee-cleaned three-word ABI.
unsigned int *__stdcall EngineSoundMixerDecode479030(void *, EngineSoundMixerChannel *channel, unsigned int *record) {
    const unsigned int id = record[0], data = record[1];
    unsigned int *next = record + 2;
    int *bounds = reinterpret_cast<int *>(reinterpret_cast<char *>(channel) + 0x1c);
    if (!(id & 0x10000000)) {
        const int initial = -(int)(short)(data >> 16);
        channel->value18 = initial;
        bounds[1] = initial - 10000;
        bounds[0] = data >> 16;
        const unsigned int output = *next++;
        typedef void (__fastcall *Setter)(EngineSoundMixerChannel *, void *, unsigned int);
        reinterpret_cast<Setter>(channel->vtable0->valueC)(channel, 0, output);
        EngineSoundDiagnostic4691C0(0, engineSoundMixerDecodeMaster78BA1C, id, output, data >> 16);
    } else {
        bounds[0] = (data >> 16) & 0x7fff;
        bounds[1] = data | 0xffff0000;
        channel->value18 = 0;
        EngineSoundDiagnostic4691C0(0, engineSoundMixerDecodeSub78B9F0, id, (data >> 16) & 0x7fff, data | 0xffff0000);
    }
    channel->inputs34 = next;
    return next;
}
// 0x4683e0: spatial map entry initialization.
void EngineSoundMixerSpatial::Setup4683E0(unsigned int *record, unsigned int instance, unsigned int index) {
    record28 = record;
    const unsigned int key = *record | instance;
    key1C = key;
    key20 = (key & 0xff0000) | instance | index | 0x90000000;
    data24 = record + 1;
    engineSoundMixerScratch82BCA0 = 0;
    engineSoundMixerScratch82BC9C = 0;
    value10 = -10000;
}
// 0x468430: the first two attachment arguments are unused.
void EngineSoundMixerSpatial::Attach468430(EngineSoundMixerChannel *, EngineSound *, EngineSoundScheduledNode *input) {
    input4 = reinterpret_cast<char *>(input) + 4;
    *reinterpret_cast<unsigned int *>(reinterpret_cast<char *>(input) + 0x38) = *data24 & 0xff00;
    const unsigned short *data = reinterpret_cast<unsigned short *>(data24);
    unsigned int maximum = data[5] & 0x7fff;
    if (maximum < (data[7] & 0x7fffu)) maximum = data[7] & 0x7fff;
    if (maximum < (data[9] & 0x7fffu)) maximum = data[9] & 0x7fff;
    if (maximum < (data[11] & 0x7fffu)) maximum = data[11] & 0x7fff;
    *reinterpret_cast<unsigned int *>(input4 + 0x30) = maximum;
}
// Original vtable leaves 0x503b00 / 0x6e18a0 / 0x40bde0.
unsigned int EngineSoundMixerSpatial::Key503B00() { return key20; }
unsigned int EngineSoundMixerSpatial::Input6E18A0() { return key1C; }
int *EngineSoundMixerSpatial::Values40BDE0() { return &value10; }
