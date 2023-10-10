#ifndef NESE_MAPPERCD_HXX_
#define NESE_MAPPERCD_HXX_

namespace NESE {

    class MapperCD : public Mapper {
    private:
        NameTableMirroring m_Mirroring;
        
        std::uint32_t m_PRGBank;
        std::uint32_t m_CHRBank;

        std::function<void(void)> m_MirroringCallback;
    public:
        MapperCD(Cartridge& cart, std::function<void(void)> mirroring_cb);

        NameTableMirroring GetNameTableMirroring();

        void WritePRG(Address addr, BYTE val);
        void ReadPRG(Address addr);

        void WriteCHR(Address addr, BYTE val);
        void ReadCHR(Address addr);
    };

};

#endif // NESE_MAPPERCD_HXX_