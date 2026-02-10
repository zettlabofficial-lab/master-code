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

int bomb_map[5][5] = {0};
int bomb_show_turn[5][5] = {0};
int bomb_happened = 0;

int game_result = -1;
// -1 = ยังไม่จบ
//  0 = A ชนะ
//  1 = B ชนะ
//  2 = DRAW

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

// return:  1 = atk ชนะ
//          0 = เสมอ
//         -1 = atk แพ้
int fight(char atk, char def){
    atk = tolower(atk);
    def = tolower(def);

    if(atk == def) return 0;

    if(atk=='j' && def=='k') return 1;
    if(atk=='k' && def=='q') return 1;
    if(atk=='q' && def=='j') return 1;

    return -1;
}


int win(char atk, char def)
{
    char A = tolower(atk);
    char D = tolower(def);

    if (A == D) return 0;   // เสมอ

    // K > Q, Q > J, J > K
    if ((A=='k' && D=='q') ||
        (A=='q' && D=='j') ||
        (A=='j' && D=='k'))
        return 1;           // atk ชนะ

    return -1;              // atk แพ้
}


/* =======================
   MAIN
======================= */

int main(){
    srand(time(NULL));
    random_bomb();

    while(1){
        int result = check_win();

        if(result != -1){
            build_board();
            if(result == 0) printf("PLAYER A WIN!\n");
            else if(result == 1) printf("PLAYER B WIN!\n");
            else printf("DRAW!! Both last pieces destroyed\n");
            break;
        }

        bomb_happened = 0;

        build_board();        // 1️⃣ แสดงก่อนเดิน
        player_turn();        // 2️⃣ เดิน (อาจโดนระเบิด)

        if(bomb_happened)     // ⭐ 3️⃣ ถ้าโดนระเบิด
            build_board();    //    แสดงกระดานซ้ำ (โชว์ *)

        player = (player==0)?1:0;
        turn++;               // 4️⃣ ค่อยเปลี่ยนเทิร์น
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
            printf("Out of board (0-4)\n");
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

    if(player==0 && itemA>0){
        printf("Use item y/n: ");
        scanf(" %c",&use);
        if(use=='y'){
            change_chess();
            if(game_result!=-1) return;

            change_chess();
            if(game_result!=-1) return;

            itemA--;
            return;
        }
    }

    if(player==1 && itemB>0){
        printf("Use item y/n: ");
        scanf(" %c",&use);
        if(use=='y'){
            change_chess();
            if(game_result!=-1) return;

            change_chess();
            if(game_result!=-1) return;

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

    if (bomb_map[nr][nc]) {
    // ❌ ลบหมากออกจากตำแหน่งเดิมก่อน
    board[row][col] = ' ';
    board_A[row][col] = ' ';
    board_B[row][col] = ' ';

    check_bomb(piece, nr, nc);
    return;
}


    board[row][col]=' ';
    board_A[row][col]=' ';
    board_B[row][col]=' ';

    board[nr][nc]=piece;
    if(player==0) board_A[nr][nc]=piece;
    else board_B[nr][nc]=piece;

    row=nr; col=nc;
}

/* =======================
   ATTACK
======================= */

void attack(int r,int c){
    char atk = (player==0)?board_A[row][col]:board_B[row][col];
    char def = (player==0)?board_B[r][c]:board_A[r][c];

    int result = fight(atk, def);

    // ⚖️ เสมอ → หายทั้งคู่
    if(result == 0){
        board[row][col] = board[r][c] = ' ';
        board_A[row][col] = board_B[row][col] = ' ';
        board_A[r][c] = board_B[r][c] = ' ';
        return;
    }

    // 🏆 atk ชนะ
    if(result == 1){
        // ลบ atk เดิม
        board[row][col] = ' ';
        if(player==0) board_A[row][col] = ' ';
        else board_B[row][col] = ' ';

        // ล้างเป้าหมาย
        board_A[r][c] = ' ';
        board_B[r][c] = ' ';

        // วาง atk
        board[r][c] = atk;
        if(player==0) board_A[r][c] = atk;
        else board_B[r][c] = atk;

        row = r;
        col = c;

        if(bomb_map[r][c])
            check_bomb(atk, r, c);
        return;
    }

    // ❌ atk แพ้ → ไม่ลบ ไม่ย้าย ไม่แตะอะไรเลย
    // แค่จบตา
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
    if(game_result!=-1) return game_result;

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
