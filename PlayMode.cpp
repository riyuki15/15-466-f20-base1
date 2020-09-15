#include "PlayMode.hpp"
#include "load_save_png.hpp"
#include "glm/gtx/string_cast.hpp"
#include <stdlib.h>     /* srand, rand */
#include <chrono>

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>

PlayMode::PlayMode() {

  //background
  ppu.palette_table[0] = {glm::u8vec4(0), glm::u8vec4(0), glm::u8vec4(0), glm::u8vec4(0)};

  auto set_palette = [this](int palette_index, glm::uvec2 size, std::vector< glm::u8vec4 > data) {
    glm::u8vec4 color0 = glm::u8vec4(0);
    glm::u8vec4 color1 = glm::u8vec4(0);
    glm::u8vec4 color2 = glm::u8vec4(0);
    glm::u8vec4 color3 = glm::u8vec4(0);
    int color_count = 0;
    for (int j = 0; j < size[0] * size[1]; j++) {
      glm::u8vec4 color = data[j];
      if (color0 == color || color1 == color || color2 == color || color3 == color) continue;
      if (color_count == 0) color0 = color;
      else if (color_count == 1) color1 = color;
      else if (color_count == 2) color2 = color;
      else color3 = color;
      color_count++;
      if (color_count == 4) break;
    }
    ppu.palette_table[palette_index] = {color0, color1, color2, color3};
  };

  auto set_bits = [this](int r0, int c0,
          int tile_index, int palette_index, glm::uvec2 size, std::vector< glm::u8vec4 > data) {
    std::array< uint8_t, 8 > bit0;
    std::array< uint8_t, 8 > bit1;
    glm::u8vec4 color0 = ppu.palette_table[palette_index][0];
    glm::u8vec4 color1 = ppu.palette_table[palette_index][1];
    glm::u8vec4 color2 = ppu.palette_table[palette_index][2];
    glm::u8vec4 color3 = ppu.palette_table[palette_index][3];
    for (int r = 0; r < 8; r++) {
      for (int c = 0; c < 8; c++) {
        int sprite_index = (r0 + r) * size[0] + (c0 + c);
        glm::u8vec4 color = data[sprite_index];
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
    ppu.tile_table[tile_index].bit0 = bit0;
    ppu.tile_table[tile_index].bit1 = bit1;
  };

  // load lil' bean
  glm::uvec2 player_size;
  glm::uvec2 player_size2;
  std::vector< glm::u8vec4 > player_data;
  std::vector< glm::u8vec4 > player_data2;
  OriginLocation origin = LowerLeftOrigin;
  load_png("bean.png", &player_size, &player_data, origin);
  load_png("bean2.png", &player_size2, &player_data2, origin);

  set_palette(7, player_size, player_data);

  // separate into 16 tiles because the player sprite is 32 x 32
  int index = 32;
  for (int i = 0; i < player_size[0]/8; i++) {
    for (int j = 0; j < player_size[1]/8; j++) {
      set_bits(i*8, j*8, index, 7, player_size, player_data);
      index++;
    }
  }
  index = 48;
  for (int i = 0; i < player_size[0]/8; i++) {
    for (int j = 0; j < player_size[1]/8; j++) {
      set_bits(i*8, j*8, index, 7, player_size, player_data2);
      index++;
    }
  }

  // load color blocks
  glm::uvec2 color_size;
  std::vector< glm::u8vec4 > pink_data;
  std::vector< glm::u8vec4 > yellow_data;
  std::vector< glm::u8vec4 > blue_data;
  std::vector< glm::u8vec4 > purple_data;
  std::vector< glm::u8vec4 > red_data;
  std::vector< glm::u8vec4 > gray_data;
  load_png("pink.png", &color_size, &pink_data, origin);
  load_png("yellow.png", &color_size, &yellow_data, origin);
  load_png("blue.png", &color_size, &blue_data, origin);
  load_png("purple.png", &color_size, &purple_data, origin);
  load_png("red.png", &color_size, &red_data, origin);
  load_png("gray.png", &color_size, &gray_data, origin);

  set_palette(1, color_size, pink_data);
  set_palette(2, color_size, yellow_data);
  set_palette(3, color_size, blue_data);
  set_palette(4, color_size, purple_data);
  set_palette(5, color_size, red_data);
  set_palette(6, color_size, gray_data);

  set_bits(0, 0, 64, 1, color_size, pink_data);
  set_bits(0, 0, 65, 2, color_size, yellow_data);
  set_bits(0, 0, 66, 3, color_size, blue_data);
  set_bits(0, 0, 67, 4, color_size, purple_data);
  set_bits(0, 0, 68, 5, color_size, red_data);
  set_bits(0, 0, 69, 6, color_size, gray_data);


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

void PlayMode::update(float elapsed, bool &player_state, uint8_t &bean_palette, bool &bg_change) {

  bg_count++;
  if (bg_count == 200) {
    bg_change = true;
    bg_count = 0;
  }

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

	for (int i = 0; i < colors_at.size(); i++) {
	  int palette_index = 1 + i;
    glm::vec2 const color_at = colors_at[i];
    glm::vec2 min = glm::max(color_at - color_radius, player_at - player_radius);
    glm::vec2 max = glm::min(color_at + color_radius, player_at + player_radius);
    if (min.x <= max.x && min.y <= max.y) bean_palette = palette_index;;
	}

}

void PlayMode::draw(glm::uvec2 const &drawable_size, bool player_state,
        uint8_t bean_palette, bool &bg_change, uint8_t &bg_palette) {


  if (bg_change) {
    game_started = true;
    bg_palette = rand() % colors_at.size() + 1;
    bg_change = false;
  }
  ppu.background_color = ppu.palette_table[bg_palette][1];

	if (game_started && bean_palette == bg_palette && !started_counting) {
    begin = std::chrono::steady_clock::now();
    started_counting = true;
    stopped_counting = false;
	} else if (game_started && bean_palette != bg_palette && !stopped_counting) {
	  stopped_counting = true;
	  started_counting = false;
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "You were invisible for " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]!" << std::endl;
	}

  for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
    for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
      ppu.background[x+PPU466::BackgroundWidth*y] = 0;
    }
  }

	//player sprite:
	for (int i = 0; i < 4; i++) {
	  for (int j = 0; j < 4; j++) {
	    int index = i * 4 + j;
      ppu.sprites[index].x = int32_t(player_at.x) + j*8;
      ppu.sprites[index].y = int32_t(player_at.y) + i*8;
      if (player_state) ppu.sprites[index].index = 32 + index;
      else ppu.sprites[index].index = 48 + index;
      ppu.sprites[index].attributes = bean_palette;
	  }
	}

	//color sprites:
  for (int i = 16; i < 16 + colors_at.size(); i++) {
    int index = i - 16;
    ppu.sprites[i].index = 64 + index;
    ppu.sprites[i].attributes = index + 1;
    ppu.sprites[i].x = colors_at[index].x;
    ppu.sprites[i].y = colors_at[index].y;
  }

	for (int i = 16 + colors_at.size(); i < 64; i++) {
    ppu.sprites[i].x = 240;
    ppu.sprites[i].y = 250;
	}

	//--- actually draw ---
	ppu.draw(drawable_size);
}
