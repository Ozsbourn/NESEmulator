#include <functional>
#include <array>

#include "NESE_PictureBus.hxx"
#include "NESE_MainBus.hxx"
#include "NESE_VirtualScreen.hxx"
#include "NESE_PaletteColors.hxx"

#ifndef NESE_PPU_HXX_
#define NESE_PPU_HXX_

namespace NESE {

    const int ScanlineCycleLength = 341;
    const int ScanlineEndCycle    = 340;
    const int VisibleScanlines    = 240;
    const int ScanlineVisibleDots = 256;
    const int FrameEndScanline    = 261;

    const int AttributeOffset     = 0x3C0;

    class PPU {
    public:
        PPU(PictureBus &bus, VirtualScreen &screen);
        void Step();
        void Reset();

        void SetInterruptCallback(std::function<void(void)> cb);

        void DoDMA(const BYTE* page_ptr);

        // Callbacks mapped to CPU address space
        // Addresses written to by the program
        void Control(BYTE ctrl);
        void SetMask(BYTE mask);
        void SetOAMAddress(BYTE addr);
        void SetDataAddress(BYTE addr);
        void SetScroll(BYTE scroll);
        void SetData(BYTE data);

        // Read by the program
        BYTE GetStatus();
        BYTE GetData(void);
        BYTE GetOAMData();
        void SetOAMData(BYTE value);
    private:
        BYTE readOAM(BYTE addr);
        void writeOAM(BYTE addr, BYTE value);
        BYTE read(Address addr);

        PictureBus    &m_Bus;
        VirtualScreen &m_Screen;

        std::function<void(void)> m_VBlankCallback;

        std::vector<BYTE> m_SpriteMemory;

        std::vector<BYTE> m_ScanlineSprites;

        enum State {
            PreRender,
            Render,
            PostRender,
            VerticalBlank
        } m_PipelineState;

        int  m_Cycle;
        int  m_Scanline;
        bool m_EvenFrame;

        bool m_VBlank;
        bool m_SprZeroHit;
        bool m_SpriteOverflow;

        //Registers
        Address m_DataAddress;
        Address m_TempAddress;
        BYTE    m_FineXScroll;
        bool    m_FirstWrite;
        BYTE    m_DataBuffer;

        BYTE    m_SpriteDataAddress;

        // Setup flags and variables
        bool    m_LongSprites;
        bool    m_GenerateInterrupt;

        bool    m_GreyscaleMode;
        bool    m_ShowSprites;
        bool    m_ShowBackground;
        bool    m_HideEdgeSprites;
        bool    m_HideEdgeBackground;

        enum CharacterPage {
            Low,
            High,
        } m_bgPage, m_sprPage;

        Address m_DataAddrIncrement;

        std::vector<std::vector<sf::Color>> m_PictureBuffer;
    };

};

#endif // NESE_PPU_HXX_