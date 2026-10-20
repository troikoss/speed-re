#include <SDL2/SDL.h>
#include <stdio.h>
#include "../../Speed/Indep/Src/EAXSound/Ginsu/EngineSound.h"

// Asset-free integration of the recovered traversal and the native mutex seam.
static EngineParameterDispatcher dispatcher;
static EngineParameterCallback nodes[3];
static SDL_sem *entered;
static SDL_sem *finished;
static int payload, calls, failed;

static void __cdecl Nested(void *values, void *data) {
    if (values != &payload || data != &nodes[2]) failed = 1;
    ++calls;
}

static void __cdecl First(void *values, void *data) {
    if (values != &payload || data != &nodes[0]) failed = 1;
    ++calls;
    // Native SDL mutexes must permit a callback to reenter dispatch.
    EngineParameterDispatcher nested = {};
    nested.first8 = &nodes[2];
    if (nested.Dispatch735580(values) != 0) failed = 1;
    nodes[0].next0 = 0; // the original loop reads this after the callback
}

static void __cdecl Blocked(void *values, void *data) {
    if (values != &payload || data != &nodes[1]) failed = 1;
    SDL_SemPost(entered);
}

static int Worker(void *) {
    const int result = dispatcher.Dispatch735580(&payload);
    SDL_SemPost(finished);
    return result;
}

int main() {
    engineSoundMutex8B4F38 = SDL_CreateMutex();
    entered = SDL_CreateSemaphore(0);
    finished = SDL_CreateSemaphore(0);
    if (!engineSoundMutex8B4F38 || !entered || !finished) return 1;
    nodes[0].next0 = &nodes[1];
    nodes[0].callback8 = First;
    nodes[0].dataC = &nodes[0];
    nodes[1].callback8 = Blocked;
    nodes[1].dataC = &nodes[1];
    nodes[2].callback8 = Nested;
    nodes[2].dataC = &nodes[2];
    dispatcher.first8 = &nodes[0];
    if (dispatcher.Dispatch735580(&payload) != 0 || calls != 2 || failed || SDL_SemValue(entered)) return 2;
    dispatcher.first8 = &nodes[1];
    if (EngineDispatchWait(engineSoundMutex8B4F38, 0xffffffffu) != 0) return 3;
    SDL_Thread *worker = SDL_CreateThread(Worker, "dispatch", 0);
    if (!worker) return 4;
    // While the main thread owns the mutex the worker cannot enter its callback.
    if (SDL_SemWaitTimeout(entered, 50) != SDL_MUTEX_TIMEDOUT) return 5;
    if (EngineDispatchRelease(engineSoundMutex8B4F38) != 1) return 6;
    if (SDL_SemWaitTimeout(finished, 2000) != 0) return 7;
    int result = -1;
    SDL_WaitThread(worker, &result);
    if (result != 0 || failed || SDL_SemTryWait(entered) != 0) return 8;
    dispatcher.first8 = 0;
    if (dispatcher.Dispatch735580(&payload) != 0) return 9;
    SDL_DestroyMutex(static_cast<SDL_mutex *>(engineSoundMutex8B4F38));
    engineSoundMutex8B4F38 = 0;
    SDL_DestroySemaphore(entered);
    SDL_DestroySemaphore(finished);
    puts("audio dispatch: empty, callback mutation, nested dispatch and mutex contention passed");
    return 0;
}
