#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int BOARD_SIZE = 5;

// ===== board =====
char board[5][5];
int owner[5][5];

// ===== trap =====
int trapRow[2];
int trapCol[2];
int trapActive[2];

// ===== shield =====
int shieldRow[3];
int shieldCol[3];
int shieldUsed[3];

// ===== game =====
int currentPlayer = 1;

char pieceInput[8];
char dirInput[8];

// ----------------------------

void clearInputBuffer()
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;
}

// check inside board
int inBoard(int row, int col)
{
    return (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE);
}

// count pieces
int countPieces(int player)
{
    int count = 0;
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            if (owner[row][col] == player)
                count++;
        }
    }
    return count;
}

// capture rule
int canCapture(char attacker, char defender)
{
    if (attacker == 'K' && defender == 'Q')
        return 1;
    if (attacker == 'Q' && defender == 'J')
        return 1;
    if (attacker == 'J' && defender == 'K')
        return 1;
    return 0;
}

// find piece
int findPiece(char piece, int player, int *row, int *col)
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] == piece && owner[i][j] == player)
            {
                *row = i;
                *col = j;
                return 1;
            }
        }
    }
    return 0;
}

// has shield
int hasShield(int row, int col, int player)
{
    return (shieldRow[player] == row && shieldCol[player] == col);
}

// remove shield
void removeShield(int row, int col)
{
    for (int i = 1; i <= 2; i++)
    {
        if (shieldRow[i] == row && shieldCol[i] == col)
        {
            shieldRow[i] = -1;
            shieldCol[i] = -1;
        }
    }
}

// move shield
void moveShield(int fromRow, int fromCol, int toRow, int toCol)
{
    for (int i = 1; i <= 2; i++)
    {
        if (shieldRow[i] == fromRow && shieldCol[i] == fromCol)
        {
            shieldRow[i] = toRow;
            shieldCol[i] = toCol;
        }
    }
}

// ----------------------------

// setup board
void setupBoard()
{
    srand((unsigned int)time(NULL));

    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            board[row][col] = '.';
            owner[row][col] = 0;
        }
    }

    // Player 1
    board[0][1] = 'J';
    owner[0][1] = 1;
    board[0][2] = 'K';
    owner[0][2] = 1;
    board[0][3] = 'Q';
    owner[0][3] = 1;

    // Player 2
    board[4][1] = 'J';
    owner[4][1] = 2;
    board[4][2] = 'K';
    owner[4][2] = 2;
    board[4][3] = 'Q';
    owner[4][3] = 2;

    // reset shield
    for (int i = 1; i <= 2; i++)
    {
        shieldRow[i] = shieldCol[i] = -1;
        shieldUsed[i] = 0;
    }

    // random traps
    int range[2][2] = {{1, 2}, {2, 3}};

    for (int i = 0; i < 2; i++)
    {
        trapActive[i] = 1;

        do
        {
            trapRow[i] = range[i][0] + rand() % (range[i][1] - range[i][0] + 1);
            trapCol[i] = rand() % BOARD_SIZE;
        } while (board[trapRow[i]][trapCol[i]] != '.' ||
                 (i == 1 && trapRow[1] == trapRow[0] && trapCol[1] == trapCol[0]));
    }

    printf("[Feature 1] 2 traps have been secretly placed!\n");
}

// ----------------------------

// print board
void printBoard()
{
    printf("\n  *** PLAYER 1 *** (pieces: %d)\n", countPieces(1));

    printf("        ");
    for (int col = 0; col < BOARD_SIZE; col++)
        printf("  %d   ", col);
    printf("\n");

    for (int row = 0; row < BOARD_SIZE; row++)
    {
        printf("        ");
        for (int col = 0; col < BOARD_SIZE; col++)
            printf("+-----");
        printf("+\n");

        printf("   %d    ", row);

        for (int col = 0; col < BOARD_SIZE; col++)
        {
            if (board[row][col] == '.')
            {
                printf("|  .  ");
            }
            else
            {
                if (hasShield(row, col, owner[row][col]))
                    printf("|[%c%d] ", board[row][col], owner[row][col]);
                else
                    printf("| %c%d  ", board[row][col], owner[row][col]);
            }
        }
        printf("|\n");
    }

    printf("        ");
    for (int col = 0; col < BOARD_SIZE; col++)
        printf("+-----");
    printf("+\n");

    printf("  *** PLAYER 2 *** (pieces: %d)\n", countPieces(2));

    for (int i = 1; i <= 2; i++)
    {
        if (shieldRow[i] != -1)
        {
            printf("  [Shield] Player %d: %c%d at (%d,%d) is shielded\n",
                   i,
                   board[shieldRow[i]][shieldCol[i]],
                   i,
                   shieldRow[i],
                   shieldCol[i]);
        }
    }
}

// ----------------------------

// trap
void triggerTrap(int row, int col, int player, int index)
{
    printf("\n");
    printf("  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printf("  !!  TRAP triggered at (%d,%d)! !!\n", row, col);
    printf("  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

    int direction = (player == 1) ? 1 : -1;

    int forwardRow = row + direction;
    int backwardRow = row - direction;

    if (inBoard(forwardRow, col) && board[forwardRow][col] == '.')
    {
        printf("  >> Trap: pushed FORWARD -> (%d,%d)\n", forwardRow, col);

        board[forwardRow][col] = board[row][col];
        owner[forwardRow][col] = owner[row][col];
        moveShield(row, col, forwardRow, col);

        board[row][col] = '.';
        owner[row][col] = 0;
    }
    else if (inBoard(backwardRow, col) && board[backwardRow][col] == '.')
    {
        printf("  >> Trap: pushed BACKWARD -> (%d,%d)\n", backwardRow, col);

        board[backwardRow][col] = board[row][col];
        owner[backwardRow][col] = owner[row][col];
        moveShield(row, col, backwardRow, col);

        board[row][col] = '.';
        owner[row][col] = 0;
    }
    else
    {
        printf("  >> Trap: no room to move, piece stays.\n");
    }

    trapActive[index] = 0;
    printf("  >> Trap is now gone.\n");
}

// ----------------------------

// play turn
int playTurn(int player)
{
    printBoard();
    printf("\n=== Player %d's Turn ===\n", player);

    // ===== shield =====
    if (!shieldUsed[player])
    {
        int use = -1;

        while (use != 0 && use != 1)
        {
            printf("  [Feature 2] Use Shield this turn? (1=Yes / 0=No): ");

            if (scanf("%d", &use) != 1)
            {
                use = -1; // scanf ล้มเหลว (พิมพ์ตัวอักษร) → reset ค่า
            }
            clearInputBuffer(); // ล้าง buffer ทุกกรณี

            if (use != 0 && use != 1)
            {
                printf("  ! Please enter 1 or 0 only.\n");
            }
        }

        if (use == 1)
        {
            while (1)
            {
                printf("  Select piece to shield (J / K / Q) or '0' to cancel: ");
                scanf("%s", pieceInput);

                if (pieceInput[0] == '0')
                    break;

                char piece = pieceInput[0];
                if (piece >= 'a' && piece <= 'z')
                    piece -= 32;

                int row, col;

                if (!findPiece(piece, player, &row, &col))
                {
                    printf("  ! You don't have '%c'\n", piece);
                    continue;
                }

                shieldRow[player] = row;
                shieldCol[player] = col;
                shieldUsed[player] = 1;

                printf("  >> Shield applied to '%c' at (%d,%d)\n",
                       board[row][col], row, col);

                printBoard();
                break;
            }
        }
    }

    // ===== select piece =====
    printf("  Select piece to move (J / K / Q): ");
    scanf("%s", pieceInput);

    char piece = pieceInput[0];
    if (piece >= 'a' && piece <= 'z')
        piece -= 32;

    int row, col;

    if (!findPiece(piece, player, &row, &col))
    {
        printf("  ! You don't have '%c'\n", piece);
        return 0;
    }

    printf("  >> Selected '%c' at (%d,%d)%s\n",
           board[row][col], row, col,
           hasShield(row, col, player) ? "  [** SHIELDED **]" : "");

    // ===== direction =====
    printf("  Direction (W=Up / S=Down / A=Left / D=Right): ");
    scanf("%s", dirInput);

    char direction = dirInput[0];
    if (direction >= 'a' && direction <= 'z')
        direction -= 32;

    int dRow = 0, dCol = 0;

    if (direction == 'W')
        dRow = -1;
    else if (direction == 'S')
        dRow = 1;
    else if (direction == 'A')
        dCol = -1;
    else if (direction == 'D')
        dCol = 1;
    else
    {
        printf("  ! Invalid key.\n");
        return 0;
    }

    int newRow = row + dRow;
    int newCol = col + dCol;

    if (!inBoard(newRow, newCol))
    {
        printf("  ! Out of bounds.\n");
        return 0;
    }

    if (owner[newRow][newCol] == player)
    {
        printf("  ! Cannot move onto your own piece.\n");
        return 0;
    }

    int hitTrap = -1;
    for (int i = 0; i < 2; i++)
    {
        if (trapActive[i] && trapRow[i] == newRow && trapCol[i] == newCol)
            hitTrap = i;
    }

    char defender = board[newRow][newCol];

    // ===== move =====
    if (defender == '.' || hitTrap != -1)
    {
        board[newRow][newCol] = board[row][col];
        owner[newRow][newCol] = player;
        moveShield(row, col, newRow, newCol);

        board[row][col] = '.';
        owner[row][col] = 0;

        if (hitTrap != -1)
            triggerTrap(newRow, newCol, player, hitTrap);
        else
            printf("  >> Moved '%c' to (%d,%d)\n", piece, newRow, newCol);
    }
    else
    {
        int defenderPlayer = owner[newRow][newCol];
        int defenderShield = hasShield(newRow, newCol, defenderPlayer);
        int attackerShield = hasShield(row, col, player);

        if (defenderShield)
        {
            printf("  *** Defender shielded! Your piece is destroyed! ***\n");
            removeShield(newRow, newCol);
            board[row][col] = '.';
            owner[row][col] = 0;
        }
        else if (canCapture(piece, defender))
        {
            printf("  >> '%c' captures '%c'!\n", piece, defender);

            removeShield(newRow, newCol);

            board[newRow][newCol] = piece;
            owner[newRow][newCol] = player;
            moveShield(row, col, newRow, newCol);

            board[row][col] = '.';
            owner[row][col] = 0;
        }
        else if (canCapture(defender, piece))
        {
            if (attackerShield)
            {
                printf("  *** Shield saved your piece! ***\n");
                removeShield(row, col);
            }
            else
            {
                printf("  >> Your '%c' was captured!\n", piece);
                board[row][col] = '.';
                owner[row][col] = 0;
            }
        }
        else
        {
            if (attackerShield && !defenderShield)
            {
                removeShield(row, col);
                board[newRow][newCol] = piece;
                owner[newRow][newCol] = player;
                board[row][col] = '.';
                owner[row][col] = 0;
            }
            else if (defenderShield && !attackerShield)
            {
                removeShield(newRow, newCol);
                board[row][col] = '.';
                owner[row][col] = 0;
            }
            else if (!attackerShield && !defenderShield)
            {
                board[row][col] = '.';
                owner[row][col] = 0;
                board[newRow][newCol] = '.';
                owner[newRow][newCol] = 0;
            }
            else
            {
                removeShield(row, col);
                removeShield(newRow, newCol);
            }
        }
    }

    return 1;
}

// ----------------------------

// check result
int checkGame()
{
    int p1 = countPieces(1);
    int p2 = countPieces(2);

    if (p1 == 0 && p2 == 0)
        return 3;
    if (p1 == 0)
        return 2;
    if (p2 == 0)
        return 1;

    return 0;
}

// ----------------------------

int main()
{
    printf("============================================\n");
    printf("          JKQ Board Game\n");
    printf("       Mak Ruk Yuttha Hatthi\n");
    printf("============================================\n");

    setupBoard();

    while (1)
    {
        while (!playTurn(currentPlayer))
            ;

        int result = checkGame();

        if (result == 1)
        {
            printBoard();
            printf("============================================\n");
            printf("         *** Player 1 WINS! ***\n");
            printf("============================================\n");
            break;
        }
        else if (result == 2)
        {
            printBoard();
            printf("============================================\n");
            printf("        *** Player 2 WINS! ***\n");
            printf("============================================\n");
            break;
        }
        else if (result == 3)
        {
            printBoard();
            printf("============================================\n");
            printf("            *** DRAW! ***\n");
            printf("============================================\n");
            break;
        }

        currentPlayer = (currentPlayer == 1) ? 2 : 1;
    }

    return 0;
}
