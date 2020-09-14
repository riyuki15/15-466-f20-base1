#include "PlayMode.hpp"
#include "load_save_png.hpp"
#include "glm/gtx/string_cast.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>

PlayMode::PlayMode() {

  // load lil' bean
  glm::uvec2 player_size;
  std::vector< glm::u8vec4 > player_data;
  OriginLocation player_origin = LowerLeftOrigin;
  load_png("bean.png", &player_size, &player_data, player_origin);
  glm::uvec2 player_size2;
  std::vector< glm::u8vec4 > player_data2;
  load_png("bean2.png", &player_size2, &player_data2, player_origin);

  auto set_palette = [this](int index, glm::uvec2 size, std::vector< glm::u8vec4 > data) {
    glm::u8vec4 color0;
    glm::u8vec4 color1;
    glm::u8vec4 color2;
    glm::u8vec4 color3;
    int color_count = 0;
    for (int j = 0; j < size[0] * size[1]; j++) {
      glm::u8vec4 color = data[j];
      if (color0 == color || color1 == color || color2 == color || color3 == color) continue;
      std::cout<<glm::to_string(color);
      if (color_count == 0) color0 = color;
      else if (color_count == 1) color1 = color;
      else if (color_count == 2) color2 = color;
      else color3 = color;
      color_count++;
      if (color_count == 4) break;
    }
    ppu.palette_table[7] = {color0, color1, color2, color3};
  };

  set_palette(7, player_size, player_data);

  auto set_bits = [this]
          (int r0, int c0, int index, int palette_index, std::vector< glm::u8vec4 > data) {
    std::array< uint8_t, 8 > bit0;
    std::array< uint8_t, 8 > bit1;
    glm::u8vec4 color0 = ppu.palette_table[palette_index][0];
    glm::u8vec4 color1 = ppu.palette_table[palette_index][1];
    glm::u8vec4 color2 = ppu.palette_table[palette_index][2];
    glm::u8vec4 color3 = ppu.palette_table[palette_index][3];
    for (int r = 0; r < 8; r++) {
      for (int c = 0; c < 8; c++) {
        int player_index = (r0 + r) * 32 + (c0 + c);
        glm::u8vec4 color = data[player_index];
        if (color == color0) { //00
          bit0[r] &= ~(0b1 << c);
          bit1[r] &= ~(0b1 << c);
        } else if (color == color1) { //01
          bit0[r] |= (0b1 << c);
          bit1[r] &= ~(0b1 << c);
        } else if (color == color2) { //10
          bit0[r] &= ~(0b1 << c);
          bit1[r] |= (0b1 << c);
        } else if (color == color3) { //11
          bit0[r] |= (0b1 << c);
          bit1[r] |= (0b1 << c);
        }
      }
    }
    ppu.tile_table[index].bit0 = bit0;
    ppu.tile_table[index].bit1 = bit1;
  };

  int index = 32;
  for (int i = 0; i < player_size[0]/8; i++) {
    for (int j = 0; j < player_size[1]/8; j++) {
      set_bits(i*8, j*8, index, 7, player_data);
      index++;
    }
  }
  index = 48;
  for (int i = 0; i < player_size[0]/8; i++) {
    for (int j = 0; j < player_size[1]/8; j++) {
      set_bits(i*8, j*8, index, 7, player_data2);
      index++;
    }
  }

}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed, bool &player_state) {

	constexpr float PlayerSpeed = 100.0f;
	if (left.pressed) player_at.x -= PlayerSpeed * elapsed;
	if (right.pressed) player_at.x += PlayerSpeed * elapsed;
	if (down.pressed) player_at.y -= PlayerSpeed * elapsed;
	if (up.pressed) player_at.y += PlayerSpeed * elapsed;

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;

	breath_count++;
	if (breath_count == 10) {
	  player_state = !player_state;
	  breath_count = 0;
	}
}

void PlayMode::draw(glm::uvec2 const &drawable_size, bool player_state) {

	//--- set ppu state based on game state ---

	//player sprite:
	for (int i = 0; i < 4; i++) {
	  for (int j = 0; j < 4; j++) {
	    int index = i * 4 + j;
      ppu.sprites[index].x = int32_t(player_at.x) + j*8;
      ppu.sprites[index].y = int32_t(player_at.y) + i*8;
      if (player_state) ppu.sprites[index].index = 32 + index;
      else ppu.sprites[index].index = 48 + index;
      ppu.sprites[index].attributes = 7;
	  }
	}

	for (int i = 16; i < 64; i++) {
    ppu.sprites[i].x = 240;
    ppu.sprites[i].y = 240;
	}


	//--- actually draw ---
	ppu.draw(drawable_size);
}
