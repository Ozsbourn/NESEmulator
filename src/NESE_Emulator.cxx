#include "NESE_Emulator.hxx"
#include "NESE_CPUOpCodes.hxx"
#include "NESE_Util.hxx"

#include <thread>
#include <chrono>

namespace NESE {

	Emulator::Emulator(void)
	: m_CPU(m_Bus),
	  m_PPU(m_PictureBus, m_EmulatorScreen),
	  m_ScreenScale(3.0f),
	  m_CycleTimer(),
	  m_CPUCycleDuration(std::chrono::nanoseconds(559))
	{
		// TO-DO: [change it to normal checker methods]
		if(!m_Bus.SetReadCallback(PPUSTATUS, [&](void) { return m_PPU.GetStatus(); })    ||
            !m_Bus.SetReadCallback(PPUDATA, [&](void)  { return m_PPU.GetData(); })      ||
            !m_Bus.SetReadCallback(JOY1, [&](void)     { return m_Controller1.Read(); }) ||
            !m_Bus.SetReadCallback(JOY2, [&](void)     { return m_Controller2.Read(); }) ||
            !m_Bus.SetReadCallback(OAMDATA, [&](void)  { return m_PPU.GetOAMData(); }))
        {
            LOG(Error) << "Critical error: Failed to set I/O callbacks" << std::endl;
        }


        if(!m_Bus.SetWriteCallback(PPUCTRL, [&](Byte b)   {m_PPU.Control(b);}) ||
            !m_Bus.SetWriteCallback(PPUMASK, [&](Byte b)  {m_PPU.SetMask(b);}) ||
            !m_Bus.SetWriteCallback(OAMADDR, [&](Byte b)  {m_PPU.SetOAMAddress(b);}) ||
            !m_Bus.SetWriteCallback(PPUADDR, [&](Byte b)  {m_PPU.SetDataAddress(b);}) ||
            !m_Bus.SetWriteCallback(PPUSCROL, [&](Byte b) {m_PPU.SetScroll(b);}) ||
            !m_Bus.SetWriteCallback(PPUDATA, [&](Byte b)  {m_PPU.SetData(b);}) ||
            !m_Bus.SetWriteCallback(OAMDMA, [&](Byte b)   {DMA(b);}) ||
            !m_Bus.SetWriteCallback(JOY1, [&](Byte b)     {m_Controller1.Strobe(b); m_Controller2.Strobe(b);}) ||
            !m_Bus.SetWriteCallback(OAMDATA, [&](Byte b)  {m_PPU.SetOAMData(b);}))
        {
            LOG(Error) << "Critical error: Failed to set I/O callbacks" << std::endl;
        }

        m_PPU.SetInterruptCallback([&](){ m_CPU.Interrupt(InterruptType::NMI); });
	}



	void Emulator::Run(std::string rom_path) {
        if (!m_Cartridge.LoadFromFile(rom_path)) {
            return;
        }

        m_Mapper = Mapper::CreateMapper(static_cast<Mapper::Type>(m_Cartridge.GetMapper()),
                                        m_Cartridge,
                                        [&](){ m_CPU.Interrupt(InterruptType::IRQ); },
                                        [&](){ m_PictureBus.UpdateMirroring(); });
        if (!m_Mapper) {
            LOG(Error) << "Creating Mapper failed. Probably unsupported." << std::endl;
            return;
        }

        if (!m_Bus.SetMapper(m_Mapper.Get()) || !m_PictureBus.SetMapper(m_Mapper.Get())) {
            return;
        }

        m_CPU.Reset();
        m_PPU.Reset();

        m_Window.create(sf::VideoMode(NESVideoWidth * m_ScreenScale, NESVideoHeight * m_ScreenScale),
                        "SimpleNES", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
        m_Window.setVerticalSyncEnabled(true);
        m_emulatorScreen.create(NESVideoWidth, NESVideoHeight, m_ScreenScale, sf::Color::White);

        m_CycleTimer = std::chrono::high_resolution_clock::now();
        m_ElapsedTime = m_CycleTimer - m_CycleTimer;

        sf::Event event;
        bool focus = true, pause = false;
        while (m_Window.isOpen())
        {
            while (m_Window.pollEvent(event)) {
                if (event.type == sf::Event::Closed ||
                (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
                    m_Window.close();
                    return;
                }
                else if (event.type == sf::Event::GainedFocus)
                {
                    focus = true;
                    m_CycleTimer = std::chrono::high_resolution_clock::now();
                }
                else if (event.type == sf::Event::LostFocus) {
                    focus = false;
                } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F2) {
                    pause = !pause;
                    if (!pause) {
                        m_CycleTimer = std::chrono::high_resolution_clock::now();
                        LOG(Info) << "Paused." << std::endl;
                    } else {
                        LOG(Info) << "Unpaused." << std::endl;
                    }
                } else if (pause && event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::F3) {
                	//Around one fram
                    for (int i = 0; i < 29781; ++i) e {
                        //PPU
                        m_PPU.Step();
                        m_PPU.Step();
                        m_PPU.Step();
                        //CPU
                        m_CPU.Step();
                    }
                }
                else if (focus   && event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::F4) {
                    Log::get().SetLevel(Info);
                } else if (focus && event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::F5) {
                    Log::get().SetLevel(InfoVerbose);
                }
            }

            if (focus && !pause) {
                m_ElapsedTime += std::chrono::high_resolution_clock::now() - m_CycleTimer;
                m_CycleTimer   = std::chrono::high_resolution_clock::now();

                while (m_ElapsedTime > m_CPUCycleDuration) {
                    //PPU
                    m_PPU.Step();
                    m_PPU.Step();
                    m_PPU.Step();
                    //CPU
                    m_CPU.Step();

                    m_ElapsedTime -= m_CPUCycleDuration;
                }

                m_Window.draw(m_EmulatorScreen);
                m_Window.display();
            } else {
                sf::sleep(sf::milliseconds(1000/60));
            }
        }
    }



    void Emulator::DMA(Byte page) {
        m_CPU.SkipDMACycles();
        auto page_ptr = m_Bus.GetPagePtr(page);
        if (page_ptr != nullptr) {
            m_PPU.DoDMA(page_ptr);
        } else {
            LOG(Error) << "Can't get pageptr for DMA" << std::endl;
        }
    }



    void Emulator::SetVideoHeight(int height) {
        m_ScreenScale = height / float(NESVideoHeight);
        LOG(Info) << "Scale: " << m_ScreenScale << " set. Screen: "
                  << int(NESVideoWidth * m_ScreenScale) << "x" << int(NESVideoHeight * m_ScreenScale) << std::endl;
    }

    void Emulator::SetVideoWidth(int width) {
        m_ScreenScale = width / float(NESVideoWidth);
        LOG(Info) << "Scale: " << m_ScreenScale << " set. Screen: "
                  << int(NESVideoWidth * m_ScreenScale) << "x" << int(NESVideoHeight * m_ScreenScale) << std::endl;
    }



    void Emulator::setVideoScale(float scale)
    {
        m_ScreenScale = scale;
        LOG(Info) << "Scale: " << m_ScreenScale << " set. Screen: "
                  << int(NESVideoWidth * m_ScreenScale) << "x" << int(NESVideoHeight * m_ScreenScale) << std::endl;
    }



    void Emulator::setKeys(std::vector<sf::Keyboard::Key>& p1, std::vector<sf::Keyboard::Key>& p2)
    {
        m_Controller1.SetKeyBindings(p1);
        m_Controller2.SetKeyBindings(p2);
    }

};
