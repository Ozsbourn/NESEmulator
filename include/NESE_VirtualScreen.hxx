#include <D:/Dev/Libs/SFML-2.5.1/include/SFML/Graphics.hpp>


#ifndef NESE_VIRTUALSCREEN_HXX_
#define NESE_VIRTUALSCREEN_HXX_

namespace NESE {

    class VirtualScreen : public sf::Drawable {
    private:
        sf::VertexArray m_Vertices;
        sf::Vector2u    m_ScreenSize;

        float           m_PixelSize;


        void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    public:
        explicit VirtualScreen(void) = default;

        void Create(unsigned int width, unsigned int height, float pixel_size, sf::Color color);
        void SetPixel(std::size_t x, std::size_t y, sf::Color color);
    
        ~VirtualScreen(void) = default;
    };

};

#endif // NESE_VIRTUALSCREEN_HXX_