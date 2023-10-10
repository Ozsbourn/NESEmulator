#ifndef NESE_MAPPERCNROM_HXX_
#define NESE_MAPPERCNROM_HXX_

namespace NESE {

    class MapperCNROM : public Mapper {
    public:
        MapperCNROM(Cartridge& cart);

        void WritePRG(Address addr, BYTE val);
        void ReadPRG(Address addr);

        void WriteCHR(Address addr, BYTE val);
        void ReadCHR(Address addr);
        
    private:
        bool    m_OneBank;
        Address m_SelectCHR;
    };

};

#endif // NESE_MAPPERCNROM_HXX_