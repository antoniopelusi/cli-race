#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#define W 80
#define H 24
#define D 75000
#define RW (W / 5)

#define ESC "\x1b"
#define C_RESET ESC "[0m"
#define C_GREEN ESC "[42m"
#define C_WHITE ESC "[47m"
#define C_BLACK ESC "[30m"
#define C_RED ESC "[41m"
#define C_DARK ESC "[40m"
#define C_BOLD ESC "[1m"
#define C_HIDE_CURSOR ESC "[?25l"
#define C_SHOW_CURSOR ESC "[?25h"
#define CURSOR_POS(row, col) ESC "[" #row ";" #col "H"
#define CLEAR_SCREEN "clear"
#define ENTER_ALTERNATE_BUFFER "\033[?1049h"
#define EXIT_ALTERNATE_BUFFER "\033[?1049l"

#define C_SCORE_TEXT C_BOLD C_BLACK C_GREEN

int d = D, rw = RW, s = 0;

void set_mode(int e) {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    if (e) {
        t.c_lflag |= ICANON | ECHO;
    } else {
        t.c_lflag &= ~(ICANON | ECHO);
        t.c_cc[VMIN] = 1;
        t.c_cc[VTIME] = 0;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

int kbhit(void) {
    struct termios t, o;
    int ch, f;
    tcgetattr(STDIN_FILENO, &o);
    t = o;
    t.c_lflag &= ~(ICANON | ECHO);
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
    f = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, f | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &o);
    fcntl(STDIN_FILENO, F_SETFL, f);
    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    } else {
        return 0;
    }
}

char get_input(void) {
    char c = getchar();
    tcflush(STDIN_FILENO, TCIFLUSH);
    return c;
}

void hide_cursor() {
    printf(C_HIDE_CURSOR);
    fflush(stdout);
}

void show_cursor() {
    printf(C_SHOW_CURSOR);
    fflush(stdout);
}

void enter_alternate_buffer(){
	printf(ENTER_ALTERNATE_BUFFER);
    fflush(stdout);
}

void exit_alternate_buffer(){
	printf(EXIT_ALTERNATE_BUFFER);
    fflush(stdout);
}

void print_score(int s){
	printf("%s", C_SCORE_TEXT);
	printf(CURSOR_POS(1, 1) "Score: %d\n", s);
	printf("%s", C_RESET);
	fflush(stdout);
}

void print_crash_prompt() {
    int text_length = 10;
    int start_row = H / 2;
    int start_col = (W - text_length) / 2;
    printf(CURSOR_POS(%d, %d) C_WHITE C_RED C_BOLD "GAME OVER!" C_RESET, start_row, start_col);

    int prompt_length = 17;
    int prompt_start_col = (W - prompt_length) / 2;
    printf(CURSOR_POS(%d, %d) C_WHITE C_RED C_BOLD "Play again? [Y/n]" C_RESET, start_row + 1, prompt_start_col);
    fflush(stdout);
}

void print_pause_screen() {
    int text_length = 5;
    int start_row = H / 2;
    int start_col = (W - text_length) / 2;
    printf(CURSOR_POS(%d, %d) C_WHITE C_RED C_BOLD "PAUSE" C_RESET, start_row, start_col);

    int prompt_length = 24;
    int prompt_start_col = (W - prompt_length) / 2;
    printf(CURSOR_POS(%d, %d) C_WHITE C_RED C_BOLD "Press 'r' to resume, 'q' to quit" C_RESET, start_row + 1, prompt_start_col);
    fflush(stdout);
}

void generate_row(char row[], int prev_center, int *new_center) {
    static int curvature = 0;
    curvature += (rand() % 3) - 1;
    if (curvature < -1) {
        curvature = -1;
    } else if (curvature > 1) {
        curvature = 1;
    }
    *new_center = prev_center + curvature;
    if (*new_center - rw / 2 < 0) {
        *new_center = rw / 2;
    } else if (*new_center + rw / 2 >= W) {
        *new_center = W - rw / 2 - 1;
    }
    int start = *new_center - rw / 2;
    int end = *new_center + rw / 2;
    for (int i = 0; i < W; i++) {
        if (i < start || i > end) {
            row[i] = 'G';
        } else if (i == start || i == end) {
            row[i] = 'E';
        } else {
            row[i] = 'D';
        }
    }
}

void print_row(char row[], int row_num) {
    printf(CURSOR_POS(%d, 1), row_num + 1);
    for (int i = 0; i < W; i++) {
        if (row[i] == 'G') {
            printf(C_GREEN " " C_RESET);
        } else if (row[i] == 'E') {
            printf(C_WHITE " " C_RESET);
        } else if (row[i] == 'D') {
            printf(C_DARK " " C_RESET);
        } else {
            printf(" ");
        }
    }
}

int check_collision(int car_pos, char map[][W]) {
    if (map[19][car_pos - 1] == 'G' || map[19][car_pos] == 'G') {
        return 1;
    } else {
        return 0;
    }
}

void move_car(int *car_pos, char input) {
    if (input == 'a' && *car_pos > 1) {
        *car_pos -= 2;
    } else if (input == 'd' && *car_pos < W - 1) {
        *car_pos += 2;
    }
}

void print_top_row(void) {
    printf(CURSOR_POS(1, 1) C_DARK);
    for (int i = 0; i < W; i++) {
        printf(" ");
    }
    printf(C_RESET);
}

void clear_map(char map[H][W], int prev_center) {
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            if (j < prev_center - rw / 2 || j > prev_center + rw / 2) {
                map[i][j] = 'G';
            } else if (j == prev_center - rw / 2 || j == prev_center + rw / 2) {
                map[i][j] = 'E';
            } else {
                map[i][j] = 'D';
            }
        }
    }
}

void bot(int *car_pos, char map[H][W]) {
    if (map[18][*car_pos - 1] == 'G' || map[18][*car_pos] == 'G') {
        if (map[18][*car_pos - 2] != 'G') {
        	move_car(car_pos, 'a');
        } else if (map[18][*car_pos + 1] != 'G') {
       		move_car(car_pos, 'd');
        }
    }
}

int main() {
    char map[H][W];
    srand(time(NULL));

    int highest_score = 0;

    enter_alternate_buffer();

    set_mode(0);
    hide_cursor();
    while (1) {
        int prev_center = W / 2;
        clear_map(map, prev_center);

        system(CLEAR_SCREEN);
        int car_pos = W / 2;
        s = 0;
        rw = RW;
        d = D;

        while (1) {
            if (kbhit()) {
                char input = get_input();
                if (input == 27) {
                    print_pause_screen();
                    while (1) {
                        if (kbhit()) {
                            char pause_input = get_input();
                            if (pause_input == 'r' || pause_input == 'R') {
                                system(CLEAR_SCREEN);
                                break;
                            } else if (pause_input == 'q' || pause_input == 'Q') {
                                system(CLEAR_SCREEN);
                                exit_alternate_buffer();
                                set_mode(1);
                                show_cursor();
                                printf(C_BOLD "Highest Score: %d\n" C_RESET, highest_score);
                                return 0;
                            }
                        }
                    }
                }
                move_car(&car_pos, input);
            } else {
                bot(&car_pos, map);
            }

            char new_row[W];
            int new_center;
            generate_row(new_row, prev_center, &new_center);
            prev_center = new_center;

            for (int i = H - 1; i > 0; i--) {
                for (int j = 0; j < W; j++) {
                    map[i][j] = map[i - 1][j];
                }
            }

            for (int j = 0; j < W; j++) {
                map[0][j] = new_row[j];
            }

            for (int i = 0; i < H; i++) {
                print_row(map[i], i);
            }

            s++;

            print_score(s);
            printf(CURSOR_POS(20, %d) C_RED "**" C_RESET, car_pos);

            if (check_collision(car_pos, map)) {
                print_crash_prompt();
                while (1) {
                    if (kbhit()) {
                        char input = get_input();
                        if (input == 'Y' || input == 'y') {
                            if (s > highest_score) {
                                highest_score = s;
                            }
                            break;
                        } else if (input == 'N' || input == 'n' || input == 27) {
                            if (s > highest_score) {
                                highest_score = s;
                            }
                            system(CLEAR_SCREEN);
                            exit_alternate_buffer();
                            printf(C_BOLD "Highest Score: %d\n" C_RESET, highest_score);
                            set_mode(1);
                            show_cursor();
                            return 0;
                        }
                    }
                }
                break;
            }

            fflush(stdout);
            usleep(d);

            if (s % 100 == 0 && rw > 8) {
                rw--;
            }
            d -= 10;
        }

        system(CLEAR_SCREEN);
    }

    return 0;
}
