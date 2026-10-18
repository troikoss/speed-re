#include "FEngPackage.h"
#include "FEngResponse.h"

// Original 0x51b000 / 0x52c750: special object handlers precede response lookup.
unsigned char __stdcall FEngHandleObject4(HudVisibilityObject *object, FEPackage *, unsigned int message) {
    switch (message) {
    case 0x030471ac: object->Call516E90(1, 0); return 1;
    case 0xe10814a6: object->Call516E90(0, 1); return 1;
    case 0xe10c4af9: object->Call516E90(-1, 0); return 1;
    case 0xfb814f13: object->Call516E90(0, -1); return 1;
    }
    return 0;
}
unsigned char __stdcall FEngHandleObject6(HudVisibilityObject *object, FEPackage *, unsigned int message) {
    switch (message) {
    case 0x030471ac: object->Call52BD70(1, 0); return 1;
    case 0xe10814a6: object->Call52BD70(0, 1); return 1;
    case 0xe10c4af9: object->Call52BD70(-1, 0); return 1;
    case 0xfb814f13: object->Call52BD70(0, -1); return 1;
    }
    return 0;
}

// Original 0x5367e0: each type is checked independently, response first-match wins.
void FEngQueue::DeliverObject(HudVisibilityObject *object, FEPackage *package, unsigned int message, unsigned int mask) {
    if (object->type18 == 4 && FEngHandleObject4(object, package, message)) return;
    if (object->type18 == 6 && FEngHandleObject6(object, package, message)) return;
    FEResponse *response = object->responses3C;
    while (response && response->messageC != message) response = response->next4;
    if (response) ExecuteResponse(response, object, package, mask);
}

// Original 0x541fc0. First unlink is an actual call; subsequent unlinks are inline.
void FEngQueue::DeliverMessages() {
    FEngQueueNode *node = messages4114.first8;
    if (!node) return;
    messages4114.Remove(node);
    if (!node) return;
    for (;;) {
        if (value5251)
            value10C->MessageDelivering(node->value18, (int)node->value10, (void *)node->valueC,
                                       (FEPackage *)node->value14, node->value1C);
        unsigned int target = node->value10;
        if (target == 0 || target == 0xfffffffc) {
            FEPackage *package = packagesDC.first8;
            if (target == 0xfffffffc) {
                while (package && package != (FEPackage *)node->value14) package = package->next4;
            }
            while (package) {
                FEResponse *response = package->responses5C;
                unsigned int mask = node->value1C, message = node->value18;
                while (response && response->messageC != message) response = response->next4;
                if (response) ExecuteResponse(response, 0, package, mask);
                unsigned int size = package->associationCount6C;
                if (size) {
                    FEngMessageAssociation *table = package->associations70;
                    unsigned int associatedMessage = node->value18;
                    unsigned int i = 0;
                    while (i < size && table[i].message0 != associatedMessage) ++i;
                    if (i < size) {
                        FEngMessageAssociation *association = table + i;
                        if (association) {
                            unsigned int count = association->count8;
                            for (unsigned int j = 0; j < count; ++j)
                                DeliverObject(association->objectsC[j], package, associatedMessage, node->value1C);
                        }
                    }
                }
                if (target == 0xfffffffc) break;
                package = package->next4;
            }
        } else if (target == 0xfffffffd) {
            FEPackage *package = (FEPackage *)node->value14;
            FEResponse *response = package->responses5C;
            unsigned int mask = node->value1C, message = node->value18;
            while (response && response->messageC != message) response = response->next4;
            if (response) ExecuteResponse(response, 0, package, mask);
        } else if (target == 0xfffffffe) {
            for (FEPackage *package = packagesDC.first8; package; package = package->next4) {
                FEResponse *response = package->responses5C;
                unsigned int mask = node->value1C, message = node->value18;
                while (response && response->messageC != message) response = response->next4;
                if (response) ExecuteResponse(response, 0, package, mask);
            }
        } else if (target == 0xffffffff) {
            value10C->GameMessage(node->value18, (void *)node->valueC, node->value1C, (FEPackage *)node->value14);
        } else if (target == 0xfffffffb) {
            value10C->InputMessage(node->value18, (void *)node->valueC, node->value1C, (FEPackage *)node->value14);
        } else if (target == 0xfffffffa) {
            if (node->value18 == 0x59bed120) value1 = 1;
            else if (node->value18 == 0x5d4ce32d) value1 = 0;
        } else {
            DeliverObject((HudVisibilityObject *)target, (FEPackage *)node->value14, node->value18, node->value1C);
        }
        typedef FEngQueueNode *(__thiscall *DeletingDestructor)(FEngQueueNode *, unsigned int);
        ((DeletingDestructor *)node->vtable0)[0](node, 1);
        node = messages4114.first8;
        if (!node) return;
        messages4114.first8 = node->next4;
        if (node == messages4114.valueC) messages4114.valueC = node->previous8;
        if (node->previous8) node->previous8->next4 = node->next4;
        if (node->next4) node->next4->previous8 = node->previous8;
        node->next4 = (FEngQueueNode *)0xabadcafe;
        node->previous8 = (FEngQueueNode *)0xabadcafe;
        --messages4114.count4;
    }
}
