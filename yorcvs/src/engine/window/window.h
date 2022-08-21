#pragma once
#include "../../common/types.h"
namespace yorcvs {
template <typename Window_Implementation>
class window {
public:
    window() = default;
    window(const window& other) = delete;
    window(window&& other) = delete;
    window& operator=(const window& other) = delete;
    window& operator=(window&& other) = delete;
    virtual ~window() = default;
    /**
     * @brief Clears the screen
     *
     */
    void clear()
    {
        static_cast<Window_Implementation*>(this)->clear();
    }
    /**
     * @brief draws rendered sprites to the screen
     *
     */
    void present()
    {
        static_cast<Window_Implementation*>(this)->present();
    }
    /**
     * @brief Renders the texture to the screen
     *
     * @param path path to texture
     * @param dstRect part of the srceen the sprite will be drawn at
     * @param srcRect what part of the sprite to draw
     * @param angle  angle of the sprite
     */
    void draw_texture(const std::string& path, const yorcvs::rect<float>& dstRect,
        const yorcvs::rect<size_t>& srcRect, double angle = 0.0)
    {
        static_cast<Window_Implementation*>(this)->draw_texture(path, dstRect, srcRect, angle);
    }

    /**
     * @brief Renders the texture to the screen
     *
     * @param path path to texture
     * @param dstRectPos position of the texture
     * @param dstRectSize size of the texture
     * @param srcRect what part of the sprite to draw
     * @param angle angle of the sprite
     */
    void draw_texture(const std::string& path, const yorcvs::vec2<float>& dstRectPos,
        const yorcvs::vec2<float>& dstRectSize, const yorcvs::rect<size_t>& srcRect, double angle = 0.0)
    {
        static_cast<Window_Implementation*>(this)->draw_texture(path, dstRectPos, dstRectSize, srcRect, angle);
    }
    /**
     * @brief Draws the text to the screen
     *
     * @param message message to be drawn
     * @param path path to font
     * @param dstRect
     * @param r
     * @param g
     * @param b
     * @param a
     * @param charSize
     * @param lineLength
     */
    void draw_text(const std::string& message, const std::string& path, const yorcvs::rect<float>& dstRect, unsigned char r, unsigned char g,
        unsigned char b, unsigned char a, size_t charSize, size_t lineLength)
    {
        static_cast<Window_Implementation*>(this)->draw_text(message, path, dstRect, r, g, b, a, charSize, lineLength);
    }
    /**
     * @brief Draws a rectangle to the screen
     *
     * @param rect - the position and dimesnions of the rectangle
     * @param r - red color component
     * @param g - green
     * @param b - blue
     * @param a - alpha
     */
    void draw_rect(const yorcvs::rect<float>& rect, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        static_cast<Window_Implementation*>(this)->draw_rect(rect, r, g, b, a);
    }
    /**
     * @brief Set the viewport
     *
     * @param viewport Area to be drawn
     */
    void set_viewport(const yorcvs::rect<float>& viewport);

    /**
     * @brief Get the viewport
     *
     * @return yorcvs::Rect<float> the viewport
     */
    yorcvs::rect<float> get_viewport();
    /**
     * @brief Set the render scale
     *
     * @param scale The scale x and y
     */
    void set_render_scale(const yorcvs::vec2<float>& scale)
    {
        return static_cast<Window_Implementation*>(this)->set_render_scale(scale);
    }

    /**
     * @brief Get the render scale
     *
     * @return yorcvs::Vec2<float> Render scale
     */
    yorcvs::vec2<float> get_render_scale()
    {
        return static_cast<Window_Implementation*>(this)->get_render_scale();
    }

    /**
     * @brief Set the  the offset at which the drawing will happen
     *
     * @param newOffset the offset
     */
    void set_drawing_offset(const yorcvs::vec2<float>& newOffset);

    /**
     * @brief Get the drawing offset
     *
     * @return yorcvs::Vec2<float> the offset
     */
    yorcvs::vec2<float> get_drawing_offset()
    {
        return static_cast<Window_Implementation*>(this)->get_drawing_offset();
    }

    yorcvs::vec2<float> get_window_size()
    {
        return static_cast<Window_Implementation*>(this)->get_window_size();
    }

private:
};
}
