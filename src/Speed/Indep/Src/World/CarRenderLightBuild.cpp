#include "CarRenderLightBuild.h"

float EngineSine(float angle); // 0x43c350

namespace {
// Retain original binary32 constant loads in C++11 x87 expressions.
const volatile float light784E8C = 90.0f;
const volatile float light784510 = 180.0f;
const volatile float light7D7D3C = 0.0f;
const volatile float light784544 = 255.0f;
const volatile float light784260 = 0.5f;
const volatile float light784254 = 1.5707963705062866f;
const volatile float light784250 = 1.0f;
const volatile float light78D77C = 0.75f;
const volatile float light784548 = 0.003921568859368563f;
const volatile float light78D9B4 = 0.54f;
const volatile float light78D9B8 = 0.17f;
const volatile float light78D9BC = 0.29f;
}

// Original 0x48ed60. Arrays have 19 four-lane entries. The selector's count is
// clamped to 15 (not 16) if count+3 exceeds 19, exactly as at 0x48ef54..0x48ef6b.
unsigned int CarRenderBuildLightRecord(CarRenderRecord *output, const CarRenderLightProfile *profile,
                                      const float *position, WheelMatrix *matrixArg4,
                                      WheelMatrix *matrixArg5, const CarRenderView *view) {
    if (!output) {
        return 0xffffffff;
    }
    if (view && view->frame40) {
        matrixArg5 = (WheelMatrix *)view->frame40;
    }
    WheelMatrix basis;
    CarRenderDirectionMatrix(&basis, matrixArg5);
    float directions[19][4] = {{0.0f}};
    float normalized[19][4] = {{0.0f}};
    float colors[19][4] = {{0.0f}};
    for (int i = 0; i < 3; ++i) {
        const CarRenderProfileLight &light = profile->lights20[i];
        volatile float polar = static_cast<long double>(light784E8C) - light.angle08;
        volatile float azimuth = static_cast<long double>(light.angle04) + light784510;
        CarRenderLightVector(directions[i], azimuth, polar, 1.0f);
        if (light.flag00) {
            const float x = directions[i][0], y = -directions[i][1], z = -directions[i][2];
            volatile float tx = ((static_cast<long double>(basis.elements[8]) * x
                               + static_cast<long double>(basis.elements[4]) * z)
                               + static_cast<long double>(basis.elements[0]) * y)
                               + static_cast<long double>(basis.elements[12]) * light7D7D3C;
            const long double ty = ((static_cast<long double>(basis.elements[9]) * x
                                  + static_cast<long double>(basis.elements[5]) * z)
                                  + static_cast<long double>(basis.elements[1]) * y)
                                  + static_cast<long double>(basis.elements[13]) * light7D7D3C;
            volatile float tz = ((static_cast<long double>(basis.elements[10]) * x
                               + static_cast<long double>(basis.elements[6]) * z)
                               + static_cast<long double>(basis.elements[2]) * y)
                               + static_cast<long double>(basis.elements[14]) * light7D7D3C;
            directions[i][0] = tx;
            directions[i][1] = ty;
            directions[i][2] = tz;
        }
        const float intensity = light.intensity18;
        for (int lane = 0; lane < 3; ++lane) {
            colors[i][lane] = (static_cast<long double>(intensity) * light.color0C[lane]) * light784544;
        }
    }
    CarRenderLightSelection selection;
    int count = CarRenderSelectLights(&selection, position);
    if (count + 3 > 19) {
        count = 15;
    }
    const float px = position[0], py = position[1], pz = position[2];
    for (int i = 0; i < count; ++i) {
        const CarRenderLight *light = selection.lights00[i];
        directions[i + 3][0] = static_cast<long double>(light->position10[0]) - px;
        directions[i + 3][1] = static_cast<long double>(light->position10[1]) - py;
        directions[i + 3][2] = static_cast<long double>(light->position10[2]) - pz;
        const float r = static_cast<float>(light->color0C & 0xff);
        const float g = static_cast<float>((light->color0C >> 8) & 0xff);
        const float b = static_cast<float>((light->color0C >> 16) & 0xff);
        if (light->kind04 == 3) {
            volatile float angle = static_cast<long double>(light->angle34) * light784260 + light784254;
            volatile float threshold = EngineSine(angle);
            WheelVector offset;
            offset.x = static_cast<long double>(position[0]) - light->position10[0];
            offset.y = static_cast<long double>(position[1]) - light->position10[1];
            offset.z = static_cast<long double>(position[2]) - light->position10[2];
            WheelNormalize(&offset, &offset);
            const long double dot = (static_cast<long double>(offset.z) * light->direction20[2]
                                   + static_cast<long double>(offset.y) * light->direction20[1])
                                   + static_cast<long double>(offset.x) * light->direction20[0];
            const long double cone = dot > threshold
                ? (dot - threshold) / (static_cast<long double>(light784250) - threshold)
                : static_cast<long double>(light7D7D3C);
            const long double weight = cone * selection.weights60[i];
            colors[i + 3][0] = (static_cast<long double>(r) * weight) * light78D77C;
            colors[i + 3][1] = (static_cast<long double>(g) * weight) * light78D77C;
            colors[i + 3][2] = (weight * b) * light78D77C;
        } else {
            colors[i + 3][0] = (static_cast<long double>(r) * light78D77C) * selection.weights60[i];
            colors[i + 3][1] = (static_cast<long double>(g) * light78D77C) * selection.weights60[i];
            colors[i + 3][2] = (static_cast<long double>(b) * light78D77C) * selection.weights60[i];
        }
    }
    if (matrixArg4) {
        WheelMatrix transform;
        CarRenderDirectionMatrix(&transform, matrixArg4);
        for (int i = 0; i < count + 3; ++i) {
            const float x = directions[i][0], y = directions[i][1], z = directions[i][2];
            const long double tx = ((static_cast<long double>(transform.elements[8]) * z
                                  + static_cast<long double>(transform.elements[4]) * y)
                                  + static_cast<long double>(transform.elements[0]) * x) + transform.elements[12];
            const long double ty = ((static_cast<long double>(transform.elements[9]) * z
                                  + static_cast<long double>(transform.elements[5]) * y)
                                  + static_cast<long double>(transform.elements[1]) * x) + transform.elements[13];
            volatile float tz = ((static_cast<long double>(transform.elements[10]) * z
                               + static_cast<long double>(transform.elements[6]) * y)
                               + static_cast<long double>(transform.elements[2]) * x) + transform.elements[14];
            directions[i][2] = tz;
            directions[i][0] = tx;
            directions[i][1] = ty;
        }
    }
    count += 3;
    for (int i = 0; i < count; ++i) {
        WheelNormalize(reinterpret_cast<WheelVector *>(normalized[i]), reinterpret_cast<WheelVector *>(directions[i]));
        for (int lane = 0; lane < 4; ++lane) {
            colors[i][lane] = static_cast<long double>(colors[i][lane]) * light784548;
        }
    }
    for (int i = 0; i < count; ++i) {
        colors[i][3] = (static_cast<long double>(colors[i][0]) * light78D9BC
                     + static_cast<long double>(colors[i][2]) * light78D9B8)
                     + static_cast<long double>(colors[i][1]) * light78D9B4;
    }
    for (int i = 0; i < 3; ++i) {
        for (int lane = 0; lane < 3; ++lane) {
            output->matrix40.elements[lane * 4 + i] = normalized[i][lane];
        }
    }
    for (int i = 0; i < 3; ++i) {
        for (int lane = 0; lane < 3; ++lane) {
            output->matrix00.elements[i * 4 + lane] = colors[i][lane];
        }
    }
    // The original reuses the selector's stack storage for the 160-byte
    // coefficient prefix. These methods read/write only that prefix.
    CarRenderRecord coefficients;
    unsigned int *words = reinterpret_cast<unsigned int *>(&coefficients);
    for (int i = 0; i < 40; ++i) {
        words[i] = 0;
    }
    for (int i = 0; i < count; ++i) {
        coefficients.AddLightPair(normalized[i], colors[i]);
    }
    coefficients.NormalizeLightCoefficients(ug2LightNormalize82DAE4, ug2LightTarget7F39B8,
                                            ug2LightNormalize7F39BC, ug2LightTarget7F39C0);
    for (int i = 0; i < 40; ++i) {
        output->value120[i] = words[i];
    }
    return 1;
}
