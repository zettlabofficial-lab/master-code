#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>

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
void attack(int row_def,int col_def);

int main()
{
    printf("Woonsen kub");
    while(1){
        if(check_win() == 1){
            build_board();
            printf("\n[Player B] Winner!!\n");
            break;
        }else if(check_win() == 0){
            build_board();
            printf("\n[Player A] Winner!!\n");
            break;
        }else if(check_win() == 2){
            build_board();
            printf("\nPlayers draw!!\n");
            break;
        }else if(turn == 21){
            build_board();
            printf("\nPlayers draw!!\n");
            break;
        }else{
            if(turn%3 == 0){
                srand(time(NULL));
                skill_r = (rand()%5),skill_c = (rand()%5);
                if(player == 0 || player == 1){
                    if(board_A[skill_r][skill_c] == 'J' || board_A[skill_r][skill_c] == 'K' || board_A[skill_r][skill_c] == 'Q'){
                        board_A[skill_r][skill_c] = ' ';

                    }else if(board_B[skill_r][skill_c] == 'j' || board_B[skill_r][skill_c] == 'k' || board_B[skill_r][skill_c] == 'q'){
                        board_B[skill_r][skill_c] = ' ';

                    }
                }
                build_board();
                player_turn();
            }else{
                build_board();
                player_turn();
            }
            if(player == 0){
                player++;  //player = player + 1
            }else{
                player--;  
            }
            turn++;
        }
        
        
    };

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
    
}

void attack(int row_def,int col_def){
    char atk = board[row][col];
    char def = board[row_def][col_def];
    printf("work : %c %c\n",tolower(atk),tolower(def));
    if(player == 0){
        if((atk == 'K' && def == 'q')||(atk == 'Q' && def == 'j')||(atk == 'J' && def == 'k')){
            if(walk == 'w'){
                board[row+1][col] = board_A[row][col];
                board_A[row+1][col] = board_A[row][col];
                board_A[row][col] = ' ';
                board_B[row+1][col] = ' ';
                board[row][col] = ' ';
            }else if(walk == 's'){
                board[row-1][col] = board_A[row][col];
                board_A[row-1][col] = board_A[row][col];
                board_A[row][col] = ' ';
                board_B[row-1][col] = ' ';
                board[row][col] = ' ';
            }else if(walk == 'a'){
                board[row][col-1] = board_A[row][col];
                board_A[row][col-1] = board_A[row][col];
                board_A[row][col] = ' ';
                board_B[row][col-1] = ' ';
                board[row][col] = ' ';
            }else if(walk == 'd'){
                board[row][col+1] = board_A[row][col];
                board_A[row][col+1] = board_A[row][col];
                board_A[row][col] = ' ';
                board_B[row][col+1] = ' ';
                board[row][col] = ' ';
            }
        }else if(tolower(atk) == tolower(def)){
            printf("work0 : %c %c\n",tolower(atk),tolower(def));
            if(walk == 'w' || walk == 's' || walk == 'a' || walk == 'd'){
                board[row_def][col_def] = ' ';
                board_A[row_def][col_def] = ' ';
                board_B[row_def][col_def] = ' ';
                board_A[row][col] = ' ';
                board_B[row][col] = ' ';
                board[row][col] = ' ';
            }
        }
    }else if(player == 1){
        if((atk == 'k' && def == 'Q')||(atk == 'q' && def == 'J')||(atk == 'j' && def == 'K')){
                if(walk == 'w'){
                    board[row-1][col] = board_B[row][col];
                    board_B[row-1][col] = board_B[row][col];
                    board_B[row][col] = ' ';
                    board_A[row-1][col] = ' ';
                    board[row][col] = ' ';
                }else if(walk == 's'){
                    board[row+1][col] = board_B[row][col];
                    board_B[row+1][col] = board_B[row][col];
                    board_B[row][col] = ' ';
                    board_A[row+1][col] = ' ';
                    board[row][col] = ' ';
                }else if(walk == 'a'){
                    board[row][col-1] = board_B[row][col];
                    board_B[row][col-1] = board_B[row][col];
                    board_B[row][col] = ' ';
                    board_A[row][col-1] = ' ';
                    board[row][col] = ' ';
                }else if(walk == 'd'){
                    board[row][col+1] = board_B[row][col];
                    board_B[row][col+1] = board_B[row][col];
                    board_B[row][col] = ' ';
                    board_A[row][col+1] = ' ';
                    board[row][col] = ' ';
                }
        }else if(tolower(atk) == tolower(def)){
            printf("work1 : %c %c\n",tolower(atk),tolower(def));
            if(walk == 'w' || walk == 's' || walk == 'a' || walk == 'd'){
                board[row_def][col_def] = ' ';
                board_A[row_def][col_def] = ' ';
                board_B[row_def][col_def] = ' ';
                board_A[row][col] = ' ';
                board_B[row][col] = ' ';
                board[row][col] = ' ';
            }
        }
    }
}


// walk
void change_chess(){
    if(player == 0){
        if(walk == 'w' ){
            if(board[row+1][col] == '*'){
                board[row+1][col] = board_A[row][col];
                board_A[row+1][col] = board_A[row][col];
                board_A[row][col] = ' ';
                board[row][col] = ' ';
            }else if(board[row+1][col] == ' ' && board_A[row+1][col] == ' '){
                board[row+1][col] = board_A[row][col];
                board_A[row+1][col] = board_A[row][col];
                board_A[row][col] = ' ';
                board[row][col] = ' ';
            }else if(board_A[row+1][col] != 'J' && board_A[row+1][col] != 'K' && board_A[row+1][col] != 'Q' ){
                attack(row+1,col);
            }
        }else if(walk == 's'){
            if(board[row-1][col] == '*'){
                board[row-1][col] = board_A[row][col];
                board_A[row-1][col] = board_A[row][col];
                board_A[row][col] = ' ';
                board[row][col] = ' ';
            }else if(board[row-1][col] == ' ' && board_A[row-1][col] == ' '){
                board[row-1][col] = board_A[row][col];
                board_A[row-1][col] = board_A[row][col];
                board_A[row][col] = ' ';
                board[row][col] = ' ';
            }else if(board_A[row-1][col] != 'J' && board_A[row-1][col] != 'K' && board_A[row-1][col] != 'Q' ){
                attack(row-1,col);
            }
        }else if(walk == 'a'){
            if(board[row][col-1] == '*'){
                board[row][col-1] = board_A[row][col];
                board_A[row][col-1] = board_A[row][col];
                board_A[row][col] = ' ';
                board[row][col] = ' ';
            }else if(board[row][col-1] == ' ' && board_A[row][col-1] == ' '){
                board[row][col-1] = board_A[row][col];
                board_A[row][col-1] = board_A[row][col];
                board_A[row][col] = ' ';
                board[row][col] = ' ';
            }else if(board_A[row][col-1] != 'J' && board_A[row][col-1] != 'K' && board_A[row][col-1] != 'Q' ){
                attack(row,col-1);
            }
            
        }else if(walk == 'd'){
            if(board[row][col+1] == '*'){
                board[row][col+1] = board_A[row][col];
                board_A[row][col+1] = board_A[row][col];
                board_A[row][col] = ' ';
                board[row][col] = ' ';
            }else if(board[row][col+1] == ' ' && board_A[row][col+1] == ' '){
                board[row][col+1] = board_A[row][col];
                board_A[row][col+1] = board_A[row][col];
                board_A[row][col] = ' ';
                board[row][col] = ' ';
            }else if(board_A[row][col+1] != 'J' && board_A[row][col+1] != 'K' && board_A[row][col+1] != 'Q' ){
                attack(row,col+1);
            }
            
        }
        check_win();
    }else{
        if(walk == 'w' ){
            if(board[row-1][col] == '*'){
                board[row-1][col] = board_B[row][col];
                board_B[row-1][col] = board_B[row][col];
                board_B[row][col] = ' ';
                board[row][col] = ' ';
            }else if(board[row-1][col] == ' ' && board_B[row-1][col] == ' '){
                board[row-1][col] = board_B[row][col];
                board_B[row-1][col] = board_B[row][col];
                board_B[row][col] = ' ';
                board[row][col] = ' ';
            }else if(board_B[row][col+1] != 'j' && board_B[row][col+1] != 'k' && board_B[row][col+1] != 'q' ){
                attack(row-1,col);
            }
        }else if(walk == 's'){
            if(board[row+1][col] == '*'){
                board[row+1][col] = board_B[row][col];
                board_B[row+1][col] = board_B[row][col];
                board_B[row][col] = ' ';
                board[row][col] = ' ';
            }else if(board[row+1][col] == ' ' && board_B[row+1][col] == ' '){
                board[row+1][col] = board_B[row][col];
                board_B[row+1][col] = board_B[row][col];
                board_B[row][col] = ' ';
                board[row][col] = ' ';
            }else if(board_B[row+1][col] != 'j' && board_B[row+1][col] != 'k' && board_B[row+1][col] != 'q' ){
                attack(row+1,col);
            }
        }else if(walk == 'a'){
            if(board[row][col-1] == '*'){
                board[row][col-1] = board_B[row][col];
                board_B[row][col-1] = board_B[row][col];
                board_B[row][col] = ' ';
                board[row][col] = ' ';
            }else if(board[row][col-1] == ' ' && board_B[row][col-1] == ' '){
                board[row][col-1] = board_B[row][col];
                board_B[row][col-1] = board_B[row][col];
                board_B[row][col] = ' ';
                board[row][col] = ' ';
            }else if(board_B[row][col-1] != 'j' && board_B[row][col-1] != 'k' && board_B[row][col-1] != 'q' ){
                attack(row,col-1);
            }
            
        }else if(walk == 'd'){
            if(board[row][col+1] == '*'){
                board[row][col+1] = board_B[row][col];
                board_B[row][col+1] = board_B[row][col];
                board_B[row][col] = ' ';
                board[row][col] = ' ';
            }else if(board[row][col+1] == ' ' && board_B[row][col+1] == ' '){
                board[row][col+1] = board_B[row][col];
                board_B[row][col+1] = board_B[row][col];
                board_B[row][col] = ' ';
                board[row][col] = ' ';
            }else if(board_B[row][col+1] != 'j' && board_B[row][col+1] != 'k' && board_B[row][col+1] != 'q' ){
                attack(row,col+1);
            }
            
        }
        check_win();
    }
}

// create board
void build_board(){
    printf("=======[ CHESS ]=======\n");
    printf("\tPLAYER A\n");
    printf("-----------------------\n");
    printf("  | 0 | 1 | 2 | 3 | 4 |\n");
    printf("-----------------------\n");
    for(int i=0;i<5;i++){
        printf("%d",i);
        for(int j=0;j<5;j++){
            if(board_A[i][j] == 'J' || board_A[i][j] == 'K' || board_A[i][j] == 'Q'){
                printf(" | %c",board[i][j]);
            }else if(board_B[i][j] == 'j' || board_B[i][j] == 'k' || board_B[i][j] == 'q'){
                printf(" | %c",board[i][j]);
            }else{
                printf(" | %c",board[i][j]);
            }
            
        }
        printf(" |\n");
        printf("-----------------------\n");
    }
    printf("\tPLAYER B\n");
    printf("=======================\n\n");

    printf("PLAYER %c\tTURN : %d\n",player==0 ? playerA : playerB,turn);

    if(player == 0){
        printf("SKILL : long walk : %d\n",itemA);
    }else{
        printf("SKILL : long walk : %d\n",itemB);
    }
}
