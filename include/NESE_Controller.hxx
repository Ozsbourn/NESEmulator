#include <cstdint>
#include <vector>
#include <D:/Dev/Libs/SFML-2.5.1/include/SFML/Graphics.hpp>

#include "NESE_Util.hxx"



#ifndef NESE_CONTROLLER_HXX_
#define NESE_CONTROLLER_HXX_

namespace NESE {

	class Controller {
	private:
		bool     m_Strobe;
		uint32_t m_KeyStates;

		std::vector<sf::Keyboard::Key> m_KeyBindings;

	public:
		enum class Buttons {
			A,
			B,

			Select,
			Start,

			Up,
			Down,
			Left,
			Right,

			TotalButtons
		};

		Controller(void);


		void Strobe(BYTE byte);
		BYTE Read(void);
		void SetKeyBindings(const std::vector<sf::Keyboard::Key>& keybinds);


		~Controller(void);
	};

};

#endif // NESE_CONTROLLER_HXX_