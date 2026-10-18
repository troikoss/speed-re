#include "FEngResponse.h"

// Original 0x504490. The loop can read commands14[commandCount10] when no
// matching delimiter is found; do not replace the original bound with count-1.
unsigned int FEResponse::SkipConditionalBlock(unsigned int index) {
    unsigned int count = commandCount10;
    if (index == count - 1) return count;
    int depth = 1;
    const FEResponseCommand *command = commands14 + index;
    do {
        unsigned int type = command[1].type;
        ++command;
        ++index;
        if (type <= 0x500) {
            if (type == 0x500) {
                if (depth == 1) depth = 0;
            } else if (type >= 0x300 && type <= 0x301) {
                ++depth;
            }
        } else if (type == 0x501) {
            --depth;
        }
    } while (index < count && depth != 0);
    return index;
}

#include "FEngPackage.h"

// Original 0x52c7c0. Count is cached once; command storage is reloaded per iteration.
void FEngQueue::ExecuteResponse(FEResponse *response, HudVisibilityObject *object,
                               FEPackage *package, unsigned int mask) {
    unsigned int count = response->commandCount10;
    for (unsigned int index = 0; index < count; ++index) {
        const FEResponseCommand *command = response->commands14 + index;
        switch (command->type) {
        case 0:
            if (object) {
                FEScript *script = object->FindScriptByHash(command->parameter);
                if (script) { object->SetScript(script); script->time10 = 0; }
            }
            break;
        case 1: {
            unsigned int target = command->target;
            if (target != 0xfffffffc && target != 0xffffffff)
                target = (unsigned int)package->FindObjectByGuid(target);
            Post(command->parameter, object, package, (int)target, mask);
            break;
        }
        case 2: Post(command->parameter, object, package, -1, mask); break;
        case 3: Post(command->parameter, object, package, -5, mask); break;
        case 0x100: {
            HudVisibilityObject *focus = 0;
            if (command->parameter) focus = package->FindObjectByGuid(command->parameter);
            if (focus || !command->parameter) package->SetFocus(this, focus, focus != 0);
            break;
        }
        case 0x101: value1 = command->parameter == 1; break;
        case 0x102: StoreFocus(package->nameHash10, package->focus8C ? package->focus8C->guidC : 0); break;
        case 0x103: {
            unsigned int guid = LoadFocus(package->nameHash10);
            HudVisibilityObject *focus = guid ? package->FindObjectByGuid(guid) : 0;
            if (!focus && command->parameter) focus = package->FindObjectByGuid(command->parameter);
            if (focus || !command->parameter) package->SetFocus(this, focus, focus != 0);
            break;
        }
        case 0x105: Call51AF50(package, 1, mask); break;
        case 0x106: Call51AF50(package, 1, 0xff); break;
        case 0x107: Call51AF50(package, 0, mask); break;
        case 0x108: Call51AF50(package, 0, 0xff); break;
        case 0x200: Call51AB20((const char *)command->parameter, package); break;
        case 0x201: Call51AC10((const char *)command->parameter, package, package->inputMask1C); break;
        case 0x202:
            for (unsigned int bit = 0; bit < 8; ++bit) {
                unsigned int selected = 1u << bit;
                if (mask & selected) Call51AC10((const char *)command->parameter, package, selected);
            }
            break;
        case 0x203: Call51ADB0(package); break;
        case 0x204: Call51AC10((const char *)command->parameter, package, 0); break;
        case 0x2c0: Call51B070(package->nameC); break;
        case 0x2c1:
            if (value4148) {
                --value4148;
                const char *name = (const char *)((unsigned int)this + 0x414c + value4148 * 0x100);
                if (name) Call51AB20(name, package);
            }
            break;
        case 0x2c2: ClearPackageStack(); break;
        case 0x300:
            if (object->currentScript54->hash30 != command->parameter) index = response->SkipConditionalBlock(index);
            break;
        case 0x301:
            if (object->currentScript54->hash30 == command->parameter) index = response->SkipConditionalBlock(index);
            break;
        case 0x500: index = response->SkipConditionalBlock(index); break;
        }
    }
}
