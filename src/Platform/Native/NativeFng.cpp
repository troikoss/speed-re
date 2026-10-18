#include <sys/mman.h>
#include "OriginalImage.h"
#include "NativeFng.h"
#include "NativeAssets.h"
#include "NativeHud.h"
#include "../../Speed/Indep/Src/Frontend/FEngColour.h"
#include "../../Speed/Indep/Src/Frontend/FEngText.h"
#include "../../Speed/Indep/Src/Frontend/FEngStringReader.h"
#include "../../Speed/Indep/Src/Frontend/FEngResponseReader.h"
#include "../../Speed/Indep/Src/Frontend/FEngListReader.h"
#include "../../Speed/Indep/Src/Frontend/FEngScript.h"
#include "../../Speed/Indep/Src/Frontend/FEngPackage.h"
#include "../../Speed/Indep/Src/Frontend/FEngObject.h"
#include "../../Speed/Indep/Src/Frontend/FEngObjectFactory.h"
#include "../../Speed/Indep/Src/Hud/HudAnimation.h"

#include <algorithm>
#include <utility>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void NativeTrap(const char *what);

// Native storage for recovered original data. Vtable/default callback words
// retain retail address tokens; verified response virtual slots have native bindings.
unsigned int FEngNextObjectGuid = 1; // original 0x7f7cf8 initial value
unsigned int FEngValue7F7CC4 = 0x518e20;
FEngStringObserver *fengStringObserver838478 = 0; // Relocated native storage for original 0x838478.
FEngStatePoolManager FEngObjectStateManager = {}; // Original 0x863770; native strings still bind inline state.
FEngDestructionObserver *FEngObjectDestructionObserver = 0; // Original 0x838474.

namespace {
void BindNativeFengVtables();
// Native heap owner only: original resizing/free calls remain in recovered code.
// Retain surviving allocations until wrapper teardown, including original zero-
// dimension/storage-release cases that leave cell allocations unfreed.
std::vector<void *> *nativeFengAllocationOwner = 0;
class NativeFengAllocationScope {
    std::vector<void *> *previous;
public:
    explicit NativeFengAllocationScope(std::vector<void *> *owner) : previous(nativeFengAllocationOwner) {
        nativeFengAllocationOwner = owner;
    }
    ~NativeFengAllocationScope() { nativeFengAllocationOwner = previous; }
};
}
// Native heap adapter for original 0x575620/0x575630 boundaries.
void *FEngAllocateQueueNode(unsigned int size) {
    void *pointer = malloc(size);
    if (pointer && nativeFengAllocationOwner) nativeFengAllocationOwner->push_back(pointer);
    return pointer;
}
void FEngFreeQueueNode(void *pointer) {
    if (nativeFengAllocationOwner) {
        std::vector<void *> &owned = *nativeFengAllocationOwner;
        std::vector<void *>::iterator entry = std::find(owned.begin(), owned.end(), pointer);
        if (entry != owned.end()) owned.erase(entry);
    }
    free(pointer);
}
void *FEngGridAllocateCells(unsigned int bytes) { return malloc(bytes); }
void FEngGridFreeCells(void *cells) { free(cells); }

FengObject::~FengObject() {
    NativeFengAllocationScope heapOwner(&ownedAllocations);
    if (type18 == 1 || type18 == 2 || type18 == 4 || type18 == 5) {
        // Native ownership adapter: the state is inline, and script/response
        // headers belong to vectors. Detach those borrowed prefixes before
        // original string/base cleanup; C++ members release their own storage.
        data2C = 0;
        FEngQueueList *scripts = reinterpret_cast<FEngQueueList *>((unsigned char *)this + 0x44);
        FEngQueueList *responses = reinterpret_cast<FEngQueueList *>((unsigned char *)this + 0x34);
        scripts->count4 = responses->count4 = 0;
        scripts->first8 = responses->first8 = 0;
        scripts->valueC = responses->valueC = 0;
        switch (type18) {
        case 1: reinterpret_cast<FEngLeafObject *>(this)->DestructImage(); break;
        case 2: DestructString(); break;
        case 4: {
            FEListObject *list = reinterpret_cast<FEListObject *>(this);
            list->Destruct();
            break;
        }
        case 5: {
            FEngGroupObject *group = reinterpret_cast<FEngGroupObject *>(this);
            // Package objects own the children; this list borrows their pointers.
            group->children5C.count4 = 0;
            group->children5C.first8 = group->children5C.valueC = 0;
            group->Destruct();
            break;
        }
        }
    }
    // Platform heap-owner teardown, after original normal cleanup. This is not
    // an extra recovered game cleanup step or a change to its release ordering.
    for (size_t i = 0; i < ownedAllocations.size(); ++i) free(ownedAllocations[i]);
    ownedAllocations.clear();
}

#define IMSTB_TRUETYPE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "third_party/imstb_truetype.h"

namespace {

// Localization/byte conversion remain native behind the original observer ABI.
class NativeStringObserver : public FEngStringObserver {
public:
    const std::vector<unsigned char> *language;
    void StringChanged(HudVisibilityObject *value) {
        if (!value || value->type18 != 2 || !language) return;
        FengObject *object = static_cast<FengObject *>(value);
        object->font = object->languageHash60;
        std::string text = object->languageHash60 ? LookupNativeString(*language, object->languageHash60) : "";
        FengPackage::SetText(object, text.c_str());
    }
};
NativeStringObserver nativeStringObserver;

unsigned int Rd32(const unsigned char *p) { return p[0] | p[1] << 8 | p[2] << 16 | (unsigned int)p[3] << 24; }
unsigned int Rd16(const unsigned char *p) { return p[0] | p[1] << 8; }
// Native wrapper allocation needs the type before constructing its retail prefix.
// Original factory allocation/state ownership and surrounding reader remain separate.
unsigned int NativeConstructionType(const unsigned char *data, size_t start, size_t end) {
    for (size_t chunk = start; chunk + 8 <= end;) {
        size_t next = chunk + 8 + Rd32(data + chunk + 4);
        if (next > end || next < chunk + 8) break;
        if (Rd32(data + chunk) == 0x446a624f) {
            for (size_t property = chunk + 8; property + 4 <= next;) {
                unsigned int length = Rd16(data + property + 2);
                if (property + 4 + length > next) break;
                if (Rd16(data + property) == 0x744f && length >= 4) return Rd32(data + property + 4);
                property += 4 + length;
            }
        }
        chunk = next;
    }
    return 0;
}
float RdF(const unsigned char *p) {
    unsigned int v = Rd32(p);
    float f;
    memcpy(&f, &v, 4);
    return f;
}

const unsigned int kInitScript = 0x1744b3;  // 0x5515e0 / 0x5554a0: the "Init" script hash

// Native bounds/termination and ownership adapter. All response semantics and
// list writes execute the complete recovered 0x54d1d0 reader.
void ReadResponses(const unsigned char *data, size_t size, std::vector<FengResponse> *responses,
                   FEPackage *package, HudVisibilityObject *object) {
    std::vector<unsigned char> bounded;
    for (size_t t = 0; t + 4 <= size;) {
        unsigned int tag = Rd16(data + t), len = Rd16(data + t + 2);
        if (len > size - t - 4) break;
        if ((tag == 0x694d || tag == 0x434d || tag == 0x6952 || tag == 0x7552 || tag == 0x7452) && len < 4) break;
        bool terminate = tag == 0x7352 && !memchr(data + t + 4, 0, len);
        if (terminate && len == 0xffff) break;
        unsigned int retained = len + (terminate ? 1 : 0);
        bounded.push_back((unsigned char)tag); bounded.push_back((unsigned char)(tag >> 8));
        bounded.push_back((unsigned char)retained); bounded.push_back((unsigned char)(retained >> 8));
        bounded.insert(bounded.end(), data + t + 4, data + t + 4 + len);
        if (terminate) bounded.push_back(0);
        t += 4 + len;
    }
    BindNativeFengVtables();
    FEngResponseReader reader = {};
    reader.package0 = package;
    reader.objectC = object;
    // Original outer-reader setup is pending; native objects are not cloned yet.
    reader.value18 = 0;
    reader.Read(bounded.data(), bounded.size(), object ? 0 : 1);
    FEResponse *header = object ? object->responses3C : package->responses5C;
    while (header) {
        bool owned = false;
        for (size_t i = 0; i < responses->size(); ++i)
            if ((*responses)[i].original == header) { owned = true; break; }
        if (!owned) responses->emplace_back(header);
        header = header->next4;
    }
}

// Retained retail tracks use the -1 key as a baseline, and later keys as
// offsets (0x52bdc0). This bounded platform adapter supports message tracks
// of types 4/6, interpolation 1 with cycle 0/1, and constant tracks at time zero.
// Other animation forms remain explicit seams.
void ApplyTrack(const FengTrack &track, unsigned char *state, int stateBytes, int time) {
    if (track.keys.empty() || track.offset < 0 || track.offset * 4 + track.size > stateBytes) return;
    unsigned char *out = state + track.offset * 4;
    const unsigned char *base = track.baseline.empty() ? &track.keys[0].second[0] : &track.baseline[0];
    if (track.keys.size() == 1) { memcpy(out, base, track.size); return; }
    if (track.interpolation != 1 || track.cycle > 1 || (track.type != 4 && track.type != 6)) {
        // The loader can still use a constant first key for unrecovered tracks.
        if (time == 0) { memcpy(out, base, track.size); return; }
        fprintf(stderr, "FEng track type=%d interpolation=%d cycle=%d duration=%d offset=%d time=%d\n", track.type, track.interpolation, track.cycle, track.duration, track.offset, time);
        NativeTrap("FEng message track interpolation/cycle");
    }
    // 0x52bdc0: cycle 1 joins the last key to the first over the remaining duration.
    if (track.cycle == 1 && track.duration > 0) time %= track.duration;
    size_t upper = 1;
    while (upper + 1 < track.keys.size() && track.keys[upper].first < time) ++upper;
    size_t lower = upper > 1 ? upper - 1 : upper;
    float fraction = 1.0f;
    if (track.cycle == 1) {
        int lowerTime, upperTime;
        if (time > track.keys[upper].first) {
            lower = upper; upper = 1;
            lowerTime = track.keys[lower].first;
            upperTime = track.duration + track.keys[upper].first;
        } else if (upper == 1 && time < track.keys[upper].first) {
            lower = track.keys.size() - 1;
            lowerTime = track.keys[lower].first - track.duration;
            upperTime = track.keys[upper].first;
        } else {
            lowerTime = track.keys[lower].first;
            upperTime = track.keys[upper].first;
        }
        fraction = upperTime > lowerTime ? (float)(time - lowerTime) / (upperTime - lowerTime) : 0;
    } else if (track.keys[upper].first > time) {
        if (upper == 1) fraction = 0.0f;
        else fraction = (float)(time - track.keys[lower].first) /
                        (float)(track.keys[upper].first - track.keys[lower].first);
    }
    const unsigned char *a = &track.keys[lower].second[0], *b = &track.keys[upper].second[0];
    if (track.type == 4)
        HudLerpVector3(reinterpret_cast<const float *>(a), reinterpret_cast<const float *>(b), fraction,
                       reinterpret_cast<const float *>(base), reinterpret_cast<float *>(out));
    else HudLerpColour(reinterpret_cast<const int *>(a), reinterpret_cast<const int *>(b), fraction,
                       reinterpret_cast<const int *>(base), reinterpret_cast<int *>(out));
}

void ReadState(FengObject *object, const unsigned char *state, int size) {
    if (size < 68) return;
    for (int i = 0; i < 4; ++i) object->colour[i] = Rd32(state + i * 4);
    for (int i = 0; i < 3; ++i) {
        object->pivot[i] = RdF(state + 16 + i * 4);
        object->position[i] = RdF(state + 28 + i * 4);
        object->size[i] = RdF(state + 56 + i * 4);
    }
    for (int i = 0; i < 4; ++i) object->rotation[i] = RdF(state + 40 + i * 4);
    if (size >= 84) for (int i = 0; i < 4; ++i) object->uv[i] = RdF(state + 68 + i * 4);
    object->angle = 2.0f * atan2f(object->rotation[2], object->rotation[3]) * 180.0f / 3.14159265f;
}

}  // namespace

void BindNativeStringObserver(const std::vector<unsigned char> *language) {
    nativeStringObserver.language = language;
    fengStringObserver838478 = &nativeStringObserver;
}

unsigned int FengHash(const char *text) {
    return FngHash(text); // Native naming adapter; original 0x505450 executes recovered code.
}

// Native relocation of retail vtable tokens. Recovered code still performs the
// original virtual calls; mapped retail data or fixture pages bind the slots.
namespace {
void *__thiscall NativeDeleteBaseNode(void *pointer, unsigned int flags) {
    return ((FEngQueueNode *)pointer)->DestroyBase(flags);
}
void *__thiscall NativeDeleteResponseBlock(void *pointer, unsigned int flags) {
    return ((FEngResponsePoolBlock *)pointer)->Destroy(flags);
}
void *__thiscall NativeDeleteResponseHeader(void *pointer, unsigned int flags) {
    return ((FEResponse *)pointer)->Destroy(flags);
}
void *__thiscall NativeDeleteScriptKey(void *pointer, unsigned int flags) {
    return ((FEngScriptKey *)pointer)->Destroy(flags);
}
void *__thiscall NativeDeleteScriptKeyBlock(void *pointer, unsigned int flags) {
    return ((FEngScriptKeyPoolBlock *)pointer)->Destroy(flags);
}
void BindNativeFengVtables() {
    static bool bound = false;
    if (bound) return;
    if (IsOriginalImageMapped()) {
        // MapOriginalImage owns this writable data range and verifies its SHA-256.
        // Preserve retail constants and replace only these verified virtual slots.
        if (*(unsigned int *)0x79a9e8 != 0x4fdb00 || *(unsigned int *)0x79aa38 != 0x53ee60 ||
            *(unsigned int *)0x79bf50 != 0x53eb20 || *(unsigned int *)0x79aa34 != 0x531fe0 ||
            *(unsigned int *)0x79b9ec != 0x531bc0) NativeTrap("FEng response vtable contents");
    } else {
        const unsigned int pages[] = {0x79a000, 0x79b000};
        for (unsigned int i = 0; i < 2; ++i) {
            void *page = mmap((void *)pages[i], 4096, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
            if (page != (void *)pages[i]) NativeTrap("FEng response vtable relocation");
        }
    }
    *(void **)0x79a9e8 = (void *)NativeDeleteBaseNode;
    *(void **)0x79aa38 = (void *)NativeDeleteResponseHeader;
    *(void **)0x79bf50 = (void *)NativeDeleteResponseBlock;
    *(void **)0x79aa34 = (void *)NativeDeleteScriptKey;
    *(void **)0x79b9ec = (void *)NativeDeleteScriptKeyBlock;
    bound = true;
}
}
FEngResponsePool FEngResponseHeaderPool = {{0x79a9ec, 0, 0, 0}};
FEngScriptKeyPool FEngScriptKeysPool = {{0x79a9ec, 0, 0, 0}};

// Native movable reference owner. Header allocation, destruction, return-to-pool
// and empty-block cleanup all execute recovered code.
FengResponse::FengResponse() : original(0) {
    BindNativeFengVtables();
    original = FEngResponseHeaderPool.Acquire();
    FEngConstructResponseHeader(original);
}
FengResponse::FengResponse(FEResponse *header) : original(header) {}
FengResponse::~FengResponse() { if (original) original->Destroy(1); }
FengResponse::FengResponse(FengResponse &&other) noexcept : original(other.original) {
    other.original = 0;
}
FengResponse &FengResponse::operator=(FengResponse &&other) noexcept {
    if (this != &other) {
        if (original) original->Destroy(1);
        original = other.original;
        other.original = 0;
    }
    return *this;
}

// Inline script headers remain native owners. Their owned tracks, keys, names
// and events now use recovered construction and normal destruction.
FengScript::FengScript() : header() {
    BindNativeFengVtables();
    FEngConstructScript(&header);
    header.time10 = 0; // Native reader binding; the original constructor leaves it untouched.
}
FengScript::~FengScript() { FEngDestructScript(&header); }
FengScript::FengScript(FengScript &&other) noexcept
    : header(other.header), tracks(std::move(other.tracks)), originalKeys(std::move(other.originalKeys)) {
    other.header.name2C = 0;
    other.header.trackCount1C = 0; other.header.tracks20 = 0;
    other.header.eventCount24 = 0; other.header.events28 = 0;
}
FengScript &FengScript::operator=(FengScript &&other) noexcept {
    if (this != &other) {
        FEngDestructScript(&header);
        header = other.header;
        tracks = std::move(other.tracks);
        originalKeys = std::move(other.originalKeys);
        other.header.name2C = 0;
        other.header.trackCount1C = 0; other.header.tracks20 = 0;
        other.header.eventCount24 = 0; other.header.events28 = 0;
    }
    return *this;
}

FengPackage::FengPackage() : responsePackage_() {}

FengPackage::~FengPackage() {
    for (size_t i = 0; i < objects_.size(); ++i) delete objects_[i];
}

bool FengPackage::Load(const unsigned char *data, size_t size, size_t offset, std::string *error, bool retainNames) {
    if (offset + 16 > size || Rd32(data + offset) != 0x30203) {
        *error = "not an FNG chunk";
        return false;
    }
    size_t end = offset + 8 + Rd32(data + offset + 4);
    std::vector<unsigned int> resourceHashes;
    // Package: 'FEn\xe7' holds PkHd, TypS, Res\xcc (RsNm, RsRq) and Obj\xcc (Butn, FOb\xea...).
    size_t p = offset + 8;
    while (p + 8 <= end) {
        unsigned int id = Rd32(data + p);
        size_t chunkEnd = p + 8 + Rd32(data + p + 4);
        if (id == 0xe76e4546) {
            std::vector<unsigned char> names;
            std::vector<unsigned char> requests;
            size_t q = p + 8;
            while (q + 8 <= chunkEnd) {
                unsigned int id2 = Rd32(data + q);
                size_t end2 = q + 8 + Rd32(data + q + 4);
                if (id2 == 0xcc736552) {
                    size_t r = q + 8;
                    while (r + 8 <= end2) {
                        unsigned int id3 = Rd32(data + r);
                        size_t end3 = r + 8 + Rd32(data + r + 4);
                        if (id3 == 0x6d4e7352) names.assign(data + r + 8, data + end3);
                        if (id3 == 0x71527352) requests.assign(data + r + 8, data + end3);
                        r = end3;
                    }
                    if (requests.size() >= 4) {
                        unsigned int count = Rd32(&requests[0]);
                        for (unsigned int i = 0; i < count && 4 + (i + 1) * 24 <= requests.size(); ++i) {
                            unsigned int nameOffset = Rd32(&requests[4 + i * 24 + 4]);
                            std::string name;
                            if (nameOffset < names.size()) name = (const char *)&names[nameOffset];
                            std::string::size_type dot = name.rfind('.');
                            if (dot != std::string::npos) name.resize(dot);
                            resourceHashes.push_back(FengHash(name.c_str()));
                        }
                    }
                } else if (id2 == 0x52676b50) {  // 'PkgR'
                    ReadResponses(data + q + 8, end2 - q - 8, &responses_, &responsePackage_, 0);
                } else if (id2 == 0x67726154) { // Targ: 0x5184e0 reads Tc/Mt, then resolves GUIDs in row order.
                    for (size_t t = q + 8; t + 4 <= end2;) {
                        unsigned int tag = Rd16(data + t), len = Rd16(data + t + 2);
                        if (t + 4 + len > end2) break;
                        if (tag == 0x744d && len >= 4) {
                            NativeAssociation association;
                            association.message = Rd32(data + t + 4);
                            for (unsigned int i = 4; i + 4 <= len; i += 4)
                                association.guids.push_back(Rd32(data + t + 4 + i));
                            nativeAssociations_.push_back(association);
                        }
                        t += 4 + len;
                    }
                } else if (id2 == 0xcc6a624f) {
                    size_t r = q + 8;
                    while (r + 8 <= end2) {
                        unsigned int id3 = Rd32(data + r);
                        size_t end3 = r + 8 + Rd32(data + r + 4);
                        if (id3 == 0xea624f46) {
                            FengObject *object = new FengObject;
                            memset(object->colour, 0, sizeof(object->colour));
                            // Native deterministic defaults only for words the original
                            // base constructor leaves untouched; derived readers are pending.
                            object->type18 = 0;
                            object->resource22 = 0;
                            object->currentScript54 = 0;
                            *(unsigned int *)(object->unidentified58 + 4) = 0;
                            object->childCount60 = 0;
                            object->firstChild64 = 0;
                            // Execute each recovered constructor once; native metadata
                            // requires a larger C++ wrapper than the original allocation.
                            switch (NativeConstructionType(data, r + 8, end3)) {
                            case 1: reinterpret_cast<FEngLeafObject *>(object)->ConstructImage(); break;
                            case 2: object->Construct(); break;
                            case 4: reinterpret_cast<FEListObject *>(object)->Construct(); break;
                            case 5: reinterpret_cast<FEngGroupObject *>(object)->Construct(); break;
                            case 7: reinterpret_cast<FEngLeafObject *>(object)->ConstructType7(); break;
                            case 9: reinterpret_cast<FEngLeafObject *>(object)->ConstructType9(); break;
                            case 10: reinterpret_cast<FEngLeafObject *>(object)->ConstructType10(); break;
                            case 11: reinterpret_cast<FEngType11Object *>(object)->Construct(); break;
                            default: object->ConstructBase(); break; // Type6 needs a native 0xc8 prefix/owner binding.
                            }
                            object->parentGuid = 0;
                            object->resource = -1;
                            object->data2C = reinterpret_cast<float *>(object->colour);
                            object->font = 0;
                            object->fontHash = 0;
                            object->parent = 0;
                            object->imageMask = 0;
                            object->scriptRemainder = 0;
                            object->angle = 0;
                            object->depth = 0;
                            for (int i = 0; i < 3; ++i) object->pivot[i] = object->position[i] = 0, object->size[i] = 1;
                            object->rotation[0] = object->rotation[1] = object->rotation[2] = 0;
                            object->rotation[3] = 1;
                            object->uv[0] = object->uv[1] = 0;
                            object->uv[2] = object->uv[3] = 1;
                            std::vector<unsigned char> state;
                            std::vector<std::vector<unsigned char> > scripts;
                            FEngListPropertyReader listReader = {};
                            listReader.objectC = reinterpret_cast<FEListObject *>(object);
                            listReader.value24 = listReader.value28 = listReader.value2C = 0xffffffff;
                            size_t s = r + 8;
                            while (s + 8 <= end3) {
                                unsigned int id4 = Rd32(data + s);
                                size_t end4 = s + 8 + Rd32(data + s + 4);
                                if (id4 == 0x446a624f) {  // 'ObjD'
                                    size_t t = s + 8;
                                    while (t + 4 <= end4) {
                                        unsigned int tag = Rd16(data + t);
                                        unsigned int len = Rd16(data + t + 2);
                                        const unsigned char *pl = data + t + 4;
                                        if (len > end4 - t - 4) break; // Native container bounds.
                                        if (object->type18 == 2 && (tag == 0x4853 || tag == 0x4c53 || tag == 0x6253 ||
                                            tag == 0x6a53 || tag == 0x6c53 || tag == 0x7453 || tag == 0x7753)) {
                                            FEngStringPropertyReader reader = {};
                                            reader.objectC = object;
                                            // Native option binds the observed optional-name byte at reader+9.
                                            reader.value9 = retainNames;
                                            const unsigned short *property = reinterpret_cast<const unsigned short *>(data + t);
                                            std::vector<unsigned short> bounded;
                                            if (tag == 0x7453) {
                                                // Native bounded input seam: preserve valid St code units,
                                                // terminate truncated input before the original unbounded copy.
                                                bounded.push_back((unsigned short)tag);
                                                bounded.push_back((unsigned short)len);
                                                for (unsigned int i = 0; i + 1 < len; i += 2) {
                                                    unsigned short c = (unsigned short)Rd16(pl + i);
                                                    if (!c) break;
                                                    bounded.push_back(c);
                                                }
                                                bounded.push_back(0);
                                                property = bounded.data();
                                            } else if (tag == 0x4c53 && retainNames) {
                                                bounded.push_back((unsigned short)tag);
                                                bounded.push_back((unsigned short)len);
                                                for (unsigned int i = 0; i < len && pl[i]; ++i) {
                                                    if (!(i & 1)) bounded.push_back(pl[i]);
                                                    else bounded.back() |= (unsigned short)pl[i] << 8;
                                                }
                                                // Two zero bytes also terminate an odd-length byte string.
                                                bounded.push_back(0);
                                                property = bounded.data();
                                            } else if (tag != 0x4c53 && len < 4) {
                                                t += 4 + len;
                                                continue;
                                            }
                                            reader.ReadProperty(property);
                                            if (tag == 0x4853 || (tag == 0x4c53 && retainNames)) object->font = object->languageHash60;
                                            if (tag == 0x7453) {
                                                object->text.clear();
                                                for (const unsigned short *c = object->text64; c && *c; ++c)
                                                    object->text += (char)(*c < 256 ? *c : '?');
                                            }
                                            t += 4 + len;
                                            continue;
                                        }
                                        if (object->type18 == 4) {
                                            unsigned int required = 0;
                                            switch (tag) {
                                            case 0x5443: case 0x6343: case 0x7243: case 0x774c: required = 4; break;
                                            case 0x634c: case 0x644c: case 0x724c: case 0x7343:
                                            case 0x734c: case 0x764c: required = 8; break;
                                            case 0x6943: required = 16; break;
                                            case 0x7443: required = 2; break;
                                            }
                                            if (required) {
                                                FEListObject *list = listReader.objectC;
                                                bool valid = len >= required;
                                                if (tag == 0x634c) valid = valid && listReader.value24 + 1 < list->count60;
                                                if (tag == 0x724c) valid = valid && listReader.value28 + 1 < list->count64;
                                                if (tag == 0x5443 || tag == 0x6343 || tag == 0x6943 ||
                                                    tag == 0x7243 || tag == 0x7343 || tag == 0x7443)
                                                    valid = valid && list->value90 && list->value88 < list->count60 && list->value8C < list->count64;
                                                if (valid) {
                                                    const unsigned short *property = reinterpret_cast<const unsigned short *>(data + t);
                                                    std::vector<unsigned short> bounded;
                                                    if (tag == 0x7443) {
                                                        bounded.push_back((unsigned short)tag); bounded.push_back((unsigned short)len);
                                                        for (unsigned int i = 0; i + 1 < len; i += 2) {
                                                            unsigned short c = (unsigned short)Rd16(pl + i);
                                                            if (!c) break;
                                                            bounded.push_back(c);
                                                        }
                                                        bounded.push_back(0); property = bounded.data();
                                                    }
                                                    // Bounds/termination and heap lifetime are native seams.
                                                    NativeFengAllocationScope heapOwner(&object->ownedAllocations);
                                                    listReader.ReadProperty(property);
                                                }
                                                t += 4 + len;
                                                continue;
                                            }
                                        }
                                        switch (tag) {
                                        case 0x744f:
                                            object->type18 = (int)Rd32(pl); // 'Ot'
                                            break;
                                        case 0x684f: object->hash10 = Rd32(pl); break;       // 'Oh'
                                        case 0x504f:                                       // 'OP'
                                            object->guidC = Rd32(pl);
                                            object->flags1C = Rd32(pl + 8);
                                            object->resource = (short)Rd32(pl + 12);
                                            break;
                                        case 0x4150: object->parentGuid = Rd32(pl); break;  // 'PA'
                                        case 0x4153: state.assign(pl, pl + len); break;     // 'SA'
                                        case 0x6e4f: // On: original reader+9 gates actual owned name setter.
                                            if (retainNames) {
                                                std::string bounded;
                                                for (unsigned int i = 0; i < len && pl[i]; ++i) bounded += (char)pl[i];
                                                NativeFengAllocationScope heapOwner(&object->ownedAllocations);
                                                object->SetName(bounded.c_str());
                                            }
                                            break;
                                        }
                                        t += 4 + len;
                                    }
                                } else if (id4 == 0x5267734d) {  // 'MsgR'
                                    ReadResponses(data + s + 8, end4 - s - 8, &object->responses, &responsePackage_, object);
                                } else if (id4 == 0x70726353) {  // 'Scrp'
                                    scripts.push_back(std::vector<unsigned char>(data + s + 8, data + end4));
                                }
                                s = end4;
                            }
                            if (object->type18 == 4) {
                                FEListObject *list = reinterpret_cast<FEListObject *>(object);
                                list->CalculateOffsets(); // 0x5554a0 calls this before script initialization.
                            }
                            // Run the INIT script's tracks over the state block (0x541f60 at time 0).
                            for (size_t si = 0; si < scripts.size(); ++si) {
                                const std::vector<unsigned char> &sc = scripts[si];
                                object->scripts.emplace_back();
                                FengScript &script = object->scripts.back();
                                std::vector<FengTrack> &tracks = script.tracks;
                                int nextState = 0;  // 0x5515e0: FI advances by value size / 4
                                size_t t = 0;
                                while (t + 4 <= sc.size()) {
                                    unsigned int tag = Rd16(&sc[t]);
                                    unsigned int len = Rd16(&sc[t + 2]);
                                    const unsigned char *pl = &sc[t + 4];
                                    if (len > sc.size() - t - 4) break;
                                    if (tag == 0x6e53 && retainNames) { // Sn; original script reader uses byte +0xa.
                                        std::string bounded;
                                        for (unsigned int i = 0; i < len && pl[i]; ++i) bounded += (char)pl[i];
                                        script.header.SetName(bounded.c_str());
                                    } else if (tag == 0x6853) {
                                        if (len >= 4) script.header.hash30 = Rd32(pl);
                                        if (len >= 8) script.header.lengthC = (int)Rd32(pl + 4);
                                    } else if (tag == 0x4946) {            // 'FI'
                                        FengTrack track;
                                        track.type = pl[0];
                                        track.size = pl[1];
                                        track.interpolation = pl[2];
                                        track.cycle = pl[3] & 0x7f;
                                        track.flags3 = pl[3];
                                        track.state7 = (signed char)nextState;
                                        track.duration = (int)(Rd32(pl + 4) & 0xffffff);
                                        track.offset = (signed char)nextState;
                                        nextState += track.size >> 2;
                                        tracks.push_back(track);
                                    } else if (tag == 0x6f54 && !tracks.empty()) {  // 'To'
                                        tracks.back().offset = (signed char)pl[0];
                                        tracks.back().state7 = (signed char)pl[0];
                                    } else if (tag == 0x6254 && !tracks.empty()) {  // 'Tb': inline base, after time dword
                                        FengTrack &track = tracks.back();
                                        if (len >= 4 + track.size) track.baseline.assign(pl + 4, pl + 4 + track.size);
                                    } else if (tag == 0x6154 && !tracks.empty()) {  // 'Ta'
                                        tracks.back().flags3 = pl[0];
                                        tracks.back().cycle = pl[0] & 0x7f;
                                    } else if (tag == 0x7454 && !tracks.empty()) {  // 'Tt'
                                        tracks.back().interpolation = pl[0];
                                    } else if (tag == 0x5645) {  // EV replaces, rather than appends, event storage.
                                        FEngScriptEventStorage *events = reinterpret_cast<FEngScriptEventStorage *>(&script.header.eventCount24);
                                        events->Resize((int)(len / 12));
                                        for (unsigned int k = 0; k < len / 12; ++k) {
                                            events->data4[k].message = Rd32(pl + k * 12);
                                            events->data4[k].target = Rd32(pl + k * 12 + 4);
                                            events->data4[k].time = (int)Rd32(pl + k * 12 + 8);
                                        }
                                    } else if (tag == 0x644b && !tracks.empty()) {  // 'Kd'
                                        FengTrack &track = tracks.back();
                                        unsigned int stride = track.size + 4;
                                        for (unsigned int k = 0; k + stride <= len; k += stride) {
                                            int time = (int)Rd32(pl + k);
                                            if (track.keys.empty()) track.baseline.assign(pl + k + 4, pl + k + stride);
                                            track.keys.push_back(std::make_pair(
                                                time, std::vector<unsigned char>(pl + k + 4, pl + k + stride)));
                                        }
                                    }
                                    t += 4 + len;
                                }
                                if (script.header.hash30 == kInitScript) {
                                    for (size_t k = 0; k < tracks.size(); ++k)
                                        ApplyTrack(tracks[k], state.empty() ? 0 : &state[0], (int)state.size(), 0);
                                }
                            }
                            if (!state.empty()) ReadState(object, &state[0], (int)state.size());
                            if (object->resource >= 0 && object->resource < (int)resourceHashes.size()) {
                                if (object->type18 == 1) object->texture24 = resourceHashes[object->resource];
                                else if (object->type18 == 2) object->texture24 = object->fontHash = resourceHashes[object->resource];
                            }
                            // Rotation about z from the quaternion (screen y down, positive = clockwise).
                            object->angle = 2.0f * atan2f(object->rotation[2], object->rotation[3]) * 180.0f / 3.14159265f;
                            objects_.push_back(object);
                        }
                        r = end3;
                    }
                }
                q = end2;
            }
        }
        p = chunkEnd;
    }
    // Parent links by GUID, in file order.
    for (size_t i = 0; i < objects_.size(); ++i) {
        FengObject *object = objects_[i];
        if (object->parentGuid) {
            for (size_t j = 0; j < objects_.size(); ++j) {
                if (objects_[j]->guidC == object->parentGuid) {
                    object->parent = objects_[j];
                    objects_[j]->children.push_back(object);
                    break;
                }
            }
        }
        if (!object->parent) roots_.push_back(object);
    }
    // Native loader seam: root and child chains retain parsed file order.
    for (size_t i = 0; i < roots_.size(); ++i) {
        roots_[i]->next4 = i + 1 < roots_.size() ? roots_[i + 1] : 0;
        roots_[i]->previous8 = i ? roots_[i - 1] : 0;
    }
    // Build the original FEng child-chain prefix from PA links.
    for (size_t i = 0; i < objects_.size(); ++i) {
        FengObject *object = objects_[i];
        if (object->type18 != 2 && object->type18 != 4) object->childCount60 = (int)object->children.size();
        if (object->type18 != 4 && object->type18 != 2)
            object->firstChild64 = object->children.empty() ? 0 : object->children[0];
        for (size_t j = 0; j < object->children.size(); ++j) {
            object->children[j]->next4 = j + 1 < object->children.size() ? object->children[j + 1] : 0;
            object->children[j]->previous8 = j ? object->children[j - 1] : 0;
        }
        // Native loader seam: original FEScript headers of the loaded scripts, linked in file order.
        for (size_t j = 0; j < object->scripts.size(); ++j) {
            FengScript &script = object->scripts[j];
            script.header.SetTracks(script.tracks.size());
            script.originalKeys.resize(script.tracks.size());
            for (size_t k = 0; k < script.tracks.size(); ++k) {
                const FengTrack &native = script.tracks[k];
                FEScriptTrack &track = static_cast<FEScriptTrack *>(script.header.tracks20)[k];
                // Native reader binding of known input data; remaining parser is a seam.
                memset(track.base18, 0, sizeof(track.base18));
                track.key8.timeC = native.keys.empty() ? 0 : native.keys[0].first;
                track.value0 = native.type;
                track.value1 = native.size;
                track.value2 = native.interpolation;
                track.flags3 = native.flags3;
                *(unsigned int *)track.unidentified4 = (unsigned int)native.duration & 0xffffff;
                track.state7 = native.state7;
                // 0x5515e0 puts the first Kd key inline at track+8; later keys form the delta list.
                std::vector<FengScriptKey *> &keys = script.originalKeys[k];
                keys.resize(native.keys.empty() ? 0 : native.keys.size() - 1);
                for (size_t n = 0; n < keys.size(); ++n) {
                    keys[n] = FEngScriptKeysPool.Acquire();
                    FEngConstructScriptKey(keys[n]);
                    keys[n]->timeC = native.keys[n + 1].first;
                    memset(keys[n]->value10, 0, sizeof(keys[n]->value10));
                    memcpy(keys[n]->value10, &native.keys[n + 1].second[0], native.size < 16 ? native.size : 16);
                }
                for (size_t n = 0; n < keys.size(); ++n) {
                    keys[n]->next4 = n + 1 < keys.size() ? keys[n + 1] : 0;
                    keys[n]->previous8 = n ? keys[n - 1] : 0;
                }
                track.value30 = keys.empty() ? 0 : keys[0];
                *(FengScriptKey **)track.unidentified34 = keys.empty() ? 0 : keys.back();
                if (!native.baseline.empty()) memcpy(track.base18, &native.baseline[0], native.size < 16 ? native.size : 16);
            }
            FEScript &header = script.header;
            header.previous8 = j ? &object->scripts[j - 1].header : 0;
            header.next4 = j + 1 < object->scripts.size() ? &object->scripts[j + 1].header : 0;
        }
        object->scripts4C = object->scripts.empty() ? 0 : &object->scripts[0].header;
        // Native loader seam: original script reader 0x5515e0 selects Init;
        // package reader 0x5554a0 selects it again, initializes tracks and resets time.
        object->currentScript54 = object->FindScriptByHash(kInitScript);
        if (object->currentScript54) {
            object->InitializeScript();
            object->currentScript54->time10 = 0;
        }
    }
    originalAssociations_.resize(nativeAssociations_.size());
    for (size_t i = 0; i < nativeAssociations_.size(); ++i) {
        NativeAssociation &association = nativeAssociations_[i];
        for (size_t j = 0; j < association.guids.size(); ++j)
            association.objects.push_back(FindGuid(association.guids[j]));
        originalAssociations_[i].message0 = association.message;
        originalAssociations_[i].capacity4 = association.objects.size();
        originalAssociations_[i].count8 = association.objects.size();
        originalAssociations_[i].objectsC = association.objects.data();
    }
    return !objects_.empty();
}

void FengPackage::BindMessageData(FEPackage *package) {
    package->responseCount58 = responsePackage_.responseCount58;
    package->responses5C = OriginalResponses();
    package->responseCursor60 = responsePackage_.responseCursor60;
    package->associationCount6C = originalAssociations_.size();
    package->associations70 = originalAssociations_.empty() ? 0 : &originalAssociations_[0];
}

FengObject *FengPackage::FindGuid(unsigned int guid) {
    // Native loading supplies the tree and observed package prefix; the lookup
    // itself uses original visitor order rather than decoded-vector order.
    FEPackage package = {};
    package.firstObject4C = FirstRoot();
    return static_cast<FengObject *>(package.FindObjectByGuid(guid));
}

const FengResponse *FindFengResponse(const std::vector<FengResponse> &responses, unsigned int message) {
    for (size_t i = 0; i < responses.size(); ++i) {
        if (responses[i].original->messageC == message) return &responses[i];
    }
    return 0;
}

FengObject *FengPackage::Find(unsigned int hash) {
    FEPackage package = {};
    package.firstObject4C = FirstRoot();
    return static_cast<FengObject *>(package.FindObject(hash));
}

void FengPackage::Describe() const {
    for (size_t i = 0; i < objects_.size(); ++i) {
        const FengObject *o = objects_[i];
        fprintf(stderr, "#%zu type %d hash %08x guid %x parent %x flags %08x pos %.0f,%.0f,%.1f size %.1f,%.1f tex %08x pivot %.1f,%.1f angle %.1f\n", i,
                o->type18, o->hash10, o->guidC, o->parentGuid, o->flags1C, o->position[0], o->position[1], o->position[2], o->size[0],
                o->size[1], o->texture24, o->pivot[0], o->pivot[1], o->angle);
    }
}

void FengPackage::Hide(FengObject *object) { HudHideObject(object); }

void FengPackage::Show(FengObject *object) { HudShowObject(object); }

void FengPackage::SetRotation(FengObject *object, float degrees) {
    if (!object) return;
    object->angle = degrees;
    float half = degrees * 3.14159265f / 360.0f;
    object->rotation[0] = object->rotation[1] = 0;
    object->rotation[2] = sinf(half);
    object->rotation[3] = cosf(half);
}

void FengPackage::PlayScript(FengObject *object, const char *name, bool restart) {
    FngSetObjectScript(object, name, restart);
}

void FengPackage::AdvanceScripts(float seconds) {
    for (size_t i = 0; i < objects_.size(); ++i) {
        FengObject *object = objects_[i];
        if (!object->currentScript54) continue;
        // FengScript begins with its FEScript header.
        FengScript &script = *reinterpret_cast<FengScript *>(object->currentScript54);
        if (!script.header.time10) object->scriptRemainder = 0; // Native sub-millisecond clock seam.
        unsigned char state[84] = {};
        memcpy(state, object->colour, 16);
        memcpy(state + 16, object->pivot, 12);
        memcpy(state + 28, object->position, 12);
        memcpy(state + 40, object->rotation, 16);
        memcpy(state + 56, object->size, 12);
        memcpy(state + 68, object->uv, 16);
        for (size_t k = 0; k < script.tracks.size(); ++k) {
            FengTrack &track = script.tracks[k];
            if (!track.baseline.empty()) {
                memcpy(&track.baseline[0], static_cast<FEScriptTrack *>(script.header.tracks20)[k].base18, track.size < 16 ? track.size : 16);
            }
            for (size_t n = 0; n < script.originalKeys[k].size(); ++n)
                memcpy(&track.keys[n + 1].second[0], script.originalKeys[k][n]->value10, track.size < 16 ? track.size : 16);
            ApplyTrack(track, state, sizeof(state), script.header.time10);
        }
        ReadState(object, state, sizeof(state));
        float milliseconds = (float)script.header.time10 + object->scriptRemainder + seconds * 1000.0f;
        bool cycling = false;
        for (size_t k = 0; k < script.tracks.size(); ++k) cycling |= script.tracks[k].cycle == 1;
        if (!cycling && milliseconds > script.header.lengthC) milliseconds = (float)script.header.lengthC + 1.0f;
        script.header.time10 = (int)milliseconds;
        object->scriptRemainder = milliseconds - (float)script.header.time10;
        // Native clock adapter; 0x54ce60 ticks type4 after script state application.
        if (object->type18 == 4) reinterpret_cast<FEListObject *>(object)->Tick(seconds * 1000.0f);
    }
}

void FengPackage::SetColour(FengObject *object, unsigned int argb) {
    FEngSetColour(object, argb);
}

void FengPackage::SetTexture(FengObject *object, unsigned int hash) {
    if (object) object->texture24 = hash;
}

void FengPackage::SetPosition(FengObject *object, float x, float y) {
    if (!object) return;
    object->position[0] = x;
    object->position[1] = y;
}

void FengPackage::GetPosition(FengObject *object, float *x, float *y) {
    if (!object) return;
    *x = object->position[0];
    *y = object->position[1];
}

void FengPackage::GetSize(FengObject *object, float *x, float *y) {
    if (!object) return;
    *x = object->size[0];
    *y = object->size[1];
}

void FengPackage::SetState(FengObject *object, int key, const float *value) {
    if (!object) return;
    if (key == 5 || key == 6) {
        int offset = key == 5 ? 0 : 2;
        object->uv[offset] = value[0];
        object->uv[offset + 1] = value[1];
    }
}

void FengPackage::SetText(FengObject *object, const char *text) {
    if (object) {
        object->text = text;
        if (object->type18 == 2) {
            // Renderer/localization byte conversion is scaffolding; allocation,
            // UTF-16 assignment and lifetime use the recovered storage methods.
            std::vector<unsigned short> input;
            for (size_t i = 0; i < object->text.size(); ++i) input.push_back((unsigned char)object->text[i]);
            input.push_back(0);
            reinterpret_cast<FEngTextStorage *>(&object->text64)->Assign(input.data());
        }
    }
}

void FengPackage::Gather(FengObject *object, const float *parent, float parentAlpha, std::vector<DrawItem> *items) {
    if (object->Hidden()) return;
    float a = parent[0], b = parent[1], c = parent[2], d = parent[3], tx = parent[4], ty = parent[5];
    // Rotate about the FEng pivot relative to the object's centre. The north
    // marker's (0, 72) pivot puts its rotation centre at the minimap centre.
    // Positive z angles rotate clockwise in the screen's y-down coordinates.
    // The full quaternion is projected onto the screen plane: UI_Pause mirrors
    // frame corners and bar caps with 180-degree turns about x or y, which a
    // z-only angle drew as 180-degree spins (wrong corners).
    float qx = object->rotation[0], qy = object->rotation[1], qz = object->rotation[2], qw = object->rotation[3];
    float r00 = 1 - 2 * (qy * qy + qz * qz), r01 = 2 * (qx * qy - qz * qw);
    float r10 = 2 * (qx * qy + qz * qw), r11 = 1 - 2 * (qx * qx + qz * qz);
    float pivotX = object->pivot[0], pivotY = object->pivot[1];
    float px = object->position[0] + pivotX - r00 * pivotX - r01 * pivotY;
    float py = object->position[1] + pivotY - r10 * pivotX - r11 * pivotY;
    float m[6];
    m[0] = a * r00 + c * r10;
    m[1] = b * r00 + d * r10;
    m[2] = a * r01 + c * r11;
    m[3] = b * r01 + d * r11;
    m[4] = a * px + c * py + tx;
    m[5] = b * px + d * py + ty;
    std::map<FengObject *, std::pair<float, float> >::const_iterator translation = modTranslations_.find(object);
    if (translation != modTranslations_.end()) {
        m[4] += translation->second.first;
        m[5] += translation->second.second;
    }
    float alpha = parentAlpha * (float)object->colour[3] / 255.0f;
    if (object->type18 == 5) {
        // Groups scale their children by their size.
        float child[6];
        child[0] = m[0] * object->size[0];
        child[1] = m[1] * object->size[0];
        child[2] = m[2] * object->size[1];
        child[3] = m[3] * object->size[1];
        child[4] = m[4];
        child[5] = m[5];
        for (size_t i = 0; i < object->children.size(); ++i) {
            FengObject *kid = object->children[i];
            kid->depth = object->depth + kid->position[2];
            Gather(kid, child, alpha, items);
        }
        return;
    }
    if (alpha <= 0.0f) return;
    DrawItem item;
    item.object = object;
    memcpy(item.matrix, m, sizeof(m));
    item.depth = object->depth;
    item.alpha = alpha;
    items->push_back(item);
}

static bool DepthFar(const std::pair<float, int> &a, const std::pair<float, int> &b) {
    return a.first > b.first;
}

bool FengPackage::Contains(FengObject *object, float x, float y) {
    if (!object) return false;
    std::vector<DrawItem> items;
    const float identity[6] = {1, 0, 0, 1, 0, 0};
    for (size_t i = 0; i < roots_.size(); ++i) Gather(roots_[i], identity, 1, &items);
    for (size_t i = 0; i < items.size(); ++i) {
        const DrawItem &item = items[i];
        if (item.object != object) continue;
        const float *m = item.matrix;
        float det = m[0] * m[3] - m[1] * m[2];
        if (fabsf(det) < 0.00001f) return false;
        float dx = x - m[4], dy = y - m[5];
        float px = (m[3] * dx - m[2] * dy) / det;
        float py = (m[0] * dy - m[1] * dx) / det;
        return fabsf(px) <= fabsf(object->size[0]) * 0.5f &&
               fabsf(py) <= fabsf(object->size[1]) * 0.5f;
    }
    return false;
}

void FengPackage::Draw(NativeTextureCache *textures, NativeFont *font, int width, int height) {
    std::vector<DrawItem> items;
    const float identity[6] = {1, 0, 0, 1, 0, 0};
    for (size_t i = 0; i < roots_.size(); ++i) {
        roots_[i]->depth = roots_[i]->position[2];
        Gather(roots_[i], identity, 1.0f, &items);
    }
    std::vector<std::pair<float, int> > order;
    for (size_t i = 0; i < items.size(); ++i) order.push_back(std::make_pair(items[i].depth, (int)i));
    std::stable_sort(order.begin(), order.end(), DepthFar);

    float scale = height / 480.0f;
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    for (size_t n = 0; n < order.size(); ++n) {
        const DrawItem &item = items[order[n].second];
        const FengObject *o = item.object;
        float R = o->colour[2] / 255.0f, G = o->colour[1] / 255.0f, B = o->colour[0] / 255.0f;
        // virtual -> window: origin at the centre, uniform scale by height
        float ma = item.matrix[0] * scale, mb = item.matrix[1] * scale, mc = item.matrix[2] * scale,
              md = item.matrix[3] * scale;
        float mtx = item.matrix[4] * scale + width * 0.5f, mty = item.matrix[5] * scale + height * 0.5f;
        if (o->type18 == 1) {
            // Original 0x5bcc30 selects SRCALPHA/ONE for mode 2 and
            // DESTALPHA/ONE for mode 6. Ordinary source-alpha blending made
            // the black NOS mask/gamebreaker background an opaque rectangle.
            // Destination-alpha/stencil composition remains a native seam.
            const NativeImage *image = textures->Find(o->texture24);
            int blend = image ? image->alphaBlend : 1;
            if (blend == 2) glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            else if (blend == 5) glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
            else if (blend == 6) glBlendFunc(GL_DST_ALPHA, GL_ONE);
            else glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            bool alpha = false;
            GLuint tex = o->texture24 ? textures->Get(o->texture24, &alpha) : 0;
            // Native render seam: GL textures default to GL_REPEAT, so linear filtering at the
            // edge of a sprite blended in texels from the opposite side (bright seams at the
            // corners of the pause frame and icons). FEng sprites inside [0,1] are clamped.
            bool clamp = o->uv[0] >= 0 && o->uv[1] >= 0 && o->uv[2] >= 0 && o->uv[3] >= 0 &&
                         o->uv[0] <= 1 && o->uv[1] <= 1 && o->uv[2] <= 1 && o->uv[3] <= 1;
            if (tex) {
                glBindTexture(GL_TEXTURE_2D, tex);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);
            } else {
                glDisable(GL_TEXTURE_2D);
                static bool warned[1] = {false};
                if (o->texture24 && getenv("UG2_DEBUG_HUD") && !warned[0]) fprintf(stderr, "HUD: texture %08x of object %08x not loaded\n", o->texture24, o->hash10);
            }
            glColor4f(R, G, B, item.alpha);
            float hw = o->size[0] * 0.5f, hh = o->size[1] * 0.5f;
            float xs[4] = {-hw, hw, hw, -hw}, ys[4] = {-hh, -hh, hh, hh};
            float us[4] = {o->uv[0], o->uv[2], o->uv[2], o->uv[0]};
            float vs[4] = {o->uv[1], o->uv[1], o->uv[3], o->uv[3]};
            bool masked = false;
            if (tex && o->imageMask) {
                for (size_t mi = 0; mi < items.size(); ++mi) {
                    const DrawItem &mask = items[mi];
                    if (mask.object != o->imageMask) continue;
                    GLuint maskTexture = textures->Get(mask.object->texture24, 0);
                    if (!maskTexture) break;
                    // Native render seam: TRACKMAP4000's DXT3 alpha is zero; its
                    // observed TextureInfo blend byte is 5. Sample its RGB through
                    // the retail Minimap_Mask silhouette, without claiming D3D parity.
                    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT);
                    glEnable(GL_STENCIL_TEST);
                    glStencilMask(0xff);
                    glClearStencil(0);
                    glClear(GL_STENCIL_BUFFER_BIT);
                    glStencilFunc(GL_ALWAYS, 1, 0xff);
                    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                    glEnable(GL_ALPHA_TEST);
                    glAlphaFunc(GL_GREATER, 0.01f);
                    glBindTexture(GL_TEXTURE_2D, maskTexture);
                    glBegin(GL_QUADS);
                    for (int k = 0; k < 4; ++k) {
                        float x = (k == 0 || k == 3 ? -0.5f : 0.5f) * mask.object->size[0];
                        float y = (k < 2 ? -0.5f : 0.5f) * mask.object->size[1];
                        glTexCoord2f(mask.object->uv[k == 0 || k == 3 ? 0 : 2],
                                     mask.object->uv[k < 2 ? 1 : 3]);
                        glVertex2f((mask.matrix[0] * x + mask.matrix[2] * y + mask.matrix[4]) * scale + width * 0.5f,
                                   (mask.matrix[1] * x + mask.matrix[3] * y + mask.matrix[5]) * scale + height * 0.5f);
                    }
                    glEnd();
                    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                    glDisable(GL_ALPHA_TEST);
                    glStencilFunc(GL_EQUAL, 1, 0xff);
                    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
                    glBindTexture(GL_TEXTURE_2D, tex);
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
                    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
                    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
                    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PRIMARY_COLOR);
                    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
                    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
                    masked = true;
                    break;
                }
            }
            glBegin(GL_QUADS);
            for (int k = 0; k < 4; ++k) {
                glTexCoord2f(us[k], vs[k]);
                glVertex2f(ma * xs[k] + mc * ys[k] + mtx, mb * xs[k] + md * ys[k] + mty);
            }
            glEnd();
            if (masked) glPopAttrib();
            if (!tex) glEnable(GL_TEXTURE_2D);
        } else if (o->type18 == 2 && font && font->Ready() && !o->text.empty()) {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(R, G, B, item.alpha);
            glPushMatrix();
            float m16[16] = {ma, mb, 0, 0, mc, md, 0, 0, 0, 0, 1, 0, mtx, mty, 0, 1};
            glMultMatrixf(m16);
            font->DrawString(o);
            glPopMatrix();
        }
    }
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

namespace {

// FNTF header (0x30201 chunk data + 0x200): +10 glyph count, +0xc flags (0x40000: 16-byte glyph
// records, else 12), +0x14 glyph table offset, +0x18 kerning table offset, +0x24 line height.
// Glyph records: +0 code, +2 width, +3 height, +4 x, +6 y, +9 x offset, +0xa y offset,
// +0xb kerning pair count, +0xc first kerning pair, +0xe advance.
const unsigned int kFntfMagic = 0x46544e46;

int GlyphStride(const unsigned char *fntf) { return (Rd32(fntf + 0xc) & 0x40000) ? 0x10 : 0xc; }

// 0x6f45b0: binary search of the glyph table.
const unsigned char *SearchGlyph(unsigned int code, const unsigned char *table, unsigned int count, int stride) {
    while (count != 0) {
        const unsigned char *glyph = table + (count >> 1) * stride;
        unsigned int found = Rd16(glyph);
        if (code == found) return glyph;
        if ((int)(code - found) > 0) {
            table = glyph + stride;
            count -= 1;
        }
        count >>= 1;
    }
    return 0;
}

// 0x4fda70: direct index from 0x20, else the search.
const unsigned char *FindGlyph(const unsigned char *fntf, unsigned int code) {
    const unsigned char *table = fntf + Rd32(fntf + 0x14);
    unsigned int count = Rd16(fntf + 10);
    int stride = GlyphStride(fntf);
    int index = (int)(code & 0xffff) - 0x20;
    if (index >= 0 && index < (int)count && Rd16(table + index * stride) == (code & 0xffff))
        return table + index * stride;
    return SearchGlyph(code & 0xffff, table, count, stride);
}

// 0x6f45f0: kerning of `glyph` after `previous`.
int Kerning(const unsigned char *fntf, const unsigned char *glyph, unsigned short previous) {
    unsigned int pairs = glyph[0xb];
    if (pairs == 0) return 0;
    const unsigned char *kerning = fntf + Rd32(fntf + 0x18);
    if ((Rd32(fntf + 0xc) & 0x40000) == 0) {
        int count = (int)Rd32(kerning);
        for (int i = 0; i < count; ++i) {
            const unsigned char *pair = kerning + 4 + i * 4;
            if ((short)Rd16(pair) == (short)previous && pair[3] == Rd16(glyph)) return (signed char)pair[2];
        }
        return 0;
    }
    const unsigned char *pair = kerning + 4 + Rd16(glyph + 0xc) * 4;
    for (unsigned int i = 0; i < pairs; ++i)
        if ((short)Rd16(pair + i * 4) == (short)previous) return (signed char)pair[i * 4 + 2];
    return 0;
}

int GlyphAdvance(const unsigned char *glyph) { return (short)Rd16(glyph + 0xe); }

// Character remapping shared by the measuring and drawing loops.
unsigned short RemapCharacter(unsigned short c) {
    if (c > 0xff7f) c &= 0xff;
    if (c == 0x99) c = 0x2122;
    else if (c == 0x9c) c = 0x153;
    return c;
}

// 0x50ba70: "$name$" tokens are not drawn unless flags 0x820 are set.
bool IsToken(const unsigned short *p, unsigned int flags) {
    if (!p || (flags & 0x820) != 0 || *p != '$') return false;
    int length = 0;
    for (const unsigned short *q = p + 1; *q != 0 && *q != '$'; ++q) ++length;
    return length != 0;
}

// 0x50bac0: skips a token and its closing '$'. Native guard: stop at an unterminated token's end.
const unsigned short *SkipToken(const unsigned short *p, unsigned int flags) {
    if (!p || (flags & 0x820) != 0 || *p != '$') return p;
    ++p;
    if (*p != '$') {
        while (*p != 0 && *p != '$') ++p;
        if (*p != 0) ++p;
    }
    return p;
}

// 0x50bce0: width of the line starting at `p`, stopping before a line break or, with a wrap
// width, at the last space before the width is reached (`wrapped` is then set).
float LineWidth(const unsigned char *fntf, const unsigned short *p, unsigned int flags, unsigned int maxWidth,
                bool *wrapped) {
    if (!p) return 0.0f;
    unsigned short c = *p;
    bool reset = false;
    int count = 0;
    float width = 0.0f, lastSpace = 0.0f;
    if (c == 0) return width;
    for (;;) {
        float before = width;
        if (reset) {
            count = 0;
            reset = false;
        }
        if ((flags & 0x20) == 0 && (c == 10 || c == 0x5e)) return width;
        if (c == 0xd) {
            ++p;
        } else {
            if (c == 0x20) lastSpace = width;
            if ((flags & 0x80) != 0 && c == 0xa0) {
                count = 0;
                c = 0x20;
            }
            if (!IsToken(p, flags)) {
                c = RemapCharacter(c);
                const unsigned char *glyph = FindGlyph(fntf, c);
                if (glyph) {
                    if (count != 0) width += (float)Kerning(fntf, glyph, p[-1]);
                    width += (float)GlyphAdvance(glyph);
                }
                ++p;
            } else {
                p = SkipToken(p, flags);
                reset = true;
            }
            if (maxWidth != 0 && *p != 0 && (float)maxWidth <= width) {
                if ((flags & 0x200) != 0) lastSpace = before;
                if (lastSpace > 0.0f) width = lastSpace;
                if (wrapped) *wrapped = true;
                return width;
            }
        }
        c = *p;
        ++count;
        if (c == 0) return width;
    }
}

// 0x50bf40: height of all lines, `leading` added per line break.
float TextHeight(const unsigned char *fntf, float lineHeight, const unsigned short *p, int leading) {
    FEngFont font;
    font.fntfC = const_cast<unsigned char *>(fntf);
    font.lineHeight24 = lineHeight;
    return font.TextHeight(p, leading);
}

// 0x50c080: number of lines. A line break only counts once a space has been seen on its line.
int LineCount(const unsigned char *fntf, const unsigned short *p, unsigned int flags, unsigned int maxWidth) {
    if (!p) return 0;
    if ((flags & 0x20) != 0) return 1;
    unsigned short c = *p;
    const unsigned short *space = 0;
    bool reset = false;
    float width = 0.0f;
    int lines = 1, count = 0;
    for (;;) {
        if (c == 0) return lines;
        bool newLine = false;
        if ((c == 10 || c == 0x5e) && space) {
            p = space + 1;
            newLine = true;
        } else if (c == 0xd) {
            ++p;
        } else {
            if (reset) {
                count = 0;
                reset = false;
            }
            if (c == 0x20) space = p;
            if (!IsToken(p, flags)) {
                c = RemapCharacter(c);
                const unsigned char *glyph = FindGlyph(fntf, c);
                if (glyph) {
                    if (count != 0) width += (float)Kerning(fntf, glyph, p[-1]);
                    width += (float)GlyphAdvance(glyph);
                }
                ++p;
            } else {
                p = SkipToken(p, flags);
                reset = true;
            }
            if (maxWidth != 0 && *p != 0 && (float)maxWidth <= width && space) {
                p = space;
                while (*p == 0x20) ++p;
                newLine = true;
            }
        }
        if (newLine) {
            ++lines;
            width = 0.0f;
            space = 0;
            reset = true;
        }
        c = *p;
        ++count;
    }
}

// Horizontal start of a line for 'Sj' bits 0 (centre) and 1 (right).
float LineStart(int justification, float width) {
    if (justification & 1) return width * -0.5f;
    if (justification & 2) return -width;
    return 0.0f;
}

// 0x7f918c: per-font vertical offset and leading scale, keyed by name hash.
const struct { unsigned int hash; float offsetY, leadingScale; } kFontAdjustments[] = {
    {0xdca5485a, 18.0f, 2.0f},
    {0x833a8678, 21.5f, 2.0f},
    {0xf88a75f9, 18.0f, 1.0f},
    {0x71c777d7, 23.0f, 1.0f},
};
// 0x7f9184: substitute fonts, tried when a font is not loaded.
const unsigned int kFontSubstitutes[][2] = {
    {0x9583aa1a, 0x5b9d88b9},  // Font_ConduitMdITCTT38bi -> ConduitMdITC_TT21i
};

}  // namespace

// Process-lifetime lookup storage survives global font destruction ordering.
static std::vector<NativeFont *> *coordinateFonts = new std::vector<NativeFont *>;

NativeFont::NativeFont() : textures_(0), texture_(0), bakedSize_(48.0f) { memset(glyphs_, 0, sizeof(glyphs_)); }
NativeFont::~NativeFont() {
    coordinateFonts->erase(std::remove(coordinateFonts->begin(), coordinateFonts->end(), this), coordinateFonts->end());
}

// Platform seam for the 0x30201 handler 0x52cd00 and constructor 0x51bdc0: font name at +0,
// texture name at +0x100, FNTF at +0x200. Language-suffix stripping of the name is not needed for
// the English data loaded here.
void NativeFont::AddFonts(const std::vector<unsigned char> &bundle) {
    for (size_t offset = 0; offset + 8 <= bundle.size();) {
        unsigned int id = Rd32(&bundle[offset]);
        unsigned int length = Rd32(&bundle[offset + 4]);
        if (length > bundle.size() - offset - 8) break;
        const unsigned char *data = &bundle[offset + 8];
        if (id == 0x30201 && length > 0x228 && Rd32(data + 0x200) == kFntfMagic) {
            std::string name((const char *)data, strnlen((const char *)data, 0x100));
            std::string texture((const char *)data + 0x100, strnlen((const char *)data + 0x100, 0x100));
            FengFont font;
            font.hash = FengHash(name.c_str());
            font.textureHash = FengHash(texture.c_str());
            font.fntf.assign(data + 0x200, data + length);
            const unsigned char *fntf = &font.fntf[0];
            size_t glyphEnd = Rd32(fntf + 0x14) + (size_t)Rd16(fntf + 10) * GlyphStride(fntf);
            if (glyphEnd > font.fntf.size() || Rd32(fntf + 0x18) + 4 > font.fntf.size()) {
                offset += 8 + length;
                continue;
            }
            font.lineHeight24 = (float)(int)Rd32(fntf + 0x24);
            font.offset28 = 0.0f;
            font.leadingScale2C = 1.0f;
            for (size_t i = 0; i < sizeof(kFontAdjustments) / sizeof(kFontAdjustments[0]); ++i) {
                if (kFontAdjustments[i].hash == font.hash) {
                    font.offset28 = kFontAdjustments[i].offsetY;
                    font.leadingScale2C = kFontAdjustments[i].leadingScale;
                }
            }
            fonts_.push_back(font);
        }
        offset += 8 + length;
    }
}

const NativeFont::FengFont *NativeFont::Find(unsigned int hash) const {
    for (int guard = 0; guard < 4; ++guard) {
        for (size_t i = 0; i < fonts_.size(); ++i)
            if (fonts_[i].hash == hash) return &fonts_[i];
        size_t i = 0, n = sizeof(kFontSubstitutes) / sizeof(kFontSubstitutes[0]);
        while (i < n && kFontSubstitutes[i][0] != hash) ++i;
        if (i == n) return 0;
        hash = kFontSubstitutes[i][1];
    }
    return 0;
}

// Native resource lookup and line measurement boundaries for recovered coordinate logic.
FEngFont *NativeFont::FindOriginalFont(unsigned int hash) const {
    FengFont *font = const_cast<FengFont *>(Find(hash));
    if (font) font->fntfC = &font->fntf[0];
    return font;
}
FEngFont *FindFEngFont(unsigned int hash) {
    for (size_t i = 0; i < coordinateFonts->size(); ++i) {
        if (FEngFont *font = (*coordinateFonts)[i]->FindOriginalFont(hash)) return font;
    }
    return 0;
}
float FEngFont::LineWidth(const unsigned short *text, unsigned int flags, unsigned int maxWidth, unsigned char *wrapped) {
    bool nativeWrapped = false;
    float width = ::LineWidth(fntfC, text, flags, maxWidth, wrapped ? &nativeWrapped : 0);
    if (wrapped) *wrapped = nativeWrapped;
    return width;
}

static bool LoadTrueType(const char *explicitPath, GLuint *texture, float bakedSize, NativeFont::Glyph *glyphs);

bool NativeFont::Load(const char *gameDirectory, NativeTextureCache *textures, const char *explicitPath) {
    textures_ = textures;
    if (std::find(coordinateFonts->begin(), coordinateFonts->end(), this) == coordinateFonts->end()) coordinateFonts->push_back(this);
    std::string base = gameDirectory ? gameDirectory : ".";
    const char *bundles[][2] = {
        {"/GLOBAL/GLOBALA.BUN", 0},
        {"/LANGUAGES/English.bin", "/LANGUAGES/LanguageTextures.bin"},
    };
    if (!explicitPath) {
        for (size_t i = 0; i < sizeof(bundles) / sizeof(bundles[0]); ++i) {
            std::vector<unsigned char> bundle;
            if (!ReadNativeFile(base + bundles[i][0], &bundle) || bundle.empty()) continue;
            AddFonts(bundle);
            NativeImageMap images;
            if (bundles[i][1]) {
                std::vector<unsigned char> pack;
                if (ReadNativeFile(base + bundles[i][1], &pack) && !pack.empty())
                    LoadNativeTextures(&pack[0], pack.size(), &images);
            } else {
                LoadNativeTextures(&bundle[0], bundle.size(), &images);
            }
            if (textures) textures->Add(images);
        }
    }
    // The TrueType face draws strings whose font is missing, or all strings with `--hud-font`.
    bool trueType = LoadTrueType(explicitPath, &texture_, bakedSize_, glyphs_);
    return Ready() || trueType;
}

void NativeFont::DrawString(const FengObject *object) {
    const FengFont *font = Find(object->fontHash);
    if (!font) {
        if (texture_) DrawTrueType(object->text, object->size[0], object->justification6C);
        return;
    }
    // Native seam: the string's size scales its FEng units like the retail object matrix.
    glPushMatrix();
    glScalef(object->size[0], object->size[1], 1.0f);
    DrawRetail(*font, object);
    glPopMatrix();
}

// 0x51c110 layout with 0x51bfc0 glyph quads, in the string's local FEng units.
void NativeFont::DrawRetail(const FengFont &font, const FengObject *object) {
    const NativeImage *image = textures_ ? textures_->Find(font.textureHash) : 0;
    GLuint texture = textures_ ? textures_->Get(font.textureHash, 0) : 0;
    if (!image || !texture || image->width <= 0 || image->height <= 0) return;
    std::vector<unsigned short> text;
    for (size_t i = 0; i < object->text.size(); ++i) text.push_back((unsigned char)object->text[i]);
    text.push_back(0);
    const unsigned char *fntf = &font.fntf[0];
    const unsigned short *p = &text[0];
    int justification = object->justification6C;
    unsigned int flags = object->flags1C;
    unsigned int maxWidth = object->maxWidth74;
    int leading = (int)((float)object->leading70 * font.leadingScale2C);
    float height;
    if ((justification & 5) == 0) height = TextHeight(fntf, font.lineHeight24, p, leading);
    else height = ((float)object->leading70 * font.leadingScale2C + font.lineHeight24) *
                  (float)LineCount(fntf, p, flags, maxWidth);
    float y = 0.0f;
    if (justification & 4) y = height * -0.5f;
    else if (justification & 8) y = -height;
    float lineStart = LineStart(justification, LineWidth(fntf, p, flags, maxWidth, 0));
    float pen = lineStart;
    bool wrapped = false;
    float lineWidth = LineWidth(fntf, p, flags, maxWidth, &wrapped);
    float textureWidth = (float)image->width, textureHeight = (float)image->height;
    bool reset = false;
    int count = 0;
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
    unsigned short c = *p;
    while (c != 0) {
        if (reset) {
            count = 0;
            reset = false;
        }
        bool newLine = false;
        if ((flags & 0x20) == 0 && (c == 10 || c == 0x5e)) {
            ++p;
            if (*p == 0) break;
            lineWidth = LineWidth(fntf, p, flags, maxWidth, &wrapped);
            lineStart = LineStart(justification, lineWidth);
            newLine = true;
        } else {
            if (IsToken(p, flags)) {
                p = SkipToken(p, flags);
                reset = true;
            } else {
                bool kern = true;
                if ((flags & 0x80) != 0 && c == 0xa0) {
                    kern = false;
                    c = 0x20;
                } else {
                    c = RemapCharacter(c);
                }
                const unsigned char *glyph = FindGlyph(fntf, c);
                if (glyph) {
                    if (count != 0 && kern) pen += (float)Kerning(fntf, glyph, p[-1]);
                    // 0x51bfc0: quad at least 4 units wide; UVs take the transparent texel ring
                    // around the glyph. GL seam: a half-texel inset keeps bilinear filtering from
                    // reaching the neighbouring glyphs of the packed texture when scaled up.
                    float x0 = (float)(signed char)glyph[9] + pen;
                    float y0 = (float)(signed char)glyph[10] + font.offset28 + y;
                    float w = glyph[2] < 4 ? 4.0f : (float)glyph[2];
                    float x1 = x0 + w, y1 = (float)glyph[3] + y0;
                    float u0 = ((float)((int)Rd16(glyph + 4) - 1) + 0.5f) / textureWidth;
                    float v0 = ((float)((int)Rd16(glyph + 6) - 1) + 0.5f) / textureHeight;
                    float u1 = ((float)(glyph[2] + 1 + Rd16(glyph + 4)) - 0.5f) / textureWidth;
                    float v1 = ((float)(glyph[3] + 1 + Rd16(glyph + 6)) - 0.5f) / textureHeight;
                    glTexCoord2f(u0, v0); glVertex2f(x0, y0);
                    glTexCoord2f(u1, v0); glVertex2f(x1, y0);
                    glTexCoord2f(u1, v1); glVertex2f(x1, y1);
                    glTexCoord2f(u0, v1); glVertex2f(x0, y1);
                    pen += (float)GlyphAdvance(glyph);
                }
                ++p;
            }
            if (maxWidth != 0 && wrapped && lineWidth <= pen - lineStart) {
                while (*p == 0x20) ++p;
                lineWidth = LineWidth(fntf, p, flags, maxWidth, &wrapped);
                lineStart = LineStart(justification, lineWidth);
                newLine = true;
            }
        }
        if (newLine) {
            y = (float)leading + font.lineHeight24 + y;
            pen = lineStart;
        }
        c = *p;
        ++count;
    }
    glEnd();
}

static bool LoadTrueType(const char *explicitPath, GLuint *texture, float bakedSize, NativeFont::Glyph *glyphs) {
    const char *candidates[] = {
        explicitPath,
        "/usr/share/fonts/truetype/lato/Lato-SemiboldItalic.ttf",
        "/usr/share/fonts/truetype/lato/Lato-BoldItalic.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-BoldOblique.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-BoldItalic.ttf",
        "/usr/share/fonts/TTF/DejaVuSans-BoldOblique.ttf",
    };
    std::vector<unsigned char> file;
    for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); ++i) {
        if (candidates[i] && ReadNativeFile(candidates[i], &file)) break;
        file.clear();
    }
    if (file.empty()) return false;
    const int atlas = 512;
    std::vector<unsigned char> alpha(atlas * atlas);
    stbtt_pack_context context;
    if (!stbtt_PackBegin(&context, &alpha[0], atlas, atlas, 0, 1, 0)) return false;
    stbtt_packedchar packed[96];
    stbtt_PackFontRange(&context, &file[0], 0, bakedSize, 32, 95, packed);
    stbtt_PackEnd(&context);
    for (int i = 0; i < 95; ++i) {
        NativeFont::Glyph &g = glyphs[i];
        g.x0 = packed[i].xoff;
        g.y0 = packed[i].yoff;
        g.x1 = packed[i].xoff2;
        g.y1 = packed[i].yoff2;
        g.u0 = packed[i].x0 / (float)atlas;
        g.v0 = packed[i].y0 / (float)atlas;
        g.u1 = packed[i].x1 / (float)atlas;
        g.v1 = packed[i].y1 / (float)atlas;
        g.advance = packed[i].xadvance;
    }
    std::vector<unsigned char> rgba(atlas * atlas * 4);
    for (int i = 0; i < atlas * atlas; ++i) {
        rgba[i * 4] = rgba[i * 4 + 1] = rgba[i * 4 + 2] = 255;
        rgba[i * 4 + 3] = alpha[i];
    }
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlas, atlas, 0, GL_RGBA, GL_UNSIGNED_BYTE, &rgba[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return true;
}

float NativeFont::Measure(const std::string &text) const {
    float width = 0;
    for (size_t i = 0; i < text.size(); ++i) {
        int c = (unsigned char)text[i] - 32;
        if (c >= 0 && c < 95) width += glyphs_[c].advance;
    }
    return width;
}

void NativeFont::DrawTrueType(const std::string &text, float scale, int justification) {
    const float x = 0, y = 0;
    // The text scale of an FEng string is relative to a ~32 unit glyph height; the baked face is
    // bakedSize_ high, so scale to 32/bakedSize_ per unit.
    float unit = scale * 32.0f / bakedSize_;
    float width = Measure(text) * unit;
    float left = x;
    // 'Sj' flags, read from retail string data: bit 0 centres, bit 1 right-aligns (bits 2/3
    // are the vertical equivalents; the baseline below approximates them).
    if (justification & 1) left = x - width * 0.5f;
    else if (justification & 2) left = x - width;
    glBindTexture(GL_TEXTURE_2D, texture_);
    glBegin(GL_QUADS);
    float pen = left;
    float baseline = y + bakedSize_ * unit * 0.35f;
    for (size_t i = 0; i < text.size(); ++i) {
        int c = (unsigned char)text[i] - 32;
        if (c < 0 || c >= 95) continue;
        const Glyph &g = glyphs_[c];
        glTexCoord2f(g.u0, g.v0); glVertex2f(pen + g.x0 * unit, baseline + g.y0 * unit);
        glTexCoord2f(g.u1, g.v0); glVertex2f(pen + g.x1 * unit, baseline + g.y0 * unit);
        glTexCoord2f(g.u1, g.v1); glVertex2f(pen + g.x1 * unit, baseline + g.y1 * unit);
        glTexCoord2f(g.u0, g.v1); glVertex2f(pen + g.x0 * unit, baseline + g.y1 * unit);
        pen += g.advance * unit;
    }
    glEnd();
}
