#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

/* =======================
   BOARD
======================= */

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

/* =======================
   VARIABLE
======================= */

int player = 0, turn = 1;
int itemA = 1, itemB = 1;
int row, col;
char walk;

int bomb_map[5][5] = {0};        // 1 = มีระเบิด (ซ่อน)
int bomb_show_turn[5][5] = {0};  // แสดงระเบิดเฉพาะเทิร์นที่โดน
int bomb_happened = 0;
int skip_bomb_check = 0;         //  แก้บั๊กไอเท็ม

/* =======================
   PROTOTYPE
======================= */

void build_board();
void player_turn();
void change_chess();
void attack(int r,int c);
int check_win();
void random_bomb();
void check_bomb(char piece,int r,int c);

/* =======================
   MAIN
======================= */

int main(){
    srand(time(NULL));
    random_bomb();

    while(1){
        int result = check_win();
        if(result==0){ printf("PLAYER A WIN!\n"); break; }
        if(result==1){ printf("PLAYER B WIN!\n"); break; }

        bomb_happened = 0;

        // แสดงตารางก่อนเดิน
        build_board();

        player_turn();

        // ถ้าโดนระเบิด แสดงตารางซ้ำ (โชว์ *)
        if(bomb_happened)
            build_board();

        player = (player==0)?1:0;
        turn++;
    }
    return 0;
}

/* =======================
   PLAYER TURN
======================= */

void player_turn(){
    int check;
    char use;

    while(1){
        printf("\nPLAYER %c TURN:%d ITEM:%d\n",
               player==0?'A':'B',
               turn,
               player==0?itemA:itemB);

        printf("Enter row and column: ");
        check = scanf("%d %d",&row,&col);

        if(check!=2){
            printf("Please enter NUMBER only!\n");
            while(getchar()!='\n');
            continue;
        }

        if(row<0||row>4||col<0||col>4){
            printf("Position out of board (0-4)\n");
            continue;
        }

        if(player==0 && board_A[row][col]==' '){
            printf("Not your piece (PLAYER A)\n");
            continue;
        }
        if(player==1 && board_B[row][col]==' '){
            printf("Not your piece (PLAYER B)\n");
            continue;
        }
        break;
    }

    do{
        printf("Select w/a/s/d: ");
        scanf(" %c",&walk);
        walk = tolower(walk);
    }while(walk!='w'&&walk!='a'&&walk!='s'&&walk!='d');

    // ใช้ไอเท็ม
    if(player==0 && itemA>0){
        printf("Use item y/n: ");
        scanf(" %c",&use);
        if(use=='y'){
            skip_bomb_check = 1;
            change_chess();     // ก้าวที่ 1 (ไม่เช็คระเบิด)
            skip_bomb_check = 0;
            change_chess();     // ก้าวที่ 2 (เช็คระเบิด)
            itemA--;
            return;
        }
    }

    if(player==1 && itemB>0){
        printf("Use item y/n: ");
        scanf(" %c",&use);
        if(use=='y'){
            skip_bomb_check = 1;
            change_chess();
            skip_bomb_check = 0;
            change_chess();
            itemB--;
            return;
        }
    }

    change_chess();
}

/* =======================
   CHANGE CHESS
======================= */

void change_chess(){
    char piece = (player==0)?board_A[row][col]:board_B[row][col];
    int nr=row,nc=col;

    if(player==0){
        if(walk=='w') nr++;
        else if(walk=='s') nr--;
        else if(walk=='a') nc--;
        else if(walk=='d') nc++;
    }else{
        if(walk=='w') nr--;
        else if(walk=='s') nr++;
        else if(walk=='a') nc--;
        else if(walk=='d') nc++;
    }

    if(nr<0||nr>4||nc<0||nc>4) return;
    if(player==0 && board_A[nr][nc]!=' ') return;
    if(player==1 && board_B[nr][nc]!=' ') return;

    if(player==0 && board_B[nr][nc]!=' '){ attack(nr,nc); return; }
    if(player==1 && board_A[nr][nc]!=' '){ attack(nr,nc); return; }

    board[row][col]=' ';
    board_A[row][col]=' ';
    board_B[row][col]=' ';

    if(!skip_bomb_check && bomb_map[nr][nc]){
        check_bomb(piece,nr,nc);
        return;
    }

    board[nr][nc]=piece;
    if(player==0) board_A[nr][nc]=piece;
    else board_B[nr][nc]=piece;

    row=nr; col=nc;
}

/* =======================
   ATTACK
======================= */

void attack(int r,int c){
    char atk=(player==0)?board_A[row][col]:board_B[row][col];
    char def=(player==0)?board_B[r][c]:board_A[r][c];

    if(tolower(atk)==tolower(def)){
        board[row][col]=board[r][c]=' ';
        board_A[row][col]=board_B[row][col]=' ';
        board_A[r][c]=board_B[r][c]=' ';
        return;
    }

    int winA=(atk=='J'&&def=='k')||(atk=='K'&&def=='q')||(atk=='Q'&&def=='j');
    int winB=(atk=='j'&&def=='K')||(atk=='k'&&def=='Q')||(atk=='q'&&def=='J');

    if((player==0&&winA)||(player==1&&winB)){
        board[row][col]=' ';
        if(player==0) board_A[row][col]=' ';
        else board_B[row][col]=' ';

        board[r][c]=atk;
        if(player==0) board_A[r][c]=atk;
        else board_B[r][c]=atk;

        row=r; col=c;
        if(bomb_map[r][c]) check_bomb(atk,r,c);
    }
}

/* =======================
   BOMB
======================= */

void random_bomb(){
    int c=0,r,n;
    while(c<3){
        r=rand()%5; n=rand()%5;
        if(!bomb_map[r][n] && board[r][n]==' '){
            bomb_map[r][n]=1;
            c++;
        }
    }
}

void check_bomb(char piece,int r,int c){
    printf("BOOM!! %c hit bomb\n",piece);
    bomb_happened=1;
    bomb_show_turn[r][c]=turn;
    bomb_map[r][c]=0;

    if(piece=='J'){row=0;col=1;}
    else if(piece=='K'){row=0;col=2;}
    else if(piece=='Q'){row=0;col=3;}
    else if(piece=='j'){row=4;col=1;}
    else if(piece=='k'){row=4;col=2;}
    else if(piece=='q'){row=4;col=3;}

    board[row][col]=piece;
    if(player==0) board_A[row][col]=piece;
    else board_B[row][col]=piece;
}

/* =======================
   CHECK WIN
======================= */

int check_win(){
    int a=0,b=0;
    for(int i=0;i<5;i++)
        for(int j=0;j<5;j++){
            if(board_A[i][j]=='J'||board_A[i][j]=='K'||board_A[i][j]=='Q') a++;
            if(board_B[i][j]=='j'||board_B[i][j]=='k'||board_B[i][j]=='q') b++;
        }
    if(a==0) return 1;
    if(b==0) return 0;
    return -1;
}

/* =======================
   BUILD BOARD
======================= */

void build_board(){
    char display[5][5];

    for(int i=0;i<5;i++)
        for(int j=0;j<5;j++)
            display[i][j]=board[i][j];

    for(int i=0;i<5;i++)
        for(int j=0;j<5;j++)
            if(bomb_show_turn[i][j]==turn)
                display[i][j]='*';

    printf("\n=======[ CHESS ]=======\n");
    printf("  | 0 | 1 | 2 | 3 | 4 |\n");
    printf("-----------------------\n");
    for(int i=0;i<5;i++){
        printf("%d",i);
        for(int j=0;j<5;j++)
            printf(" | %c",display[i][j]);
        printf(" |\n-----------------------\n");
    }
}
