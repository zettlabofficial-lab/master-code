#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int BOARD_SIZE = 5;

/* ============================================================
 *  ส่วนที่ 1 : กล่องเก็บข้อมูลของเกม
 *
 *  เปรียบเหมือนกระดาษจดบันทึก ก่อนเริ่มเกมจะต้องเตรียม
 *  กล่องพวกนี้ไว้รับข้อมูลตลอดการเล่น ทั้งตำแหน่งหมาก
 *  กับดัก โล่ และว่าตอนนี้ถึงเทิร์นของใคร
 * ============================================================ */

// ===== กระดาน =====
char board[5][5]; // เก็บตัวหมากในแต่ละช่อง ('J', 'K', 'Q' หรือ '.' ถ้าว่าง)
int owner[5][5];  // เก็บว่าช่องนั้นเป็นของใคร (0=ว่าง, 1=ผู้เล่น1, 2=ผู้เล่น2)

// ===== กับดัก =====
int trapRow[2];    // แถวที่กับดักซ่อนอยู่ (มี 2 อัน)
int trapCol[2];    // คอลัมน์ที่กับดักซ่อนอยู่
int trapActive[2]; // กับดักยังอยู่ไหม (1=ยังซ่อนอยู่, 0=ถูกเหยียบไปแล้ว)

// ===== โล่ป้องกัน =====
int shieldRow[3];  // แถวของหมากที่ถือโล่อยู่
int shieldCol[3];  // คอลัมน์ของหมากที่ถือโล่อยู่
int shieldUsed[3]; // เคยใช้โล่ไปแล้วยัง (0=ยังไม่ได้ใช้, 1=ใช้ไปแล้ว)

// ===== ตัวแปรเกม =====
int currentPlayer = 1; // ตอนนี้เป็นเทิร์นของผู้เล่นคนไหน

char pieceInput[8]; // รับชื่อหมากที่ผู้เล่นพิมพ์เข้ามา
char dirInput[8];   // รับทิศทางที่ผู้เล่นพิมพ์เข้ามา

/* ============================================================
 *  ส่วนที่ 2 : เครื่องมือพื้นฐาน
 *
 *  ฟังก์ชันเล็กๆ ที่ถูกเรียกใช้ซ้ำหลายที่ตลอดโปรแกรม
 *  เปรียบเหมือนอุปกรณ์ในกล่องเครื่องมือ เช่น
 *  ไขควง ค้อน ที่ใช้กับงานหลายๆ อย่าง
 * ============================================================ */

// ล้างข้อความที่ค้างอยู่หลังจากผู้เล่นกด Enter
// (ถ้าไม่ล้าง โปรแกรมอาจดึงค่าเก่ามาใช้แทนที่จะรอรับค่าใหม่)
void clearInputBuffer()
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;
}

// เช็คว่าตำแหน่งที่ระบุอยู่บนกระดานหรือเปล่า
// เช่น ถ้าหมากอยู่ขอบกระดานแล้วจะเดินออกนอก → ฟังก์ชันนี้จะบอกว่าออกนอกแล้ว
int inBoard(int row, int col)
{
    return (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE);
}

// นับว่าผู้เล่นคนนั้นยังมีหมากเหลืออยู่บนกระดานกี่ตัว
int countPieces(int player)
{
    int count = 0;
    for (int row = 0; row < BOARD_SIZE; row++)
        for (int col = 0; col < BOARD_SIZE; col++)
            if (owner[row][col] == player)
                count++;
    return count;
}

// ตรวจสอบกฎการกินตามชนิดหมาก
// K กิน Q ได้ / Q กิน J ได้ / J กิน K ได้ (วนเหมือน เป่ายิ้งฉุบ)
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

// ค้นหาว่าหมากชนิดที่ระบุของผู้เล่นคนนั้นอยู่ที่ช่องไหนบนกระดาน
// ถ้าเจอคืนค่า 1 พร้อมบอกตำแหน่ง / ถ้าหาไม่เจอคืนค่า 0
int findPiece(char piece, int player, int *row, int *col)
{
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            if (board[i][j] == piece && owner[i][j] == player)
            {
                *row = i;
                *col = j;
                return 1;
            }
    return 0;
}

// ตรวจสอบว่าหมากที่ตำแหน่งนั้นมีโล่ปกป้องอยู่ไหม
int hasShield(int row, int col, int player)
{
    return (shieldRow[player] == row && shieldCol[player] == col);
}

// เอาโล่ออกจากหมาก (เรียกเมื่อโล่ถูกใช้ไปแล้ว หรือหมากถูกกิน)
void removeShield(int row, int col)
{
    for (int i = 1; i <= 2; i++)
        if (shieldRow[i] == row && shieldCol[i] == col)
        {
            shieldRow[i] = -1;
            shieldCol[i] = -1;
        }
}

// ย้ายโล่ตามหมากไปด้วยทุกครั้งที่หมากเดิน
// (โล่ต้องอยู่กับหมากตัวที่รับโล่เสมอ ไม่ใช่ตรึงอยู่กับที่)
void moveShield(int fromRow, int fromCol, int toRow, int toCol)
{
    for (int i = 1; i <= 2; i++)
        if (shieldRow[i] == fromRow && shieldCol[i] == fromCol)
        {
            shieldRow[i] = toRow;
            shieldCol[i] = toCol;
        }
}

/* ============================================================
 *  ส่วนที่ 3 : ผลของการถูกกับดักผลัก
 *
 *  เมื่อหมากถูกผลักจากกับดักไปชนช่องใดช่องหนึ่ง
 *  ฟังก์ชันนี้จะตัดสินว่าเกิดอะไรขึ้น
 *  ถ้าช่องนั้นว่าง → ย้ายไปเลย
 *  ถ้าช่องนั้นมีข้าศึก → เกิดการปะทะตามกฎปกติทุกอย่าง
 *  (รวมถึงเรื่องโล่ด้วย)
 * ============================================================ */

void resolvePush(int fromRow, int fromCol, int toRow, int toCol, int player)
{
    char attacker = board[fromRow][fromCol];
    char defender = board[toRow][toCol];

    if (defender == '.')
    {
        // ช่องว่าง → ย้ายหมากไปเลย
        board[toRow][toCol] = attacker;
        owner[toRow][toCol] = player;
        moveShield(fromRow, fromCol, toRow, toCol);
        board[fromRow][fromCol] = '.';
        owner[fromRow][fromCol] = 0;
        printf("  >> Trap: pushed to (%d,%d)\n", toRow, toCol);
    }
    else
    {
        // มีหมากข้าศึก → ปะทะตามกติกา
        int defenderPlayer = owner[toRow][toCol];
        int attackerShield = hasShield(fromRow, fromCol, player);
        int defenderShield = hasShield(toRow, toCol, defenderPlayer);

        printf("  >> Trap: pushed into enemy '%c' at (%d,%d)!\n",
               defender, toRow, toCol);

        if (defenderShield)
        {
            // ฝ่ายรับมีโล่ → โล่ดูดซับ หมากที่ถูกผลักตาย
            printf("  *** Defender shielded! Pushed piece is destroyed! ***\n");
            removeShield(toRow, toCol);
            board[fromRow][fromCol] = '.';
            owner[fromRow][fromCol] = 0;
        }
        else if (canCapture(attacker, defender))
        {
            // หมากที่ถูกผลักกินข้าศึกได้
            printf("  >> '%c' captures '%c'!\n", attacker, defender);
            removeShield(toRow, toCol);
            board[toRow][toCol] = attacker;
            owner[toRow][toCol] = player;
            moveShield(fromRow, fromCol, toRow, toCol);
            board[fromRow][fromCol] = '.';
            owner[fromRow][fromCol] = 0;
        }
        else if (canCapture(defender, attacker))
        {
            // ข้าศึกกินหมากที่ถูกผลักได้
            if (attackerShield)
            {
                // แต่มีโล่ → รอด โล่หมด
                printf("  *** Shield saved the pushed piece! ***\n");
                removeShield(fromRow, fromCol);
            }
            else
            {
                // ไม่มีโล่ → ตาย
                printf("  >> Pushed piece '%c' was captured!\n", attacker);
                board[fromRow][fromCol] = '.';
                owner[fromRow][fromCol] = 0;
            }
        }
        else
        {
            // หมากชนิดเดียวกันชนกัน
            if (attackerShield && !defenderShield)
            {
                removeShield(fromRow, fromCol);
                board[toRow][toCol] = attacker;
                owner[toRow][toCol] = player;
                board[fromRow][fromCol] = '.';
                owner[fromRow][fromCol] = 0;
            }
            else if (defenderShield && !attackerShield)
            {
                removeShield(toRow, toCol);
                board[fromRow][fromCol] = '.';
                owner[fromRow][fromCol] = 0;
            }
            else if (!attackerShield && !defenderShield)
            {
                printf("  >> Same type collision! Both removed!\n");
                board[fromRow][fromCol] = '.';
                owner[fromRow][fromCol] = 0;
                board[toRow][toCol] = '.';
                owner[toRow][toCol] = 0;
            }
            else
            {
                printf("  >> Both shielded! Both survived!\n");
                removeShield(fromRow, fromCol);
                removeShield(toRow, toCol);
            }
        }
    }
}

/* ============================================================
 *  ส่วนที่ 4 : เตรียมกระดานก่อนเริ่มเกม
 *
 *  ทำครั้งเดียวตอนเริ่ม ทำหน้าที่เหมือนการ "จัดโต๊ะ"
 *  ก่อนเล่นเกมกระดาน ได้แก่
 *  — ล้างกระดานให้ว่างทุกช่อง
 *  — วางหมากของแต่ละฝ่ายในตำแหน่งเริ่มต้น
 *  — รีเซ็ตโล่ให้ยังไม่มีใครมี
 *  — สุ่มซ่อนกับดัก 2 อันบนกระดาน
 * ============================================================ */

void setupBoard()
{
    srand((unsigned int)time(NULL));

    for (int row = 0; row < BOARD_SIZE; row++)
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            board[row][col] = '.';
            owner[row][col] = 0;
        }

    board[0][1] = 'J';
    owner[0][1] = 1;
    board[0][2] = 'K';
    owner[0][2] = 1;
    board[0][3] = 'Q';
    owner[0][3] = 1;

    board[4][1] = 'J';
    owner[4][1] = 2;
    board[4][2] = 'K';
    owner[4][2] = 2;
    board[4][3] = 'Q';
    owner[4][3] = 2;

    for (int i = 1; i <= 2; i++)
    {
        shieldRow[i] = shieldCol[i] = -1;
        shieldUsed[i] = 0;
    }

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

/* ============================================================
 *  ส่วนที่ 5 : วาดกระดานบนหน้าจอ
 *
 *  ทำหน้าที่เหมือน "ช่างภาพ" ที่คอยถ่ายภาพสถานะปัจจุบัน
 *  ของกระดานแล้วแสดงให้ผู้เล่นเห็น ทุกครั้งที่เรียกใช้
 *  จะเห็นตำแหน่งหมากล่าสุด พร้อมบอกว่าหมากตัวไหนมีโล่
 *  (หมากที่มีโล่จะแสดงด้วยวงเล็บ เช่น [K1])
 * ============================================================ */

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
                printf("|  .  ");
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
        if (shieldRow[i] != -1)
            printf("  [Shield] Player %d: %c%d at (%d,%d) is shielded\n",
                   i, board[shieldRow[i]][shieldCol[i]], i, shieldRow[i], shieldCol[i]);
}

/* ============================================================
 *  ส่วนที่ 6 : จัดการกับดักที่ถูกเหยียบ
 *
 *  เรียกใช้ทันทีที่หมากเดินไปเหยียบช่องที่มีกับดักซ่อนอยู่
 *  โปรแกรมจะตัดสินว่าจะผลักหมากไปทิศทางไหน
 *  — ลองผลักไปข้างหน้าก่อน (ทิศทางที่หมากกำลังเดินมา)
 *  — ถ้าข้างหน้าไม่ได้ → ลองถอยหลัง
 *  — ถ้าทั้งสองทางมีหมากตัวเอง → อยู่เดิม ขยับไม่ได้
 *  และถ้าถูกผลักไปชนข้าศึก → เกิดการปะทะทันที
 *  กับดักจะหายไปหลังจากถูกเหยียบเสมอ ไม่ว่าจะเกิดอะไรขึ้น
 * ============================================================ */

void triggerTrap(int row, int col, int player, int index)
{
    printf("\n");
    printf("  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printf("  !!  TRAP triggered at (%d,%d)! !!\n", row, col);
    printf("  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

    int direction = (player == 1) ? 1 : -1;
    int forwardRow = row + direction;
    int backwardRow = row - direction;

    if (inBoard(forwardRow, col) && owner[forwardRow][col] != player)
    {
        printf("  >> Trap: pushed FORWARD -> (%d,%d)\n", forwardRow, col);
        resolvePush(row, col, forwardRow, col, player);
    }
    else if (inBoard(backwardRow, col) && owner[backwardRow][col] != player)
    {
        printf("  >> Trap: pushed BACKWARD -> (%d,%d)\n", backwardRow, col);
        resolvePush(row, col, backwardRow, col, player);
    }
    else
    {
        printf("  >> Trap: no room to move, piece stays.\n");
    }

    trapActive[index] = 0;
    printf("  >> Trap is now gone.\n");
}

/* ============================================================
 *  ส่วนที่ 7 : จัดการ 1 เทิร์นของผู้เล่น
 *
 *  นี่คือฟังก์ชันหลักที่วิ่งทุกครั้งที่ผู้เล่นได้เล่น
 *  ทำงานเป็นลำดับขั้นตอนดังนี้
 *  1) แสดงกระดานให้ดูก่อน
 *  2) ถามว่าจะใช้โล่รอบนี้ไหม (ถ้ายังไม่เคยใช้)
 *  3) ให้เลือกว่าจะเดินหมากตัวไหน (J / K / Q)
 *  4) ให้เลือกทิศทาง (W=ขึ้น S=ลง A=ซ้าย D=ขวา)
 *  5) ตรวจว่าเดินได้ไหม → ถ้าผิดกติกาให้เดินใหม่
 *  6) ถ้าเดินได้ → ย้ายหมาก ตรวจกับดัก ตรวจการปะทะ
 *  คืนค่า 1 ถ้าเทิร์นผ่านไปได้ / คืนค่า 0 ถ้าเดินผิดพลาด
 * ============================================================ */

int playTurn(int player)
{
    printBoard();
    printf("\n=== Player %d's Turn ===\n", player);

    // ===== ถามว่าจะใช้โล่ไหม =====
    if (!shieldUsed[player])
    {
        int use = -1;
        while (use != 0 && use != 1)
        {
            printf("  [Feature 2] Use Shield this turn? (1=Yes / 0=No): ");
            if (scanf("%d", &use) != 1)
                use = -1;
            clearInputBuffer();
            if (use != 0 && use != 1)
                printf("  ! Please enter 1 or 0 only.\n");
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

    // ===== เลือกหมากที่จะเดิน =====
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

    // ===== เลือกทิศทาง =====
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
        if (trapActive[i] && trapRow[i] == newRow && trapCol[i] == newCol)
            hitTrap = i;

    char defender = board[newRow][newCol];

    // ===== เดินหมาก / ปะทะ =====
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

/* ============================================================
 *  ส่วนที่ 8 : ตรวจสอบว่าเกมจบหรือยัง
 *
 *  หลังจากผู้เล่นเดินทุกครั้ง โปรแกรมจะเรียกฟังก์ชันนี้
 *  เพื่อนับหมากที่เหลือของแต่ละฝ่าย แล้วตัดสินผล
 *  — ยังมีหมากทั้งสองฝ่าย → เล่นต่อ
 *  — ฝ่ายใดฝ่ายหนึ่งหมดหมาก → อีกฝ่ายชนะ
 *  — หมดพร้อมกันในเทิร์นเดียว → เสมอ
 * ============================================================ */

int checkGame()
{
    int p1 = countPieces(1);
    int p2 = countPieces(2);

    if (p1 == 0 && p2 == 0)
        return 3; // เสมอ
    if (p1 == 0)
        return 2; // ผู้เล่น 2 ชนะ
    if (p2 == 0)
        return 1; // ผู้เล่น 1 ชนะ
    return 0;     // เล่นต่อ
}

/* ============================================================
 *  ส่วนที่ 9 : จุดเริ่มต้นของโปรแกรม
 *
 *  ทุกโปรแกรม C ต้องมี main() เพราะคอมพิวเตอร์จะเริ่มรัน
 *  จากตรงนี้เสมอ หน้าที่ของมันคือ
 *  — แสดงหน้าจอต้อนรับ
 *  — เตรียมกระดาน
 *  — วนรับเทิร์นผู้เล่นสลับกันไปเรื่อยๆ
 *  — เมื่อมีผู้ชนะหรือเสมอ แสดงผลแล้วจบโปรแกรม
 * ============================================================ */

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
