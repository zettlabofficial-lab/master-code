#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 5

char board[SIZE][SIZE];
int skipTurn[3] = {0,0,0};

void printBoard(){
    printf("\n   0 1 2 3 4\n");
    for(int i=0;i<SIZE;i++){
        printf("%d  ",i);
        for(int j=0;j<SIZE;j++){
            if(board[i][j]=='B' || board[i][j]==0)
                printf(". ");
            else
                printf("%c ",board[i][j]);
        }
        printf("\n");
    }
}

void placeBombs(){
    int count=0;
    while(count<3){
        int r=rand()%SIZE;
        int c=rand()%SIZE;
        if(board[r][c]==0){
            board[r][c]='B';
            count++;
        }
    }
}

char fight(char a, char b){
    if(a==b) return 'D';

    if((a=='K'&&b=='Q')||(a=='Q'&&b=='J')||(a=='J'&&b=='K')||
       (a=='k'&&b=='q')||(a=='q'&&b=='j')||(a=='j'&&b=='k'))
        return 'W';

    return 'L';
}

int countPieces(int player){
    int count=0;
    for(int i=0;i<SIZE;i++){
        for(int j=0;j<SIZE;j++){
            if(player==1 && (board[i][j]=='J'||board[i][j]=='K'||board[i][j]=='Q'))
                count++;
            if(player==2 && (board[i][j]=='j'||board[i][j]=='k'||board[i][j]=='q'))
                count++;
        }
    }
    return count;
}

int main(){
    srand(time(NULL));

    for(int i=0;i<SIZE;i++)
        for(int j=0;j<SIZE;j++)
            board[i][j]=0;

    board[0][1]='j';
    board[0][2]='k';
    board[0][3]='q';

    board[4][1]='J';
    board[4][2]='K';
    board[4][3]='Q';

    placeBombs();

    int turn=1;

    printf("===== GAME START =====\n");

    while(1){

        int p1 = countPieces(1);
        int p2 = countPieces(2);

        if(p1==0){
            printf("\nAll Player 1 pieces are gone!\n");
            printf(">>> PLAYER 2 WINS <<<\n");
            break;
        }
        if(p2==0){
            printf("\nAll Player 2 pieces are gone!\n");
            printf(">>> PLAYER 1 WINS <<<\n");
            break;
        }

        printBoard();
        printf("\nRemaining Pieces -> P1: %d | P2: %d\n",p1,p2);
        printf("\n===== PLAYER %d TURN =====\n",turn);

        if(skipTurn[turn]==1){
            printf("Player %d is stunned from a bomb! Skip this turn.\n",turn);
            skipTurn[turn]=0;
            turn = (turn==1)?2:1;
            continue;
        }

        int r1,c1,r2,c2;
        printf("Enter move (r1 c1 r2 c2): ");
        scanf("%d %d %d %d",&r1,&c1,&r2,&c2);

        if(abs(r1-r2)+abs(c1-c2)!=1){
            printf("Invalid move! Move only 1 step (Up/Down/Left/Right).\n");
            continue;
        }

        char piece = board[r1][c1];

        if(turn==1 && !(piece=='J'||piece=='K'||piece=='Q')){
            printf("That is not your piece!\n");
            continue;
        }
        if(turn==2 && !(piece=='j'||piece=='k'||piece=='q')){
            printf("That is not your piece!\n");
            continue;
        }

        printf("Player %d moves %c from (%d,%d) to (%d,%d)\n",
               turn,piece,r1,c1,r2,c2);

        char target = board[r2][c2];

        if(target=='B'){
            printf("BOOM! %c stepped on a bomb!\n",piece);
            printf("Player %d will skip next turn!\n",turn);
            board[r2][c2]=piece;
            board[r1][c1]=0;
            skipTurn[turn]=1;
        }
        
        else if(target==0){
            printf("Moved successfully.\n");
            board[r2][c2]=piece;
            board[r1][c1]=0;
        }
    
        else{
            printf("Battle between %c and %c!\n",piece,target);

            char result=fight(piece,target);

            if(result=='W'){
                printf("%c wins the battle!\n",piece);
                board[r2][c2]=piece;
                board[r1][c1]=0;
            }
            else if(result=='L'){
                printf("%c loses and is removed!\n",piece);
                board[r1][c1]=0;
            }
            else{
                printf("Both pieces are destroyed!\n");
                board[r1][c1]=0;
                board[r2][c2]=0;
            }
        }

        turn = (turn==1)?2:1;
    }

    printf("===== GAME OVER =====\n");
    return 0;
}
