#include "Mapper.hxx"

#include "NESE_CPUOpCodes.hxx"

//#include <NESE_MapperNROM.hxx>
#include <NESE_MapperCNROM.hxx>
//#include <NESE_MapperSxROM.hxx>
#include <NESE_MapperAxROM.hxx>
//#include <NESE_MapperUxROM.hxx>
#include <NESE_MapperGxROM.hxx>
//#include <NESE_MapperMMC3.hxx>
#include <NESE_MapperCD.hxx>
#include <NESE_MapperMMC3.hxx>



namespace NESE {
    
    NameTableMirroring Mapper::GetNameTableMirroring(void) {
        return static_cast<NameTableMirroring>(m_Cartridge.GetNameTableMirroring());
    }

    std::unique_ptr<Mapper> Mapper::CreateMapper(Mapper::Type t, Cartridge& cart, std::function<void()> interrupt_cb, std::function<void(void)> mirroring_cb) {
        std::unique_ptr<Mapper> ret(nullptr);

        switch (t) {
            case NROM:
                //ret.reset(new MapperNROM(cart));
                break;
            case SxROM:
                //ret.reset(new MapperSxROM(cart, mirroring_cb));
                break;
            case UxROM:
                //ret.reset(new MapperUxROM(cart));
                break;
            case CNROM:
                ret.reset(new MapperCNROM(cart));
                break;
            case MMC3:
                ret.reset(new MapperMMC3(cart, interrupt_cb, mirroring_cb));
                break;
            case AxROM:
                ret.reset(new MapperAxROM(cart, mirroring_cb));
                break;
            case ColorDreams:
                ret.reset(new MapperCD(cart, mirroring_cb));
                break;
            case GxROM:
                ret.reset(new MapperGxROM(cart, mirroring_cb));
                break;
            default:
                break;
        }

        return ret;
    }

};