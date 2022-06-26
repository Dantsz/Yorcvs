#pragma once
#include <cmath>
#include <tuple>
namespace yorcvs {
/**
 * @brief Transform between coordinates on the spiral and their index
 * Example Spiral:
  20--21--22--23--24--25
   |                   |
  19   6---7---8---9  26
   |   |           |   |
  18   5   0---1  10  27
   |   |       |   |   |
  17   4---3---2  11  28
   |               |   |
  16--15--14--13--12  29
                       |
  35--34--33--32--31--30

 * Coordinates:
 (-1,1)---(0,1) ---  (1,1)
    |
    |
 (-1,0)    (0,0) --- (1,0)
    |                  |
    |                  |
 (-1,-1)---( 0,1 )--- (1,-1)
 *
 *
 *
 */
namespace spiral {
    /**
     * @brief Transform the index of an element in the spiral in its coordinates
     *
     * @param n the index
     * @return constexpr std::tuple<intmax_t,intmax_t> - first element is x and the second y
     */
    inline std::tuple<intmax_t, intmax_t> wrap(const size_t n)
    {
        // no constexppr array
        constexpr intmax_t c[4][6] = {
            { -1, 0, 0, -1, 1, 0 }, { -1, 1, 1, 1, 0, 0 }, { 1, 0, 1, 1, -1, -1 }, { 1, -1, 0, -1, 0, -1 }
        };

        const intmax_t square = static_cast<intmax_t>(std::floor(std::sqrt(n / 4)));

        const intmax_t index = (static_cast<intmax_t>(n) - 4 * square * square) % (2 * square + 1);
        const intmax_t side = (static_cast<intmax_t>(n) - 4 * square * square) / (2 * square + 1);

        return std::make_tuple<intmax_t, intmax_t>(c[side][0] * square + c[side][1] * index + c[side][2],
            c[side][3] * square + c[side][4] * index + c[side][5]);
    }
    /**
     * @brief Transform the coordinates of the element in its index in the spiral
     *
     * @param x x coordinate
     * @param y y coordinate
     * @return constexpr size_t the index in the spiral
     */
    inline size_t unwrap(const intmax_t x, const intmax_t y)
    {
        const intmax_t ax = std::abs(x);
        const intmax_t ay = std::abs(y);
        // find loop number in spiral
        const intmax_t loop = std::max(ax, ay);
        // one less than the edge length of the current loop
        const intmax_t edgelen = 2 * loop;
        // the numbers in the inner loops
        const intmax_t prev = (edgelen - 1) * (edgelen - 1);
        if (x == loop && y < loop) {
            // right edge
            return prev - y + (loop - 1);
        }
        if (y == -loop) {
            // top edge
            return prev + loop - x + edgelen - 1;
        }
        if (x == -1 * loop) {
            // left edge
            return prev + loop + y + 2 * edgelen - 1;
        }
        if (y == loop) {
            // bottom edge
            return prev + x + loop + 3 * edgelen - 1;
        }
        return 0;
    }
} // namespace spiral
}
