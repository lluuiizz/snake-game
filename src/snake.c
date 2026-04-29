#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

#define SNAKE_CHAR '#'
#define INITIAL_SNAKE_LENGTH 5
#define SIDE_SIZE 30 
#define SNAKE_SIZE_LIMIT (SIDE_SIZE * SIDE_SIZE) - (4 * SIDE_SIZE - 4)
#define STDIN 0     // INPUT FILE DESCRIPTOR
#define TIME_SEC 0
#define TIME_USEC 250 * 1000    //250

typedef struct snake_t {
    unsigned int pos_x, pos_y;
    struct snake_t *next_body_cell, *prev_body_cell;
} snake_t; 

snake_t *snake_head = NULL;
snake_t *snake_tail = NULL;
unsigned int snake_size = 0;

typedef struct fruit_t {
    bool put_fruit_on_board;
    unsigned int pos_x, pos_y;
} fruit_t;

fruit_t *fruit = NULL;

char board[SIDE_SIZE][SIDE_SIZE];

const char MOVE_UP = 'w';
const char MOVE_DOWN = 's';
const char MOVE_RIGHT = 'd';
const char MOVE_LEFT = 'a';

void new_cell (char);
void new_fruit ();
void initialize_board();
void update_snake(char, unsigned int *);
bool check_next_move_collision(int pos_x, int pos_y);
void decide_next_movement(char* curr_direction);
void update_board_state();
bool check_snake_body_collision();
bool check_win();
void keyboard_listener(char *);
void print_board_on_screen();
unsigned int gen_num_in_board();

int main (void) {
    char snake_move_direction = 's';
    
    // ENABLE NON BLOCKING INPUT BY TURNING OFF ECHO AND ICANON 
    static struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    
    for (int i = 1; i < INITIAL_SNAKE_LENGTH; i++) {
        new_cell(snake_move_direction);
    }
    fruit = (fruit_t*) malloc(sizeof(fruit_t));
    initialize_board();
    new_fruit();
    unsigned int fruits_eaten_count = 0;
    //new_fruit();
    //updateBoard(gameBoard); 
    while(true) {
        system("clear");
        printf("\t\t\t SNAKE GAME\n\t\t\t FRUITS EATEN: %u\n", fruits_eaten_count);
        decide_next_movement(&snake_move_direction);
        update_snake(snake_move_direction, &fruits_eaten_count);
        update_board_state();
        print_board_on_screen();
        keyboard_listener(&snake_move_direction); 
        
        if (fruit->put_fruit_on_board) {
            new_fruit();
            printf("\a");

        }

        if (check_snake_body_collision()) {
            printf("\t\tYour body collided, sorry! :(");
            printf("\n\t\tPress any key to exit: ");
            break;
        }
       if (check_win()) {
            printf("\t\tCongratulations, You Won the game!!!"); 
            printf("\n\t\tPress any key to exit: ");
            break;
        }
    }    
    getchar();
    printf("\n");
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // RETURN TO THE DEFAULT OPTIONS AFTER EXECUTION
    return 0;
}

void new_cell (char snake_move_direction) {
    
    if (snake_head == NULL) {
        snake_head = (snake_t*) malloc(sizeof(snake_t));
        snake_head->pos_y = SIDE_SIZE / 2 - 1;
        snake_head->pos_x = SIDE_SIZE / 2 - 1; 
        snake_head->next_body_cell = NULL;
        snake_head->prev_body_cell = NULL;
        snake_tail = snake_head;
    } else { 
        
        snake_t *new_cell = (snake_t*) malloc(sizeof(snake_t));
        switch (snake_move_direction) {
            case 'w':
                new_cell->pos_x = snake_tail->pos_x;
                new_cell->pos_y = snake_tail->pos_y + 1;
                break;
            case 'd':
                new_cell->pos_x = snake_tail->pos_x - 1;
                new_cell->pos_y = snake_tail->pos_y;
                break;
            case 'a':
                new_cell->pos_x = snake_tail->pos_x + 1;
                new_cell->pos_y = snake_tail->pos_y;
                break;
            case 's':  
                new_cell->pos_x = snake_tail->pos_x;
                new_cell->pos_y = snake_tail->pos_y - 1;
                break;

        }
        snake_tail->next_body_cell = new_cell;
        new_cell->prev_body_cell = snake_tail;

        snake_tail = new_cell;
   }
    snake_size += 1;
}

void new_fruit () {
    while (board[fruit->pos_y][fruit->pos_x] != ' ') {
        do {
            fruit->pos_x = gen_num_in_board();
            fruit->pos_y = gen_num_in_board();

        } while (check_next_move_collision(fruit->pos_x, fruit->pos_y) == true);
    }
   
    board[fruit->pos_y][fruit->pos_x] = '*';
    fruit->put_fruit_on_board = false;

}

void initialize_board() {
   	for (unsigned int y = 0; y < SIDE_SIZE; y++){
		for (unsigned int x = 0; x < SIDE_SIZE; x++) {
			if (y == 0 || y == SIDE_SIZE - 1 || x == 0 || x == SIDE_SIZE - 1) {
				board[y][x] = '.';
		    }   else {
                    board[y][x] = ' ';
            }   
	    }
	}
}


double calculate_distance (int snake_x, int snake_y, int fruit_x, int fruit_y) {
    double dist = sqrt(pow((fruit_x - snake_x), 2) + pow((fruit_y - snake_y),2 ));
    return dist;
}

bool check_next_move_collision(int pos_x, int pos_y) {
    snake_t *body_cell = snake_head->next_body_cell;
    while (body_cell != NULL) {
        if (pos_x == body_cell->pos_x && pos_y == body_cell->pos_y) {
            return true;
        }   
       body_cell = body_cell->next_body_cell; 
    }
    return false;
}
void decide_next_movement(char* curr_direction) {
    const char direction[] = {MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT};
    char avoid_direction;

    if (*curr_direction == MOVE_UP)
        avoid_direction = MOVE_DOWN;
    else if (*curr_direction == MOVE_DOWN)
        avoid_direction = MOVE_UP;
    else if (*curr_direction == MOVE_RIGHT)
        avoid_direction = MOVE_LEFT;
    else if (*curr_direction == MOVE_LEFT)
        avoid_direction = MOVE_RIGHT;


    double last_distance = calculate_distance(snake_head->pos_x, snake_head->pos_y, fruit->pos_x, fruit->pos_y);
    char last_direction = *curr_direction;

    for (size_t i = 0; i < sizeof(direction); i++) {
        int newer_pos_x = snake_head->pos_x, newer_pos_y = snake_head->pos_y;
        
        // Restriction 1: Can't move in the opposite direction
        if (direction[i] == avoid_direction)
            continue;

        if (direction[i] == MOVE_UP) 
            newer_pos_y = (snake_head->pos_y - 1 >= 1)
            ? snake_head->pos_y - 1 : SIDE_SIZE - 2;
        else if (direction[i] == MOVE_DOWN)
          newer_pos_y = (snake_head->pos_y + 1 <= SIDE_SIZE - 2)
          ? snake_head->pos_y + 1 : 1;
        else if (direction[i] == MOVE_RIGHT)
            newer_pos_x = (snake_head->pos_x + 1 <= SIDE_SIZE - 2)
            ? snake_head->pos_x + 1 : 1;
        else if (direction[i] == MOVE_LEFT)
           newer_pos_x = (snake_head->pos_x - 1 >= 1) 
          ? snake_head->pos_x - 1 : SIDE_SIZE - 2;           

        // Restriction 2: Can't collide with own body
        if (check_next_move_collision(newer_pos_x, newer_pos_y))
                continue;
        double newer_distance = calculate_distance(newer_pos_x, newer_pos_y, fruit->pos_x, fruit->pos_y);
        printf("Distance : %f\n", newer_distance);

        // Restriction 3: Newer position distance can't be greater than the last
        if (newer_distance <= last_distance) {
            last_distance = newer_distance;
            last_direction = direction[i];
        }
    }

    *curr_direction = last_direction;
}
void update_snake (char snake_move_direction, unsigned int *fruits_eaten_count) {
    snake_t *head_buff = snake_head;
    snake_t *tail_buff = snake_tail;
    
    board[snake_tail->pos_y][snake_tail->pos_x] = ' ';
    
    while(snake_tail->prev_body_cell != NULL) {
        snake_tail->pos_x = snake_tail->prev_body_cell->pos_x;
        snake_tail->pos_y = snake_tail->prev_body_cell->pos_y;
        snake_tail = snake_tail->prev_body_cell;
    
    }
    
    switch(snake_move_direction) {
        case 'w':
            snake_head->pos_y = (snake_head->pos_y-- > 1)
            ? snake_head->pos_y-- : SIDE_SIZE - 2;
            break;
        case 'd':
            snake_head->pos_x = (snake_head->pos_x++ < SIDE_SIZE - 2)
            ? snake_head->pos_x++ : 1;
            break;
        case 'a':
          snake_head->pos_x = (snake_head->pos_x-- > 1) 
          ? snake_head->pos_x-- : SIDE_SIZE - 2;
          break;
        case 's':
          snake_head->pos_y = (snake_head->pos_y++ < SIDE_SIZE - 2)
          ? snake_head->pos_y++ : 1;
          break;
      }   
    snake_head = head_buff;
    snake_tail = tail_buff;

    if (snake_head->pos_x == fruit->pos_x && snake_head->pos_y == fruit->pos_y) {
        new_cell(snake_move_direction);
        fruit->put_fruit_on_board = true;
        *fruits_eaten_count += 1;
    }
}

void update_board_state() {
    snake_t *head_buff = snake_head;
    snake_t* tail_buff = snake_tail;
    
    while (snake_head != NULL) {
        if (snake_head == head_buff)
            board[snake_head->pos_y][snake_head->pos_x] = '8';
        else if (snake_head == tail_buff)
            board[snake_head->pos_y][snake_head->pos_x] = '0';
        else
            board[snake_head->pos_y][snake_head->pos_x] = '#';
        snake_head = snake_head->next_body_cell;   
    }

    snake_head = head_buff;
} 

bool check_snake_body_collision() {
    snake_t *body_cell = snake_head->next_body_cell;
    bool collide = false;
    while (body_cell != NULL) {
        if (snake_head->pos_x == body_cell->pos_x && snake_head->pos_y == body_cell->pos_y) {
            collide = true;
            break;
        }   
       body_cell = body_cell->next_body_cell; 
    }
    return collide;

}

bool check_win() {
    if (snake_size == SNAKE_SIZE_LIMIT) {
        return true;
    }
    return false;

}
void keyboard_listener (char *current_direction) {
    fd_set input;
    int fds, ret_val, num;
    struct timeval tv;
    char ch;

    tv.tv_sec = TIME_SEC;
    tv.tv_usec = TIME_USEC;

    fds = STDIN + 1;

    FD_ZERO(&input);
    FD_SET(STDIN, &input);

    ret_val = select(fds, &input , NULL, NULL, &tv);

    if (ret_val == -1) {
        perror("select()");
    }else if (ret_val > 0) {
        if(FD_ISSET(0, &input))  {
            ch = getchar();
        }   
    } else {
        return;
    }

    
}
unsigned int gen_num_in_board() {
    unsigned int numValue = rand() % SIDE_SIZE;
    srand(time(0)); 
    return numValue;
}

void print_board_on_screen() {
	for (unsigned int y = 0; y < SIDE_SIZE; y++) {
		for (unsigned int x = 0; x < SIDE_SIZE; x++) {
			printf("%c ", board[y][x]);
		}
		
		printf("\n");
	}	

    
}
