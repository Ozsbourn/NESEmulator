#include <SFML/Graphics.hpp>

#include <chrono>

#include "NESE_CPU.hxx"
#include "NESE_PPU.hxx"
#include "NESE_MainBus.hxx"
#include "NESE_PictureBus.hxx"
#include "NESE_Controller.hxx"


#ifndef NESE_EMULATOR_HXX_
#define NESE_EMULATOR_HXX_

namespace NESE {

    using timepoint_t = std::chrono::high_resolution_clock::time_point;


    const int32_t Width = ScanlineVisibleDots;
    const int32_t Width = VisibleScanlines;



    class Emulator {
    private:
        MainBus                   m_Bus;
        PictureBus                m_PictureBus;
        CPU                       m_CPU;
        PPU                       m_PPU;
        Cartridge                 m_Cartridge;
        std::unique_ptr<Mapper>   m_Mapper;

        Controller m_Controller1, m_Controller2;

        sf::RenderWindow          m_Window;
        VirtualScreen             m_EmulatorScreen;
        float                     m_ScreenScale;

        TimePoint                 m_CycleTimer;

        std::chrono::high_resolution_clock::duration m_ElapsedTime;
        std::chrono::nanoseconds                     m_CPUCycleDuration;
    
    public:
        Emulator(void);

        void Run(std::string rom_path);

        void SetVideoWidth(int width);
        void SetVideoHeight(int height);

        void SetVideoScale(float scale);

        void SetKeys(std::vector<sf::Keyboard::Key>& p1, std::vector<sf::Keyboard::Key>& p2);
    };

};

#endif // NESE_EMULATOR_HXX_