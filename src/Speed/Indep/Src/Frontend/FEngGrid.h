#ifndef UG2_FENG_GRID_H
#define UG2_FENG_GRID_H
#include "FEngQueueNodes.h"
struct FEGridObject;
// 0x555800 -> reader+0x38 -> grid+0x5c supplies the queue's owner instance.
typedef FEngQueueObserver FEGridOwner;
struct FEGridCell { unsigned int value0[12]; };
typedef void (__cdecl *FEGridRefill)(void *context, FEGridObject *grid, int column, int row);
struct FEGridObject {
    char unidentified0[0x5c];
    FEGridOwner *owner5C;
    unsigned int count60, count64, flags68;
    int value6C, value70;
    unsigned int value74, value78, value7C, value80;
    float value84, value88;
    FEGridCell *cells8C;
    unsigned int value90, value94, value98;
    unsigned int *value9C;
    unsigned int valueA0;
    char unidentifiedA4[0x1c];
    FEGridRefill callbackC0;
    void *contextC4;
    void InitialFill(); // 0x504c40; observed caller ignores incidental EAX.
    void SetLogicalColumns(unsigned int total); // 0x518bb0; observed caller ignores EAX.
    void SetLogicalRows(unsigned int total); // 0x518c00; observed caller ignores EAX.
    void ResizeCells(unsigned int columns, unsigned int rows); // 0x52bb30
    void ResizeStringPool(unsigned int count, unsigned int capacity); // 0x518c60, UTF-16 units
    void SetCellValue0(unsigned int x, unsigned int y, unsigned int value, unsigned int columns, unsigned int rows); // 0x5196b0
    void SetCellPair4(unsigned int x, unsigned int y, const unsigned int *pair, unsigned int columns, unsigned int rows); // 0x5197d0
    void SetCellValue1C(unsigned int x, unsigned int y, unsigned int value, unsigned int columns, unsigned int rows); // 0x5198f0
    unsigned char CanShift(int delta, int selection, int offset, int total, int visible); // 0x518e70
    unsigned char MoveSelection(int delta, unsigned int *selection, unsigned int *offset, int total, unsigned int visible); // 0x504e40
    unsigned char ShiftAxis(int delta, unsigned int *selection, unsigned int *offset, int total, unsigned int visible, unsigned char horizontal); // 0x518f20
    void Navigate(int x, int y); // 0x52bd70
};
int FEngGridWrapIndex(int value, int total); // 0x504790, cdecl; negative exact multiples return total.
void FEngGridResetCells(FEGridCell *cells, unsigned int count); // 0x504290, cdecl
void *FEngGridAllocateCells(unsigned int bytes); // allocation boundary 0x575620
void FEngGridFreeCells(void *cells); // deallocation boundary 0x575630
UG2_CHECK_SIZE(FEGridCell, 0x30);
UG2_CHECK_OFFSET(FEGridObject, owner5C, 0x5c);
UG2_CHECK_OFFSET(FEGridObject, cells8C, 0x8c);
UG2_CHECK_OFFSET(FEGridObject, callbackC0, 0xc0);
UG2_CHECK_OFFSET(FEGridObject, value98, 0x98);
UG2_CHECK_OFFSET(FEGridObject, value9C, 0x9c);
UG2_CHECK_SIZE(FEGridObject, 0xc8);
#endif
