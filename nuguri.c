#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32//window
     #include <windows.h>
     #include <conio.h>

     void coin_Beep(){ // 코인 먹는 사운드 함수 입니다.
        Beep(1000, 30);  // Beep(주파수, 지속시간)
        Beep(1500, 40);
     }
     void collision_Beep(){ // 부딪혔을때 나오는 사운드 함수 입니다.
        Beep(400, 100);
        Beep(200, 150);
     }
     void delay(int ms){ // 기존에있던 usleep함수를 delay로 변경후 각 os 에 맞게 분기 둘다 delay()를 사용
        Sleep(ms);
     }

     void clrhwamyeon(){
          system("cls"); // 윈도우에서 화면 전환 시에 덮어씌워 출력하는 현상 수정을 위해 특정 상황에서만 cls를 실행하게 변경했습니다.
     }
     
     void clrscr(){
        //system("cls"); cls로 화면을 매 프레임 초기화하면 눈뽕이 심해서

        COORD pos = {0, 0}; // 커서 위치를 (0, 0)으로 이동시킨 뒤에
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
     }

     void hide_cursor() { // 그 커서를 숨겼습니다.
        HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        info.dwSize = 100;
        info.bVisible = FALSE;
        SetConsoleCursorInfo(consoleHandle, &info);
     }
     void disable_raw_mode(){} // Linux에서는 disable, enable을 사용하는데 windows에서는 사용을 안하지만 
     void enable_raw_mode(){} // 빼면 undefined가 나와서 선언만 해두었습니다.

#else//Linux, macOS
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>

    // 터미널 설정
    struct termios orig_termios; // else밖에 선언되어있던 struct termios를 else안에 추가
 
    void coin_Beep() { // 코인 먹는 사운드 함수 입니다.
        printf("\a"); // \a는 ASCII 벨 문자로 터미널 벨소리가 나오는 것입니다.(이스케이프 코드이기도 하답니다)
        fflush(stdout);
    }
    void collision_Beep(){ // 부딪혔을때 나오는 사운드 함수 입니다.
        printf("\a"); 
        fflush(stdout);
        usleep(200000);  // 터미널 벨소리 하나밖에 없어 coin과 차별성을 주기위해 0.2초 딜레이를 주고 2번 나오게 했습니다.
        printf("\a");
        fflush(stdout);
    }

    void delay(int ms){ // 기존에있던 usleep을 delay함수로 변경하여 추가
        usleep(ms * 1000); // Linux에선 90000이 0.09초인데 windows에선 90000이 90초여서 delay를 90으로 바꾼후 Linux에서 *1000을 해줬습니다.
    }
    
    void clrscr(){
        printf("\033[2J\033[H");
    }

    // 터미널 Raw 모드 활성화/비활성화 else밖에 선언되어있던 disable, enable else안에 추가
    void disable_raw_mode() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); }
    void enable_raw_mode() {
        tcgetattr(STDIN_FILENO, &orig_termios);
        atexit(disable_raw_mode);
        struct termios raw = orig_termios;
        raw.c_lflag &= ~(ECHO | ICANON);
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }

    // 비동기 키보드 입력 확인
    int kbhit() { //제일 밑에 선언되어있던 kbhit()을 분기하기위해 else에 추가
        struct termios oldt, newt;
        int ch;
        int oldf;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);
        if(ch != EOF) {
            ungetc(ch, stdin);
            return 1;
        }
        return 0;
    }
#endif

// 맵 및 게임 요소 정의 (수정된 부분)
#define MAP_WIDTH 40  // 맵 너비를 40으로 변경
#define MAP_HEIGHT 20
#define MAX_STAGES 2
#define MAX_ENEMIES 15 // 최대 적 개수 증가
#define MAX_COINS 30   // 최대 코인 개수 증가

// 구조체 정의
typedef struct {
    int x, y;
    int dir; // 1: right, -1: left
} Enemy;

typedef struct {
    int x, y;
    int collected;
} Coin;



// 전역 변수
char map[MAX_STAGES][MAP_HEIGHT][MAP_WIDTH + 1];
int player_x, player_y;
int stage = 0;
int score = 0;

// 플레이어 상태
int is_jumping = 0;
int velocity_y = 0;
int on_ladder = 0;
int Heart=3;

// 게임 객체
Enemy enemies[MAX_ENEMIES];
int enemy_count = 0;
Coin coins[MAX_COINS];
int coin_count = 0;



// 함수 선언
void disable_raw_mode();
void enable_raw_mode();
void load_maps();
void init_stage();
void draw_game();
void update_game(char input);
void move_player(char input);
void move_enemies();
void check_collisions();
int kbhit();

void delay(int ms);
void clrscr();
void title_screen1();
void title_screen2();
void ending_clear(int final_score);
void ending_gameover(int final_score);


void title_screen1(){//게임 시작시 나오는 화면   
    clrhwamyeon();
    clrscr();
    printf("\n\n\n\n\n");
    printf ("              =======================\n");
    printf("                     N U G U R I    \n"); 
    printf("                       G A M E       \n");
    printf("              =======================\n\n\n");
    delay(3000);
}
void title_screen2(){ //title_screen1 다음에 선택지 화면
    clrhwamyeon();  
    clrscr();
    printf("\n\n\n\n\n");
	printf("          -----------------------------------\n");
	printf("          |                                 |\n");
	printf("          |          1. start game          |\n");
    printf("          |          2. end game            |\n");
    printf("          |                                 |\n");
    printf("          -----------------------------------\n");

    
}




void ending_clear(int final_score){//클리어 시 엔딩화면 함수 추가
    clrhwamyeon();
    clrscr();
    printf("\n\n\n\n\n");
    printf("              ===========================       \n");
    printf("                     C L E A R ! !              \n"); 
    printf("                      최종 점수: %d \n", final_score);
    printf("              ===========================   \n\n\n");
    delay(5000);
}

void ending_gameover(int final_score){//게임 오버 시 엔딩화면 함수 추가
    clrhwamyeon();
    clrscr(); 
    printf("\n\n\n\n\n");
    printf("               ==============================       \n");
    printf("                     G A M E   O V E R. . .              \n"); 
    printf("                        최종 점수: %d    \n", final_score);
    printf("               ==============================   \n\n\n");
    delay(5000);

}


int main() {
    srand(time(NULL));
    enable_raw_mode();

    #ifdef _WIN32 // 윈도우일때만 그 커서 숨기기를 진행하게 했습니다.
        hide_cursor();
    #endif

    char choice ='\0';  
    title_screen1();  
    title_screen2();

    while(choice!='1' && choice!='2'){ 
        
        if(kbhit())
        { 
            choice=getchar();
        }
    } 
        if (choice == '2') {
        clrscr();
        printf("게임 종료.\n");
        disable_raw_mode();
        return 0;
        }


    load_maps();
    init_stage();
    

    char c = '\0';
    int game_over = 0;

    #ifdef _WIN32 // windows에서 컴파일 시 유니코드 문제로 깨져서 출력되어서 UTF-8 고정 시키게끔 했습니다.
      SetConsoleCP(65001); // windows에서 컴파일시 한글이 깨져 기존 Diablo에 있는 SetConsole을 추가하였습니다.
      SetConsoleOutputCP(65001);
    #endif

    while (!game_over && stage < MAX_STAGES) {
        #ifdef _WIN32
            if (_kbhit()) { // conio.h 안에 있는 _kbhit() _getch() 를 불러와 windows에서도 키를 입력받게 바꾸고 분기처리하였습니다. 
                c = _getch();

                if (c == -32 || c == 224){ // 윈도우에서도 방향키 입력 받을 수 있게 추가했습니다.
                    c = _getch();
                    switch (c) {
                        case 72: c = 'w'; break; // Up
                        case 80: c = 's'; break; // Down
                        case 77: c = 'd'; break; // Right
                        case 75: c = 'a'; break; // Left
                    }
                }

                if (c == 'q') {
                    game_over = 1;
                    continue;
                }
            } else {
                c = '\0';
            }
        #else
            if (kbhit()) {
                c = getchar();
                if (c == 'q') {
                    game_over = 1;
                    continue;
                }
                if (c == '\x1b') {
                    getchar(); // '['
                    switch (getchar()) {
                        case 'A': c = 'w'; break; // Up
                        case 'B': c = 's'; break; // Down
                        case 'C': c = 'd'; break; // Right
                        case 'D': c = 'a'; break; // Left
                    }
                }
            } else {
                c = '\0';
            }
        #endif
   
        update_game(c);
        draw_game();
        delay(90); //windows에선 ms단위로 받아 90으로 변경하였습니다.(Linux는 μs단위)

        if (map[stage][player_y][player_x] == 'E') {
            stage++;
            score += 100;
            if (stage < MAX_STAGES) {
                init_stage();
            } else {
                game_over = 1;
                ending_clear(score);//클리어시 엔딩화면 출력 
            }
        }
    }

    disable_raw_mode();
    return 0;
}




// 맵 파일 로드
void load_maps() {
    FILE *file = fopen("map.txt", "r");
    if (!file) {
        perror("map.txt 파일을 열 수 없습니다.");
        exit(1);
    }
    int s = 0, r = 0;
    char line[MAP_WIDTH + 2]; // 버퍼 크기는 MAP_WIDTH에 따라 자동 조절됨
    while (s < MAX_STAGES && fgets(line, sizeof(line), file)) {
        if ((line[0] == '\n' || line[0] == '\r') && r > 0) {
            s++;
            r = 0;
            continue;
        }
        if (r < MAP_HEIGHT) {
            line[strcspn(line, "\n\r")] = 0;
            strncpy(map[s][r], line, MAP_WIDTH + 1);
            r++;
        }
    }
    fclose(file);
}


// 현재 스테이지 초기화
void init_stage() {
    clrhwamyeon();
    enemy_count = 0;
    coin_count = 0;
    is_jumping = 0;
    velocity_y = 0;

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            char cell = map[stage][y][x];
            if (cell == 'S') {
                player_x = x;
                player_y = y;
            } else if (cell == 'X' && enemy_count < MAX_ENEMIES) {
                enemies[enemy_count] = (Enemy){x, y, (rand() % 2) * 2 - 1};
                enemy_count++;
            } else if (cell == 'C' && coin_count < MAX_COINS) {
                coins[coin_count++] = (Coin){x, y, 0};
            }
        }
    }
}

// 게임 화면 그리기
void draw_game() {
    //printf("\x1b[2J\x1b[H"); 이건 리눅스랑 맥용이라서 clrscr로 바꿨습니다.
    clrscr();
    printf("Stage: %d | Score: %d, ♥️x %d\n", stage + 1, score, Heart);
    printf("조작: ← → or A D (이동), ↑ ↓ or W S (사다리), Space (점프), q (종료)\n");

    char display_map[MAP_HEIGHT][MAP_WIDTH + 1];
    for(int y=0; y < MAP_HEIGHT; y++) {
        for(int x=0; x < MAP_WIDTH; x++) {
            char cell = map[stage][y][x];
            if (cell == 'S' || cell == 'X' || cell == 'C') {
                display_map[y][x] = ' ';
            } else {
                display_map[y][x] = cell;
            }
        }
    }
    
    for (int i = 0; i < coin_count; i++) {
        if (!coins[i].collected) {
            display_map[coins[i].y][coins[i].x] = 'C';
        }
    }

    for (int i = 0; i < enemy_count; i++) {
        display_map[enemies[i].y][enemies[i].x] = 'X';
    }

    display_map[player_y][player_x] = 'P';

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for(int x=0; x< MAP_WIDTH; x++){
            printf("%c", display_map[y][x]);
        }
        printf("\n");
    }
}

// 게임 상태 업데이트
void update_game(char input) {
    move_player(input);
    move_enemies();
    check_collisions();
}

// 플레이어 이동 로직
void move_player(char input) {
    int next_x = player_x, next_y = player_y;
    char floor_tile = (player_y + 1 < MAP_HEIGHT) ? map[stage][player_y + 1][player_x] : '#';
    char current_tile = map[stage][player_y][player_x];

    on_ladder = (current_tile == 'H');

    switch (input) {
        case 'a': next_x--; break;
        case 'd': next_x++; break;
        case 'w': if (on_ladder) next_y--; break;
        case 's': if (on_ladder && (player_y + 1 < MAP_HEIGHT) && map[stage][player_y + 1][player_x] != '#') next_y++; break;
        case ' ':
            if (!is_jumping && (floor_tile == '#' || on_ladder)) {
                is_jumping = 1;
                velocity_y = -2;
            }
            break;
    }

    if (next_x >= 0 && next_x < MAP_WIDTH && map[stage][player_y][next_x] != '#') player_x = next_x;
    
    if (on_ladder && (input == 'w' || input == 's')) {
        if(next_y >= 0 && next_y < MAP_HEIGHT && map[stage][next_y][player_x] != '#') {
            player_y = next_y;
            is_jumping = 0;
            velocity_y = 0;
        }
    } 
    else {
        if (is_jumping) {
            next_y = player_y + velocity_y;
            if(next_y < 0) next_y = 0;
            velocity_y++;
            if(velocity_y > 1) velocity_y = 1; // 벽뚫 방지 겸 게임 낙하 속도 강제 조정했습니다.

            if (velocity_y < 0 && next_y < MAP_HEIGHT && map[stage][next_y][player_x] == '#') {
                velocity_y = 0;
            } else if (next_y < MAP_HEIGHT) {
                player_y = next_y;
            }
            
            if ((player_y + 1 < MAP_HEIGHT) && map[stage][player_y + 1][player_x] == '#') {
                is_jumping = 0;
                velocity_y = 0;
            }
        } else {
            if (floor_tile != '#' && floor_tile != 'H') {
                 if (player_y + 1 < MAP_HEIGHT) player_y++;
                 else init_stage();
            }
        }
    }
    
    if (player_y >= MAP_HEIGHT) init_stage();
}


// 적 이동 로직
void move_enemies() {
    for (int i = 0; i < enemy_count; i++) {
        int next_x = enemies[i].x + enemies[i].dir;
        if (next_x < 0 || next_x >= MAP_WIDTH || map[stage][enemies[i].y][next_x] == '#' || (enemies[i].y + 1 < MAP_HEIGHT && map[stage][enemies[i].y + 1][next_x] == ' ')) {
            enemies[i].dir *= -1;
        } else {
            enemies[i].x = next_x;
        }
    }
}

// 충돌 감지 로직
void check_collisions() {
    for (int i = 0; i < enemy_count; i++) {
        if (player_x == enemies[i].x && player_y == enemies[i].y) {
            score = (score > 50) ? score - 50 : 0;
            collision_Beep(); 
            Heart--;//충돌시 Heart감소  

            if(Heart<=0){
                ending_gameover(score);//Heart가 0이면 게임오버 창
                disable_raw_mode(); //터미널 모드 원상복구
                exit(0);
            }
            init_stage();
            return;
        }
    }
    for (int i = 0; i < coin_count; i++) {
        if (!coins[i].collected && player_x == coins[i].x && player_y == coins[i].y) {
            coins[i].collected = 1;
            score += 20;
            coin_Beep(); //코인을 먹었을때 비프음 추가
        }
    }
}

