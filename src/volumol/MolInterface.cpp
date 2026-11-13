#include "MolInterface.h"

#include "MolRenderer.h"
#include "Settings.h"

#include "../graphics/Window.h"

#include "../logic/Input.h"
#include "../logic/Time.h"

#include <sstream>

namespace mol {
	extern RenderProperties settings;

	void createWindow() {
		fgr::window::openWindow(800, 600, "VoluMol", true, 1);
		
		mol::Renderer::init();
	}

	void createContext() {
		fgr::window::openWindow(800, 600, "VoluMol", true, 1, false);

		mol::Renderer::init();
	}

	void interactiveInterface() {
		uint key_w = flo::registerInputKey(GLFW_KEY_W);
		uint key_a = flo::registerInputKey(GLFW_KEY_A);
		uint key_s = flo::registerInputKey(GLFW_KEY_S);
		uint key_d = flo::registerInputKey(GLFW_KEY_D);
		uint key_ctrl = flo::registerInputKey(GLFW_KEY_LEFT_CONTROL);
		uint key_shift = flo::registerInputKey(GLFW_KEY_LEFT_SHIFT);

		glm::vec3 position = glm::vec3(-15., 0., 0.);
		glm::vec3 direction = glm::vec3(1., 0., 0.);
		glm::vec3 up = glm::vec3(0.0, 0.0, 1.0);

		float rate = 2.f * settings.fov;
		if (settings.orthographic) rate = 140.f;

		float yaw = 0.f;
		float pitch = 0.f;

		glm::vec2 mouse_start;
		float roll_start = 0.0;

		float speed = 10.f;

		flo::Stopclock clock;

		uint frames = 0;
		uint seconds = 0;

		while (!fgr::window::isClosing()) {
			if ((uint)clock.stop().asSeconds() > seconds) {
				std::ostringstream oss;
				oss << "VoluMol | fps: " << frames;
				fgr::window::setTitle(oss.str());
				frames = 0;
				seconds = clock.stop().asSeconds();
			}

			fgr::window::checkEvents();

			if (flo::getMouseButton(flo::MouseButton::right) & flo::InputType::hit) {
				mouse_start = flo::getMousePosition();
			}
			else if (flo::getMouseButton(flo::MouseButton::right) & flo::InputType::held) {
				glm::vec2 mp = -(flo::getMousePosition() - mouse_start) * glm::radians(rate) / (float)fgr::window::width;
				yaw = glm::mod(mp.x, 6.283185307f);
				pitch = glm::clamp(mp.y, -1.5707f, 1.5707f);
				if (yaw || pitch) {
					glm::vec3 cross = glm::cross(direction, up);
					glm::vec3 new_direction = glm::normalize(glm::cos(pitch) * (glm::cos(yaw) * direction + glm::sin(yaw) * cross) - glm::sin(pitch) * up);
					glm::vec3 new_up = glm::normalize(glm::sin(pitch) * (glm::cos(yaw) * direction + glm::sin(yaw) * cross) + glm::cos(pitch) * up);
					direction = new_direction;
					up = new_up;

					mouse_start = flo::getMousePosition();
				}
			}
			if (flo::getMouseButton(flo::MouseButton::left) & flo::InputType::hit) {
				roll_start = flo::getMousePosition().x;
			}
			else if (flo::getMouseButton(flo::MouseButton::left) & flo::InputType::held) {
				float angle = 6.283185307f * (flo::getMousePosition().x - roll_start) / (float)fgr::window::width;
				glm::vec3 cross = glm::cross(direction, up);
				up = glm::normalize(up * glm::cos(angle) + cross * glm::sin(angle));
				roll_start = flo::getMousePosition().x;
			}

			float deltaTime = fgr::window::getDeltaTime();

			glm::vec3 side = glm::normalize(glm::cross(direction, up));

			if (flo::getKey(key_w) & flo::InputType::held) position += speed * direction * deltaTime;
			if (flo::getKey(key_s) & flo::InputType::held) position -= speed * direction * deltaTime;
			if (flo::getKey(key_d) & flo::InputType::held) position += speed * side * deltaTime;
			if (flo::getKey(key_a) & flo::InputType::held) position -= speed * side * deltaTime;
			if (flo::getKey(key_shift) & flo::InputType::held) position += up * speed * deltaTime;
			if (flo::getKey(key_ctrl) & flo::InputType::held) position -= up * speed * deltaTime;

			if (flo::getScroll().y > 0.00001) {
				speed *= 1.5;
			}
			else if (flo::getScroll().y < -0.00001) {
				speed /= 1.5;
			}
			speed = glm::clamp(speed, 0.1f, 100.f);

			fgr::window::clear(glm::vec3(1.));

			mol::Renderer::orientCamera(position, direction, up);

			mol::Renderer::renderFrame(fgr::window::width, fgr::window::height);

			++frames;

			fgr::window::flush();
		}
		fgr::window::keepOpen();
	}
}
