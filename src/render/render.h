/**
 * @file render.h
 * @author Dansz
 * @brief Basic 2D rendering 
 * @version 0.1
 * @date 2021-07-29
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once
#include <type_traits>
#include <string>
#include "../common/types.h"
#include <cstdint>
namespace yorcvs
{   
  
    /*
    Interface = api used to draw to the screen
    How to:
    Specialize window,texture,and text with a class(concept) and implement their methods using the api
    */

    /* Concepts are still not supported yet
    template<typename T>
    concept Interface = requires(T interface)
    {
        {
            interface.name
        }->std::same_as<const char*>;

       
    };
    */

    /**
    * @brief - Texture for drawing sprite
    * 
    * @tparam interface - interface the texture uses
    */
    template<typename interface>
    class Texture
    {

    };
    /**
     * @brief -Contains data for rendering text
     * 
     * @tparam interface - interface used for text
     */
    template<typename interface>
    class Text
    {

    };

    /**
     * @brief Windows that can render sprites and text to the screen
     * 
     * @tparam interface -interface used to implement the window, 
     */
    template<typename interface>
    class Window
    {
        public:
            void Init(const char* name , size_t w , size_t h);
            void setSize(size_t w,size_t h);
            void cleanup();

            /**
             * @brief Handles window events
             * 
             */
            void handleEvents();

            /**
             * @brief Renders a sprite to the screen with the specified parameters
             * 
             * @param path -path to the texture
             * @param dstRect -part of the scree the sprite should occupy
             * @param srcRect -part of the texture to be drawn
             * @param angle  -angle of the texture(default is 0)
             */
            void drawSprite(const std::string& path,const yorcvs::Rect<float>& dstRect ,const yorcvs::Rect<size_t>& srcRect , double angle = 0.0 );

            /**
             * @brief Create a Text Texture object
             * 
             * @param path - path to the font to be used
             * @param message - the message to display
             * @param r - red
             * @param g - green
             * @param b - blue
             * @param a - alpha
             * @param charSize - the resolution of a glyph
             * @param lineLength - length of a line before a break
             * @return std::shared_ptr<Texture<interfaceWindow>> - the texture to be passed to draw_texture
             */
            Text<interface> createText(const std::string& path,const std::string& message, uint8_t r, uint8_t g, uint8_t b, uint8_t a,size_t charSize,size_t lineLength);
        

            /**
             * @brief Draws the text to the screen, fitting it in dstRect
             * 
             * @param text - Text created using creaText method
             * @param dstRect - A rectangle
             */
            void drawText(const Text<interface>& text,const Rect<float>& dstRect);



            /**
             * @brief Set the Text Message 
             * 
             * @param text - the text to be set
             * @param message - new message
             */
            void setTextMessage(const Text<interface>& text , const std::string& message);







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



         
            bool isActive;
        private:
            

    };

    

    



}
