#include <array>

#include "NESE_Mapper.hxx"

#ifndef NESE_MAPPERMMC3_HXX_
#define NESE_MAPPERMMC3_HXX_

namespace NESE {

    class MapperMMC3 : public Mapper {
    private:
        // Control vars
        std::uint32_t m_TargetRegister;
        bool          m_PRGBankMode;
        bool          m_CHRInversion;

        std::uint32_t m_BankRegister;

        bool m_IRQEnabled;
        BYTE m_IRQCounter;
        BYTE m_IRQLatch;
        bool m_IRQReloadPending;

        std::vector<BYTE> m_PRGRAM;
        std::vectro<BYTE> m_MirroringRAM;
        const BYTE*       m_PRGBank0;
        const BYTE*       m_PRGBank1;
        const BYTE*       m_PRGBank2;
        const BYTE*       m_PRGBank3;

        std::array<std::uint32_t, 0x8> m_CHRBanks;

        NameTableMirroring m_Mirroring;
        std::function<void(void)> m_MirroringCallback;
        std::function<void(void)> m_InterruptCallback;

    public:
        MapperMMC3(Cartridge& cart, std::function<void(void)> interrupt_cb, std::function<void(void)> mirroring_cb);

        BYTE ReadPRG(Address addr);
        void WritePRG(Address addr, BYTE val);

        NameTableMirroring GetNameTableMirroring(void);

        BYTE ReadCHR(Address addr);
        void WriteCHR(Address addr, BYTE val);

        void ScanLineIRQ(void);

    };

};

#endif // NESE_MAPPERMMC3_HXX_