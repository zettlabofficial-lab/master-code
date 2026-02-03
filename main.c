#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#define SIZE 5
int trapA[SIZE][SIZE] = {0};
int trapB[SIZE][SIZE] = {0};
int setupPhase = 1; 

//woonsen56
char board_A[5][5] = {
    {' ','J','K','Q',' '},
    {' ',' ',' ',' ',' '},
    {' ',' ',' ',' ',' '},
    {' ',' ',' ',' ',' '},
    {' ',' ',' ',' ',' '}
};

char board_B[5][5] = {
    {' ',' ',' ',' ',' '},
    {' ',' ',' ',' ',' '},
    {' ',' ',' ',' ',' '},
    {' ',' ',' ',' ',' '},
    {' ','j','k','q',' '}
};

char board[5][5] = {
    {' ','J','K','Q',' '},
    {' ',' ',' ',' ',' '},
    {' ',' ',' ',' ',' '},
    {' ',' ',' ',' ',' '},
    {' ','j','k','q',' '}
};

//variable
int player = 0,turn = 1;
char playerA = 'A',playerB = 'B';
int itemA,itemB;
int row,col,skill_r,skill_c;
char walk;


//function
void build_board();
void player_turn();
void change_chess();
int check_win();
void placeTrap(void);
void attack(int row_def,int col_def);

int main()
{
    setupPhase = 1;   // ===== ช่วงวางกับดัก =====

    printf("=== PLACE TRAPS BEFORE GAME ===\n");

    // ---------- Player A ----------
    player = 0;
    printf("Player A place a trap\n");
    build_board();        // แสดงตารางก่อนวาง
    placeTrap();          // วางกับดัก
    build_board();        // แสดงตารางหลังวาง

    // ---------- Player B ----------
    player = 1;
    printf("Player B place a trap\n");
    build_board();        // แสดงตารางก่อนวาง (ไม่เห็นของ A)
    placeTrap();          // วางกับดัก
    build_board();        // แสดงตารางหลังวาง

    // ===== เริ่มเกม =====
    printf("\n=== GAME START ===\n");
    setupPhase = 0;

    player = 0;
    turn = 1;
    itemA = 0;
    itemB = 0;

    while (1) {

        if (check_win() == 1) {
            build_board();
            printf("\n[Player B] Winner!!\n");
            break;
        }
        else if (check_win() == 0) {
            build_board();
            printf("\n[Player A] Winner!!\n");
            break;
        }
        else if (check_win() == 2 || turn == 21) {
            build_board();
            printf("\nPlayers draw!!\n");
            break;
        }
        else {
            build_board();
            player_turn();

            player = (player == 0) ? 1 : 0;
            turn++;
        }
    }

    return 0;
}

void placeTrap(void)
{
    int row, col;

    while (1) {
        printf("Enter trap position (row col): ");
        scanf("%d %d", &row, &col);

        if (row < 0 || row >= SIZE || col < 0 || col >= SIZE) {
            printf("Out of board. Try again.\n");
            continue;
        }

        // Player A
        if (player == 0) {
            if (trapA[row][col] == 0) {
                trapA[row][col] = 1;
                break;
            }
        }
        // Player B
        else {
            if (trapB[row][col] == 0) {
                trapB[row][col] = 1;
                break;
            }
        }

        printf("Already has a trap. Try again.\n");
    }
}


int check_win(){
    int check = 3,countA=0,countB=0;
    for(int i=0;i<5;i++){
        for(int j=0;j<5;j++){
            if((board_A[i][j] == 'J' || board_A[i][j] == 'K' || board_A[i][j] == 'Q')){
                countA++;
            }
            if((board_B[i][j] == 'j' || board_B[i][j] == 'k' || board_B[i][j] == 'q')){
                countB++;
            }
        }
    }
    if(countA <= 0 && countB <= 0){
        check = 2;
    }else if(countA <= 0){
        check = 1;
    }else if(countB <= 0){
        check = 0;
    }

    return check;
}

void player_turn(){
    char logic;
    do {
        printf("Enter row and column: ");
        scanf("%d %d", &row, &col);
        getchar();
    } while (row < 0 || row > 4 || col < 0 || col > 4 || (board_A[row][col] == ' ' && player == 0) || (board_B[row][col] == ' ' && player == 1));

    do{
        printf("Please select w/a/s/d : ");
        scanf(" %c",&walk);
        if(walk == 'W'){
            walk = 'w';
        }else if(walk == 'A'){
            walk = 'a';
        }else if(walk == 'S'){
            walk = 's';
        }else if(walk == 'D'){
            walk = 'd';
        }
    }while(walk != 'w' && walk != 'a'&& walk != 's' && walk != 'd');

    if(player == 0){
        if(itemA >= 1){
            printf("Use item y/n : ");
            getchar();
            scanf(" %c", &logic);
            printf("%c\n",logic);
            if(logic == 'y'){
                
                itemA--;
            }else{
                change_chess();
            }
        }else{
            change_chess();
        }
    }else{
        if(itemB >= 1){
            printf("Use item y/n : ");
            getchar();
            scanf(" %c", &logic);
            if(logic == 'y'){
                itemB--;
            }else{
                change_chess();
            }
        }else{
            change_chess();
        }

    }
    // ===== UPDATE row,col AFTER MOVE =====
    if(walk == 'w') row--;
    else if(walk == 's') row++;
    else if(walk == 'a') col--;
    else if(walk == 'd') col++;

// ===== CHECK TRAP =====


// Player A เดิน → เช็กกับดักของ B
    if (player == 0 && trapB[row][col] == 1) {

        printf("\n!!! Player A stepped on Player B's trap !!!\n");
    itemA = 1;                 // ได้ item
    trapB[row][col] = 0;       // ลบกับดัก
}

// Player B เดิน → เช็กกับดักของ A
    else if (player == 1 && trapA[row][col] == 1) {

        printf("\n!!! Player B stepped on Player A's trap !!!\n");
    itemB = 1;                 // ได้ item
    trapA[row][col] = 0;       // ลบกับดัก
}

}

void checkTrapAfterMove(int r, int c)
{
    // Player A เดิน → เจอกับดักของ Player B
    if (player == 0 && trapB[r][c] == 1) {

        printf(">>> Player A stepped on Player B's trap!\n");
        itemA = 1;          // ได้ item
        trapB[r][c] = 0;    // ลบกับดัก
    }

    // Player B เดิน → เจอกับดักของ Player A
    else if (player == 1 && trapA[r][c] == 1) {

        printf(">>> Player B stepped on Player A's trap!\n");
        itemB = 1;          // ได้ item
        trapA[r][c] = 0;    // ลบกับดัก
    }
}



void attack(int row_def, int col_def){
    char atk = board[row][col];
    char def = board[row_def][col_def];

    printf("work : %c %c\n", tolower(atk), tolower(def));

    if(player == 0){

        if((atk == 'K' && def == 'q') ||
           (atk == 'Q' && def == 'j') ||
           (atk == 'J' && def == 'k')){

            if(walk == 'w' && row > 0){
                board[row-1][col] = board_A[row][col];
                board_A[row-1][col] = board_A[row][col];
                board_B[row-1][col] = ' ';
            }
            else if(walk == 's' && row < SIZE-1){
                board[row+1][col] = board_A[row][col];
                board_A[row+1][col] = board_A[row][col];
                board_B[row+1][col] = ' ';
            }
            else if(walk == 'a' && col > 0){
                board[row][col-1] = board_A[row][col];
                board_A[row][col-1] = board_A[row][col];
                board_B[row][col-1] = ' ';
            }
            else if(walk == 'd' && col < SIZE-1){
                board[row][col+1] = board_A[row][col];
                board_A[row][col+1] = board_A[row][col];
                board_B[row][col+1] = ' ';
            }

            board[row][col] = ' ';
            board_A[row][col] = ' ';
        }

        else if(tolower(atk) == tolower(def)){
            board[row][col] = ' ';
            board[row_def][col_def] = ' ';
            board_A[row][col] = ' ';
            board_A[row_def][col_def] = ' ';
            board_B[row][col] = ' ';
            board_B[row_def][col_def] = ' ';
        }
    }

    else if(player == 1){

        if((atk == 'k' && def == 'Q') ||
           (atk == 'q' && def == 'J') ||
           (atk == 'j' && def == 'K')){

            if(walk == 'w' && row > 0){
                board[row-1][col] = board_B[row][col];
                board_B[row-1][col] = board_B[row][col];
                board_A[row-1][col] = ' ';
            }
            else if(walk == 's' && row < SIZE-1){
                board[row+1][col] = board_B[row][col];
                board_B[row+1][col] = board_B[row][col];
                board_A[row+1][col] = ' ';
            }
            else if(walk == 'a' && col > 0){
                board[row][col-1] = board_B[row][col];
                board_B[row][col-1] = board_B[row][col];
                board_A[row][col-1] = ' ';
            }
            else if(walk == 'd' && col < SIZE-1){
                board[row][col+1] = board_B[row][col];
                board_B[row][col+1] = board_B[row][col];
                board_A[row][col+1] = ' ';
            }

            board[row][col] = ' ';
            board_B[row][col] = ' ';
        }

        else if(tolower(atk) == tolower(def)){
            board[row][col] = ' ';
            board[row_def][col_def] = ' ';
            board_A[row][col] = ' ';
            board_A[row_def][col_def] = ' ';
            board_B[row][col] = ' ';
            board_B[row_def][col_def] = ' ';
        }
    }
}


// walk
void change_chess(){
    int newRow = row;
    int newCol = col;

    if(player == 0){
        if(walk == 'w') newRow = row + 1;
        else if(walk == 's') newRow = row - 1;
        else if(walk == 'a') newCol = col - 1;
        else if(walk == 'd') newCol = col + 1;

        if(newRow < 0 || newRow >= SIZE || newCol < 0 || newCol >= SIZE) return;

        if(board[newRow][newCol] == ' '){
            board[newRow][newCol] = board_A[row][col];
            board_A[newRow][newCol] = board_A[row][col];
            board[row][col] = ' ';
            board_A[row][col] = ' ';
        }else{
            attack(newRow, newCol);
        }

        row = newRow;
        col = newCol;
        checkTrapAfterMove(row, col);
        check_win();
    }
    else{
        if(walk == 'w') newRow = row - 1;
        else if(walk == 's') newRow = row + 1;
        else if(walk == 'a') newCol = col - 1;
        else if(walk == 'd') newCol = col + 1;

        if(newRow < 0 || newRow >= SIZE || newCol < 0 || newCol >= SIZE) return;

        if(board[newRow][newCol] == ' '){
            board[newRow][newCol] = board_B[row][col];
            board_B[newRow][newCol] = board_B[row][col];
            board[row][col] = ' ';
            board_B[row][col] = ' ';
        }else{
            attack(newRow, newCol);
        }

        row = newRow;
        col = newCol;
        checkTrapAfterMove(row, col);
        check_win();
    }
}

// create board
void build_board()
{
    printf("=======[ CHESS ]=======\n");
    printf("\tPLAYER A\n");
    printf("-----------------------\n");
    printf("  | 0 | 1 | 2 | 3 | 4 |\n");
    printf("-----------------------\n");

    for (int i = 0; i < SIZE; i++) {
    printf("%d", i);
    for (int j = 0; j < SIZE; j++) {

        // ช่วงวางกับดัก
        if (setupPhase == 1) {

            // Player A วาง → เห็นของ A
            if (player == 0 && trapA[i][j] == 1) {
                printf(" | T");
            }
            // Player B วาง → เห็นของ B
            else if (player == 1 && trapB[i][j] == 1) {
                printf(" | T");
            }
            else {
                printf(" | %c", board[i][j]);
            }
        }
        // ช่วงเล่นจริง
        else {
            printf(" | %c", board[i][j]);
        }
    }
    printf(" |\n");
    printf("-----------------------\n");
    }

    // แสดง TURN + SKILL เฉพาะตอนเริ่มเกม
    if (setupPhase == 0) {
        printf("PLAYER %c\tTURN : %d\n",
               (player == 0) ? playerA : playerB, turn);

        if (player == 0) {
            printf("SKILL : long walk : %d\n", itemA);
        } else {
            printf("SKILL : long walk : %d\n", itemB);
        }
    }
}
