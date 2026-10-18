// Native ABI probe for the recovered grid body. All cells and strings are synthetic.
#include "../../Speed/Indep/Src/Frontend/FEngGrid.h"
#include "../../Speed/Indep/Src/Frontend/FEngHash.h"
#include "../../Speed/Indep/Src/Frontend/FEngObject.h"
#include "../../Speed/Indep/Src/Frontend/FEngGridReader.h"
#include "../../Speed/Indep/Src/Frontend/FEngLifecycle.h"
#include "../../Speed/Indep/Src/Frontend/FEngObjectFactory.h"
#include "../../Speed/Indep/Src/Frontend/FEngText.h"
#include "../../Speed/Indep/Src/Frontend/FEngResponse.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <vector>

static unsigned int allocationBytes, freeCalls;
unsigned int FEngNextObjectGuid = 1;
unsigned int FEngValue7F7CC4 = 0x518e20;
FEngStatePoolManager FEngObjectStateManager = {};
FEngDestructionObserver *FEngObjectDestructionObserver = 0;
FEngStringObserver *fengStringObserver838478 = 0;
void *FEngGridAllocateCells(unsigned int bytes) { allocationBytes = bytes; return malloc(bytes); }
void FEngGridFreeCells(void *cells) { ++freeCalls; free(cells); }
void *FEngAllocateQueueNode(unsigned int bytes) { return malloc(bytes); }
void FEngFreeQueueNode(void *node) { ++freeCalls; free(node); }
static FEngQueueList *deletingList;
static FEngQueueNode *injectedNode;
static std::vector<void *> deletedNodes;
static void __thiscall DeleteFixtureNode(void *pointer, unsigned int flags) {
    FEngQueueNode *node = (FEngQueueNode *)pointer;
    assert(flags == 1 && (unsigned int)node->next4 == 0xabadcafe && (unsigned int)node->previous8 == 0xabadcafe);
    deletedNodes.push_back(pointer);
    if (injectedNode) {
        injectedNode->next4 = deletingList->first8;
        injectedNode->previous8 = 0;
        if (deletingList->first8) deletingList->first8->previous8 = injectedNode;
        deletingList->first8 = injectedNode;
        if (!deletingList->valueC) deletingList->valueC = injectedNode;
        ++deletingList->count4;
        injectedNode = 0;
    }
}
static void *__thiscall DeleteImage(void *pointer, unsigned int flags) {
    return ((FEngLeafObject *)pointer)->DestroyImage(flags);
}
static void *__thiscall DeleteStateBlock(void *pointer, unsigned int flags) {
    return ((FEngStatePoolBlock *)pointer)->Destroy(flags);
}
static void *__thiscall DeleteStateClass(void *pointer, unsigned int flags) {
    return ((FEngStatePoolClass *)pointer)->Destroy(flags);
}
class CleanupObserver : public FEngDestructionObserver {
public:
    unsigned int calls;
    CleanupObserver() : calls(0) {}
    void ObjectDestroyed(HudVisibilityObject *object) {
        assert(*(unsigned int *)object->unidentified0 == 0x79b8e0);
        void *payload = malloc(8); assert(payload);
        *(unsigned int *)object->unidentified14 = (unsigned int)payload;
        ++calls;
    }
};

class Owner : public FEGridOwner {
public:
    struct Event { unsigned int hash; void *sender; unsigned int value8, valueC; };
    struct Fill { FEGridObject *grid; int column, row; };
    std::vector<Event> messages;
    std::vector<Fill> fills;
    FEGridOwner *next;
    Owner() : next(0) {}
#define UNUSED_SLOT(name) void name() { abort(); }
    UNUSED_SLOT(Unidentified00) UNUSED_SLOT(Unidentified04) UNUSED_SLOT(Unidentified08) UNUSED_SLOT(Unidentified0C)
    UNUSED_SLOT(Unidentified10) UNUSED_SLOT(Unidentified14) UNUSED_SLOT(Unidentified20)
    UNUSED_SLOT(Unidentified24) UNUSED_SLOT(Unidentified28) UNUSED_SLOT(Unidentified2C) UNUSED_SLOT(Unidentified30)
    UNUSED_SLOT(Unidentified34) UNUSED_SLOT(Unidentified38) UNUSED_SLOT(Unidentified3C) UNUSED_SLOT(Unidentified40)
    UNUSED_SLOT(Unidentified44) UNUSED_SLOT(Unidentified48) UNUSED_SLOT(Unidentified50)
#undef UNUSED_SLOT
    void InputMessage(unsigned int, void *, unsigned int, FEPackage *) { abort(); }
    void MessagePosted(unsigned int, void *, FEPackage *, int, int) { abort(); }
    void MessageDelivering(unsigned int, int, void *, FEPackage *, unsigned int) { abort(); }
    void GameMessage(unsigned int hash, void *sender, unsigned int value8, FEPackage *package) {
        Event e = {hash, sender, value8, (unsigned int)package}; messages.push_back(e);
        if (next) reinterpret_cast<FEGridObject *>(sender)->owner5C = next;
    }
    void RefillGrid(FEGridObject *grid, int column, int row) {
        Fill fill = {grid, column, row}; fills.push_back(fill);
    }
};
static void Custom(void *context, FEGridObject *grid, int column, int row) {
    static_cast<Owner *>(context)->RefillGrid(grid, column, row);
}
static void Property(FEngGridPropertyReader &reader, unsigned int tag, unsigned int one, unsigned int two = 0) {
    unsigned int property[3] = {tag | (8u << 16), one, two};
    reader.ReadProperty((const unsigned short *)property);
}
static void Initialize(FEGridObject &grid, FEGridCell *cells, Owner &owner) {
    memset(&grid, 0, sizeof(grid));
    grid.owner5C = &owner; grid.count60 = 3; grid.count64 = 2;
    grid.value6C = 6; grid.value70 = 5; grid.flags68 = 8;
    grid.value74 = grid.value7C = 1; grid.value78 = grid.value80 = 2;
    grid.cells8C = cells;
    for (unsigned int i = 0; i < 6; ++i)
        for (unsigned int j = 0; j < 12; ++j) cells[i].value0[j] = 0x10000 + i * 256 + j;
}
static void ResponseCommands() {
    FEResponseCommand source, copy;
    assert(FEngConstructResponseCommand(&source) == &source);
    assert(FEngConstructResponseCommand(&copy) == &copy);
    assert(!source.type && !source.parameter && !source.target);
    source.type = 0x200; source.target = 0xabcdef01;
    source.SetString("original");
    copy.type = 0x204;
    unsigned int before = freeCalls;
    assert(copy.Assign(&source) == &copy && freeCalls == before + 2); // Includes the second free(NULL).
    assert(copy.parameter != source.parameter && !strcmp((const char *)copy.parameter, "original"));
    assert(copy.type == 0x200 && copy.target == source.target);
    source.SetString("replacement");
    assert(!strcmp((const char *)copy.parameter, "original"));
    before = freeCalls;
    assert(source.Assign(&source) == &source && freeCalls == before + 2);
    assert(!source.parameter && source.type == 0x200 && source.target == 0xabcdef01);
    before = freeCalls;
    FEngDestructResponseCommand(&copy);
    FEngDestructResponseCommand(&source);
    assert(freeCalls == before + 2 && !copy.parameter && !source.parameter);
    assert(copy.type == 0x200 && copy.target == 0xabcdef01);
    copy.type = 0x203; copy.parameter = 0x12345678;
    before = freeCalls; FEngDestructResponseCommand(&copy);
    assert(freeCalls == before && !copy.parameter && copy.type == 0x203);
}
int main() {
    void *page = mmap((void *)0x79b000, 4096, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    assert(page == (void *)0x79b000);
    memcpy((void *)0x79b140, "grid-high", 10);
    memcpy((void *)0x79b14c, "grid-low", 9);
    // Test-only native relocation of the two recovered virtual destructor slots.
    *(void **)0x79b13c = (void *)DeleteStateBlock;
    *(void **)0x79ba0c = (void *)DeleteImage;
    *(void **)0x79b8dc = (void *)DeleteStateClass;
    assert(FngHash(0) == 0xffffffff && FngHash("a") == FngHash("A") && FngHash("\x80") == 95);
    assert(FEngGridWrapIndex(-6, 3) == 3 && FEngGridWrapIndex((int)0x80000000u, 3) == 5);
    Owner owner, replacement;
    FEGridObject grid; FEGridCell cells[6];
    Initialize(grid, cells, owner);
    grid.InitialFill();
    assert(owner.fills.size() == 6);
    for (unsigned int i = 0; i < 6; ++i) {
        assert(owner.fills[i].column == (int)(1 + i % 3));
        assert(owner.fills[i].row == (int)(2 + i / 3));
    }
    owner.fills.clear();
    grid.callbackC0 = Custom; grid.contextC4 = &replacement;
    grid.value6C = 2; grid.value70 = 1;
    grid.value74 = 1; grid.value78 = 0;
    grid.InitialFill();
    assert(owner.fills.empty() && replacement.fills.size() == 2);
    assert(replacement.fills[0].column == 1 && replacement.fills[1].column == 0);
    assert(replacement.fills[0].row == 0 && replacement.fills[1].row == 0);
    replacement.fills.clear();
    grid.count60 = 0;
    grid.InitialFill();
    assert(replacement.fills.empty());
    Initialize(grid, cells, owner);
    grid.Navigate(0, -1);
    assert(owner.fills.size() == 3 && grid.value78 == 1 && grid.value80 == 1);
    for (unsigned int i = 0; i < 3; ++i) {
        assert(owner.fills[i].grid == &grid && owner.fills[i].column == 1 && owner.fills[i].row == 1);
        assert(cells[i].value0[0] == 0x10000 + i * 256);
        assert(cells[i].value0[8] == 0x10000 + (i + 3) * 256 + 8);
        assert(cells[i + 3].value0[0] == 0x10000 + i * 256);
    }
    owner.fills.clear(); Initialize(grid, cells, owner);
    grid.callbackC0 = Custom; grid.contextC4 = &replacement;
    grid.Navigate(1, 0);
    assert(owner.fills.empty() && replacement.fills.size() == 2);
    assert(replacement.fills[0].column == 4 && replacement.fills[0].row == 2);
    assert(replacement.fills[1].column == 4 && replacement.fills[1].row == 3);
    assert(cells[0].value0[0] == 0x10100 && cells[2].value0[0] == 0x10200 && cells[2].value0[8] == 0x10008);
    Initialize(grid, cells, owner); grid.flags68 = 4;
    owner.next = &replacement;
    assert(!grid.CanShift(1, 0, 0, 3, 3));
    assert(owner.messages.size() == 1 && replacement.messages.size() == 1);
    assert(owner.messages[0].hash == FngHash("grid-low") && replacement.messages[0].hash == FngHash("grid-high"));
    assert(owner.messages[0].sender == &grid && owner.messages[0].value8 == 0xff && !owner.messages[0].valueC);
    // Compose recovered allocation, full reset, setters, fill and navigation.
    Owner storageOwner;
    FEGridObject storage; memset(&storage, 0xa5, sizeof(storage));
    unsigned int guidBefore = FEngNextObjectGuid;
    assert(storage.Construct() == &storage && FEngNextObjectGuid == guidBefore + 1);
    assert(((HudVisibilityObject *)&storage)->guidC == guidBefore);
    assert(((HudVisibilityObject *)&storage)->type18 == 6);
    assert(*(unsigned int *)(storage.unidentifiedA4 + 0x18) == FEngValue7F7CC4);
    assert(!storage.cells8C && !storage.callbackC0 && storage.flags68 == 2);
    unsigned int pool[16] = {};
    storage.owner5C = &storageOwner; storage.flags68 = 8;
    storage.value98 = 8; storage.value9C = pool;
    FEngGridPropertyReader reader = {}; reader.objectC = &storage;
    Property(reader, 0x744c, 8); // LT retains the original ownership bit.
    Property(reader, 0x444c, 2, 2); // LD composes allocation/reset/dimension setters.
    assert(allocationBytes == 4 * 48 && !freeCalls);
    assert(storage.count60 == 2 && storage.count64 == 2 && storage.value6C == 2 && storage.value70 == 2);
    assert(storage.value74 == 1 && storage.value78 == 1 && storage.flags68 == 9);
    for (unsigned int i = 0; i < 4; ++i) {
        assert(storage.cells8C[i].value0[0] == 0xffffffff);
        assert(storage.cells8C[i].value0[9] == 0 && storage.cells8C[i].value0[11] == 0x3f800000);
    }
    storage.InitialFill();
    assert(storageOwner.fills.size() == 4);
    assert(storageOwner.fills[0].column == 1 && storageOwner.fills[0].row == 1);
    assert(storageOwner.fills[3].column == 0 && storageOwner.fills[3].row == 0);
    storage.Navigate(1, 0);
    assert(storageOwner.fills.size() == 6);
    for (unsigned int i = 1; i < 4; ++i) {
        storage.cells8C[i].value0[6] = 2;
        storage.cells8C[i].value0[8] = 100 + i;
    }
    Property(reader, 0x444c, 1, 1);
    assert(allocationBytes == 48 && freeCalls == 1 && storage.value98 == 5);
    assert(pool[7] == 101 && pool[6] == 102 && pool[5] == 103);
    FEngGridFreeCells(storage.cells8C);
    FEGridObject poolGrid; memset(&poolGrid, 0xa5, sizeof(poolGrid));
    poolGrid.Construct();
    FEGridCell textCells[2]; memset(textCells, 0, sizeof(textCells));
    poolGrid.count60 = 2; poolGrid.count64 = 1; poolGrid.cells8C = textCells;
    poolGrid.value6C = 2; poolGrid.value70 = 1;
    reader.objectC = &poolGrid;
    Property(reader, 0x4953, 2, 6);
    unsigned short *text = (unsigned short *)poolGrid.value9C[0];
    text[0] = 'A'; text[1] = 0x3a9; text[2] = 'C'; text[3] = 0;
    textCells[0].value0[6] = textCells[1].value0[6] = 2;
    textCells[0].value0[8] = poolGrid.value9C[0]; textCells[1].value0[8] = 0;
    poolGrid.value98 = 2;
    unsigned int beforePoolFree = freeCalls;
    Property(reader, 0x4953, 2, 3);
    assert(freeCalls == beforePoolFree + 2 && poolGrid.value98 == 2);
    assert(textCells[0].value0[8] == poolGrid.value9C[0] && textCells[1].value0[8] == poolGrid.value9C[1]);
    text = (unsigned short *)textCells[0].value0[8];
    assert(text[0] == 'A' && text[1] == 0x3a9 && text[2] == 0);
    assert(!*(unsigned short *)textCells[1].value0[8]);
    Property(reader, 0x6343, 0x11223344);
    Property(reader, 0x6a4c, 0x55667788);
    assert(textCells[0].value0[0] == 0x11223344 && textCells[1].value0[0] == 0x11223344);
    assert(textCells[0].value0[7] == 0x55667788 && textCells[1].value0[7] == 0x55667788);
    poolGrid.SetCellPair4(0, 0, textCells[0].value0, 1, 1);
    assert(textCells[0].value0[1] == 0x11223344 && textCells[0].value0[2] == 0x11223344);
    Property(reader, 0x7343, 0x3f800000, 0x40000000);
    assert(textCells[0].value0[1] == 0x3f800000 && textCells[1].value0[2] == 0x40000000);
    Property(reader, 0x764c, 0x3f000000, 0xbf000000);
    assert(poolGrid.value84 == .5f && poolGrid.value88 == -.5f);
    unsigned int retiredPayload = textCells[0].value0[8];
    Property(reader, 0x4953, 0, 0);
    assert(!poolGrid.valueA0 && !poolGrid.value9C && !poolGrid.value98 && !poolGrid.value90 && !poolGrid.value94);
    assert(textCells[0].value0[8] == retiredPayload); // Original disable does not clear cell payloads.
    FEGridObject cleanup; memset(&cleanup, 0xa5, sizeof(cleanup));
    cleanup.Construct();
    cleanup.ResizeCells(2, 2);
    cleanup.ResizeStringPool(4, 8);
    unsigned int beforeDestruct = freeCalls;
    CleanupObserver cleanupObserver;
    FEngObjectDestructionObserver = &cleanupObserver;
    cleanup.Destruct();
    FEngObjectDestructionObserver = 0;
    assert(freeCalls == beforeDestruct + 4 && cleanupObserver.calls == 1);
    assert(*(unsigned int *)cleanup.unidentified0 == 0x79a9e8);
    void *deletingVtable[1] = {(void *)DeleteFixtureNode};
    FEngQueueNode first = {}, last = {}, injected = {};
    first.vtable0 = last.vtable0 = injected.vtable0 = deletingVtable;
    first.next4 = &last; last.previous8 = &first;
    FEngQueueList owned = {0, 2, &first, &last};
    deletingList = &owned; injectedNode = &injected;
    owned.ClearOwned();
    assert(!owned.count4 && !owned.first8 && !owned.valueC && deletedNodes.size() == 3);
    assert(deletedNodes[0] == &first && deletedNodes[1] == &injected && deletedNodes[2] == &last);
    const unsigned int beforePoolSequence = freeCalls;
    assert(!FEngObjectStateManager.AcquireState(0));
    std::vector<void *> slots;
    for (unsigned int i = 0; i < 40; ++i) {
        void *slot = FEngObjectStateManager.AcquireState(64); assert(slot);
        for (unsigned int j = 0; j < slots.size(); ++j) assert(slot != slots[j]);
        slots.push_back(slot);
    }
    assert(FEngObjectStateManager.classes0.count4 == 1);
    FEngStatePoolClass *poolClass = (FEngStatePoolClass *)FEngObjectStateManager.classes0.first8;
    assert(poolClass->blocksC.count4 == 2);
    void *otherKey = FEngObjectStateManager.AcquireState(0x10040);
    assert(otherKey && FEngObjectStateManager.classes0.count4 == 2);
    FEngStatePoolClass *otherClass = (FEngStatePoolClass *)FEngObjectStateManager.classes0.first8;
    assert(otherClass->stride1C == 0x10040 && ((FEngStatePoolBlock *)otherClass->blocksC.first8)->strideC == 64);
    FEngObjectStateManager.ReleaseState(otherKey);
    FEGridObject pooledObject; memset(&pooledObject, 0xa5, sizeof(pooledObject));
    pooledObject.Construct();
    ((HudVisibilityObject *)&pooledObject)->data2C = (float *)slots.back(); slots.pop_back();
    pooledObject.Destruct(); // Actual base cleanup releases the allocated state slot.
    for (unsigned int i = 0; i < slots.size(); ++i) FEngObjectStateManager.ReleaseState(slots[i]);
    assert(!FEngObjectStateManager.classes0.count4 && !FEngObjectStateManager.classes0.first8 && !FEngObjectStateManager.classes0.valueC);
    assert(freeCalls == beforePoolSequence + 8);
    FEngObjectStateSpec specs[] = {{1,64}, {2,80}, {4,64}, {5,64}, {6,64}, {7,64}, {9,64}, {10,64}, {11,64}};
    FEngObjectFactoryContext factory = {};
    factory.count30 = sizeof(specs) / sizeof(specs[0]); factory.specs34 = specs; factory.owner38 = &storageOwner;
    const int types[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, -1};
    std::vector<HudVisibilityObject *> factoryObjects;
    for (unsigned int i = 0; i < sizeof(types) / sizeof(types[0]); ++i) {
        unsigned int before = FEngNextObjectGuid;
        HudVisibilityObject *object = factory.Create(types[i]);
        if (!types[i]) { assert(!object && FEngNextObjectGuid == before); continue; }
        assert(object && object->type18 == types[i] && object->guidC == before && FEngNextObjectGuid == before + 1);
        unsigned int expectedSize = types[i] == 2 ? 80 : types[i] == 3 || types[i] == 8 || types[i] == -1 ? 0 : 64;
        assert(*(unsigned int *)object->unidentified30 == expectedSize && !!object->data2C == !!expectedSize);
        if (types[i] == 6) assert(((FEGridObject *)object)->owner5C == &storageOwner);
        factoryObjects.push_back(object);
    }
    for (unsigned int i = 0; i < factoryObjects.size(); ++i) {
        // Factory return-path fixture: derived-owned pointers are still empty.
        factoryObjects[i]->DestructBase(); FEngFreeQueueNode(factoryObjects[i]);
    }
    assert(!FEngObjectStateManager.classes0.count4 && !FEngObjectStateManager.classes0.first8);
    FEStringObject pooledString; memset(&pooledString, 0xa5, sizeof(pooledString));
    pooledString.Construct();
    pooledString.SetLabelBytes("owned-label");
    unsigned short lifecycleText[] = {'a', 0x3a9, 0};
    reinterpret_cast<FEngTextStorage *>(&pooledString.text64)->Assign(lifecycleText);
    unsigned short *ownedText = pooledString.text64;
    unsigned int ownedLabel = *(unsigned int *)(pooledString.unidentified58 + 4);
    pooledString.data2C = (float *)FEngObjectStateManager.AcquireState(64);
    *(void **)pooledString.unidentified14 = malloc(8);
    unsigned int beforeString = freeCalls;
    pooledString.DestructString();
    assert(freeCalls == beforeString + 6); // label, text, state data/block/class, +14.
    assert(!FEngObjectStateManager.classes0.count4 && !FEngObjectStateManager.classes0.first8);
    assert(*(unsigned int *)pooledString.unidentified0 == 0x79a9e8);
    assert(pooledString.text64 == ownedText && *(unsigned int *)(pooledString.unidentified58 + 4) == ownedLabel);
    const unsigned int deleteFlags[] = {0, 1, 0x100, 0x101};
    for (unsigned int i = 0; i < 4; ++i) {
        FEStringObject *string = (FEStringObject *)malloc(sizeof(FEStringObject));
        string->Construct(); string->SetLabelBytes("delete-label");
        reinterpret_cast<FEngTextStorage *>(&string->text64)->Assign(lifecycleText);
        unsigned int before = freeCalls;
        assert(string->DestroyString(deleteFlags[i]) == string);
        assert(freeCalls == before + 2 + (deleteFlags[i] & 1));
        if (!(deleteFlags[i] & 1)) free(string);
    }
    FEngGroupObject ownedGroup; ownedGroup.Construct();
    FEngLeafObject *childA = (FEngLeafObject *)malloc(sizeof(FEngLeafObject));
    FEngLeafObject *childB = (FEngLeafObject *)malloc(sizeof(FEngLeafObject));
    childA->ConstructImage(); childB->ConstructImage();
    HudVisibilityObject *a = (HudVisibilityObject *)childA, *b = (HudVisibilityObject *)childB;
    a->next4 = b; a->previous8 = 0; b->next4 = 0; b->previous8 = a;
    ownedGroup.children5C.first8 = (FEngQueueNode *)a; ownedGroup.children5C.valueC = (FEngQueueNode *)b;
    ownedGroup.children5C.count4 = 2;
    unsigned int beforeGroup = freeCalls;
    ownedGroup.Destruct(); // Actual child deleting slots bridge into recovered image destruction.
    assert(freeCalls == beforeGroup + 2 && !ownedGroup.children5C.count4 && !ownedGroup.children5C.first8);
    FEListObject ownedList; ownedList.Construct();
    ownedList.count60 = 3; ownedList.count64 = 2;
    unsigned int *listCells = (unsigned int *)calloc(6, 0x30);
    ownedList.value90 = listCells;
    for (unsigned int i = 0; i < 6; ++i) {
        listCells[i * 12 + 6] = i % 2 ? 1 : 2;
        if (i == 0 || i == 2) listCells[i * 12 + 8] = (unsigned int)malloc(8);
    }
    unsigned int beforeCells = freeCalls;
    ownedList.ReleaseCells();
    assert(freeCalls == beforeCells + 3 && !ownedList.value90 && ownedList.count60 == 3 && ownedList.count64 == 2);
    ownedList.value5C |= 1;
    ownedList.value78 = (FEListAxisEntry *)malloc(3 * sizeof(FEListAxisEntry));
    ownedList.value7C = (FEListAxisEntry *)malloc(2 * sizeof(FEListAxisEntry));
    void *retainedCells = calloc(6, 0x30); ownedList.value90 = retainedCells;
    unsigned int beforeAxes = freeCalls;
    ownedList.ReleaseStorage();
    assert(freeCalls == beforeAxes + 2 && !ownedList.count60 && !ownedList.count64);
    assert(!ownedList.value78 && !ownedList.value7C && ownedList.value90 == retainedCells);
    free(retainedCells); // Fixture cleanup: original ReleaseStorage leaves this pointer untouched.
    ownedList.Destruct();
    for (unsigned int i = 0; i < 4; ++i) {
        FEngLeafObject *image = (FEngLeafObject *)malloc(sizeof(FEngLeafObject)); image->ConstructImage();
        FEngGroupObject *group = (FEngGroupObject *)malloc(sizeof(FEngGroupObject)); group->Construct();
        FEListObject *list = (FEListObject *)malloc(sizeof(FEListObject)); list->Construct();
        unsigned int before = freeCalls;
        assert(image->DestroyImage(deleteFlags[i]) == image);
        assert(group->Destroy(deleteFlags[i]) == group);
        assert(list->Destroy(deleteFlags[i]) == list);
        assert(freeCalls == before + 3 * (deleteFlags[i] & 1));
        if (!(deleteFlags[i] & 1)) { free(image); free(group); free(list); }
    }
    ResponseCommands();
    munmap(page, 4096);
    puts("PASS recovered grid storage/pool migration/cell writes/fill/navigation/callbacks/hash (no retail assets or display)");
}
