// Asset-free projection/integration fixture. Its FNG bytes are synthetic test scaffolding.
#include "NativeFng.h"
#include "../../Speed/Indep/Src/Frontend/FEngObject.h"
#include "../../Speed/Indep/Src/Frontend/FEngObjectFactory.h"
#include "../../Speed/Indep/Src/Frontend/FEngText.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Unexercised script/localization boundaries must not silently satisfy the fixture.
void NativeTrap(const char *) { abort(); }
std::string LookupNativeString(const std::vector<unsigned char> &, unsigned int) { abort(); }

static void Word(std::vector<unsigned char> &bytes, unsigned int value) {
    for (unsigned int i = 0; i < 4; ++i) bytes.push_back((unsigned char)(value >> (i * 8)));
}
static void Float(std::vector<unsigned char> &bytes, float value) {
    unsigned int bits; memcpy(&bits, &value, 4); Word(bytes, bits);
}
static void Tag(std::vector<unsigned char> &bytes, unsigned int tag, const std::vector<unsigned char> &value) {
    bytes.push_back(tag); bytes.push_back(tag >> 8);
    bytes.push_back(value.size()); bytes.push_back(value.size() >> 8);
    bytes.insert(bytes.end(), value.begin(), value.end());
}
static std::vector<unsigned char> Chunk(unsigned int id, const std::vector<unsigned char> &value) {
    std::vector<unsigned char> bytes; Word(bytes, id); Word(bytes, value.size());
    bytes.insert(bytes.end(), value.begin(), value.end()); return bytes;
}
static void Run(unsigned int wrap, bool explicitGuid) {
    std::vector<unsigned char> data, value;
    Word(value, 4); Tag(data, 0x744f, value); value.clear(); // Ot
    Word(value, 0x12345678); Tag(data, 0x684f, value); value.clear(); // Oh
    if (explicitGuid) {
        Word(value, 0); Word(value, 0); Word(value, 0); Word(value, 0xffff);
        Tag(data, 0x504f, value); value.clear(); // OP: explicit GUID0, resource -1.
    }
    Word(value, 3); Word(value, 2); Tag(data, 0x644c, value); value.clear(); // Ld
    for (int i = 0; i < 3; ++i) {
        Float(value, (i + 1) * .25f); Float(value, 10 + i); Tag(data, 0x634c, value); value.clear();
    }
    for (int i = 0; i < 2; ++i) {
        Float(value, i + 1); Float(value, 20 + i); Tag(data, 0x724c, value); value.clear();
    }
    Float(value, .01f); Float(value, .02f); Tag(data, 0x734c, value); value.clear(); // Ls
    Float(value, 0); Float(value, 0); Tag(data, 0x764c, value); value.clear(); // Lv
    Word(value, wrap); Tag(data, 0x774c, value); value.clear(); // Lw
    for (int i = 0; i < 3; ++i) { Word(value, i); Tag(data, 0x6343, value); value.clear(); } // Cc
    std::vector<unsigned char> objectData = Chunk(0x446a624f, data), script;
    Word(value, 0x1744b3); Word(value, 1000); Tag(script, 0x6853, value); value.clear(); // Init, no tracks.
    script = Chunk(0x70726353, script);
    objectData.insert(objectData.end(), script.begin(), script.end());
    data = Chunk(0x30203, Chunk(0xe76e4546, Chunk(0xcc6a624f, Chunk(0xea624f46, objectData))));
    FengPackage package; std::string error;
    const unsigned int generatedGuid = FEngNextObjectGuid;
    assert(package.Load(data.data(), data.size(), 0, &error));
    assert(package.Objects().size() == 1);
    FengObject *object = package.Objects()[0];
    assert(object->type18 == 4 && object->hash10 == 0x12345678 && object->resource == -1);
    assert(object->currentScript54 && object->currentScript54->hash30 == 0x1744b3 && !object->currentScript54->trackCount1C);
    assert(object->parentGuid == 0 && object->parent == 0 && object->children.empty());
    assert(package.Find(0x12345678) == object && package.Find(0x87654321) == 0);
    const unsigned int expectedGuid = explicitGuid ? 0 : generatedGuid;
    assert(object->guidC == expectedGuid && FEngNextObjectGuid == generatedGuid + 1);
    assert(package.FindGuid(expectedGuid) == object && package.FindGuid(expectedGuid + 1) == 0);
    assert(!object->next4 && !object->previous8);
    FEListObject *list = reinterpret_cast<FEListObject *>(object);
    assert(list->count60 == 3 && list->count64 == 2 && list->value88 == 2 && list->value8C == 0);
    assert(list->value78 && list->value7C && list->value90 && object->listAllocations.size() == 3);
    const unsigned int *cells = static_cast<const unsigned int *>(list->value90);
    assert(cells[0] == 0xffffffff && cells[1] == 0x3f800000 && cells[6] == 0 && cells[7] == 0 && cells[10] == 0x3f800000);
    assert(list->value78[0].value4 == 0 && list->value78[1].value4 == .25f && list->value78[2].value4 == .75f);
    assert(list->value7C[0].value4 == 0 && list->value7C[1].value4 == 1);
    assert(list->value78[2].value8 == 12 && list->value7C[1].value8 == 21);
    list->Navigate(1, 0);
    assert(list->value88 == (wrap ? 0u : 2u) && list->value94 == .75f && list->value9C == 1);
    package.AdvanceScripts(.1f); // Native milliseconds adapter following parsed Init.
    assert(!(list->value5C & 0x62) && list->value70 == (wrap ? 0 : .75f));
    assert(object->hash10 == 0x12345678 && object->resource == -1 && object->parentGuid == 0);
}
class StringObserver : public FEngStringObserver {
public:
    unsigned int calls;
    StringObserver() : calls(0) {}
    void StringChanged(HudVisibilityObject *object) {
        assert(object->type18 == 2 && object->languageHash60 == 0x12345678);
        assert((object->flags1C & 0x400002) == 0x400002);
        ++calls;
    }
};
class StringTeardownObserver : public FEngDestructionObserver {
public:
    unsigned int calls;
    StringTeardownObserver() : calls(0) {}
    void ObjectDestroyed(HudVisibilityObject *value) {
        FengObject *object = static_cast<FengObject *>(value);
        assert(*(unsigned int *)object->unidentified0 == 0x79b8e0);
        assert(!object->data2C && !object->scripts4C && !object->responses3C);
        assert(object->scripts.size() == 1 && object->responses.size() == 1);
        ++calls;
    }
};
static void RunString() {
    std::vector<unsigned char> data, value;
    Word(value, 2); Tag(data, 0x744f, value); value.clear(); // Ot
    Word(value, 0); Word(value, 0); Word(value, 2); Word(value, 0xffff);
    Tag(data, 0x504f, value); value.clear(); // OP preserves bit2 through the member setter.
    Word(value, 0x12345678); Tag(data, 0x4853, value); Tag(data, 0x4853, value); value.clear();
    Word(value, 'z'); Tag(data, 0x4c53, value); value.clear(); // SL disabled by native flag9.
    Word(value, 2); Tag(data, 0x6a53, value); value.clear();
    Word(value, 0xfffffffe); Tag(data, 0x6c53, value); value.clear();
    Word(value, 0x80000000); Tag(data, 0x7753, value); value.clear();
    Word(value, 9); Tag(data, 0x6253, value); value.clear();
    Word(value, 'X'); Tag(data, 0x7453, value); value.clear(); // Earlier St must be replaced.
    // Synthetic St payload retains non-ASCII code units in recovered storage.
    const unsigned short text[] = {'A', 0x3a9, 0xd83d, 0xde00, 0};
    for (unsigned int i = 0; i < sizeof(text) / sizeof(text[0]); ++i) {
        value.push_back(text[i]); value.push_back(text[i] >> 8);
    }
    Tag(data, 0x7453, value); value.clear();
    Word(value, 6); Tag(data, 0x6253, value); // Sb reallocates despite capacity9 > request6.
    std::vector<unsigned char> objectData = Chunk(0x446a624f, data), script, response;
    value.clear(); Word(value, 0x1744b3); Word(value, 1000); Tag(script, 0x6853, value);
    script = Chunk(0x70726353, script); objectData.insert(objectData.end(), script.begin(), script.end());
    value.clear(); Word(value, 0x1234); Tag(response, 0x694d, value);
    response = Chunk(0x5267734d, response); objectData.insert(objectData.end(), response.begin(), response.end());
    data = Chunk(0x30203, Chunk(0xe76e4546, Chunk(0xcc6a624f, Chunk(0xea624f46, objectData))));
    StringTeardownObserver teardown;
    FEngObjectDestructionObserver = &teardown;
    StringObserver observer;
    fengStringObserver838478 = &observer;
    for (int repeat = 0; repeat < 3; ++repeat) {
        FengPackage package; std::string error;
        assert(package.Load(data.data(), data.size(), 0, &error));
        FengObject *object = package.Objects()[0];
        FEngTextStorage *storage = reinterpret_cast<FEngTextStorage *>(&object->text64);
        assert(object->type18 == 2 && object->text == "A???");
        assert(object->data2C == reinterpret_cast<float *>(object->colour));
        assert(object->scripts4C && object->responses3C && object->scripts.size() == 1 && object->responses.size() == 1);
        assert(storage->capacity4 == 6 && !memcmp(storage->text0, text, sizeof(text)));
        assert(storage->GetBuffer(6) == object->text64);
        assert(object->languageHash60 == 0x12345678 && object->font == 0x12345678);
        assert(object->justification6C == 2 && object->leading70 == -2 && object->maxWidth74 == 0x80000000);
        assert(!*(unsigned int *)(object->unidentified58 + 4) && (object->flags1C & 2));
        unsigned short *buffer = storage->text0;
        FengPackage::SetText(object, "xy");
        assert(storage->text0 == buffer && storage->capacity4 == 6);
        assert(storage->text0[0] == 'x' && storage->text0[1] == 'y' && !storage->text0[2]);
        FengPackage::SetText(object, "0123456789");
        assert(storage->capacity4 == 11 && storage->text0[9] == '9' && !storage->text0[10]);
        buffer = storage->text0;
        FengPackage::SetText(object, "");
        assert(object->text.empty() && storage->text0 == buffer && !*storage->text0 && storage->capacity4 == 11);
        // Each package destructor releases this owned buffer.
    }
    assert(observer.calls == 6 && teardown.calls == 3);
    fengStringObserver838478 = 0;
    FEngObjectDestructionObserver = 0;
}
class TypedTeardownObserver : public FEngDestructionObserver {
public:
    unsigned int calls;
    TypedTeardownObserver() : calls(0) {}
    void ObjectDestroyed(HudVisibilityObject *value) {
        FengObject *object = static_cast<FengObject *>(value);
        assert(!object->data2C && !object->scripts4C && !object->responses3C);
        if (object->type18 == 4) {
            FEListObject *list = reinterpret_cast<FEListObject *>(object);
            assert(!(list->value5C & 1));
            assert(list->count60 == 3 && list->count64 == 2);
            assert(list->value78 && list->value7C && list->value90 && object->listAllocations.size() == 3);
            assert(std::find(object->listAllocations.begin(), object->listAllocations.end(), list->value78) != object->listAllocations.end());
            assert(std::find(object->listAllocations.begin(), object->listAllocations.end(), list->value7C) != object->listAllocations.end());
            assert(std::find(object->listAllocations.begin(), object->listAllocations.end(), list->value90) != object->listAllocations.end());
            assert(object->scripts.size() == 1);
        } else if (object->type18 == 5) {
            FEngGroupObject *group = reinterpret_cast<FEngGroupObject *>(object);
            assert(!group->children5C.count4 && !group->children5C.first8 && !group->children5C.valueC);
            assert(object->children.size() == 1);
        } else assert(object->type18 == 1);
        ++calls;
    }
};
static void RunHierarchy() {
    std::vector<unsigned char> group, image, value;
    Word(value, 5); Tag(group, 0x744f, value); value.clear();
    Word(value, 1111); Word(value, 0); Word(value, 0); Word(value, 0xffff); Tag(group, 0x504f, value); value.clear();
    Word(value, 1); Tag(image, 0x744f, value); value.clear();
    Word(value, 2222); Word(value, 0); Word(value, 0); Word(value, 0xffff); Tag(image, 0x504f, value); value.clear();
    Word(value, 1111); Tag(image, 0x4150, value);
    group = Chunk(0xea624f46, Chunk(0x446a624f, group));
    image = Chunk(0xea624f46, Chunk(0x446a624f, image));
    group.insert(group.end(), image.begin(), image.end());
    group = Chunk(0x30203, Chunk(0xe76e4546, Chunk(0xcc6a624f, group)));
    FengPackage package; std::string error;
    assert(package.Load(group.data(), group.size(), 0, &error));
    assert(package.Objects().size() == 2);
    assert(package.Objects()[0]->children.size() == 1 && package.Objects()[0]->firstChild64 == package.Objects()[1]);
}
static void RunZeroDimension() {
    std::vector<unsigned char> data, value;
    Word(value, 4); Tag(data, 0x744f, value); value.clear();
    Word(value, 3); Word(value, 2); Tag(data, 0x644c, value); value.clear();
    Word(value, 0); Word(value, 2); Tag(data, 0x644c, value);
    data = Chunk(0x30203, Chunk(0xe76e4546, Chunk(0xcc6a624f,
        Chunk(0xea624f46, Chunk(0x446a624f, data)))));
    FengPackage package; std::string error;
    assert(package.Load(data.data(), data.size(), 0, &error));
    FengObject *object = package.Objects()[0];
    FEListObject *list = reinterpret_cast<FEListObject *>(object);
    assert(!list->count60 && list->count64 == 2 && !list->value78 && list->value7C && !list->value90);
    // Original zero/product behavior can drop the cell field without freeing it.
    // The native heap owner retains the old cells plus the replacement row axis.
    assert(object->listAllocations.size() == 2);
    assert(std::find(object->listAllocations.begin(), object->listAllocations.end(), list->value7C) != object->listAllocations.end());
}
int main() {
    RunString();
    TypedTeardownObserver teardown;
    FEngObjectDestructionObserver = &teardown;
    Run(0, false); Run(1, false); Run(0, true); Run(1, true);
    RunHierarchy();
    assert(teardown.calls == 6);
    FEngObjectDestructionObserver = 0;
    RunZeroDimension();
    puts("PASS native FNG owned UTF-16 storage and type4 projection/offsets/Cc selection/navigation/ticking (no assets or display)");
}
