#pragma once

#include <SDL.h>
#include <glm/glm.hpp>

#include <memory>

struct Mode : std::enable_shared_from_this< Mode > {
	virtual ~Mode() { }

	//handle_event is called when new mouse or keyboard events are received:
	// (note that this might be many times per frame or never)
	//The function should return 'true' if it handled the event.
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) { return false; }

	//update is called at the start of a new frame, after events are handled:
	// 'elapsed' is time in seconds since the last call to 'update'
	virtual void update(float elapsed, bool &player_state, uint8_t &bean_palette, bool &bg_change) { }

	//draw is called after update:
	virtual void draw(glm::uvec2 const &drawable_size, bool player_state, uint8_t bean_palette, bool &bg_change,
	        uint8_t &bg_palette) = 0;

	//Mode::current is the Mode to which events are dispatched.
	// use 'set_current' to change the current Mode (e.g., to switch to a menu)
	static std::shared_ptr< Mode > current;
	static void set_current(std::shared_ptr< Mode > const &);
};

