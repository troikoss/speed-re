#ifndef UG2_NATIVE_SCENE_H
#define UG2_NATIVE_SCENE_H

// Platform scaffolding: OpenGL drawing of UG2's own car and track assets. The original
// render path (CarRenderInfo 0x638190, eSolid rendering 0x5cf420) is not recovered.
#include "NativeAssets.h"

#include <GL/gl.h>
#include <map>
#include <string>
#include <vector>

class NativeTextureCache {
public:
    void Add(const NativeImageMap &images);
    // Returns 0 when the hash is not loaded.
    GLuint Get(unsigned int hash, bool *alpha);
    const NativeImage *Find(unsigned int hash) const;
    size_t Count() const { return images_.size(); }

private:
    struct Entry {
        GLuint id;
        bool alpha;
    };
    NativeImageMap images_;
    std::map<unsigned int, Entry> uploaded_;
};

struct WheelMatrix;
struct CarRenderWheelInfo;

struct NativeDrawSolid {
    const NativeSolid *solid;
    GLuint opaqueList;
    GLuint translucentList;
};

// Runs the recovered wheel layout 0x61fe10 and the wheel block 0x62700e..0x62df66 on the render info `ri`.
// Solids are drawn through the platform seams in NativeCarWheels.cpp (with the GL list of each solid).
void NativeRunCarWheelBlock(CarRenderWheelInfo *ri, const unsigned char *carTypeRow, const float *body,
                            WheelMatrix tires[4], WheelMatrix brakes[4], WheelMatrix third[4], bool translucent);

// Sets the car draw's view for the pixel size (0x48acb0) and the wheel LOD: the camera origin, the unit view direction and
// the focal length in pixels. Call it before NativeCarModel::Draw of each frame.
void NativeSetCarView(const float eye[3], const float forward[3], float focalPixels);

class NativeCarModel {
public:
    bool Load(const char *gameDirectory, const char *carName, NativeTextureCache *textures, char *error,
              int errorSize);
    // `body` is the column-major render matrix of the car origin (the body frame of the wheel block).
    // `tires`, `brakes` and `third` are the car-local frames of 0x5f6820 (arg5, arg6) and 0x5ec7f0 (arg7);
    // the recovered wheel block 0x62700e..0x62df66 (CarWheelDraw) composes them with `body` and draws.
    void Draw(const float *body, WheelMatrix tires[4], WheelMatrix brakes[4], WheelMatrix third[4],
              bool translucent);
    // Platform seam: CarTypeInfo row of the car (ri+8 for 0x61fe10).
    void SetCarTypeRow(const unsigned char *row) { carTypeRow_ = row; }
    int SolidCount() const { return (int)solids_.size(); }
    int TexturedParts() const { return texturedParts_; }
    int Parts() const { return parts_; }

private:
    GLuint Build(const NativeSolid &solid, bool translucent);
    unsigned int Resolve(unsigned int hash) const;
    std::vector<NativeSolid> solids_;
    std::vector<NativeDrawSolid> body_;
    NativeDrawSolid wheel_;
    CarRenderWheelInfo *wheelInfo_ = 0;   // recovered CarRenderInfo of the wheel block (ri)
    unsigned char *carObject_ = 0;        // ri+4 car object: gate byte, no rim parts
    const unsigned char *carTypeRow_ = 0;
    NativeDrawSolid frontBrake_;
    NativeDrawSolid rearBrake_;
    std::map<unsigned int, unsigned int> replacements_;
    NativeTextureCache *textures_;
    float paint_[3];
    unsigned int skinHash_;
    int texturedParts_;
    int parts_;
};

class NativeTrackModel {
public:
    bool Load(const char *gameDirectory, const char *region, NativeTextureCache *textures, char *error,
              int errorSize);
    void Draw(const float *eye, const float *forward, float range);
    int InstanceCount() const { return (int)instances_.size(); }
    int SolidCount() const { return (int)solids_.size(); }
    // Prints the instances whose bounds contain (x, y) with their solids and textures.
    void DebugAt(float x, float y) const;
    // Redraws the scenery into the stencil buffer and prints the instance drawn at window
    // pixel (x, y). Needs a stencil buffer and the frame's projection and view matrices.
    void DebugPick(const float *eye, const float *forward, float range, int x, int y);

private:
    struct Placed {
        GLuint lists[3];  // opaque, blended, additive parts
        float centre[3];
        float radius;
        const float *matrix;
        bool sky;
        int instance;
        int section;
    };
    GLuint Build(const NativeSolid &solid, int pass);
    void DrawPlaced(const float *eye, const float *forward, float range, int pickShift, int pass);
    void PrintInstance(size_t index) const;
    void SelectSections(const float *eye);
    std::vector<NativeSolid> solids_;
    std::vector<NativeInstance> instances_;
    std::vector<Placed> placed_;
    std::vector<NativeDrivableSection> drivable_;
    std::vector<bool> visible_;  // by section number
    int currentSection_;
    NativeTextureCache *textures_;
};

#endif
