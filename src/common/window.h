/**
 * @file render.h
 * @author Dansz
 * @brief Basic 2D rendering and input handling
 * @version 0.1
 * @date 2021-07-29
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once
#include "../common/types.h"
#include <string>

namespace yorcvs
{

/**
 * @brief - Texture for drawing sprite
 *
 * @tparam interface - interface the texture uses
 */
template <typename interface> class Texture
{
};
/**
 * @brief -Contains data for rendering text
 *
 * @tparam interface - interface used for text
 */
template <typename interface> class Text
{
};

/**
 * @brief Called in the event loop
 *
 */
template <typename interface> class Callback
{
};

/**
 * @brief Class for specifying which key should be check as being pressed
 * @note Is currently no api-independent
 */
template <typename interface> class Key
{
};

/**
 * @brief Windows that can render sprites and text to the screen
 *
 * @tparam interface -interface used to implement the window,
 */
template <typename interface> class Window
{
  public:
    void Init(const char *name, size_t w, size_t h);
    void set_size(size_t w, size_t h);
    void cleanup();

    /**
     * @brief Renders a sprite to the screen with the specified parameters
     *
     * @param path path to the texture
     * @param dstRect part of the screen the sprite should occupy
     * @param srcRect part of the texture to be drawn
     * @param angle angle of the texture(default is 0)
     */
    void draw_sprite(const std::string &path, const yorcvs::Rect<float> &dstRect, const yorcvs::Rect<size_t> &srcRect,
                    double angle = 0.0);

    /**
     * @brief Renders a sprite to the screen with the specified parameters
     * 
     * @param path path to the texture
     * @param dstRectPos position of the texture
     * @param dstRectSize size of the texture
     * @param srcRect  part of the texture to be drawn
     * @param angle angle of the texture(default is 0)
     */
    void draw_sprite(const std::string &path, const yorcvs::Vec2<float>& dstRectPos , const yorcvs::Vec2<float>& dstRectSize, const yorcvs::Rect<size_t> &srcRect,
                    double angle = 0.0);

    /**
     * @brief Creates a Texture object,
     *  
     * @param path - path to the texture
     * @return Texture<interface>
     * NOTE: CREATED TEXTURE IS MANAGED BY THE WINDOW CLASS
     */
    Texture<interface> create_texture(const std::string &path);
  
    /**
     * @brief Renders the texture to the screen
     *
     * @param texture the texture object
     * @param dstRect part of the srceen the sprite will be drawn at
     * @param srcRect what part of the sprite to draw
     * @param angle  angle of the sprite
     */
    void draw_texture(const Texture<interface> &texture, const yorcvs::Rect<float> &dstRect,
                     const yorcvs::Rect<size_t> &srcRect, double angle = 0.0);

    /**
     * @brief Renders the texture to the screen
     * 
     * @param texture the texture object
     * @param dstRectPos position of the texture
     * @param dstRectSize size of the texture
     * @param srcRect what part of the sprite to draw
     * @param angle angle of the sprite
     */
    void draw_texture(const Texture<interface> &texture, const yorcvs::Vec2<float>& dstRectPos ,const yorcvs::Vec2<float>& dstRectSize,
                     const yorcvs::Rect<size_t> &srcRect, double angle = 0.0);

    /**
     * @brief Create a Text Texture object
     *
     * @param path  path to the font to be used
     * @param message  the message to display
     * @param r  red
     * @param g  green
     * @param b  blue
     * @param a  alpha
     * @param charSize  the resolution of a glyph
     * @param lineLength  length of a line before a break
     * @return std::shared_ptr<Texture<interfaceWindow>>  the texture 
     * passed to draw_texture
     */
    Text<interface> create_text(const std::string &path, const std::string &message, unsigned char r, unsigned char g,
                               unsigned char b, unsigned char a, size_t charSize, size_t lineLength);

    /**
     * @brief Draws the text to the screen, fitting it in dstRect
     *
     * @param text - Text created using creaText method
     * @param dstRect - A rectangle
     */
    void draw_text(const Text<interface> &text, const Rect<float> &dstRect);

    /**
     * @brief Set the Text Message
     *
     * @param text - the text to be set
     * @param message - new message
     */
    void set_text_message(const Text<interface> &text, const std::string &message);

    /**
     * @brief Set the Text Color
     *
     * @param text - text object
     * @param r - red
     * @param g - green
     * @param b - blue
     * @param a - alpha
     */
    void set_text_color(const Text<interface> &text, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    /**
     * @brief Set the Text Char Size
     *
     * @param text - text
     * @param charSize - the new size
     */
    void set_text_char_size(const Text<interface> &text, size_t charSize);

    /**
     * @brief Set the Text Line Length
     *
     * @param text - text
     * @param lineLength - new length of message before a new line
     */
    void set_text_line_length(const Text<interface> &text, size_t lineLength);

    /**
     * @brief Set the Text Font
     *
     * @param text - text
     * @param fontPath - path to the new font
     */
    void set_text_font(const Text<interface> &text, const std::string &fontPath);

    /**
     * @brief Clears the screen
     *
     */
    void clear();

    /**
     * @brief draws rendered sprites to the screen
     *
     */
    void present();

    // INPUT

    /**
     * @brief Handles window events
     *
     */
    void handle_events();

    /**
     * @brief Get the cursor position on the screen
     *
     * @return yorcvs::Vec2<float> - the position
     */
    yorcvs::Vec2<float> get_cursor_position();

    /**
     * @brief Represents a key
     *
     * @param key - the key to be checked
     * @return true -  the key is pressed
     * @return false - the key is not pressed
     * @note  Currently has no api-independent way of specifying which key to be checked
     */

    bool is_key_pressed(yorcvs::Key<interface> key);

    /**
     * @brief Adds the callback to the event loop and will execute each time handleEvent is being called
     *
     * @param callback - the callback
     *
     *
     * @return size_t - the index of the callback in the vector in case it shouldn't remain for the duration of the loop
     * NOTE: CALLBACKS MUST PROBABLY CONTAIN API SPECIFIC HANDLING OF EVENTS
     */
    size_t register_callback(const Callback<interface> &callback);

    /**
     * @brief Deletes the callback from the callback array
     *
     * @param index - index returned from resgisterCallBack
     *
     */
    void unregister_callback(size_t index);

    /**
     * @brief Set the  the offset at which the drawing will happen
     * 
     * @param newOffset the offset
     */
    void set_drawing_offset(const yorcvs::Vec2<float>& newOffset);


    /**
    * @brief Get the window size 
    * 
    * @return yorcvs::Vec2<float> the size
    */
    yorcvs::Vec2<float> get_window_size();
    bool isActive;

  private:
};

} // namespace yorcvs
