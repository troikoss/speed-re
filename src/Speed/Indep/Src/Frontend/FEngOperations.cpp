#include "FEngPackage.h"
#include "../../Libs/bWare/Strings.h"

extern int FngScrollerDiagnostic(const char *, ...); // recovered 0x50d510

// Original 0x5559c0: complete operation consumer, including lifecycle flags.
void FEngQueue::ConsumeOperations() {
    FEPackage *savedPackage = 0;
    FEPackage *savedParent = 0;
    for (;;) {
        FEngQueueNode *node = operations4124.first8;
        if (!node) return;
        // Original compiler inlines unlinking here; no extra Remove call.
        operations4124.first8 = node->next4;
        if (node == operations4124.valueC) operations4124.valueC = node->previous8;
        if (node->previous8) node->previous8->next4 = node->next4;
        if (node->next4) node->next4->previous8 = node->previous8;
        node->next4 = (FEngQueueNode *)0xabadcafe;
        node->previous8 = (FEngQueueNode *)0xabadcafe;
        --operations4124.count4;
        FEPackage *package = (FEPackage *)node->value1C;
        int level = package ? package->value18 : -1;
        if ((node->value14 & 1) && level >= 0) {
            if (package->parent3C) {
                FEPackage *candidate = packagesDC.first8;
                bool found = false;
                while (candidate) {
                    if (found) break;
                    if (candidate == package->parent3C) found = true;
                    else candidate = candidate->next4;
                }
                if (!found) {
                    FngScrollerDiagnostic((const char *)0x79ccb8);
                    ((FEPackage *)node->value1C)->parent3C = 0;
                }
            }
            package = (FEPackage *)node->value1C;
            if (!(node->value14 & 2)) {
                FEPackage *parent = package->parent3C;
                for (FEPackage *other = packagesDC.first8; other; other = other->next4)
                    if (other->parent3C == package) other->parent3C = parent;
            } else {
                savedPackage = package;
                savedParent = package->parent3C;
            }
            FEPackage *parent = ((FEPackage *)node->value1C)->parent3C;
            if (parent) {
                unsigned int mask = parent->inputMask1C | node->value18;
                parent->inputMask1C = mask;
                parent->inputMask20 = mask;
                parent->inputMask24 = mask;
                parent->inputMask28 = mask;
            }
            if (!(node->value14 & 2)) {
                for (FEPackage *other = packagesDC.first8; other; other = other->next4)
                    other->inputMask24 = other->inputMask28;
            }
            Call5362E0((FEPackage *)node->value1C);
            --level;
        }
        if (node->value14 & 2) {
            FEngRegisteredPackageLink *root = &fengManager8384C4->names10;
            const char *name = (const char *)node->valueC;
            for (FEngRegisteredPackageLink *record = root->next0; record != root; record = record->next0) {
                if (!nfs_stricmp(record->name8, name)) {
                    FEPackage *loaded = Call555860((const char *)node->valueC, (unsigned char)((unsigned char)level + 1), node->value18);
                    if (loaded) {
                        if (!(node->value14 & 1) && level >= 0) {
                            loaded->parent3C = (FEPackage *)node->value1C;
                            FEPackage *previous = (FEPackage *)node->value1C;
                            unsigned int mask = previous->inputMask1C & ~node->value18;
                            previous->inputMask1C = mask;
                            previous->inputMask24 = mask;
                        } else if ((node->value14 & 1) == 1) {
                            loaded->parent3C = savedParent;
                            for (FEPackage *other = packagesDC.first8; other; other = other->next4)
                                if (other->parent3C == savedPackage) other->parent3C = loaded;
                        }
                    }
                    if (node->value14 & 0x10) {
                        for (FEPackage *other = packagesDC.first8; other; other = other->next4)
                            if (other != loaded) other->inputMask24 = other->inputMask1C & ~node->value18;
                    }
                    break;
                }
            }
        }
        if (node->value14 & 4) {
            package = (FEPackage *)node->value1C;
            FEPackage *parent = package->parent3C;
            if (parent) {
                unsigned int moved = package->inputMask1C & node->value18;
                unsigned int mask = ~moved & package->inputMask1C;
                package->inputMask1C = mask;
                package->inputMask20 = mask;
                package->inputMask24 = mask;
                package->inputMask28 = mask;
                mask = parent->inputMask1C | moved;
                parent->inputMask1C = mask;
                parent->inputMask20 = mask;
                parent->inputMask24 = mask;
                parent->inputMask28 = mask;
                Post(0x334c5493, 0, parent, -4, node->value18);
            }
        }
        if (node->value14 & 8) {
            FEPackage *other = packagesDC.first8;
            package = (FEPackage *)node->value1C;
            while (other) {
                if (other->parent3C == package) {
                    unsigned int moved = package->inputMask1C & node->value18;
                    unsigned int mask = ~moved & package->inputMask1C;
                    package->inputMask1C = mask;
                    package->inputMask20 = mask;
                    package->inputMask24 = mask;
                    package->inputMask28 = mask;
                    mask = other->inputMask1C | moved;
                    other->inputMask1C = mask;
                    other->inputMask20 = mask;
                    other->inputMask24 = mask;
                    other->inputMask28 = mask;
                    Post(0x334c5493, 0, other, -4, node->value18);
                    break;
                }
                other = other->next4;
            }
        }
        typedef FEngQueueNode *(__thiscall *DeletingDestructor)(FEngQueueNode *, unsigned int);
        ((DeletingDestructor *)node->vtable0)[0](node, 1);
    }
}
