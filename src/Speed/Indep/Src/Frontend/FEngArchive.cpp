#include "FEngPackage.h"
#include "../../Libs/bWare/Strings.h"

// Original 0x51bb70: hash the final backslash component, then move the first hit to front.
FEngPackageNode *FEngArchive::FindNode(const char *name) {
    const char *component = name + strlenWithNullcheck(name);
    while (component != name) {
        if (*component == '\\') {
            ++component;
            break;
        }
        --component;
    }
    unsigned int hash = cihash(component);
    FEngPackageNode *node = root4.next0 ? (FEngPackageNode *)((char *)root4.next0 - 4) : 0;
    while (node != (FEngPackageNode *)this) {
        unsigned int candidate;
        if (node->value10[0] == 0x30203) {
            unsigned int *data = node->valueC;
            if (data[0] == 0xe76e4546 && data[2] == 0x64486b50 && data[4] >= 0x20000)
                candidate = cihash((const char *)(data + 10));
            else
                candidate = 0;
        } else if (node->value10[0] == 0x30210) {
            candidate = node->value10[2];
        } else {
            candidate = 0;
        }
        if (candidate == hash) {
            FEngArchiveLink *link = &node->links4;
            link->previous4->next0 = link->next0;
            link->next0->previous4 = link->previous4;
            FEngArchiveLink *head = root4.next0;
            root4.next0 = link;
            head->previous4 = link;
            link->previous4 = &root4;
            link->next0 = head;
            return node;
        }
        node = node->links4.next0 ? (FEngPackageNode *)((char *)node->links4.next0 - 4) : 0;
    }
    return 0;
}
