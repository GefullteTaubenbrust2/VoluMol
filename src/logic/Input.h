#pragma once
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../logic/Types.h"

namespace flo {
	///<summary>
	///One or more of these states may apply for a given input, not just in this file!
	///</summary>
	enum InputType {
		none = 0,
		released = 1,
		held = 2,
		hit = 4,
	};

	enum MouseButton {
		left = 0,
		right = 1,
		middle = 2,
	};

	extern u32 input_group;

	u32 uniqueInputGroup();

	void resetInputGroup();

	///<summary>
	///Get the state of a given key.
	///</summary>
	///<param name="key">The id of the key to check.</param>
	///<returns>The state of the key.</returns>
	InputType getKey(const uint key);

	///<summary>
	/// Set a string to which you may write using the keyboard.
	///</summary>
	///<param name="str">A pointer to the string in question.</param>
	///<param name="exit_state">A pointer to a bool that will be set to true when escape is hit. This may also be a nullptr.</param>
	///<param name="no_linebreak">When true, hitting the enter key will exit instead of placing a linebreak.</param>
	void setInputString(std::string* str, bool* exit_state, bool no_linebreak = false);

	///<summary>
	///Get the state of a given mouse button.
	///</summary>
	///<param name="mouse_button">The mouse button.</param>
	///<returns>The input type.</returns>
	InputType getMouseButton(const MouseButton mouse_button);

	///<summary>
	///Get the position of the cursor, in pixels from the top left corner of the frame.
	///</summary>
	///<returns>The position.</returns>
	const glm::vec2 getMousePosition();

	///<summary>
	///Get the mouse scroll.
	///</summary>
	///<returns>The amount of scroll as a vector. Only the y component is expected to be not 0.</returns>
	const glm::vec2 getScroll();

	///<summary>
	/// Register a key to query and get the ID.
	///</summary>
	///<returns>The ID of the key.</returns>
	uint registerInputKey(i16 key);

	///<summary>
	/// Get the position of the cursor in entered text.
	///</summary>
	///<returns>The index of the cursor.</returns>
	uint getTextCursorPos();
}
