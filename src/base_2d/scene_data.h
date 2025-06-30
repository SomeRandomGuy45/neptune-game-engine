#include <cstring>
#include <cstdint>

namespace neptune {

enum class EntryType : uint8_t {
    OBJECT = 0
};

#pragma pack(push, 1)

struct RootBase {
    EntryType type;
    char objType[16];
    char name[64];
    char parent[64];
    float r, g, b;
};

struct BasicDrawObject : RootBase
{
    float x, y, w, h;
};

struct Circle : BasicDrawObject {
    float radius;
};

struct Text: BasicDrawObject {
    float b_r, b_g, b_b;
    char fontName[64];
    uint32_t length;
};

struct Sprite : BasicDrawObject {
    char path[128];
};

struct Script : RootBase {
    char path[128];
};

struct Audio : RootBase {
    char path[128];
};


#pragma pack(pop)

} // namespace neptune