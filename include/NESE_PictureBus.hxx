#include <vector>

#include "NESE_Cartridge.hxx"
#include "NESE_Mapper.hxx"


#ifndef NESE_PICTUREBUS_HXX_
#define NESE_PICTUREBUS_HXX_

namespace NESE {

    class PictureBus {
    private:
        // Indices where they start in RAM vector
        std::size_t m_NumTable0;
        std::size_t m_NumTable1;
        std::size_t m_NumTable2;
        std::size_t m_NumTable3;

        std::vector<BYTE> m_Palette;

        std::vector<BYTE> m_RAM;

        Mapper*           m_Mapper;

    public:
        explicit PictureBus(void);

        BYTE Read(Addres addr);
        void Write(Address addr, BYTE val);

        bool SetMapper(Mapper* mapper);

        BYTE ReadPallete(BYTE palleteAddr);

        void UpdateMirroring(void);

        void ScanLineIRQ(void);

        ~PictureBus(void) = default;
    };

};

#endif // NESE_PICTUREBUS_HXX_