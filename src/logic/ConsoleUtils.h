#pragma once

namespace flo {
	void setConsoleColor(int text_color, int background_color);

	void restoreConsoleDefaults();

	void setConsoleProgress(float progress);

	void printProgress(float progress);
}
