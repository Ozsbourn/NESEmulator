#ifndef NESE_MAPPERGXROM_HXX_
#define NESE_MAPPERGXROM_HXX_

namespace NESE {

	class MapperGxROM : public Mapper {
    private:
        NameTableMirroring        m_Mirroring;

        std::vector<Byte>         m_CharacterRAM;
        std::function<void(void)> m_MirroringCallback;

    public:
        MapperGxROM(Cartridge &cart, std::function<void(void)> mirroring_cb);
        NameTableMirroring GetNameTableMirroring();
        void WritePRG(Address address, Byte value);
        Byte ReadPRG(Address address);

        Byte ReadCHR(Address address);
        void WriteCHR(Address address, Byte value);
        
        Byte PRGBank;
        Byte CHRBank;

    };

};

#endif // NESE_MAPPERGXROM_HXX_