// Platform scaffolding for the recovered pause menu and in-game options. Nothing in this file is
// original code: it supplies the remaining FEng, IconScroller, dialog and game-flow interfaces
// that Speed/Indep/Src/Frontend/PauseMenu.cpp and PauseOptions.cpp call, at the original call
// boundaries. Each seam names the retail routine it stands in for; behaviour taken from a read of
// that routine is marked as such and is not verified.
#include "NativePause.h"
#include "NativeAssets.h"
#include "NativeFrontend.h"
#include "NativeControls.h"
#include "NativeHud.h"
#include "../../Speed/Indep/Src/Frontend/FEngPackage.h"
#include "../../Speed/Indep/Src/Frontend/FEngScript.h"
#include "../../Speed/Indep/Src/Frontend/PauseOptions.h"
#include "../../Speed/Indep/Libs/bWare/Compression.h"

#include <map>
#include <new>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

void NativeTrap(const char *what);

extern GameFlow8900D8 *gameFlow8900D8;
extern PCHelpBarFNGObject *pcHelpBar8384D8;
extern Replay8650B8 *replay8650B8;
extern int realClock8651AC;
extern float frontendDelta865198;

namespace {

const unsigned int kAccept = 0x0c407210;      // accept, also the item React message
const unsigned int kPadAccept = 0x406415e3;   // PAD_ACCEPT (package-level accept in 0x54d4b0)
const unsigned int kPadBack = 0x911ab364;     // PAD_BACK
const unsigned int kPadLeft = 0x9120409e;     // PAD_LEFT
const unsigned int kPadRight = 0xb5971bf1;    // PAD_RIGHT
const unsigned int kPadUp = 0x72619778;       // PAD_UP
const unsigned int kPadStart = 0xb5af2461;   // PAD_START
const unsigned int kPadDown = 0x911c0a4b;     // PAD_DOWN
const unsigned int kExitComplete = 0xe1fde1d1; // EXIT_COMPLETE
const unsigned int kStopDim = 0x251bf83e;     // 0x543d40 -> 0x538ca0
const unsigned int kFrontendTick = 0xc98356ba; // sent to every FNG object each frame (0x5663b0)

// Message targets of the FEng queue (0x51aa20 / 0x541fc0).
const int kTargetGame = -1;     // the screen's FNG object handler (owner slot +0x18)
const int kTargetPackage = -3;  // package-level responses
const int kTargetInput = -5;    // owner slot +0x1c, not modelled
const int kTargetObject = 1;    // one object's responses

unsigned int Rd32(const unsigned char *p) { return p[0] | p[1] << 8 | p[2] << 16 | (unsigned int)p[3] << 24; }

bool Debug() { return getenv("UG2_DEBUG_PAUSE") != 0; }

typedef void *(*ScreenFactory)(void *data);
void *CreatePause(void *data) { return CreatePauseFNGObject(data); }
void *CreateOptionsMain(void *data) { return CreatePauseOptionsMainFNGObject(data); }
void *CreateOptions(void *data) { return CreatePauseOptionsFNGObject(data); }

struct NativeScreen {
    NativeScreen(const char *packageName, ScreenFactory create, bool list)
        : name(packageName), factory(create), optionsList(list), package(0), loaded(false), shown(false),
          object(0) {
        memset(&fePackage, 0, sizeof(fePackage));
        memset(&record, 0, sizeof(record));
        memset(&node, 0, sizeof(node));
        node.package18 = &fePackage;
        nodeRecord[0] = 0x30210;
        nodeRecord[1] = 0;
        nodeRecord[2] = FengHash(name);
        node.value10 = nodeRecord;
    }
    // Every showing starts from the package's loaded state, as a newly pushed FEng package does.
    bool Reset(std::string *error) {
        delete package;
        package = new FengPackage;
        memset(&fePackage, 0, sizeof(fePackage));
        focusObjects.clear();
        if (!package->Load(&data[0], data.size(), 0, error)) return false;
        fePackage.nameC = name;
        fePackage.nameHash10 = FengHash(name);
        fePackage.firstObject4C = package->FirstRoot();
        const std::vector<FengObject *> &objects = package->Objects();
        for (size_t i = 0; i < objects.size(); ++i) {
            if (objects[i]->flags1C & 0x10000000) focusObjects.push_back(objects[i]);
        }
        fePackage.focusObjects90 = focusObjects.empty() ? 0 : &focusObjects[0];
        fePackage.focusCount94 = focusObjects.size();
        return true;
    }
    void Deliver(unsigned int message, int value8, int valueC) {
        if (!object) return;
        if (!strcmp(name, "UI_Options_PC_Controller.fng")) return;
        if (optionsList) static_cast<OptionsScrollerFNGObject *>(object)->MessageHandler(message, value8, valueC, 0);
        else static_cast<IconScrollerFNGObject *>(object)->MessageHandler(message, value8, valueC, 0);
    }
    void DestroyObject() {
        if (!object) return;
        void *doomed = object;
        object = 0;
        if (!strcmp(name, "UI_Options_PC_Controller.fng")) DestroyNativeControls(doomed);
        else if (optionsList) delete static_cast<OptionsScrollerFNGObject *>(doomed);
        else delete static_cast<IconScrollerFNGObject *>(doomed);
    }
    const char *name;
    ScreenFactory factory;
    bool optionsList;
    std::vector<unsigned char> data;  // 0x30203 chunk
    FengPackage *package;
    bool loaded;
    bool shown;
    FEPackage fePackage;  // original prefix, native object loading remains a seam
    FngRecord record;      // static FNG selection survives package reloads
    FEngPackageNode node;
    unsigned int nodeRecord[3];
    std::vector<HudVisibilityObject *> focusObjects;
    void *object;          // FNG object created by the package's FNGINIT factory
};

struct RunningScript {
    NativeScreen *screen;
    FengObject *object;
    const FengScript *script;
    float milliseconds;
    size_t nextEvent;
    bool finished;
};

struct PendingMessage {
    NativeScreen *screen;
    unsigned int message;
    FengObject *object;   // value8 delivered to the game, or the sender of a response command
    int target;
    FengObject *targetObject;
};

// The in-game dialog 0x558790 is not recovered. Button order, labels and result messages follow
// its arguments; reading value1C as the initially highlighted button is an interpretation. The
// "" and GenericDialog_SMALL.fng dialogs of the options screens use the same package.
struct NativeDialog {
    bool shown;
    NativeScreen *parent;
    unsigned int results[2];
    unsigned int backResult;
    int selected;
};

NativeScreen pauseScreen("UI_Pause.fng", CreatePause, false);
NativeScreen optionsMainScreen("UI_PauseOptionsMain.fng", CreateOptionsMain, false);
NativeScreen optionsScreen("UI_PauseOptions.fng", CreateOptions, true);
NativeScreen controlsScreen("UI_Options_PC_Controller.fng", CreateNativeControls, false);
NativeScreen dialogScreen("UI_InGameDialog.fng", 0, false);
NativeScreen *const screens[] = {&pauseScreen, &optionsMainScreen, &optionsScreen, &dialogScreen, &controlsScreen};
const size_t kScreenCount = sizeof(screens) / sizeof(screens[0]);
NativeDialog dialog;
NativeTextureCache *textures = 0;
NativeFont font;
std::vector<unsigned char> language;
std::vector<RunningScript> scripts;
std::vector<PendingMessage> messages;
// FngShow / FngRemove run after the current delivery: the caller keeps using its object.
NativeScreen *switchFrom = 0;
NativeScreen *switchTo = 0;
NativeScreen *removeScreen = 0;
NativePauseRequest request = kPauseRequestNone;
bool helpBarLoaded = false;
const char *returnPackage = 0;
float clockRemainder = 0.0f;
GameFlow8900D8 nativeFlow;
PCHelpBarFNGObject nativeHelpBar;
Replay8650B8 nativeReplay;
FEngQueue nativeFengQueue;
FEngManager nativeFengManager = {{0}, &nativeFengQueue};

// Native package membership/order seam; recovered lookup traverses these links.
void RefreshNativePackageChain() {
    // Archive allocation/header construction remains a native adapter.
    FEngArchiveLink *root = &fengArchive8637B0.root4;
    root->next0 = root->previous4 = root;
    FEPackage **tail = &nativeFengQueue.packagesDC.first8;
    *tail = 0;
    for (size_t i = 0; i < kScreenCount; ++i) {
        screens[i]->fePackage.next4 = 0;
        if (!screens[i]->shown) continue;
        *tail = &screens[i]->fePackage;
        tail = &screens[i]->fePackage.next4;
        FEngArchiveLink *link = &screens[i]->node.links4;
        link->next0 = root;
        link->previous4 = root->previous4;
        root->previous4->next0 = link;
        root->previous4 = link;
    }
    *tail = 0;
}

NativeScreen *FindScreen(const char *name) {
    if (!name) return 0;
    unsigned int hash = FengHash(name);
    for (size_t i = 0; i < kScreenCount; ++i) {
        if (screens[i]->loaded && hash == FengHash(screens[i]->name)) return screens[i];
    }
    return 0;
}

std::string Localised(unsigned int hash) {
    return LookupNativeString(language, hash);  // '^' line breaks are laid out by NativeFont (0x51c110)
}

// Package loading seam: UI_*.fng chunks 0x30210 hold the name hash and a JDLZ block that
// unpacks to the 'FEn\xe7' package body of a 0x30203 chunk.
bool LoadCompressedPackage(const std::vector<unsigned char> &bundle, NativeScreen *screen, std::string *error) {
    unsigned int wanted = FengHash(screen->name);
    for (size_t offset = 0; offset + 8 <= bundle.size();) {
        unsigned int id = Rd32(&bundle[offset]);
        unsigned int length = Rd32(&bundle[offset + 4]);
        if (length > bundle.size() - offset - 8) break;
        if (id == 0x30210 && length > 0x14 && Rd32(&bundle[offset + 8]) == wanted) {
            const unsigned char *block = &bundle[offset + 12];
            unsigned int size = Rd32(block + 8);
            std::vector<unsigned char> source(block, block + length - 4);
            source.resize(source.size() + 64);
            std::vector<unsigned char> package(8 + size + 64);
            if (bDecompress(reinterpret_cast<unsigned int *>(&source[0]), &package[8]) != (int)size) {
                *error = "cannot unpack " + std::string(screen->name);
                return false;
            }
            package[0] = 0x03; package[1] = 0x02; package[2] = 0x03; package[3] = 0;
            package[4] = size & 0xff; package[5] = size >> 8 & 0xff;
            package[6] = size >> 16 & 0xff; package[7] = size >> 24;
            package.resize(8 + size);
            screen->data.swap(package);
            if (!screen->Reset(error)) return false;
            screen->loaded = true;
            return true;
        }
        offset += 8 + length;
    }
    *error = std::string(screen->name) + " not found";
    return false;
}

// Plain 0x30203 packages: the PkHd name string sits at chunk +0x30.
bool LoadPlainPackage(const std::vector<unsigned char> &bundle, NativeScreen *screen, std::string *error) {
    size_t nameLength = strlen(screen->name) + 1;
    for (size_t offset = 0; offset + 8 <= bundle.size();) {
        unsigned int id = Rd32(&bundle[offset]);
        unsigned int length = Rd32(&bundle[offset + 4]);
        if (length > bundle.size() - offset - 8) break;
        if (id == 0x30203 && length > 0x30 + nameLength && !memcmp(&bundle[offset + 0x30], screen->name, nameLength)) {
            screen->data.assign(bundle.begin() + offset, bundle.begin() + offset + 8 + length);
            if (!screen->Reset(error)) return false;
            screen->loaded = true;
            return true;
        }
        offset += 8 + length;
    }
    *error = std::string(screen->name) + " not found";
    return false;
}

void Post(NativeScreen *screen, unsigned int message, FengObject *object, int target, FengObject *targetObject = 0) {
    PendingMessage pending = {screen, message, object, target, targetObject};
    messages.push_back(pending);
}

void RemoveScripts(NativeScreen *screen) {
    for (size_t i = 0; i < scripts.size();) {
        if (scripts[i].screen == screen) scripts.erase(scripts.begin() + i);
        else ++i;
    }
}

// Native decoded storage and mask adapter for recovered 0x52c7c0.
void RunResponse(NativeScreen *screen, const FengResponse &response, FengObject *object) {
    FEResponse original = {};
    original.commandCount10 = (unsigned int)response.commands.size();
    original.commands14 = response.commands.data();
    nativeFengQueue.ExecuteResponse(&original, object, &screen->fePackage, 0xff);
}

void Process(const PendingMessage &pending) {
    NativeScreen *screen = pending.screen;
    if (!screen->shown) return;
    if (pending.target == kTargetGame) {
        if (Debug()) fprintf(stderr, "pause: %s <- %08x (%p)\n", screen->name, pending.message, (void *)pending.object);
        screen->Deliver(pending.message, (int)(size_t)pending.object, 0);
    } else if (pending.target == kTargetPackage) {
        const FengResponse *response = FindFengResponse(screen->package->Responses(), pending.message);
        if (response) RunResponse(screen, *response, 0);
    } else if (pending.target == kTargetObject && pending.targetObject) {
        const FengResponse *response = FindFengResponse(pending.targetObject->responses, pending.message);
        if (response) RunResponse(screen, *response, pending.targetObject);
    }
}

void ShowScreen(NativeScreen *screen);
void CloseScreen(NativeScreen *screen);

void ApplyDeferred() {
    if (removeScreen) {
        NativeScreen *screen = removeScreen;
        removeScreen = 0;
        CloseScreen(screen);
    }
    if (switchFrom || switchTo) {
        NativeScreen *from = switchFrom, *to = switchTo;
        switchFrom = switchTo = 0;
        if (from) CloseScreen(from);
        if (to) ShowScreen(to);
    }
}

void Pump() {
    for (int guard = 0; guard < 256; ++guard) {
        ApplyDeferred();
        if (messages.empty()) break;
        PendingMessage pending = messages.front();
        messages.erase(messages.begin());
        Process(pending);
    }
    ApplyDeferred();
}

// 0x54d4b0: a pad message goes to the focus object when it responds to it, else to the
// package-level responses; nothing else is sent. Accept is 0x0c407210 on the focus object and
// PAD_ACCEPT on the package.
void Input(NativeScreen *screen, unsigned int message) {
    FengObject *focus = static_cast<FengObject *>(screen->fePackage.focus8C);
    if (focus && FindFengResponse(focus->responses, message)) {
        Post(screen, message, 0, kTargetObject, focus);
        Post(screen, message, focus, kTargetInput);
        return;
    }
    if (message == kAccept) message = kPadAccept;
    if (FindFengResponse(screen->package->Responses(), message)) {
        Post(screen, message, 0, kTargetPackage);
        Post(screen, message, 0, kTargetInput);
    }
}

FengObject *DialogObject(unsigned int hash) { return dialogScreen.package->Find(hash); }

void DialogHighlight() {
    // Button labels 0xda473d2f (left) and 0xda473d30 (right); highlight colour from 'No'.
    const unsigned int labels[2] = {0xda473d2f, 0xda473d30};
    for (int i = 0; i < 2; ++i) {
        FengObject *label = DialogObject(labels[i]);
        if (label) FengPackage::SetColour(label, i == dialog.selected ? 0xffccf48a : 0xffe1e1e1);
    }
}

void CloseDialog() {
    dialog.shown = false;
    dialogScreen.shown = false;
    RefreshNativePackageChain();
}

NativeScreen *ActiveScreen() {
    if (controlsScreen.shown) return &controlsScreen;
    for (size_t i = 0; i < kScreenCount; ++i) {
        if (screens[i] != &dialogScreen && screens[i]->shown) return screens[i];
    }
    return 0;
}

void ShowScreen(NativeScreen *screen) {
    if (!screen->loaded || screen->shown) return;
    std::string error;
    if (!screen->Reset(&error)) NativeTrap("FEng package reload");
    screen->shown = true;
    RefreshNativePackageChain();
    // FEng creation data: package name, three unidentified words, help-bar mask (+0x20).
    static unsigned int data[5];
    data[0] = (unsigned int)(size_t)screen->name;
    screen->object = screen->factory(data);
    if (Debug()) fprintf(stderr, "pause: showing %s\n", screen->name);
}

void CloseScreen(NativeScreen *screen) {
    if (!screen->shown) return;
    screen->DestroyObject();
    screen->shown = false;
    RefreshNativePackageChain();
    RemoveScripts(screen);
    for (size_t i = 0; i < messages.size();) {
        if (messages[i].screen == screen) messages.erase(messages.begin() + i);
        else ++i;
    }
    if (dialog.parent == screen) CloseDialog();
}

void CloseAll() {
    for (size_t i = 0; i < kScreenCount; ++i) CloseScreen(screens[i]);
    CloseDialog();
    scripts.clear();
    messages.clear();
    switchFrom = switchTo = removeScreen = 0;
}

// IconScroller embedded at IconScrollerFNGObject +0x4c (base vtable 0x795cbc, derived
// 0x79c0f4). Construction and ownership remain native seams. Insertion, navigation, selection,
// placement/reset/frame tick and the cubic tween call recovered routines.
class NativeIconScroller : public IconScroller4C {
public:
    struct Entry {
        IconScrollerIcon *icon;
        FengObject *object;
    };

    struct State {
        IconScrollerFNGObject *owner;
        FengObject *master;
        std::vector<Entry> entries;
        int selected;
    };
    static std::map<NativeIconScroller *, State> &States() {
        static std::map<NativeIconScroller *, State> states;
        return states;
    }
    State &S() { return States()[this]; }

    explicit NativeIconScroller(IconScrollerFNGObject *owner) {
        State &state = S();
        state.owner = owner;
        state.selected = -1;
        memset(&next4, 0, sizeof(IconScroller4C) - 4);
        next4 = previous8 = &next4;
        package1C = owner->package4;
        value2D = relayout2E = value2F = 1;
        direction34 = -9;
        alignmentC0 = 1;
        option28 = 1;
        prefix20 = "option_";
        spacing24 = 40.0f;
        windowCC = 480.0f;
        tween90.duration24 = 0.3f;
        tween90.limit28 = 1.0f;
        // 0x543a90: the OPTION_MASTER position anchors the icons; the master itself is hidden.
        S().master = static_cast<FengObject *>(FngFindObject(owner->package4, FngHash("OPTION_MASTER")));
        object14 = NativeFrontendPackage(owner->package4)->Find("LOCK");
        object18 = NativeFrontendPackage(owner->package4)->Find("BUTTON_MASTER");
        new (&scrollBar38) ScrollerScrollBar(owner->package4, "ScrollBar", 0, 0, 0);
        if (S().master) {
            masterD4 = S().master->position[0];
            masterD8 = S().master->position[1];
            FengPackage::Hide(S().master);
        }
    }
    ~NativeIconScroller() {
        for (size_t i = 0; i < S().entries.size(); ++i) delete S().entries[i].icon;
        States().erase(this);
    }

    void UnknownSlot00() { NativeTrap("IconScroller scalar destructor 0x543bf0"); }
    void UnknownSlot04() { UpdateLayout(); }  // 0x538940
    HudVisibilityObject *UnknownSlot08(IconScrollerIcon *icon) {
        HudVisibilityObject *object = InsertIcon(icon);
        if (object && icon) {
            Entry entry = {icon, static_cast<FengObject *>(object)};
            S().entries.push_back(entry);
            if (S().selected < 0) S().selected = 0;
        }
        return object;
    }
    void UnknownSlot0C(unsigned int message, int value8, int valueC, int value10) { Dispatch(message, value8, valueC, value10); }
    bool UnknownSlot10(int index) { return SelectIndex(index); }
    bool UnknownSlot14(IconScrollerIcon *icon) {
        bool accepted = SelectIcon(icon);
        if (accepted) S().selected = selectedC8 - 1;
        return accepted;
    }
    void UnknownSlot18() { NativeTrap("IconScroller slot 0x538b80"); }
    void UnknownSlot1C() { NativeTrap("IconScroller slot 0x51f870"); }
    void UnknownSlot20(int index) { ResetLayout(index); }

    bool Move(int direction) {
        IconScrollerIcon *old = currentC;
        if (direction == -1) Previous();
        else if (direction == 1) Next();
        return currentC != old;
    }

    // 0x50d980: React only when the message names the selected icon's object.
    bool Pointer(float x, float y) {
        FengPackage *package = NativeFrontendPackage(S().owner->package4);
        for (size_t i = 0; package && i < S().entries.size(); ++i) {
            if (!S().entries[i].icon->value20 && package->Contains(S().entries[i].object, x, y)) {
                if (S().selected != (int)i) { UnknownSlot14(S().entries[i].icon); S().owner->SelectedIconChanged(); }
                return true;
            }
        }
        return false;
    }

    void React(unsigned int message, int value8, int valueC, int value10) {
        Dispatch(message, value8, valueC, value10);
    }

    IconScrollerIcon *Selected() { return currentC; }
    FengObject *SelectedObject() { return S().selected >= 0 ? S().entries[S().selected].object : 0; }
    int SelectedIndex() { return IndexOf(currentC); }
    bool Empty() { return S().entries.empty(); }

};

NativeIconScroller *ScrollerOf(IconScrollerFNGObject *object) {
    return static_cast<NativeIconScroller *>(object->Scroller());
}

}  // namespace

// ---- Shared frontend seams (NativeFrontend.h) ------------------------------------------------

FengPackage *NativeFrontendPackage(const char *package) {
    NativeScreen *screen = FindScreen(package);
    return screen && screen->shown ? screen->package : 0;
}

void NativeFrontendSetFocus(const char *package, FengObject *object) {
    NativeScreen *screen = FindScreen(package);
    if (screen) screen->fePackage.SetFocus(&nativeFengQueue, object, 1);
}

std::string NativeFrontendString(unsigned int hash) { return Localised(hash); }

// Static-record lookup, archive storage and queue dispatch remain native seams.
FngRecord *FngFindRecord(unsigned int hash) {
    for (size_t i = 0; i < kScreenCount; ++i) {
        if (screens[i]->loaded && FengHash(screens[i]->name) == hash) return &screens[i]->record;
    }
    return 0;
}
void FEngQueue::Post(unsigned int message, void *sender, FEPackage *package, int target, int) {
    for (size_t i = 0; i < kScreenCount; ++i) {
        if (&screens[i]->fePackage != package) continue;
        if (target == -4) NativeTrap("FEngQueue::Post target -4");
        int route = target == -1 ? kTargetGame : target == -3 ? kTargetPackage :
                    target == -5 ? kTargetInput : kTargetObject;
        ::Post(screens[i], message, static_cast<FengObject *>(sender), route,
               route == kTargetObject ? reinterpret_cast<FengObject *>(target) : 0);
        return;
    }
}

// Explicit unrecovered package operations: current demo response fixtures do not
// exercise these commands. Keep their call boundaries, without invented behavior.
void FEngQueue::Call51AF50(FEPackage *, int, unsigned int) { NativeTrap("FEngQueue 0x51af50"); }
void FEngQueue::Call51AB20(const char *, FEPackage *) { NativeTrap("FEngQueue 0x51ab20"); }
void FEngQueue::Call51AC10(const char *, FEPackage *, unsigned int) { NativeTrap("FEngQueue 0x51ac10"); }
void FEngQueue::Call51ADB0(FEPackage *) { NativeTrap("FEngQueue 0x51adb0"); }
void FEngQueue::Call51B070(const char *) { NativeTrap("FEngQueue 0x51b070"); }

// ---- IconScroller base seams --------------------------------------------------------------

IconScrollerIcon::~IconScrollerIcon() {}  // 0x527770

// 0x552330 / 0x551de0: package name from data[0], help-bar mask +0x20 from data[3].
IconScrollerFNGObject::IconScrollerFNGObject(void *data) {
    const unsigned int *words = static_cast<const unsigned int *>(data);
    package4 = reinterpret_cast<const char *>(words[0]);
    memset(unidentified8, 0, sizeof(unidentified8));
    value20 = (int)words[3];
    memset(unidentified24, 0, sizeof(unidentified24));
    value13C = 0;
    value140 = value144 = value148 = 0;
    inputStopped14C = 0;
    memset(unidentified14D, 0, sizeof(unidentified14D));
    typedef char scroller_fits[sizeof(NativeIconScroller) <= sizeof(scroller4C) ? 1 : -1];
    (void)sizeof(scroller_fits);
    new (scroller4C) NativeIconScroller(this);
    // Original 0x552330 binds these through named type-2 string lookup.
    title134 = FngFindNamedString(package4, "ICON_TITLE");
    description138 = FngFindNamedString(package4, "");
}

IconScrollerFNGObject::~IconScrollerFNGObject() {  // 0x4defb0
    ScrollerOf(this)->~NativeIconScroller();
}

int IconScrollerFNGObject::UnknownSlot04(int) {
    NativeTrap("IconScrollerFNGObject slot 0x40d080");
    return 0;
}

int IconScrollerFNGObject::UnknownSlot0C(int, int, int, int, int) {
    NativeTrap("IconScrollerFNGObject slot 0x570a70");
    return 0;
}

// 0x543d40: the messages the native demo produces. Sounds, Dim/UNDIM scripts and the
// vertical/other cases are not modelled.
void IconScrollerFNGObject::MessageHandler(unsigned int message, int value8, int valueC, int value10) {
    NativeIconScroller *scroller = ScrollerOf(this);
    switch (message) {
    case kFrontendTick:
        scroller->UpdateLayout();
        break;
    case kPadLeft:
    case kPadRight:
        if (scroller->Empty() || !scroller->value2F) return;
        if (scroller->Move(message == kPadLeft ? -1 : 1)) {
            SelectedIconChanged();
            FngSetPackageValue10(package4, scroller->SelectedIndex());
        }
        break;
    case kAccept: {
        IconScrollerIcon *icon = scroller->Selected();
        if (scroller->Empty() || !scroller->value2F || !icon || icon->value20) return;
        if (!icon->value45) {
            StopInput();
        } else {
            scroller->React(kAccept, value8, valueC, value10);
        }
        Value50DA20(kAccept, value8, valueC, value10);
        break;
    }
    case kPadBack:
        Value50DA20(kPadBack, value8, valueC, value10);
        StopInput();
        break;
    case kExitComplete: {
        IconScrollerIcon *icon = scroller->Selected();
        if (scroller->Empty() || !icon || icon->value45) return;
        scroller->React(value13C, value140, value144, value148);
        break;
    }
    case kStopDim:
        StartInput();
        break;
    }
}

// ---- FEng seams (cdecl interfaces declared in PauseMenu.h / PauseOptions.h) -----------------

unsigned int FngHash(const char *name) { return FengHash(name); }  // 0x505450

void *FngFindObject(const char *package, unsigned int hash) {  // native compatibility adapter
    NativeScreen *screen = FindScreen(package);
    return screen ? screen->package->Find(hash) : 0;
}

// 0x537be0: formats into the string object (the options use it with literal strings only).
void FngPrintf(void *object, const char *format, ...) {
    if (object) FengPackage::SetText(static_cast<FengObject *>(object), format);
}

void FngHideObject(void *object) { FengPackage::Hide(static_cast<FengObject *>(object)); }  // 0x50ca00
void FngShowObject(void *object) { FengPackage::Show(static_cast<FengObject *>(object)); }  // 0x50ca50

// 0x50d150: menu sound from table 0x837fe0; there is no frontend audio in the demo.
void FngPlayMenuSound(const char *, int, int) {}

// The PC help bar (UI_PC_Help_Bar.fng) is tracked but not drawn: PCHelpBarFNGObject
// 0x54e6a0/0x54e6e0 is not recovered.
const char *FngHelpBarName() { return "UI_PC_Help_Bar.fng"; }  // 0x50cea0

bool FngIsLoaded(const char *package) {  // 0x52cf60
    if (FengHash(package) == FengHash(FngHelpBarName())) return helpBarLoaded;
    NativeScreen *screen = FindScreen(package);
    return screen && screen->shown;
}

// 0x5379a0: removing a screen package closes it after the current message.
void FngRemove(const char *package) {
    if (FengHash(package) == FengHash(FngHelpBarName())) {
        helpBarLoaded = false;
        return;
    }
    NativeScreen *screen = FindScreen(package);
    if (screen && screen->shown) removeScreen = screen;
}

void FngAdd(const char *package, const char *, int, int) {  // 0x555f10
    if (FengHash(package) == FengHash(FngHelpBarName())) helpBarLoaded = true;
}

// 0x537980: replaces the calling package with `name`. Screens without recovered code (replay,
// PC customize options, performance dyno) return to the game.
void FngShow(const char *from, const char *name, int) {
    NativeScreen *target = FindScreen(name);
    if (!target || !target->factory) {
        fprintf(stderr, "ug2demo: %s is not available in the native demo; resuming\n", name);
        request = kPauseRequestResume;
        return;
    }
    switchFrom = FindScreen(from);
    switchTo = target;
}

// 0x55dda0 -> 0x55b5c0: straight to the package's FNG object.
void FngSendMessage(unsigned int message, const char *package) {
    NativeScreen *screen = FindScreen(package);
    if (screen) Post(screen, message, 0, kTargetGame);
}

void FngShowDialog(const char *package, const char *, int value8, unsigned int valueC, unsigned int value10,
                   unsigned int value14, unsigned int value18, int value1C, unsigned int value20) {  // 0x558790
    if (!dialogScreen.loaded) {
        NativeTrap("UI_InGameDialog.fng is not loaded");
        return;
    }
    std::string error;
    if (!dialogScreen.Reset(&error)) NativeTrap("UI_InGameDialog.fng reload");
    if (Debug()) fprintf(stderr, "pause: dialog prompt %08x\n", value20);
    dialog.shown = true;
    dialog.parent = FindScreen(package);
    dialog.results[0] = value10;
    dialog.results[1] = value14;
    dialog.backResult = value18;
    dialog.selected = value1C == 0 ? 0 : 1;
    dialogScreen.shown = true;
    RefreshNativePackageChain();
    // Two-button form: the centre button and the memory-card header are not used here.
    const unsigned int hidden[] = {FengHash("Button3"), 0xda473d31, FengHash("Header_text_02"), 0xc26a8d5c};
    for (size_t i = 0; i < sizeof(hidden) / sizeof(hidden[0]); ++i) {
        FengObject *object = DialogObject(hidden[i]);
        if (object) FengPackage::Hide(object);
    }
    FngSetLabelString(DialogObject(FengHash("message_blurb")), value20);
    FngSetLabelString(DialogObject(0xda473d2f), (unsigned int)value8);
    FngSetLabelString(DialogObject(0xda473d30), valueC);
    DialogHighlight();
}

void FngSetReturnPackage(const char *package) { returnPackage = package; }  // 0x50c8d0

int ControllerAssigned(int) { return 1; }  // 0x571a60: the keyboard is always assigned

int FngFilterSlot0C(int, int) {  // 0x50d1d0
    NativeTrap("FEng slot filter 0x50d1d0");
    return 0;
}

// ---- Game flow and other seams ------------------------------------------------------------

// 0x5f4240. Message meanings are not established; the demo maps the ones the menus queue.
void GameFlow8900D8::Queue8(int value) {
    if (Debug()) fprintf(stderr, "pause: game flow 0x5f4240(%d)\n", value);
    switch (value) {
    case 1:  // PAD_BACK/START after EXIT_COMPLETE, "Resume Race", options closed with START
    case 2:  // "Return to Free Run"
        request = kPauseRequestResume;
        break;
    case 3:  // "Restart" dialog result
        request = kPauseRequestRestart;
        break;
    case 6:  // QuitToProfile / QuitToCrib
        request = kPauseRequestQuit;
        break;
    default:
        fprintf(stderr, "ug2demo: game-flow message %d is not modelled; resuming\n", value);
        request = kPauseRequestResume;
        break;
    }
}

void GameFlow8900D8::Queue(int type, int value) {  // 0x5f41e0
    fprintf(stderr, "ug2demo: game-flow message %d/%d is not modelled; resuming\n", type, value);
    request = kPauseRequestResume;
}

void PCHelpBarFNGObject::Show() {}                    // 0x54e6a0, not drawn
void PCHelpBarFNGObject::SyncByMask(int, const char *) {}  // 0x54e6e0, not drawn

void Career85AD40::Value513B30() { NativeTrap("career 0x513b30"); }
void Career85AD40::Value5139C0() { NativeTrap("career 0x5139c0"); }
// 0x502ff0: the front-end return package is not modelled (the demo has no front end).
bool Network89CF48::Value5F0120() {
    NativeTrap("network 0x5f0120");
    return false;
}

// ---- NativePause ----------------------------------------------------------------------------

NativePause::NativePause() {
    memset(&dialog, 0, sizeof(dialog));
    memset(&nativeFlow, 0, sizeof(nativeFlow));
    memset(&nativeReplay, 0, sizeof(nativeReplay));
}

NativePause::~NativePause() { CloseAll(); }

bool NativePause::Load(const char *gameDirectory, NativeTextureCache *cache, const char *fontPath, char *error,
                       int errorSize) {
    fengManager8384C4 = &nativeFengManager;
    fengValue8384D0 = 0; // Native demo uses the live package collections.
    textures = cache;
    std::string base = gameDirectory;
    std::vector<unsigned char> bundle, global;
    if (!ReadNativeFile(base + "/GLOBAL/InGameCommon.bun", &bundle)) {
        snprintf(error, errorSize, "cannot read InGameCommon.bun");
        return false;
    }
    NativeImageMap images;
    LoadNativeTextures(&bundle[0], bundle.size(), &images);
    if (ReadNativeFile(base + "/GLOBAL/GLOBALB.BUN", &global)) LoadNativeTextures(&global[0], global.size(), &images);
    cache->Add(images);
    std::string reason;
    if (!LoadCompressedPackage(bundle, &pauseScreen, &reason) || !LoadCompressedPackage(bundle, &dialogScreen, &reason)) {
        snprintf(error, errorSize, "%s", reason.c_str());
        return false;
    }
    if (!LoadPlainPackage(bundle, &optionsMainScreen, &reason) || !LoadPlainPackage(bundle, &optionsScreen, &reason))
        fprintf(stderr, "ug2demo: %s; pause options are not available\n", reason.c_str());
    std::vector<unsigned char> frontend;
    if (ReadNativeFile(base + "/FRONTEND/FrontB.lzc", &frontend))
        LoadPlainPackage(frontend, &controlsScreen, &reason);
    if (!controlsScreen.loaded) fprintf(stderr, "ug2demo: %s; controls screen unavailable\n", reason.c_str());
    ReadNativeFile(base + "/LANGUAGES/English.bin", &language);
    BindNativeStringObserver(&language);
    if (!font.Load(gameDirectory, cache, fontPath)) fprintf(stderr, "ug2demo: no font found; pause menu text is not drawn\n");
    // Objects behind the .bss pointers the recovered code dereferences.
    gameFlow8900D8 = &nativeFlow;
    pcHelpBar8384D8 = &nativeHelpBar;
    replay8650B8 = &nativeReplay;
    RefreshNativePackageChain();
    NativePauseOptionsLoad(gameDirectory);
    return true;
}

bool NativePause::Loaded() const { return pauseScreen.loaded; }

bool NativePause::Active() const { return ActiveScreen() != 0; }

void NativePause::Open() {
    fengManager8384C4 = &nativeFengManager;
    fengValue8384D0 = 0; // Native demo uses the live package collections.
    if (!pauseScreen.loaded || ActiveScreen()) return;
    request = kPauseRequestNone;
    ShowScreen(&pauseScreen);
    Pump();
}

void NativeFrontendOpenControls() {
    if (!dialog.shown && ActiveScreen() && !controlsScreen.shown) { ShowScreen(&controlsScreen); Pump(); }
}
void NativeFrontendCloseControls() { CloseScreen(&controlsScreen); }
void NativePause::OpenControls() { NativeFrontendOpenControls(); }
bool NativePause::CaptureKey(int scancode) {
    return controlsScreen.shown && NativeControlsCapture(controlsScreen.object, (SDL_Scancode)scancode);
}

void NativePause::Key(NativePauseKey key) {
    NativeScreen *screen = ActiveScreen();
    if (!screen) return;
    if (screen == &controlsScreen) { NativeControlsKey(screen->object, key); return; }
    if (key == kPauseKeyDevice) return;
    if (dialog.shown) {
        if (key == kPauseKeyLeft || key == kPauseKeyRight) {
            dialog.selected = key == kPauseKeyLeft ? 0 : 1;
            DialogHighlight();
        } else if (key == kPauseKeyAccept || key == kPauseKeyBack) {
            unsigned int result = key == kPauseKeyAccept ? dialog.results[dialog.selected] : dialog.backResult;
            NativeScreen *parent = dialog.parent;
            CloseDialog();
            if (parent) Post(parent, result, 0, kTargetGame);
        }
    } else {
        static const unsigned int messagesForKey[] = {kPadLeft, kPadRight, kAccept, kPadBack, kPadUp, kPadDown, kPadStart};
        Input(screen, messagesForKey[key]);
    }
    Pump();
}

void NativePause::Pointer(float x, float y, bool click, bool drag) {
    NativeScreen *screen = ActiveScreen();
    if (!screen) return;
    if (screen == &controlsScreen) { NativeControlsPointer(screen->object, x, y, click); return; }
    if (dialog.shown) {
        // Native text hit regions; dialog labels have glyph scales rather than image bounds.
        const unsigned int labels[2] = {0xda473d2f, 0xda473d30};
        for (int i = 0; i < 2; ++i) {
            FengObject *label = DialogObject(labels[i]);
            if (label && x >= label->position[0] - 55 && x <= label->position[0] + 55 &&
                y >= label->position[1] - 12 && y <= label->position[1] + 18) {
                dialog.selected = i; DialogHighlight();
                if (click) Key(kPauseKeyAccept);
                return;
            }
        }
    } else if (screen->optionsList) {
        NativePauseOptionsPointer(screen->object, x, y, click, drag);
        Pump();
    } else if (click && screen->object && ScrollerOf(static_cast<IconScrollerFNGObject *>(screen->object))->Pointer(x, y)) {
        if (click) Key(kPauseKeyAccept);
    }
}

// Native event delivery seam: observe the script header selected by recovered code.
// Retain completed cursors until selection changes, so a finished script is not replayed.
void SynchronizeNativeScriptEvents() {
    for (size_t n = 0; n < kScreenCount; ++n) {
        NativeScreen *screen = screens[n];
        if (!screen->shown) continue;
        const std::vector<FengObject *> &objects = screen->package->Objects();
        for (size_t j = 0; j < objects.size(); ++j) {
            FengObject *object = objects[j];
            size_t found = scripts.size();
            for (size_t k = 0; k < scripts.size(); ++k)
                if (scripts[k].object == object) { found = k; break; }
            if (found < scripts.size() && &scripts[found].script->header != object->currentScript54) {
                scripts.erase(scripts.begin() + found);
                found = scripts.size();
            }
            if (!object->currentScript54) continue;
            if (found == scripts.size()) {
                RunningScript running = {screen, object, reinterpret_cast<FengScript *>(object->currentScript54),
                                         (float)object->currentScript54->time10, 0, false};
                scripts.push_back(running);
            } else if (!object->currentScript54->time10 && scripts[found].milliseconds > 0.0f) {
                scripts[found].milliseconds = 0.0f;
                scripts[found].nextEvent = 0;
                scripts[found].finished = false;
            }
        }
    }
}

void NativePause::Update(float seconds) {
    frontendDelta865198 = seconds;
    // Real-time clock seam 0x8651ac (4 kHz units assumed); AcceptsInput (0x4a7bc0) waits for
    // it to change after the screen opens.
    clockRemainder += seconds * 4000.0f;
    int ticks = (int)clockRemainder;
    if (ticks < 1) ticks = 1;
    clockRemainder -= (float)ticks;
    if (clockRemainder < 0.0f) clockRemainder = 0.0f;
    realClock8651AC += ticks;
    SynchronizeNativeScriptEvents();
    for (size_t i = 0; i < scripts.size(); ++i) {
        RunningScript &running = scripts[i];
        if (running.finished) continue;
        running.milliseconds += seconds * 1000.0f;
        const std::vector<FengEvent> &events = running.script->events;
        while (running.nextEvent < events.size() && events[running.nextEvent].time <= running.milliseconds) {
            if (events[running.nextEvent].target == 0xffffffff)
                Post(running.screen, events[running.nextEvent].message, 0, kTargetGame);
            ++running.nextEvent;
        }
        if (running.nextEvent >= events.size() && running.milliseconds >= running.script->duration)
            running.finished = true;
    }
    for (size_t i = 0; i < kScreenCount; ++i) {
        if (!screens[i]->shown) continue;
        screens[i]->package->AdvanceScripts(seconds);
        if (screens[i] != &dialogScreen) Post(screens[i], kFrontendTick, 0, kTargetGame);  // 0x5663b0
    }
    Pump();
    NativePauseOptionsUpdate();
    if (controlsScreen.shown) NativeControlsUpdate(controlsScreen.object);
}

void NativePause::Draw(int width, int height) {
    if (controlsScreen.shown) { controlsScreen.package->Draw(textures, &font, width, height); return; }
    for (size_t i = 0; i < kScreenCount; ++i) {
        if (screens[i]->shown) screens[i]->package->Draw(textures, &font, width, height);
    }
}

NativePauseRequest NativePause::TakeRequest() {
    NativePauseRequest pending = request;
    request = kPauseRequestNone;
    if (pending != kPauseRequestNone) CloseAll();
    return pending;
}
