// Native ABI probe for the recovered grid body. All cells and strings are synthetic.
#include "../../Speed/Indep/Src/Frontend/FEngGrid.h"
#include "../../Speed/Indep/Src/Frontend/FEngHash.h"
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
void *FEngGridAllocateCells(unsigned int bytes) { allocationBytes = bytes; return malloc(bytes); }
void FEngGridFreeCells(void *cells) { ++freeCalls; free(cells); }

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
static void Initialize(FEGridObject &grid, FEGridCell *cells, Owner &owner) {
    memset(&grid, 0, sizeof(grid));
    grid.owner5C = &owner; grid.count60 = 3; grid.count64 = 2;
    grid.value6C = 6; grid.value70 = 5; grid.flags68 = 8;
    grid.value74 = grid.value7C = 1; grid.value78 = grid.value80 = 2;
    grid.cells8C = cells;
    for (unsigned int i = 0; i < 6; ++i)
        for (unsigned int j = 0; j < 12; ++j) cells[i].value0[j] = 0x10000 + i * 256 + j;
}
int main() {
    void *page = mmap((void *)0x79b000, 4096, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    assert(page == (void *)0x79b000);
    memcpy((void *)0x79b140, "grid-high", 10);
    memcpy((void *)0x79b14c, "grid-low", 9);
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
    FEGridObject storage; memset(&storage, 0, sizeof(storage));
    unsigned int pool[16] = {};
    storage.owner5C = &storageOwner; storage.flags68 = 8;
    storage.value98 = 8; storage.value9C = pool;
    storage.ResizeCells(2, 2);
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
    storage.ResizeCells(1, 1);
    assert(allocationBytes == 48 && freeCalls == 1 && storage.value98 == 5);
    assert(pool[7] == 101 && pool[6] == 102 && pool[5] == 103);
    FEngGridFreeCells(storage.cells8C);
    FEGridObject poolGrid; memset(&poolGrid, 0, sizeof(poolGrid));
    FEGridCell textCells[2]; memset(textCells, 0, sizeof(textCells));
    poolGrid.count60 = 2; poolGrid.count64 = 1; poolGrid.cells8C = textCells;
    poolGrid.value6C = 2; poolGrid.value70 = 1;
    poolGrid.ResizeStringPool(2, 6);
    unsigned short *text = (unsigned short *)poolGrid.value9C[0];
    text[0] = 'A'; text[1] = 0x3a9; text[2] = 'C'; text[3] = 0;
    textCells[0].value0[6] = textCells[1].value0[6] = 2;
    textCells[0].value0[8] = poolGrid.value9C[0]; textCells[1].value0[8] = 0;
    poolGrid.value98 = 2;
    unsigned int beforePoolFree = freeCalls;
    poolGrid.ResizeStringPool(2, 3);
    assert(freeCalls == beforePoolFree + 2 && poolGrid.value98 == 2);
    assert(textCells[0].value0[8] == poolGrid.value9C[0] && textCells[1].value0[8] == poolGrid.value9C[1]);
    text = (unsigned short *)textCells[0].value0[8];
    assert(text[0] == 'A' && text[1] == 0x3a9 && text[2] == 0);
    assert(!*(unsigned short *)textCells[1].value0[8]);
    poolGrid.SetCellValue0(0, 0, 0x11223344, 2, 1);
    poolGrid.SetCellValue1C(1, 0, 0x55667788, 1, 1);
    assert(textCells[0].value0[0] == 0x11223344 && textCells[1].value0[0] == 0x11223344);
    assert(textCells[1].value0[7] == 0x55667788);
    poolGrid.SetCellPair4(0, 0, textCells[0].value0, 1, 1);
    assert(textCells[0].value0[1] == 0x11223344 && textCells[0].value0[2] == 0x11223344);
    unsigned int retiredPayload = textCells[0].value0[8];
    poolGrid.ResizeStringPool(0, 0);
    assert(!poolGrid.valueA0 && !poolGrid.value9C && !poolGrid.value98 && !poolGrid.value90 && !poolGrid.value94);
    assert(textCells[0].value0[8] == retiredPayload); // Original disable does not clear cell payloads.
    munmap(page, 4096);
    puts("PASS recovered grid storage/pool migration/cell writes/fill/navigation/callbacks/hash (no retail assets or display)");
}
