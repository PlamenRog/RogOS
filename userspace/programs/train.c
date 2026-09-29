#include <string.h>
#include <unistd.h>

#define SCREEN_WIDTH 79

static void delay(void) {
	for (volatile unsigned long i = 0; i < 6000000; i++) {
		__asm__ volatile("" ::: "memory");
	}
}

#define TRAIN_HEIGHT 7

static const char* train[TRAIN_HEIGHT] = {
	"         ------+      ++          ",
	"          | +-+|      ||          ",
	"          | | ||---------\\        ",
	"          | +-+  ======== +       ",
	"         +-//~O========O--|_      ",
	"            \\_//      \\_// \\\\\\\\   ",
	"         =======================  "
};

int main(void) {
	static const char clear_screen[] = "\x1B[2J\x1B[H";

	write(1, clear_screen, sizeof(clear_screen) - 1);

	int train_width = (int)strlen(train[0]);

	char frame[TRAIN_HEIGHT * (SCREEN_WIDTH + 1)];

	for (int position = -train_width; position < SCREEN_WIDTH; position++) {
		int out = 0;

		// 4 rows for ascii
		for (int row = 0; row < TRAIN_HEIGHT; row++) {
			for (int x = 0; x < SCREEN_WIDTH; x++) {
				frame[out + x] = ' ';
			}

			for (int i = 0; i < train_width; i++) {
				int x = position + i;

				if (x >= 0 && x < SCREEN_WIDTH) {
					frame[out + x] = train[row][i];
				}
			}

			out += SCREEN_WIDTH;

			frame[out++] = '\n';
		}

		// draw complete frame
		write(1, frame, out);

		// move back to top of animation unless final frame
		if (position != SCREEN_WIDTH - 1) {
			static const char cursor_up[] = "\x1B[7A"; // ESC [ 4 A

			write(1, cursor_up, sizeof(cursor_up) - 1);
		}

		delay();
	}

	return 0;
}
