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
    munmap(page, 4096);
    puts("PASS recovered grid initial fill/navigation/shallow copies/callback ABI/owner reload/hash (no retail assets or display)");
}
