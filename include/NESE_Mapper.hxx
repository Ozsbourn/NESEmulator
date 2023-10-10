#include <memory>
#include <functional>

#include "NESE_CPUOpCodes.hxx"
#include "NESE_Cartridge.hxx"


#ifndef NESE_MAPPER_HXX_
#define NESE_MAPPER_HXX_

namespace NESE {
    
    enum class NameTableMirroring {
        Horizontal      = 0,
        Vertical        = 1,
        FourScreen      = 8,
        OneScreenLower,
        OneScreenHigher
    };



    class Mapper {
    private:
    protected:
        Cartridge& m_Cartridge;
        Type       m_Type;
    public:
        enum class Type {
            NROM        = 0,
            SxROM       = 1,
            UxROM       = 2,
            CNROM       = 3,
            MMC3        = 4,
            AxROM       = 7,
            ColorDreams = 11,
            GxROM       = 66
        };

        explicit Mapper(Cartridge& cartridge, Type t);

        virtual void WritePRG(Address addr, BYTE value) = 0;
        virtual BYTE ReadPRG(Address addr)              = 0;

        virtual BYTE ReadCHR(Address addr)              = 0;
        virtual void WriteCHR(Address addr, BYTE val)   = 0;

        virtual NameTableMirroring(void);

        bool inline HasExtendedRAM(void) {
            return m_Cartridge.HasExtendedRAM();
        }

        virtual void ScanLineIRQ(void);

        static std::unique_ptr<Mapper> CreateMapper(Type t, 
                                                    Cartridge& cart, 
                                                    std::function<void()>     interrupt_cb, 
                                                    std::function<void(void)> mirroring_cb);

        virtual ~Mapper(void);
    }

};

#endif // NESE_MAPPER_HXX_