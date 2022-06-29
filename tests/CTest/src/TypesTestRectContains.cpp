#include "common/types.h"
#include <cassert>
int main()
{
    constexpr int x = 0;
    constexpr int y = 0;
    constexpr int w = 100;
    constexpr int h = 100;
    yorcvs::Rect<int> test_rect { x, y, w, h };
    assert(test_rect.contains({ x + w / 2, y + h / 2 }));
    assert(!test_rect.contains({ x + w + 1, y + h + 1 }));
}
