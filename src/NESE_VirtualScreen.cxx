#include "NESE_VirtualScreen.hxx"



namespace NESE {

    void VirtualScreen::Create(unsigned int w, unsigned int h, float pixel_size, sf::Color color) {
        m_Vertices.resize(w * h * 6);
        m_ScreenSize = {w, h};
        m_Vertices.setPrimitiveType(sf::Triangles);
        m_PixelSize = pixel_size;

        for (std::size_t x = 0; x < w; ++x) {
            for (std::size_t y = 0; y < h; ++y) {
                auto index = (x * m_ScreenSize.y + y) * 6;
                sf::Vector2f coord2d (x * m_PixelSize, y * m_PixelSize);

                //Triangle-1
                //top-left
                m_Vertices[index].position = coord2d;
                m_Vertices[index].color    = color;

                //top-right
                m_Vertices[index + 1].position = coord2d + sf::Vector2f{m_PixelSize, 0};
                m_Vertices[index + 1].color    = color;

                //bottom-right
                m_Vertices[index + 2].position = coord2d + sf::Vector2f{m_PixelSize, m_PixelSize};
                m_Vertices[index + 2].color    = color;

                //Triangle-2
                //bottom-right
                m_Vertices[index + 3].position = coord2d + sf::Vector2f{m_PixelSize, m_PixelSize};
                m_Vertices[index + 3].color    = color;

                //bottom-left
                m_Vertices[index + 4].position = coord2d + sf::Vector2f{0, m_PixelSize};
                m_Vertices[index + 4].color    = color;

                //top-left
                m_Vertices[index + 5].position = coord2d;
                m_Vertices[index + 5].color    = color;
            }
        }
    }

    void VirtualScreen::setPixel(std::size_t x, std::size_t y, sf::Color color) {
        auto index = (x * m_ScreenSize.y + y) * 6;
        if (index >= m_Vertices.getVertexCount()) {
            return;
        }

        sf::Vector2f coord2d (x * m_pixelSize, y * m_pixelSize);

        //Triangle-1
        //top-left
        m_Vertices[index].color     = color;

        //top-right
        m_Vertices[index + 1].color = color;

        //bottom-right
        m_Vertices[index + 2].color = color;

        //Triangle-2
        //bottom-right
        m_Vertices[index + 3].color = color;

        //bottom-left
        m_Vertices[index + 4].color = color;

        //top-left
        m_Vertices[index + 5].color = color;
    }

    void VirtualScreen::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        target.draw(m_Vertices, states);
    }

};
