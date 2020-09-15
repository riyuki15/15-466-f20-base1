#include "PPU466.hpp"
#include "Mode.hpp"

#include <glm/glm.hpp>
#include <chrono>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed, bool &player_state, uint8_t &bean_palette, bool &bg_change) override;
	virtual void draw(glm::uvec2 const &drawable_size, bool player_state, uint8_t bean_palette,
	        bool &bg_change, uint8_t &bg_palette) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//some weird background animation:
	float background_fade = 0.0f;

	int breath_count = 0;
	int bg_count = 0;

	//player position:
	glm::vec2 player_at = glm::vec2(50, 150);

	//color positions:
  std::array< glm::vec2, 6 > colors_at = {
          glm::vec2(100, 150),
          glm::vec2(200, 120),
          glm::vec2(50, 50),
          glm::vec2(70, 203),
          glm::vec2(154, 90),
          glm::vec2(180, 60)
  };
  glm::vec2 color_radius = glm::vec2(4, 4);
  glm::vec2 player_radius = glm::vec2(12, 12);

  bool game_started = false;
  bool started_counting = false;
  bool stopped_counting = false;
  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();


	//----- drawing handled by PPU466 -----

	PPU466 ppu;
};
