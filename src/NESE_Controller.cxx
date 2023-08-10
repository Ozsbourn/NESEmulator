#include "../NESE_Controller.hxx"


namespace NESE {

	Controller::Controller()
	: m_KeyStates(0),
	  m_KeyBindings(TotalButtons) {}



	void Controller::Strobe(BYTE byte) {
		m_Strobe = (byte & 1);

		if (!m_Strobe) {
			m_KeyStates = 0;

			std::int32_t shift = 0;
			for (auto button = Buttons::A; button < Buttons::TotalButtons; button++) {
				m_KeyStates |= (sf::Keyboard::isKeyPressed(m_KeyBindings[static_cast<Buttons>(button)]) << shift);
				shift++;
			}
		}
	}

	BYTE Controller::Read(void) {
		BYTE ret;

		if (m_Strobe) {
			ret = sf::Keyboard::isKeyPressed(m_KeyBindings[Buttons::A]);
		} else {
			ret = (m_KeyStates & 1);
			m_KeyStates >>= 1;
		}

		return ret |= 0x40;
	}

	void Controller::SetKeyBindings(const std::vector<sf::Keyboard::Key>& keybinds) {
		m_KeyBindings = keybinds;
	}

};