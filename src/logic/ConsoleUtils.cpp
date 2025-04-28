#include "ConsoleUtils.h"
#include <glm/glm.hpp>
#ifdef _WIN32
#include <Windows.h>
#endif
#include <iostream>

namespace flo {
	void setConsoleColor(int text_color, int background_color) {
	#ifdef _WIN32
		int k = glm::clamp(text_color, 0, 15) + glm::clamp(background_color, 0, 15) * 16;
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, k);
	#elif linux
		const int text_colors[16] = {30, 34, 32, 36, 31, 35, 33, 37, 90, 94, 92, 96, 91, 95, 93, 97};
		const int background_colors[16] = {49, 44, 42, 46, 41, 45, 43, 47, 100, 104, 102, 106, 101, 105, 103, 107};
		std::cout << "\033[" << text_colors[text_color] << 'm';
		std::cout << "\033[" << background_colors[background_color] << 'm';
	#endif
	}
	void restoreConsoleDefaults() {
		setConsoleColor(7, 0);
	}

	float setpoint = 0.f;

	void setConsoleProgress(float progress) {
		setpoint = progress;
	}

	void printProgress(float progress) {
		progress = setpoint + progress * (1.f - setpoint);
		const int progress_bar_width = 40;
		int p = progress * progress_bar_width;
		if (p < 0) p = 0;
		int q = setpoint * progress_bar_width;
		if (q < 0) q = 0;
		char bar[progress_bar_width + 1];
		for (int i = 0; i < q && i < progress_bar_width; ++i) {
			bar[i] = '#';
		}
		for (int i = q; i < p && i < progress_bar_width; ++i) {
			bar[i] = '=';
		}
		for (int i = p; i < progress_bar_width; ++i) {
			bar[i] = '-';
		}
		bar[progress_bar_width] = '\0';
		std::cout << "0% " << bar << " 100%" << '\r' << std::flush;
	}
}
