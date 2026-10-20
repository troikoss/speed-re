#include "EngineSoundSpatial.h"
#include "EngineSoundPhysics.h"
#include "../../Audio/AudioSystem.h"
#include "../../Camera/Camera.h"
#include "../../Hud/HudPlayerMasks.h"
extern Audio82B884 *audio82B884;
extern float ginsuOne784250;
double bDistance(WheelVector *, WheelVector *);
unsigned short HudAngleUnits(float, float);

// The repeated searches below are present in 0x46d7e0. No caller boundary
// exists at these inline list walks; the separate 0x4690b0 calls stay separate.
static EngineSoundCar *FindInList(AudioManagedCarSound *head, int index) {
    for (; head; head = head->next4) {
        EngineSoundCar *car = reinterpret_cast<EngineSoundCar *>(head);
        if (car->bankIndex10 == index) return car;
    }
    return 0;
}

// 0x46d7e0: listener readiness is not cleared on missing owner/player/camera
// paths. The second listener's blend can still execute without its camera.
void EngineSoundUpdateListeners46D7E0() {
    MoverModeOwner *firstOwner = 0;
    AudioCarSoundManager *manager = audio82B884->managerAC;
    AudioManagedCarSound *head;
    PlayerRecord *player;
    float input[2];
    if (manager) {
        head = manager->first10;
        if (FindInList(head, 0)) {
            firstOwner = FindInList(head, 0)->car30;
            if (firstOwner && (player = reinterpret_cast<PlayerRecord *>(firstOwner->raceEntry18)) && player->camera238) {
                Camera *camera = *reinterpret_cast<Camera **>(reinterpret_cast<char *>(player->camera238) + 0x1c);
                engineSoundSpatialReady82BB40[0] = camera;
                engineSoundListenerCameraBEF0[0] = camera->current0.position40.x;
                engineSoundListenerCameraBEF0[1] = camera->current0.position40.y;
                engineSoundListenerCameraBEF0[2] = camera->current0.position40.z;
                engineSoundListenerDirectionC610[0] = camera->current0.direction50.x;
                engineSoundListenerDirectionC610[1] = camera->current0.direction50.y;
                engineSoundListenerDirectionC610[2] = camera->current0.direction50.z;
                engineSoundListenerTargetBE80[0] = camera->current0.target60.x;
                engineSoundListenerTargetBE80[1] = camera->current0.target60.y;
                engineSoundListenerTargetBE80[2] = camera->current0.target60.z;
                input[0] = engineSoundListenerDirectionC610[0];
                input[1] = engineSoundListenerDirectionC610[1];
                Normalize2(engineSoundSpatialBE70, input);
                engineSoundSpatialBECC[0] = engineSoundSpatialBE70[0];
                engineSoundSpatialBECC[1] = engineSoundSpatialBE70[1];
                engineSoundSpatialBF10[0] = engineSoundSpatialBF20[0] = engineSoundListenerCameraBEF0[0];
                engineSoundSpatialBF10[1] = engineSoundSpatialBF20[1] = engineSoundListenerCameraBEF0[1];
                const float *position = reinterpret_cast<float *>(reinterpret_cast<char *>(firstOwner) + 0x60);
                engineSoundSpatialBEDC[0] = position[0];
                engineSoundSpatialBEDC[1] = position[1];
                engineSoundSpatialC670[0] = engineSoundSpatialBEDC[0] - engineSoundListenerCameraBEF0[0];
                engineSoundSpatialC670[1] = engineSoundSpatialBEDC[1] - engineSoundListenerCameraBEF0[1];
                input[0] = *reinterpret_cast<float *>(reinterpret_cast<char *>(firstOwner) + 0x90);
                input[1] = *reinterpret_cast<float *>(reinterpret_cast<char *>(firstOwner) + 0x94);
                Normalize2(engineSoundSpatialC6C4, input);
                engineSoundListenerPrevious82BB50[0] = engineSoundListenerView82BB48[0];
                // The original still dereferences a null race-state pointer in
                // the following comparison; no invented null fallback is added.
                if (((!engineSoundMode890118 || *engineSoundMode890118 != 3) &&
                     *engineSoundMode890118 != 4 &&
                     (engineSoundModes827E88.value40 || engineSoundModes827E88.value28 || engineSoundModes827E88.value10)) ||
                    engineSoundOverride82A3A4 > 0) engineSoundListenerView82BB48[0] = 4;
                else if (player->state58 == 6 || player->state58 == 7) engineSoundListenerView82BB48[0] = 3;
                else if (engineSoundMode8900D8 && *engineSoundMode8900D8 == 5) engineSoundListenerView82BB48[0] = 5;
                else {
                    int value = *reinterpret_cast<int *>(reinterpret_cast<char *>(audio82B884->FindListener4690B0(0)) + 0xa8);
                    engineSoundListenerView82BB48[0] = value == 2 ? 1 : value == 4 ? 2 : 0;
                }
            }
        }
    }
    manager = audio82B884->managerAC;
    if (!manager) return;
    head = manager->first10;
    if (!FindInList(head, 1)) return;
    if (!FindInList(head, 1)) return;
    MoverModeOwner *secondOwner = FindInList(head, 1)->car30;
    if (!secondOwner) return;
    player = reinterpret_cast<PlayerRecord *>(secondOwner->raceEntry18);
    if (player && player->camera238) {
        Camera *camera = *reinterpret_cast<Camera **>(reinterpret_cast<char *>(player->camera238) + 0x1c);
        engineSoundSpatialReady82BB40[1] = camera;
        engineSoundListenerCameraBEF0[4] = camera->current0.position40.x;
        engineSoundListenerCameraBEF0[5] = camera->current0.position40.y;
        engineSoundListenerCameraBEF0[6] = camera->current0.position40.z;
        engineSoundListenerDirectionC610[4] = camera->current0.direction50.x;
        engineSoundListenerDirectionC610[5] = camera->current0.direction50.y;
        engineSoundListenerDirectionC610[6] = camera->current0.direction50.z;
        engineSoundListenerTargetBE80[4] = camera->current0.target60.x;
        engineSoundListenerTargetBE80[5] = camera->current0.target60.y;
        engineSoundListenerTargetBE80[6] = camera->current0.target60.z;
        input[0] = engineSoundListenerDirectionC610[4];
        input[1] = engineSoundListenerDirectionC610[5];
        Normalize2(engineSoundSpatialBE70 + 2, input);
        engineSoundSpatialBECC[2] = engineSoundSpatialBE70[2];
        engineSoundSpatialBECC[3] = engineSoundSpatialBE70[3];
        engineSoundSpatialBF10[2] = engineSoundSpatialBF20[2] = engineSoundListenerCameraBEF0[4];
        engineSoundSpatialBF10[3] = engineSoundSpatialBF20[3] = engineSoundListenerCameraBEF0[5];
        const float *position = reinterpret_cast<float *>(reinterpret_cast<char *>(secondOwner) + 0x60);
        engineSoundSpatialBEDC[2] = position[0];
        engineSoundSpatialBEDC[3] = position[1];
        engineSoundSpatialC670[2] = engineSoundSpatialBEDC[2] - engineSoundListenerCameraBEF0[4];
        engineSoundSpatialC670[3] = engineSoundSpatialBEDC[3] - engineSoundListenerCameraBEF0[5];
        input[0] = *reinterpret_cast<float *>(reinterpret_cast<char *>(secondOwner) + 0x90);
        input[1] = *reinterpret_cast<float *>(reinterpret_cast<char *>(secondOwner) + 0x94);
        Normalize2(engineSoundSpatialC6C4 + 2, input);
        engineSoundListenerPrevious82BB50[1] = engineSoundListenerView82BB48[1];
        if (player->state58 == 6 || player->state58 == 7) engineSoundListenerView82BB48[1] = 3;
        else {
            int value = *reinterpret_cast<int *>(reinterpret_cast<char *>(audio82B884->FindListener4690B0(1)) + 0xa8);
            engineSoundListenerView82BB48[1] = value == 2 ? 1 : value == 4 ? 2 : 0;
        }
    }
    const double distance = bDistance(reinterpret_cast<WheelVector *>(reinterpret_cast<char *>(firstOwner) + 0x60),
                                      reinterpret_cast<WheelVector *>(reinterpret_cast<char *>(secondOwner) + 0x60));
    const double range = (double)engineSoundListenerRange7ED12C - engineSoundListenerMargin7ED130;
    if (distance < range) {
        volatile float ratio = (float)(distance / range);
        volatile float dx = engineSoundSpatialBE70[2] - engineSoundSpatialBE70[0];
        volatile float dy = engineSoundSpatialBE70[3] - engineSoundSpatialBE70[1];
        volatile float nx = dx * engineSoundListenerNegative7842A0;
        volatile float ny = dy * engineSoundListenerNegative7842A0;
        volatile float px = engineSoundSpatialBF10[2] - engineSoundSpatialBF10[0];
        volatile float py = engineSoundSpatialBF10[3] - engineSoundSpatialBF10[1];
        volatile float qx = engineSoundSpatialBF10[0] - engineSoundSpatialBF10[2];
        volatile float qy = engineSoundSpatialBF10[1] - engineSoundSpatialBF10[3];
        volatile float rx = engineSoundSpatialC670[2] - engineSoundSpatialC670[0];
        volatile float ry = engineSoundSpatialC670[3] - engineSoundSpatialC670[1];
        volatile float sx = rx * engineSoundListenerNegative7842A0;
        volatile float sy = ry * engineSoundListenerNegative7842A0;
        HudAngleUnits(engineSoundSpatialBE70[1], engineSoundSpatialBE70[0]);
        HudAngleUnits(engineSoundSpatialBE70[3], engineSoundSpatialBE70[2]);
        const double factor = (double)ginsuOne784250 - ratio;
        dx = (float)(dx * factor);
        const double scaledY = dy * factor;
        nx = (float)(nx * factor); ny = (float)(ny * factor);
        px = (float)(px * factor); py = (float)(py * factor);
        qx = (float)(qx * factor); qy = (float)(qy * factor);
        rx = (float)(rx * factor); ry = (float)(ry * factor);
        sx = (float)(sx * factor); sy = (float)(sy * factor);
        engineSoundSpatialBECC[0] = dx + engineSoundSpatialBECC[0];
        engineSoundSpatialBECC[1] = (float)(engineSoundSpatialBECC[1] + scaledY);
        engineSoundSpatialBECC[2] = nx + engineSoundSpatialBECC[2];
        engineSoundSpatialBECC[3] = engineSoundSpatialBECC[3] + ny;
        engineSoundSpatialBF20[0] = px + engineSoundSpatialBF20[0];
        engineSoundSpatialBF20[1] = engineSoundSpatialBF20[1] + py;
        engineSoundSpatialBF20[2] = qx + engineSoundSpatialBF20[2];
        engineSoundSpatialBF20[3] = engineSoundSpatialBF20[3] + qy;
        engineSoundListenerBlendBF34[0] = rx + engineSoundListenerBlendBF34[0];
        engineSoundListenerBlendBF34[1] = engineSoundListenerBlendBF34[1] + ry;
        engineSoundListenerBlendBF34[2] = sx + engineSoundListenerBlendBF34[2];
        engineSoundListenerBlendBF34[3] = engineSoundListenerBlendBF34[3] + sy;
        HudAngleUnits(engineSoundSpatialBE70[1], engineSoundSpatialBE70[0]);
        HudAngleUnits(engineSoundSpatialBE70[3], engineSoundSpatialBE70[2]);
    }
}
