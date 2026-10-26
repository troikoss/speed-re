#include "CarRenderLightSelect.h"

extern "C" double __cdecl fabs(double);
#pragma intrinsic(fabs)
#ifdef __GNUC__
#define LIGHT_ABS(value) __builtin_fabsl(value)
#else
#define LIGHT_ABS(value) fabs(value)
#endif

namespace {
const volatile float light784250 = 1.0f;
const volatile float light7D7D3C = 0.0f;
}

// Original 0x57bcd0: both links point to this sentinel.
void CarRenderLightList::Initialise() {
    next00 = reinterpret_cast<CarRenderLightSector *>(this);
    previous04 = reinterpret_cast<CarRenderLightSector *>(this);
}

// Original 0x4893a0. Distance and outer squared radius spill to float, while
// the inner squared radius stays extended across both attenuation divisions.
void CarRenderAdmitLight(CarRenderLightSelection *output, CarRenderLightSelectState *state,
                        CarRenderLight *light, float x, float y, float z) {
    if (light->active07 != 1) return;
    const unsigned int kind = light->kind04;
    const long double dx = LIGHT_ABS(static_cast<long double>(x) - light->position10[0]);
    const long double dy = LIGHT_ABS(static_cast<long double>(y) - light->position10[1]);
    const long double dz = LIGHT_ABS(static_cast<long double>(z) - light->position10[2]);
    volatile float outer = static_cast<long double>(light->radius1C) * light->radius1C;
    volatile float distance = (dz * dz + dy * dy) + dx * dx;
    if (!(distance < outer)) return;
    const unsigned int mode = light->mode05;
    const long double inner = static_cast<long double>(light->radius30) * light->radius30;
    volatile float weight = 1.0f;
    if (distance > inner) {
        weight = static_cast<long double>(light784250)
               - (static_cast<long double>(distance) - inner) / (static_cast<long double>(outer) - inner);
    }
    if (kind == 0) {
        const unsigned int color = light->color0C;
        const float r = static_cast<float>(color & 0xff);
        const float g = static_cast<float>((color >> 8) & 0xff);
        const float b = static_cast<float>((color >> 16) & 0xff);
        const float fourth = light->value2C;
        if (mode == 2) {
            if (state->currentB4 < state->endB8) {
                state->currentB4[0] = static_cast<long double>(r) * weight;
                state->currentB4[1] = static_cast<long double>(g) * weight;
                state->currentB4[2] = static_cast<long double>(b) * weight;
                state->currentB4[3] = static_cast<long double>(fourth) * weight;
                if (weight > light7D7D3C) {
                    state->currentB4 += 4;
                    state->weightBC = static_cast<long double>(weight) + state->weightBC;
                }
            }
        } else if (distance < state->distanceC0) {
            state->nearestB0[0] = r;
            state->nearestB0[1] = g;
            state->nearestB0[2] = b;
            state->nearestB0[3] = fourth;
            state->distanceC0 = distance;
        }
    } else if ((kind == 2 || kind == 3) && state->countC8 < 24) {
        output->lights00[state->countC8] = light;
        output->weights60[state->countC8] = weight;
        ++state->countC8;
    }
}

// Original 0x48e3d0. The ten-light/ambient limits apply to the sector pass only;
// the additional-list pass still visits every node and admission caps at 24.
int CarRenderSelectLights(CarRenderLightSelection *output, const float *position) {
    output->lights00[0] = 0;
    output->weights60[0] = 0.0f;
    output->valueC0[0] = output->valueC0[1] = output->valueC0[2] = output->valueC0[3] = 0.0f;
    CarRenderLightSector *sector = ug2LightSectors830D14.next00;
    const float x = position[0], y = position[1], z = position[2];
    CarRenderLightSelectState state;
    state.fallback00[0] = state.fallback00[1] = state.fallback00[2] = 0.0f;
    state.fallback00[3] = 1.0f;
    state.nearestB0 = state.fallback00;
    state.currentB4 = state.ambient10;
    state.endB8 = state.ambient10 + 40;
    state.weightBC = 0.0f;
    state.distanceC0 = 3.4028234663852886e38f;
    state.countC8 = 0;
    CarRenderLightSector *end = ug2LightSectors830D14.End();
    while (sector != end && state.countC8 < 10 && state.currentB4 < state.endB8) {
        CarRenderLightTreeNode *node = sector->tree10->Find(x, y, z);
        if (node) {
            const int count = -static_cast<int>(node->count0E);
            CarRenderLight *lights = sector->lights18;
            for (int i = 0; i < count; ++i) {
                if (state.countC8 >= 10) break;
                CarRenderAdmitLight(output, &state, lights + node->indices1C[i], x, y, z);
            }
        }
        sector = sector->next00;
    }
    sector = ug2AdditionalLights835850.next00;
    end = ug2AdditionalLights835850.End();
    while (sector != end) {
        if (sector->flag28) {
            CarRenderLightLink *link = sector->list2C.next00;
            while (link && link != &sector->list2C) {
                CarRenderLight *light = reinterpret_cast<CarRenderLight *>(link + 1);
                CarRenderAdmitLight(output, &state, light, x, y, z);
                link = link->next00;
            }
        }
        sector = sector->next00;
    }
    volatile float remaining = static_cast<long double>(light784250) - state.weightBC;
    float scale = state.weightBC;
    if (state.weightBC > light784250) {
        remaining = 0.0f;
        scale = static_cast<long double>(light784250) / state.weightBC;
    }
    long double r = static_cast<long double>(remaining) * state.nearestB0[0];
    long double g = static_cast<long double>(remaining) * state.nearestB0[1];
    long double b = static_cast<long double>(remaining) * state.nearestB0[2];
    long double fourth = static_cast<long double>(remaining) * state.nearestB0[3];
    // Original Z/fourth values are stored even on the no-contribution path.
    volatile float storedB = b, storedFourth = fourth;
    for (float *p = state.ambient10; p < state.currentB4; p += 4) {
        r += static_cast<long double>(scale) * p[0];
        g += static_cast<long double>(scale) * p[1];
        b += static_cast<long double>(scale) * p[2];
        fourth += static_cast<long double>(scale) * p[3];
    }
    if (state.ambient10 < state.currentB4) {
        storedB = b;
        storedFourth = fourth;
    }
    output->valueC0[0] = r;
    output->valueC0[3] = storedFourth;
    output->valueC0[1] = g;
    output->valueC0[2] = storedB;
    return state.countC8;
}
