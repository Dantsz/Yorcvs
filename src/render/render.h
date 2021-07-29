#pragma once
#include <type_traits>
#include <string>
#include "../common/types.h"
#include <cstdint>
namespace yorcvs
{   
    /**
     * @brief Contains information about the api used for rendering
     * 
     *  
     
    template<typename T>
    concept Interface = requires(T interface)
    {
        {
            interface.name
        }->std::same_as<const char*>;

       
    };
    */
   
    template<typename interface>
    class Texture
    {

    };

    /**
     * @brief Windows that can render sprites and text to the screen
     * 
     * @tparam interface - the underlying wrapper
     */
    template<typename interface>
    class Window
    {
        public:
            void Init(const char* name , size_t w , size_t h)
            {
                
            }
            void setSize(size_t w,size_t h)
            {
               
            }
            void cleanup()
            {
               
            }



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
            Texture<interface> createTextTexture(const std::string& path,const std::string& message, uint8_t r, uint8_t g, uint8_t b, uint8_t a,size_t charSize,size_t lineLength);

            void drawText(const Texture<interface>& texture,const Rect<float>& dstRect);
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

        private:
            

    };

    

    



}
