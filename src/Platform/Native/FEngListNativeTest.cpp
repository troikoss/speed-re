// Asset-free projection/integration fixture. Its FNG bytes are synthetic test scaffolding.
#include "NativeFng.h"
#include "../../Speed/Indep/Src/Frontend/FEngInheritance.h"
#include "../../Speed/Indep/Src/Frontend/FEngResponseReader.h"
#include "../../Speed/Indep/Src/Frontend/FEngObject.h"
#include "../../Speed/Indep/Src/Frontend/FEngObjectFactory.h"
#include "../../Speed/Indep/Src/Frontend/FEngText.h"
#include "../../Speed/Indep/Src/Frontend/FEngColour.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <algorithm>
#include <utility>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Unexercised script/localization boundaries must not silently satisfy the fixture.
void NativeTrap(const char *) { abort(); }
bool IsOriginalImageMapped() { return false; } // Asset-free fixture owns private vtable pages.
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
    assert(list->value78 && list->value7C && list->value90 && object->ownedAllocations.size() == 3);
    const unsigned int *cells = static_cast<const unsigned int *>(list->value90);
    assert(cells[0] == 0 && cells[12] == 1 && cells[24] == 2 && cells[1] == 0x3f800000 && cells[6] == 0 && cells[7] == 0 && cells[10] == 0x3f800000);
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
    value.clear(); Word(value, 2); Tag(response, 0x434d, value); // MC owns a constructed array.
    value.clear(); Word(value, 1); Tag(response, 0x6952, value); // First command leaves defaults.
    value.clear(); Word(value, 2); Tag(response, 0x6952, value);
    value.clear(); Word(value, 0x12345678); Tag(response, 0x7552, value);
    value.clear(); Word(value, 0x87654321); Tag(response, 0x7452, value);
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
        const FEResponseCommand *commands = object->responses3C->commands14;
        assert(object->responses3C->commandCount10 == 2 && commands[0].type == 1);
        assert(!commands[0].parameter && !commands[0].target);
        assert(commands[1].type == 2 && commands[1].parameter == 0x12345678 && commands[1].target == 0x87654321);
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
            assert(list->value78 && list->value7C && list->value90 && object->ownedAllocations.size() == 3);
            assert(std::find(object->ownedAllocations.begin(), object->ownedAllocations.end(), list->value78) != object->ownedAllocations.end());
            assert(std::find(object->ownedAllocations.begin(), object->ownedAllocations.end(), list->value7C) != object->ownedAllocations.end());
            assert(std::find(object->ownedAllocations.begin(), object->ownedAllocations.end(), list->value90) != object->ownedAllocations.end());
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
    assert(object->ownedAllocations.size() == 2);
    assert(std::find(object->ownedAllocations.begin(), object->ownedAllocations.end(), list->value7C) != object->ownedAllocations.end());
}
static void RunCellPayloads() {
    FEngColour colour;
    assert(colour.Unpack(0xffaabbcc) == &colour && colour.Pack() == 0xffaabbcc);
    std::vector<unsigned char> data, value;
    Word(value, 4); Tag(data, 0x744f, value); value.clear();
    Word(value, 3); Word(value, 1); Tag(data, 0x644c, value); value.clear();
    const unsigned int colours[] = {0xff102030, 0x7fabcdef, 0x11223344};
    for (unsigned int i = 0; i < 3; ++i) {
        Word(value, colours[i]); Tag(data, 0x6343, value); value.clear();
        Word(value, i ? 2 : 1); Tag(data, 0x5443, value); value.clear();
        if (!i) {
            Word(value, 0x11223344); Word(value, 0x55667788); Word(value, 0x3f800000); Word(value, 0x40000000);
            Tag(data, 0x6943, value); value.clear();
        } else {
            const unsigned short text[] = {0x3a9, 0xd83d, 0xde00, 0};
            for (unsigned int j = 0; j < 4; ++j) { value.push_back(text[j]); value.push_back(text[j] >> 8); }
            Tag(data, 0x7443, value); value.clear();
            if (i == 1) { Word(value, 1); Tag(data, 0x5443, value); value.clear(); }
        }
    }
    Word(value, 0x7f800001); Word(value, 0x80000000); Tag(data, 0x7343, value); value.clear();
    Word(value, 0x80000000); Tag(data, 0x7243, value);
    data = Chunk(0x30203, Chunk(0xe76e4546, Chunk(0xcc6a624f,
        Chunk(0xea624f46, Chunk(0x446a624f, data)))));
    FengPackage package; std::string error;
    assert(package.Load(data.data(), data.size(), 0, &error));
    FengObject *object = package.Objects()[0];
    FEListObject *list = reinterpret_cast<FEListObject *>(object);
    const unsigned int *cells = static_cast<const unsigned int *>(list->value90);
    assert(list->value88 == 2 && list->value8C == 0 && object->ownedAllocations.size() == 4);
    for (unsigned int i = 0; i < 3; ++i) assert(cells[i * 12] == colours[i]);
    assert(cells[6] == 1 && cells[8] == 0x11223344 && cells[9] == 0x55667788 && cells[11] == 0x40000000);
    assert(cells[12 + 6] == 1 && !cells[12 + 8]); // CT frees the previous owned text.
    assert(cells[24 + 6] == 2 && cells[24 + 1] == 0x7f800001 && cells[24 + 2] == 0x80000000);
    assert(!cells[24 + 3] && !cells[24 + 4] && cells[24 + 5] == 0x80000000);
    const unsigned short *text = reinterpret_cast<const unsigned short *>(cells[24 + 8]);
    assert(text[0] == 0x3a9 && text[1] == 0xd83d && text[2] == 0xde00 && !text[3]);
}
static void RunRetainedNames() {
    std::vector<unsigned char> data, value;
    Word(value, 2); Tag(data, 0x744f, value); value.clear();
    Word(value, 0x12345678); Tag(data, 0x684f, value); value.clear();
    const char *names[] = {"previous", "CurrentName"};
    for (unsigned int i = 0; i < 2; ++i) {
        value.assign(names[i], names[i] + strlen(names[i])); // Native termination seam.
        Tag(data, 0x6e4f, value);
    }
    const char label[] = "LabelName";
    value.assign(label, label + strlen(label)); Tag(data, 0x4c53, value);
    data = Chunk(0x30203, Chunk(0xe76e4546, Chunk(0xcc6a624f,
        Chunk(0xea624f46, Chunk(0x446a624f, data)))));
    for (unsigned int enabled = 0; enabled < 2; ++enabled) {
        FengPackage package; std::string error;
        assert(package.Load(data.data(), data.size(), 0, &error, enabled != 0));
        FengObject *object = package.Objects()[0];
        const char *storedLabel = *reinterpret_cast<char **>(reinterpret_cast<char *>(object) + 0x5c);
        if (enabled) {
            assert(!strcmp(object->name14, names[1]) && object->hash10 == FngHash(names[1]));
            assert(storedLabel && !strcmp(storedLabel, label));
            assert(object->languageHash60 == FngHash(label) && object->font == FngHash(label));
            assert(object->ownedAllocations.size() == 1 && object->ownedAllocations[0] == object->name14);
        } else {
            assert(!object->name14 && object->hash10 == 0x12345678 && !storedLabel);
            assert(object->ownedAllocations.empty());
        }
    }
}
static void RunResponseStorage() {
    std::vector<unsigned char> data, value, response;
    Word(value, 1); Tag(data, 0x744f, value); value.clear();
    Word(value, 7); Tag(response, 0x694d, value); value.clear();
    Word(value, 3); Tag(response, 0x434d, value); value.clear();
    Word(value, 0x200); Tag(response, 0x6952, value); value.clear();
    const char *strings[] = {"old", "retained", "high\x80\xff"};
    for (unsigned int i = 0; i < 2; ++i) {
        value.assign(strings[i], strings[i] + strlen(strings[i]));
        Tag(response, 0x7352, value);
    }
    value.clear(); Word(value, 0xabcdef01); Tag(response, 0x7452, value); value.clear();
    Word(value, 0x204); Tag(response, 0x6952, value);
    value.assign(strings[2], strings[2] + strlen(strings[2])); Tag(response, 0x7352, value);
    value.clear(); Word(value, 0x203); Tag(response, 0x6952, value); value.clear();
    Word(value, 0x12345678); Tag(response, 0x7552, value); value.clear();
    // Another response forces native header-vector relocation, preserving owned arrays.
    Word(value, 8); Tag(response, 0x694d, value); value.clear();
    Word(value, 0); Tag(response, 0x434d, value);
    std::vector<unsigned char> object = Chunk(0x446a624f, data);
    std::vector<unsigned char> responseChunk = Chunk(0x5267734d, response);
    object.insert(object.end(), responseChunk.begin(), responseChunk.end());
    std::vector<unsigned char> packageData = Chunk(0x52676b50, response);
    std::vector<unsigned char> objectChunk = Chunk(0xcc6a624f, Chunk(0xea624f46, object));
    packageData.insert(packageData.end(), objectChunk.begin(), objectChunk.end());
    data = Chunk(0x30203, Chunk(0xe76e4546, packageData));
    FengPackage package; std::string error;
    assert(package.Load(data.data(), data.size(), 0, &error));
    assert(package.Responses().size() == 2 && package.Responses()[0].original->commandCount10 == 3);
    assert(!strcmp((const char *)package.Responses()[0].original->commands14[0].parameter, strings[1]));
    assert(package.Objects()[0]->responses.size() == 2);
    FEResponse *header = package.Objects()[0]->responses3C;
    assert(header->messageC == 7 && header->commandCount10 == 3 && header->next4->messageC == 8);
    assert(!header->next4->commandCount10 && !header->next4->commands14);
    assert(!strcmp((const char *)header->commands14[0].parameter, strings[1]));
    assert(header->commands14[0].target == 0xabcdef01);
    assert(!strcmp((const char *)header->commands14[1].parameter, strings[2]));
    assert(header->commands14[2].type == 0x203 && header->commands14[2].parameter == 0x12345678);
    header->ResizeCommands(1);
    assert(header->commandCount10 == 1 && !strcmp((const char *)header->commands14[0].parameter, strings[1]));
    assert(package.Responses()[0].original->commandCount10 == 3); // Independent package/object arrays.
    FEPackage bound = {}; package.BindMessageData(&bound);
    assert(bound.responseCount58 == 2 && bound.responses5C == package.Responses()[0].original);
    assert(bound.responseCursor60 == package.Responses()[1].original && bound.responseCursor60->previous8 == bound.responses5C);
    FEngResponseReader reader = {};
    reader.objectC = package.Objects()[0]; reader.value18 = 1;
    FEResponse *second = header->next4;
    std::vector<unsigned char> merge;
    value.clear(); Word(value, 7); Tag(merge, 0x694d, value); value.clear();
    Word(value, 2); Tag(merge, 0x434d, value); value.clear();
    Word(value, 0x203); Tag(merge, 0x6952, value); value.clear();
    Word(value, 0x11223344); Tag(merge, 0x7552, value); value.clear();
    Word(value, 8); Tag(merge, 0x694d, value); value.clear();
    Word(value, 1); Tag(merge, 0x434d, value); value.clear();
    // Mi and MC retain the cached command pointer from response7.
    Word(value, 0xcafebabe); Tag(merge, 0x7452, value); value.clear();
    Word(value, 0x201); Tag(merge, 0x6952, value);
    const char merged[] = "merged";
    value.assign(merged, merged + sizeof(merged)); Tag(merge, 0x7352, value);
    assert(reader.Read(merge.data(), merge.size(), 0) == 1);
    assert(reader.objectC->FindResponseByMessage(7) == header && header->next4 == second);
    assert(package.Objects()[0]->responses.size() == 2 && header->commandCount10 == 2);
    assert(header->commands14[0].parameter == 0x11223344 && header->commands14[0].target == 0xcafebabe);
    assert(!header->commands14[1].type && second->commandCount10 == 1);
    assert(!strcmp((const char *)second->commands14[0].parameter, merged));
    assert(second->previous8 == header);
    FengResponse owner;
    owner.original->ResizeCommands(1);
    owner.original->commands14[0].type = 0x201;
    owner.original->commands14[0].SetString("move-owned");
    FEResponseCommand *owned = owner.original->commands14;
    FengResponse moved(std::move(owner));
    assert(!owner.original && moved.original->commands14 == owned);
    FengResponse assigned;
    assigned.original->ResizeCommands(1);
    assigned.original->commands14[0].type = 0x202;
    assigned.original->commands14[0].SetString("release-on-move");
    assigned = std::move(moved);
    assert(!moved.original && assigned.original->commands14 == owned);
    assert(!strcmp((const char *)assigned.original->commands14[0].parameter, "move-owned"));
}
static void RunScriptStorage() {
    std::vector<unsigned char> object, value, first, second;
    Word(value, 1); Tag(object, 0x744f, value);
    const char firstName[] = "retainedScript", secondName[] = "other";
    value.assign(firstName, firstName + strlen(firstName)); Tag(first, 0x6e53, value);
    value.clear(); Word(value, 0x12345678); Word(value, 300); Tag(first, 0x6853, value);
    value.clear();
    for (unsigned int i = 0; i < 2; ++i) { Word(value, i + 1); Word(value, 0xffffffff); Word(value, i + 1); }
    Tag(first, 0x5645, value);
    value.clear(); Word(value, 0x11112222); Word(value, 0xffffffff); Word(value, 17); Tag(first, 0x5645, value);
    value.assign(secondName, secondName + strlen(secondName)); Tag(second, 0x6e53, value);
    value.clear();
    for (unsigned int i = 0; i < 2; ++i) { Word(value, 10 + i); Word(value, 42); Word(value, 30 + i); }
    Tag(second, 0x5645, value);
    object = Chunk(0x446a624f, object);
    first = Chunk(0x70726353, first); second = Chunk(0x70726353, second);
    object.insert(object.end(), first.begin(), first.end()); object.insert(object.end(), second.begin(), second.end());
    object = Chunk(0x30203, Chunk(0xe76e4546, Chunk(0xcc6a624f, Chunk(0xea624f46, object))));
    for (unsigned int enabled = 0; enabled < 2; ++enabled) {
        FengPackage package; std::string error;
        assert(package.Load(object.data(), object.size(), 0, &error, enabled != 0));
        assert(package.Objects()[0]->scripts.size() == 2);
        FEScript &one = package.Objects()[0]->scripts[0].header;
        FEScript &two = package.Objects()[0]->scripts[1].header;
        assert(one.hash30 == 0x12345678 && one.lengthC == 300);
        assert(one.eventCount24 == 1 && two.eventCount24 == 2 && one.events28 != two.events28);
        FEScriptEvent *event = static_cast<FEScriptEvent *>(one.events28);
        assert(event->message == 0x11112222 && event->target == 0xffffffff && event->time == 17);
        assert(one.next4 == &two && two.previous8 == &one);
        if (enabled) {
            assert(!strcmp(one.name2C, firstName) && !strcmp(two.name2C, secondName));
            assert(two.hash30 == FngHash(secondName));
        } else assert(!one.name2C && !two.name2C && !two.hash30);
        FEngScriptEventStorage *storage = reinterpret_cast<FEngScriptEventStorage *>(&one.eventCount24);
        storage->Resize(3);
        assert(storage->data4[0].message == 0x11112222);
        for (unsigned int i = 1; i < 3; ++i)
            assert(!storage->data4[i].message && !storage->data4[i].target && !storage->data4[i].time);
        event = storage->data4; storage->Resize(3); assert(storage->data4 == event);
        storage->Resize(1); assert(storage->data4[0].time == 17);
        storage->Resize(0); assert(!one.events28 && !one.eventCount24 && two.eventCount24 == 2);
    }
    FengScript owner;
    owner.header.SetName("owned");
    reinterpret_cast<FEngScriptEventStorage *>(&owner.header.eventCount24)->Resize(2);
    void *events = owner.header.events28; char *name = owner.header.name2C;
    FengScript moved(std::move(owner));
    assert(!owner.header.name2C && !owner.header.events28 && !owner.header.eventCount24);
    assert(moved.header.name2C == name && moved.header.events28 == events);
    FengScript assigned;
    assigned.header.SetName("release");
    reinterpret_cast<FEngScriptEventStorage *>(&assigned.header.eventCount24)->Resize(1);
    assigned = std::move(moved);
    assert(!moved.header.name2C && !moved.header.events28 && !moved.header.eventCount24);
    assert(assigned.header.name2C == name && assigned.header.events28 == events && assigned.header.eventCount24 == 2);
}
static FEngReference *injectedReference;
static unsigned int keyCallbackCalls;
static void *__thiscall DeleteAndInjectKey(void *pointer, unsigned int flags) {
    FEngScriptKey *key = static_cast<FEngScriptKey *>(pointer);
    assert(flags == 1 && (unsigned int)key->next4 == 0xabadcafe && (unsigned int)key->previous8 == 0xabadcafe);
    key->Destroy(flags);
    ++keyCallbackCalls;
    FEngScriptKey *injected = FEngScriptKeysPool.Acquire();
    FEngConstructScriptKey(injected);
    injected->next4 = static_cast<FEngScriptKey *>(injectedReference->value8);
    if (injected->next4) injected->next4->previous8 = injected;
    injected->previous8 = 0;
    injectedReference->value8 = injected;
    if (!injectedReference->valueC) injectedReference->valueC = injected;
    return pointer;
}
static void RunTrackOwnership() {
    assert(!FEngScriptKeysPool.blocks0.count4);
    {
        std::vector<unsigned char> data, value, script;
        Word(value, 1); Tag(data, 0x744f, value);
        value.clear(); Word(value, 0x2468); Word(value, 100); Tag(script, 0x6853, value);
        value.clear(); value.push_back(4); value.push_back(12); value.push_back(1); value.push_back(0);
        Word(value, 2400); Tag(script, 0x4946, value);
        value.clear();
        const unsigned int times[] = {0xffffffff, 0, 100};
        const unsigned int values[] = {0x11223344, 0x55667788, 0xaabbccdd};
        for (unsigned int i = 0; i < 3; ++i) {
            Word(value, times[i]);
            for (unsigned int j = 0; j < 3; ++j) Word(value, values[i] + j);
        }
        Tag(script, 0x644b, value);
        data = Chunk(0x446a624f, data); script = Chunk(0x70726353, script);
        data.insert(data.end(), script.begin(), script.end());
        data = Chunk(0x30203, Chunk(0xe76e4546, Chunk(0xcc6a624f, Chunk(0xea624f46, data))));
        FengPackage package; std::string error;
        assert(package.Load(data.data(), data.size(), 0, &error));
        FEScript &header = package.Objects()[0]->scripts[0].header;
        assert(header.trackCount1C == 1 && static_cast<unsigned int *>(header.tracks20)[-1] == 1);
        FEScriptTrack &track = static_cast<FEScriptTrack *>(header.tracks20)[0];
        assert(track.key8.vtable0 == (void *)0x79aa34 && track.key8.timeC == -1);
        assert((*(unsigned int *)track.unidentified4 & 0xffffff) == 2400);
        assert(*(unsigned int *)track.base18 == 0x11223344 && !track.value2C);
        FEngScriptKey *first = static_cast<FEngScriptKey *>(track.value30);
        FEngScriptKey *last = *reinterpret_cast<FEngScriptKey **>(track.unidentified34);
        assert(first && last && first->next4 == last && last->previous8 == first);
        assert(first->timeC == 0 && last->timeC == 100);
        assert(*(unsigned int *)first->value10 == 0x55667788 && *(unsigned int *)last->value10 == 0xaabbccdd);
        assert(FEngScriptKeysPool.blocks0.count4 == 1);
        FEScriptTrack borrowed = track;
        borrowed.value2C = 1; borrowed.value30 = track.unidentified28;
        FEngDestructScriptTrack(&borrowed);
        assert(first->vtable0 == (void *)0x79aa34); // Borrowed cleanup leaves the owned list live.
        header.SetTracks(1); // Equal count still destroys old keys and reconstructs the array.
        assert(header.trackCount1C == 1 && !static_cast<FEScriptTrack *>(header.tracks20)[0].value30);
        assert(!FEngScriptKeysPool.blocks0.count4);
    }
    assert(!FEngScriptKeysPool.blocks0.count4);
    FEngReference reference = {};
    FEngScriptKey *first = FEngScriptKeysPool.Acquire(), *second = FEngScriptKeysPool.Acquire();
    FEngConstructScriptKey(first); FEngConstructScriptKey(second);
    first->next4 = second; first->previous8 = 0; second->next4 = 0; second->previous8 = first;
    reference.value8 = first; reference.valueC = second;
    void *slots[] = {reinterpret_cast<void *>(DeleteAndInjectKey)};
    first->vtable0 = slots;
    injectedReference = &reference; keyCallbackCalls = 0;
    reference.ClearOwned(); // Callback inserts a node; recovered code must reload the head.
    injectedReference = 0;
    assert(keyCallbackCalls == 1 && !reference.value8 && !reference.valueC && !FEngScriptKeysPool.blocks0.count4);
    std::vector<FEngScriptKey *> keys;
    for (unsigned int i = 0; i < 270; ++i) {
        FEngScriptKey *key = FEngScriptKeysPool.Acquire(); FEngConstructScriptKey(key);
        keys.push_back(key);
    }
    assert(FEngScriptKeysPool.blocks0.count4 == 2);
    for (unsigned int i = 0; i < keys.size(); ++i) keys[i]->Destroy(1);
    assert(!FEngScriptKeysPool.blocks0.count4 && !FEngScriptKeysPool.blocks0.first8 && !FEngScriptKeysPool.blocks0.valueC);
}
static void RunResponsePool() {
    assert(!FEngResponseHeaderPool.blocks0.count4 && !FEngResponseHeaderPool.blocks0.first8);
    std::vector<FengResponse> responses;
    for (unsigned int i = 0; i < 70; ++i) {
        responses.emplace_back();
        responses.back().original->messageC = i;
        responses.back().original->ResizeCommands(1);
        responses.back().original->commands14[0].type = 0x200;
        responses.back().original->commands14[0].SetString("pooled");
    }
    assert(FEngResponseHeaderPool.blocks0.count4 == 2);
    for (unsigned int i = 0; i < 70; ++i) {
        assert(responses[i].original->messageC == i);
        assert(!strcmp((const char *)responses[i].original->commands14[0].parameter, "pooled"));
    }
    responses.clear();
    assert(!FEngResponseHeaderPool.blocks0.count4 && !FEngResponseHeaderPool.blocks0.first8 && !FEngResponseHeaderPool.blocks0.valueC);
}
static void RunInheritance() {
    FEPackage source = {}, first = {}, second = {};
    FEngQueue queue = {};
    HudVisibilityObject group = {}, child = {}, duplicate = {}, secondChild = {};
    const unsigned char name[] = {0, 0, 'S', 'c', 'r', 'e', 'e', 'n', 0};
    first.value38 = second.value38 = name;
    first.next4 = &second; queue.first104 = &first;
    group.type18 = 5; group.guidC = 1; group.firstChild64 = &child;
    child.guidC = duplicate.guidC = 42;
    group.next4 = &duplicate; first.firstObject4C = &group;
    FEngInheritanceRecord records[] = {{7, FngHash("screen"), 42}, {7, FngHash("other"), 999}};
    source.inheritanceCount84 = 2; source.inheritance88 = records;
    FEngInheritanceContext context = {}; context.package0 = &source; context.queue3C = &queue;
    HudVisibilityObject *object = &duplicate; FEPackage *package = &source;
    assert(source.FindInheritance(7) == records && !source.FindInheritance(99));
    assert(context.Resolve(7, &object, &package) && object == &child && package == &first);
    assert(!context.Resolve(99, &object, &package) && !object && !package);
    records[0].packageHash4 = FngHash("missing");
    assert(!context.Resolve(7, &object, &package) && !object && !package);
    records[0].packageHash4 = FngHash("screen"); records[0].guid8 = 999;
    second.firstObject4C = &secondChild; secondChild.guidC = 999;
    assert(!context.Resolve(7, &object, &package) && !object && package == &first);
    records[0].guid8 = 42;
    void *shared = 0;
    assert(context.Resolve(7, reinterpret_cast<HudVisibilityObject **>(&shared),
                           reinterpret_cast<FEPackage **>(&shared)) && shared == &child);
    child.guidC = reinterpret_cast<unsigned int>(&first);
    assert(context.Resolve(7, &object, reinterpret_cast<FEPackage **>(&records[0].guid8)) && object == &child);
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
    RunCellPayloads();
    RunRetainedNames();
    RunInheritance();
    RunResponseStorage();
    RunResponsePool();
    RunScriptStorage();
    RunTrackOwnership();
    puts("PASS native FNG owned text/names, inheritance/GUID lookup and type4 projection/navigation/ticking (no assets or display)");
}
