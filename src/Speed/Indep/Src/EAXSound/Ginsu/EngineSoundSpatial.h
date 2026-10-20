#ifndef UG2_ENGINE_SOUND_SPATIAL_H
#define UG2_ENGINE_SOUND_SPATIAL_H
#include "EngineSound.h"

extern void *engineSoundSpatialReady82BB40[2];
extern unsigned int engineSoundSpatialMode82BC14;
extern float *engineSoundSpatialDirection82BB78;
extern float *engineSoundSpatialPoint82BB7C;
extern unsigned short engineSoundSpatialAngle82BB80;
extern float engineSoundSpatialBE70[], engineSoundSpatialBECC[], engineSoundSpatialBEDC[];
extern float engineSoundSpatialBF10[], engineSoundSpatialBF20[], engineSoundSpatialC670[], engineSoundSpatialC6C4[];
extern float engineSoundDoppler7887FC;
void Normalize2(float *output, float *input);
unsigned short HudArcSineUnits(float ratio);
float EngineSoundDistance2D401150(float *first, float *second);
EngineSoundSpatialComponent *CreateEngineSoundSpatial47C7A0();
void EngineSoundUpdateListeners46D7E0();
extern int engineSoundListenerView82BB48[2], engineSoundListenerPrevious82BB50[2];
extern float engineSoundListenerCameraBEF0[], engineSoundListenerDirectionC610[];
extern float engineSoundListenerTargetBE80[], engineSoundListenerBlendBF34[];
extern float engineSoundListenerRange7ED12C, engineSoundListenerMargin7ED130;
extern float engineSoundListenerNegative7842A0;
#endif
