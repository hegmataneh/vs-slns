#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#define ROWS 3
#define COLS 3

int table[ROWS][COLS] = {
    {10, 20, 30},
    {40, 50, 60},
    {70, 80, 90}
};

struct termios orig_termios;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO); // disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void clearScreen() {
    printf("\033[2J\033[H");
}

void drawTable(int cursor_row, int cursor_col) {
    clearScreen();
    printf("Use arrow keys to navigate. Enter number to set value. Press 'q' to quit.\n\n");

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (i == cursor_row && j == cursor_col) {
                printf("\033[7m%4d\033[0m ", table[i][j]); // reverse video for cursor
            } else {
                printf("%4d ", table[i][j]);
            }
        }
        printf("\n");
    }
}

int main() {
    int row = 0, col = 0;
    enableRawMode();

    while (1) {
        drawTable(row, col);

        char c = getchar();
        if (c == 'q') break;

        if (c == '\033') { // Escape sequence
            getchar(); // skip '['
            switch(getchar()) {
                case 'A': if (row > 0) row--; break; // up
                case 'B': if (row < ROWS - 1) row++; break; // down
                case 'C': if (col < COLS - 1) col++; break; // right
                case 'D': if (col > 0) col--; break; // left
            }
        } else if (c >= '0' && c <= '9') {
            // allow user to enter 2-digit number
            int digit1 = c - '0';
            int digit2 = getchar() - '0';
            if (digit2 >= 0 && digit2 <= 9)
                table[row][col] = digit1 * 10 + digit2;
        }
    }

    disableRawMode();
    return 0;
}
