#include <stdio.h>  // ใช้สำหรับรับ/แสดงผลข้อมูล เช่น printf, scanf
#include <stdlib.h> // ใช้สำหรับฟังก์ชัน rand() สุ่มเลข
#include <time.h>   // ใช้สำหรับ time() เพื่อตั้งค่าเมล็ดสุ่มให้ไม่ซ้ำกันทุกครั้ง

// ขนาดกระดาน 5x5 (เก็บไว้ในตัวแปรเผื่อแก้ไขทีหลัง)
int BOARD_SIZE = 5;

// ===== กระดาน =====
char board[5][5]; // เก็บตัวหมากในแต่ละช่อง เช่น 'J', 'K', 'Q' หรือ '.' ถ้าช่องว่าง
int owner[5][5];  // เก็บว่าช่องนั้นเป็นของใคร (0 = ว่าง, 1 = ผู้เล่น 1, 2 = ผู้เล่น 2)

// ===== กับดัก =====
int trapRow[2];    // แถวที่กับดักซ่อนอยู่ (มี 2 อัน index 0 กับ 1)
int trapCol[2];    // คอลัมน์ที่กับดักซ่อนอยู่
int trapActive[2]; // บอกว่ากับดักยังอยู่ไหม (1 = ยังซ่อนอยู่, 0 = ถูกเหยียบไปแล้ว)

// ===== โล่ป้องกัน =====
int shieldRow[3];  // แถวของหมากที่ได้รับโล่ (index 1 = ผู้เล่น 1, index 2 = ผู้เล่น 2)
int shieldCol[3];  // คอลัมน์ของหมากที่ได้รับโล่
int shieldUsed[3]; // บอกว่าเคยใช้โล่ไปแล้วยัง (0 = ยังไม่ได้ใช้, 1 = ใช้ไปแล้ว)

// ===== ตัวแปรเกม =====
int currentPlayer = 1; // ตอนนี้เป็นเทิร์นของใคร เริ่มต้นที่ผู้เล่น 1

char pieceInput[8]; // เก็บข้อความที่ผู้เล่นพิมพ์เลือกหมาก เช่น "J", "K", "Q"
char dirInput[8];   // เก็บข้อความที่ผู้เล่นพิมพ์ทิศทาง เช่น "W", "A", "S", "D"

// ----------------------------

// ฟังก์ชันล้าง input buffer
// ปัญหา: หลังจาก scanf รับค่าแล้ว จะมีตัวอักษรเก่าค้างใน buffer
// ถ้าไม่ล้างออก การ scanf ครั้งถัดไปอาจดึงค่าเก่านั้นมาใช้แทน
void clearInputBuffer()
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
        ; // วนดึงทิ้งไปเรื่อยๆ จนกว่าจะเจอ Enter หรือสิ้นสุดไฟล์
}

// ตรวจสอบว่าตำแหน่ง (row, col) อยู่ในกระดานหรือเปล่า
// คืนค่า 1 ถ้าอยู่ในกระดาน, คืนค่า 0 ถ้าออกนอกกระดาน
int inBoard(int row, int col)
{
    return (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE);
}

// นับจำนวนหมากที่เหลืออยู่บนกระดานของผู้เล่นคนนั้น
int countPieces(int player)
{
    int count = 0;
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            // ถ้าเจ้าของช่องนี้คือผู้เล่นที่ต้องการ ให้นับเพิ่ม 1
            if (owner[row][col] == player)
                count++;
        }
    }
    return count;
}

// กฎการกิน: K กิน Q, Q กิน J, J กิน K (เหมือน Rock-Paper-Scissors)
// คืนค่า 1 ถ้ากินได้, คืนค่า 0 ถ้ากินไม่ได้
int canCapture(char attacker, char defender)
{
    if (attacker == 'K' && defender == 'Q')
        return 1; // K กิน Q ได้
    if (attacker == 'Q' && defender == 'J')
        return 1; // Q กิน J ได้
    if (attacker == 'J' && defender == 'K')
        return 1; // J กิน K ได้
    return 0;     // กรณีอื่นๆ กินไม่ได้
}

// ค้นหาหมากชื่อ piece ของผู้เล่น player ว่าอยู่ที่ช่องไหนบนกระดาน
// ถ้าเจอ จะใส่ตำแหน่งกลับไปใน *row, *col แล้วคืนค่า 1
// ถ้าหาไม่เจอ คืนค่า 0
int findPiece(char piece, int player, int *row, int *col)
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            // เจอหมากที่ตรงกันทั้งชื่อและเจ้าของ
            if (board[i][j] == piece && owner[i][j] == player)
            {
                *row = i; // บันทึกตำแหน่งแถวที่เจอ
                *col = j; // บันทึกตำแหน่งคอลัมน์ที่เจอ
                return 1;
            }
        }
    }
    return 0; // วนครบแล้วหาไม่เจอ
}

// ตรวจสอบว่าหมากที่ตำแหน่ง (row, col) ของผู้เล่น player มีโล่ปกป้องอยู่ไหม
// คืนค่า 1 ถ้ามีโล่, คืนค่า 0 ถ้าไม่มี
int hasShield(int row, int col, int player)
{
    return (shieldRow[player] == row && shieldCol[player] == col);
}

// เอาโล่ออกจากหมากที่ตำแหน่ง (row, col)
// เรียกใช้ตอนโล่ถูกใช้ไปแล้ว หรือหมากที่ถือโล่ถูกกิน
void removeShield(int row, int col)
{
    for (int i = 1; i <= 2; i++)
    {
        // เช็คทั้งสองผู้เล่น ว่าใครเป็นเจ้าของโล่ที่ตำแหน่งนี้
        if (shieldRow[i] == row && shieldCol[i] == col)
        {
            shieldRow[i] = -1; // รีเซ็ตตำแหน่งโล่ให้เป็น -1 = ไม่มีโล่แล้ว
            shieldCol[i] = -1;
        }
    }
}

// อัปเดตตำแหน่งโล่เมื่อหมากเดินจากช่องเก่าไปช่องใหม่
// โล่จะต้องตามหมากไปทุกครั้งที่หมากขยับ
void moveShield(int fromRow, int fromCol, int toRow, int toCol)
{
    for (int i = 1; i <= 2; i++)
    {
        // ถ้าโล่ของผู้เล่น i อยู่ที่ช่องเก่า ให้ย้ายไปช่องใหม่ด้วย
        if (shieldRow[i] == fromRow && shieldCol[i] == fromCol)
        {
            shieldRow[i] = toRow;
            shieldCol[i] = toCol;
        }
    }
}

// ----------------------------

// เตรียมกระดานก่อนเริ่มเกม
// ทำครั้งเดียวตอนเริ่ม: วางหมาก, รีเซ็ตโล่, สุ่มวางกับดัก
void setupBoard()
{
    // ตั้งค่าเมล็ดสุ่มด้วยเวลาปัจจุบัน ทำให้กับดักวางต่างกันทุกครั้งที่รันโปรแกรม
    srand((unsigned int)time(NULL));

    // ล้างทุกช่องบนกระดานให้ว่างก่อน
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            board[row][col] = '.'; // '.' หมายถึงช่องว่าง ไม่มีหมาก
            owner[row][col] = 0;   // 0 หมายถึงไม่มีเจ้าของ
        }
    }

    // วางหมากของผู้เล่น 1 ที่แถวบนสุด (row 0)
    board[0][1] = 'J';
    owner[0][1] = 1;
    board[0][2] = 'K';
    owner[0][2] = 1;
    board[0][3] = 'Q';
    owner[0][3] = 1;

    // วางหมากของผู้เล่น 2 ที่แถวล่างสุด (row 4)
    board[4][1] = 'J';
    owner[4][1] = 2;
    board[4][2] = 'K';
    owner[4][2] = 2;
    board[4][3] = 'Q';
    owner[4][3] = 2;

    // รีเซ็ตโล่ของทั้งสองผู้เล่น — ตอนเริ่มยังไม่มีใครมีโล่
    for (int i = 1; i <= 2; i++)
    {
        shieldRow[i] = shieldCol[i] = -1; // -1 หมายถึงไม่มีโล่
        shieldUsed[i] = 0;                // 0 หมายถึงยังไม่ได้ใช้
    }

    // สุ่มวางกับดัก 2 อัน
    // กำหนดช่วงแถวที่แต่ละกับดักจะถูกสุ่มวาง
    // กับดักอันที่ 0 → สุ่มในแถว 1-2 (ใกล้ฝั่งผู้เล่น 1)
    // กับดักอันที่ 1 → สุ่มในแถว 2-3 (ใกล้ฝั่งผู้เล่น 2)
    int range[2][2] = {{1, 2}, {2, 3}};

    for (int i = 0; i < 2; i++)
    {
        trapActive[i] = 1; // กับดักนี้ยังซ่อนอยู่บนกระดาน

        // วนสุ่มตำแหน่งซ้ำจนกว่าจะได้ตำแหน่งที่ถูกต้อง
        do
        {
            // สุ่มแถวในช่วงที่กำหนดของกับดักอันนี้
            trapRow[i] = range[i][0] + rand() % (range[i][1] - range[i][0] + 1);
            // สุ่มคอลัมน์ใดก็ได้บนกระดาน
            trapCol[i] = rand() % BOARD_SIZE;
        }
        // เงื่อนไขที่ทำให้สุ่มใหม่:
        // 1. ต้องเป็นช่องว่าง ห้ามทับหมาก
        // 2. กับดักอันที่ 2 ห้ามซ้อนตำแหน่งกับกับดักอันแรก
        while (board[trapRow[i]][trapCol[i]] != '.' ||
               (i == 1 && trapRow[1] == trapRow[0] && trapCol[1] == trapCol[0]));
    }

    printf("[Feature 1] 2 traps have been secretly placed!\n");
}

// ----------------------------

// แสดงกระดานบนหน้าจอ พร้อมบอกจำนวนหมากและสถานะโล่
// หมากที่มีโล่จะแสดงเป็น [K1] (มีวงเล็บ) แทนที่จะเป็น K1 แบบปกติ
void printBoard()
{
    // แสดงจำนวนหมากของผู้เล่น 1 ด้านบน
    printf("\n  *** PLAYER 1 *** (pieces: %d)\n", countPieces(1));

    // พิมพ์หมายเลขคอลัมน์ด้านบนสุด
    printf("        ");
    for (int col = 0; col < BOARD_SIZE; col++)
        printf("  %d   ", col);
    printf("\n");

    // วนพิมพ์ทีละแถว
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        // พิมพ์เส้นแบ่างแนวนอนก่อนแต่ละแถว
        printf("        ");
        for (int col = 0; col < BOARD_SIZE; col++)
            printf("+-----");
        printf("+\n");

        // พิมพ์เลขแถวด้านซ้าย แล้วพิมพ์หมากในแต่ละช่อง
        printf("   %d    ", row);

        for (int col = 0; col < BOARD_SIZE; col++)
        {
            if (board[row][col] == '.')
            {
                // ช่องว่าง ไม่มีหมาก
                printf("|  .  ");
            }
            else
            {
                // ช่องมีหมาก — เช็คว่าหมากตัวนี้มีโล่ไหม
                if (hasShield(row, col, owner[row][col]))
                    printf("|[%c%d] ", board[row][col], owner[row][col]); // มีโล่ → แสดง [K1]
                else
                    printf("| %c%d  ", board[row][col], owner[row][col]); // ไม่มีโล่ → แสดง K1
            }
        }
        printf("|\n");
    }

    // พิมพ์เส้นปิดด้านล่างสุด
    printf("        ");
    for (int col = 0; col < BOARD_SIZE; col++)
        printf("+-----");
    printf("+\n");

    // แสดงจำนวนหมากของผู้เล่น 2 ด้านล่าง
    printf("  *** PLAYER 2 *** (pieces: %d)\n", countPieces(2));

    // แสดงสถานะโล่ของแต่ละผู้เล่น (ถ้ามีโล่อยู่)
    for (int i = 1; i <= 2; i++)
    {
        if (shieldRow[i] != -1) // -1 หมายถึงไม่มีโล่
        {
            printf("  [Shield] Player %d: %c%d at (%d,%d) is shielded\n",
                   i,
                   board[shieldRow[i]][shieldCol[i]], // ชื่อหมากที่ถือโล่
                   i,
                   shieldRow[i],
                   shieldCol[i]);
        }
    }
}

// ----------------------------

// จัดการการปะทะเมื่อหมาก (attacker) พุ่งเข้าชนหมากข้าศึกที่ตำแหน่ง (toRow, toCol)
// ใช้กฎเดียวกับการปะทะปกติ: K>Q>J>K, โล่ดูดซับได้, ชนิดเดียวกันตายทั้งคู่
// fromRow/fromCol คือตำแหน่งที่ attacker อยู่ตอนนี้ (หลังถูกกับดักผลักมาแล้ว)
void resolveCombat(int fromRow, int fromCol, int toRow, int toCol, int player)
{
    char attacker = board[fromRow][fromCol];  // หมากที่ถูกผลักมา
    char defender = board[toRow][toCol];      // หมากข้าศึกที่อยู่ปลายทาง
    int defenderPlayer = owner[toRow][toCol]; // เจ้าของหมากฝ่ายรับ

    int attackerShield = hasShield(fromRow, fromCol, player);     // ผู้ถูกผลักมีโล่ไหม
    int defenderShield = hasShield(toRow, toCol, defenderPlayer); // ฝ่ายรับมีโล่ไหม

    printf("  >> Trap slams '%c' (Player %d) into '%c' (Player %d)!\n",
           attacker, player, defender, defenderPlayer);

    if (defenderShield)
    {
        // ฝ่ายรับมีโล่ → โล่ดูดซับ ผู้ถูกผลักตาย
        printf("  *** Defender shielded! '%c' (Player %d) is destroyed! ***\n",
               attacker, player);
        removeShield(toRow, toCol);    // โล่ถูกใช้ไปแล้ว
        board[fromRow][fromCol] = '.'; // หมากที่ถูกผลักตาย
        owner[fromRow][fromCol] = 0;
    }
    else if (canCapture(attacker, defender))
    {
        // ผู้ถูกผลักกินฝ่ายรับได้ตามกฎ K>Q>J>K
        printf("  >> '%c' (Player %d) captures '%c' (Player %d)!\n",
               attacker, player, defender, defenderPlayer);
        removeShield(toRow, toCol);     // เอาโล่ฝ่ายรับออก
        board[toRow][toCol] = attacker; // หมากพุ่งเข้าแทนที่
        owner[toRow][toCol] = player;
        moveShield(fromRow, fromCol, toRow, toCol); // โล่ตามไปด้วย
        board[fromRow][fromCol] = '.';
        owner[fromRow][fromCol] = 0;
    }
    else if (canCapture(defender, attacker))
    {
        // ฝ่ายรับแข็งแกร่งกว่า → ผู้ถูกผลักตาย
        if (attackerShield)
        {
            // แต่มีโล่ → รอดชีวิต โล่หมด
            printf("  *** Shield saved '%c' (Player %d) from being captured! ***\n",
                   attacker, player);
            removeShield(fromRow, fromCol);
        }
        else
        {
            printf("  >> '%c' (Player %d) was destroyed by '%c' (Player %d)!\n",
                   attacker, player, defender, defenderPlayer);
            board[fromRow][fromCol] = '.';
            owner[fromRow][fromCol] = 0;
        }
    }
    else
    {
        // หมากชนิดเดียวกันชนกัน
        if (attackerShield && !defenderShield)
        {
            // ผู้ถูกผลักมีโล่ ฝ่ายรับไม่มี → ผู้ถูกผลักรอด ฝ่ายรับตาย
            printf("  >> Same type! '%c' (Player %d) had a shield and SURVIVED!\n",
                   attacker, player);
            removeShield(fromRow, fromCol);
            board[toRow][toCol] = attacker;
            owner[toRow][toCol] = player;
            board[fromRow][fromCol] = '.';
            owner[fromRow][fromCol] = 0;
        }
        else if (defenderShield && !attackerShield)
        {
            // ฝ่ายรับมีโล่ ผู้ถูกผลักไม่มี → ฝ่ายรับรอด ผู้ถูกผลักตาย
            printf("  >> Same type! '%c' (Player %d) had a shield and SURVIVED!\n",
                   defender, defenderPlayer);
            removeShield(toRow, toCol);
            board[fromRow][fromCol] = '.';
            owner[fromRow][fromCol] = 0;
        }
        else if (!attackerShield && !defenderShield)
        {
            // ไม่มีโล่ทั้งคู่ → ตายทั้งคู่
            printf("  >> Same type clash! Both '%c' removed!\n", attacker);
            board[fromRow][fromCol] = '.';
            owner[fromRow][fromCol] = 0;
            board[toRow][toCol] = '.';
            owner[toRow][toCol] = 0;
        }
        else
        {
            // มีโล่ทั้งคู่ → รอดทั้งคู่ โล่หมดทั้งสอง
            printf("  >> Same type! Both had shields - BOTH SURVIVED! Shields used up.\n");
            removeShield(fromRow, fromCol);
            removeShield(toRow, toCol);
        }
    }
}

// ----------------------------

// จัดการเมื่อหมากเหยียบกับดัก
// ผลคือหมากจะถูกผลักไปข้างหน้า 1 ช่อง ถ้าข้างหน้าไม่ว่างก็ถอยหลัง 1 ช่อง
// ถ้าช่องที่ผลักไปมีหมากข้าศึก → เกิดการปะทะตามกติกาทันที
// กับดักจะหายไปจากกระดานหลังถูกเหยียบ ไม่ว่าจะเกิดอะไรขึ้น
void triggerTrap(int row, int col, int player, int index)
{
    printf("\n");
    printf("  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printf("  !!  TRAP triggered at (%d,%d)! !!\n", row, col);
    printf("  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

    // ทิศทางผลักขึ้นอยู่กับว่าเป็นผู้เล่นคนไหน
    // ผู้เล่น 1 เดินลง → ถูกผลักลงต่อไป (+1)
    // ผู้เล่น 2 เดินขึ้น → ถูกผลักขึ้นต่อไป (-1)
    int direction = (player == 1) ? 1 : -1;

    int forwardRow = row + direction;  // ช่องข้างหน้า (ทิศทางที่หมากกำลังเดิน)
    int backwardRow = row - direction; // ช่องข้างหลัง (ทิศทางตรงข้าม)

    // ช่องที่ผลักได้คือ: ว่างอยู่ หรือ มีหมากข้าศึก (ห้ามทับหมากตัวเอง)
    int canPushForward = inBoard(forwardRow, col) && (board[forwardRow][col] == '.' || owner[forwardRow][col] != player);
    int canPushBackward = inBoard(backwardRow, col) && (board[backwardRow][col] == '.' || owner[backwardRow][col] != player);

    if (canPushForward)
    {
        printf("  >> Trap: pushed FORWARD -> (%d,%d)\n", forwardRow, col);

        if (board[forwardRow][col] != '.')
        {
            // ช่องข้างหน้ามีหมากข้าศึก → ย้ายหมากไปก่อน แล้วปะทะ
            char savedPiece = board[row][col];
            int savedOwner = owner[row][col];
            board[row][col] = '.';
            owner[row][col] = 0;
            moveShield(row, col, forwardRow, col);
            resolveCombat(savedPiece, savedOwner, forwardRow, col, player);
        }
        else
        {
            // ช่องข้างหน้าว่าง → ย้ายหมากไปปกติ
            board[forwardRow][col] = board[row][col];
            owner[forwardRow][col] = owner[row][col];
            moveShield(row, col, forwardRow, col);
            board[row][col] = '.';
            owner[row][col] = 0;
        }
    }
    else if (canPushBackward)
    {
        printf("  >> Trap: pushed BACKWARD -> (%d,%d)\n", backwardRow, col);

        if (board[backwardRow][col] != '.')
        {
            // ช่องข้างหลังมีหมากข้าศึก → ย้ายหมากไปก่อน แล้วปะทะ
            char savedPiece = board[row][col];
            int savedOwner = owner[row][col];
            board[row][col] = '.';
            owner[row][col] = 0;
            moveShield(row, col, backwardRow, col);
            resolveCombat(savedPiece, savedOwner, backwardRow, col, player);
        }
        else
        {
            // ช่องข้างหลังว่าง → ย้ายหมากไปปกติ
            board[backwardRow][col] = board[row][col];
            owner[backwardRow][col] = owner[row][col];
            moveShield(row, col, backwardRow, col);
            board[row][col] = '.';
            owner[row][col] = 0;
        }
    }
    else
    {
        // ทั้งข้างหน้าและข้างหลังถูกปิดกั้นด้วยหมากตัวเอง → ขยับไม่ได้
        printf("  >> Trap: no room to move, piece stays.\n");
    }

    // กับดักถูกเหยียบแล้ว → หายไปจากกระดานทันที
    trapActive[index] = 0;
    printf("  >> Trap is now gone.\n");
}

// ----------------------------

// จัดการ 1 เทิร์นของผู้เล่น
// ขั้นตอน: แสดงกระดาน → ถามใช้โล่ → เลือกหมาก → เลือกทิศทาง → ตรวจกติกา → เดิน
// คืนค่า 1 ถ้าเดินสำเร็จ, คืนค่า 0 ถ้าเดินผิดพลาด (จะถูกวนให้เดินใหม่)
int playTurn(int player)
{
    printBoard(); // แสดงกระดานให้ผู้เล่นดูก่อน
    printf("\n=== Player %d's Turn ===\n", player);

    // ===== ถามใช้โล่ไหม (ถ้ายังไม่เคยใช้เลย) =====
    if (!shieldUsed[player])
    {
        int use = -1; // -1 คือยังไม่ได้ตอบ

        // วนถามซ้ำจนกว่าจะได้คำตอบที่ถูกต้อง (0 หรือ 1 เท่านั้น)
        while (use != 0 && use != 1)
        {
            printf("  [Feature 2] Use Shield this turn? (1=Yes / 0=No): ");

            if (scanf("%d", &use) != 1)
            {
                // scanf ล้มเหลว = ผู้เล่นพิมพ์ตัวอักษรแทนตัวเลข
                use = -1; // reset ค่าให้วนถามใหม่
            }
            clearInputBuffer(); // ล้าง buffer ทุกกรณี ป้องกันค่าค้าง

            if (use != 0 && use != 1)
            {
                // ได้ตัวเลขแต่ไม่ใช่ 0 หรือ 1 เช่น พิมพ์ "5" หรือ "99"
                printf("  ! Please enter 1 or 0 only.\n");
            }
        }

        if (use == 1)
        {
            // ผู้เล่นเลือกจะใช้โล่ → ให้เลือกว่าจะสวมโล่ให้หมากตัวไหน
            while (1)
            {
                printf("  Select piece to shield (J / K / Q) or '0' to cancel: ");
                scanf("%s", pieceInput);

                // ถ้าพิมพ์ '0' คือยกเลิก ออกจาก loop นี้เลย
                if (pieceInput[0] == '0')
                    break;

                char piece = pieceInput[0];
                // แปลงตัวพิมพ์เล็กเป็นตัวพิมพ์ใหญ่ เผื่อผู้เล่นพิมพ์ "j", "k", "q"
                if (piece >= 'a' && piece <= 'z')
                    piece -= 32; // ลบ 32 = แปลงเป็น uppercase ใน ASCII

                int row, col;

                // หาหมากที่เลือกบนกระดาน ถ้าหาไม่เจอให้ถามใหม่
                if (!findPiece(piece, player, &row, &col))
                {
                    printf("  ! You don't have '%c'\n", piece);
                    continue;
                }

                // บันทึกว่าหมากตัวนี้ได้รับโล่แล้ว
                shieldRow[player] = row;
                shieldCol[player] = col;
                shieldUsed[player] = 1; // ทำเครื่องหมายว่าใช้โล่ไปแล้ว (ใช้ได้แค่ครั้งเดียว)

                printf("  >> Shield applied to '%c' at (%d,%d)\n",
                       board[row][col], row, col);

                printBoard(); // refresh กระดานให้เห็น [K1] ก่อนเดิน
                break;
            }
        }
    }

    // ===== เลือกหมากที่จะเดิน =====
    printf("  Select piece to move (J / K / Q): ");
    scanf("%s", pieceInput);

    char piece = pieceInput[0];
    // แปลงตัวพิมพ์เล็กเป็นใหญ่อีกครั้ง
    if (piece >= 'a' && piece <= 'z')
        piece -= 32;

    int row, col;

    // ตรวจว่ามีหมากชนิดนั้นบนกระดานไหม
    if (!findPiece(piece, player, &row, &col))
    {
        printf("  ! You don't have '%c'\n", piece);
        return 0; // เดินผิดพลาด → คืนค่า 0 เพื่อวนให้เดินใหม่
    }

    // แสดงหมากที่เลือก ถ้ามีโล่จะแสดง [** SHIELDED **] ด้วย
    printf("  >> Selected '%c' at (%d,%d)%s\n",
           board[row][col], row, col,
           hasShield(row, col, player) ? "  [** SHIELDED **]" : "");

    // ===== เลือกทิศทาง WASD =====
    printf("  Direction (W=Up / S=Down / A=Left / D=Right): ");
    scanf("%s", dirInput);

    char direction = dirInput[0];
    // แปลงตัวพิมพ์เล็กเป็นใหญ่
    if (direction >= 'a' && direction <= 'z')
        direction -= 32;

    // แปลงปุ่มที่กดเป็นค่าเปลี่ยนแถว (dRow) และคอลัมน์ (dCol)
    int dRow = 0, dCol = 0;

    if (direction == 'W')
        dRow = -1; // ขึ้นบน = แถวลดลง
    else if (direction == 'S')
        dRow = 1; // ลงล่าง = แถวเพิ่มขึ้น
    else if (direction == 'A')
        dCol = -1; // ซ้าย = คอลัมน์ลดลง
    else if (direction == 'D')
        dCol = 1; // ขวา = คอลัมน์เพิ่มขึ้น
    else
    {
        printf("  ! Invalid key.\n");
        return 0; // กดปุ่มผิด → เดินใหม่
    }

    // คำนวณตำแหน่งปลายทางที่หมากจะเดินไป
    int newRow = row + dRow;
    int newCol = col + dCol;

    // ตรวจว่าปลายทางอยู่ในกระดานไหม
    if (!inBoard(newRow, newCol))
    {
        printf("  ! Out of bounds.\n");
        return 0;
    }

    // ตรวจว่าปลายทางไม่ใช่หมากของตัวเอง (ห้ามเดินทับหมากตัวเอง)
    if (owner[newRow][newCol] == player)
    {
        printf("  ! Cannot move onto your own piece.\n");
        return 0;
    }

    // เช็คว่าปลายทางมีกับดักซ่อนอยู่ไหม
    int hitTrap = -1; // -1 = ไม่เจอกับดัก
    for (int i = 0; i < 2; i++)
    {
        if (trapActive[i] && trapRow[i] == newRow && trapCol[i] == newCol)
            hitTrap = i; // บันทึกว่าเจอกับดักอันไหน
    }

    char defender = board[newRow][newCol]; // หมากที่อยู่ที่ปลายทาง (ถ้ามี)

    // ===== เดินหมาก =====
    if (defender == '.' || hitTrap != -1)
    {
        // กรณีที่ 1: ช่องปลายทางว่าง (หรือมีกับดักซ่อน) → ย้ายหมากไปเลย
        board[newRow][newCol] = board[row][col]; // วางหมากที่ช่องใหม่
        owner[newRow][newCol] = player;
        moveShield(row, col, newRow, newCol); // โล่ตามหมากไปด้วย

        board[row][col] = '.'; // ช่องเดิมกลายเป็นว่าง
        owner[row][col] = 0;

        if (hitTrap != -1)
            triggerTrap(newRow, newCol, player, hitTrap); // โดนกับดัก! ให้จัดการต่อ
        else
            printf("  >> Moved '%c' to (%d,%d)\n", piece, newRow, newCol);
    }
    else
    {
        // กรณีที่ 2: ช่องปลายทางมีหมากข้าศึก → เกิดการปะทะ
        int defenderPlayer = owner[newRow][newCol];                     // ผู้เล่นที่เป็นเจ้าของหมากฝ่ายรับ
        int defenderShield = hasShield(newRow, newCol, defenderPlayer); // ฝ่ายรับมีโล่ไหม
        int attackerShield = hasShield(row, col, player);               // ฝ่ายโจมตีมีโล่ไหม

        if (defenderShield)
        {
            // ---- ฝ่ายรับมีโล่ → โล่ดูดซับการโจมตี ผู้โจมตีตาย ----
            printf("  *** Defender shielded! Your piece is destroyed! ***\n");
            removeShield(newRow, newCol); // โล่ถูกใช้ไปแล้ว เอาออก
            board[row][col] = '.';        // หมากโจมตีถูกทำลาย
            owner[row][col] = 0;
        }
        else if (canCapture(piece, defender))
        {
            // ---- ผู้โจมตีกินฝ่ายรับได้ตามกฎ K>Q>J>K ----
            printf("  >> '%c' captures '%c'!\n", piece, defender);

            removeShield(newRow, newCol); // เอาโล่ฝ่ายรับออก (ถ้ามี)

            board[newRow][newCol] = piece; // หมากโจมตีเข้ามาแทนที่
            owner[newRow][newCol] = player;
            moveShield(row, col, newRow, newCol); // โล่ผู้โจมตีตามไปด้วย

            board[row][col] = '.'; // ช่องเดิมกลายเป็นว่าง
            owner[row][col] = 0;
        }
        else if (canCapture(defender, piece))
        {
            // ---- ฝ่ายรับกินผู้โจมตีได้ตามกฎ ----
            if (attackerShield)
            {
                // แต่ผู้โจมตีมีโล่อยู่ → โล่ดูดซับ ผู้โจมตีรอดชีวิต โล่หมด
                printf("  *** Shield saved your piece! ***\n");
                removeShield(row, col); // โล่ถูกใช้ไปแล้ว
            }
            else
            {
                // ไม่มีโล่ → หมากโจมตีถูกกินตามปกติ
                printf("  >> Your '%c' was captured!\n", piece);
                board[row][col] = '.';
                owner[row][col] = 0;
            }
        }
        else
        {
            // ---- หมากชนิดเดียวกันชนกัน ----
            if (attackerShield && !defenderShield)
            {
                // ผู้โจมตีมีโล่ ฝ่ายรับไม่มี → ผู้โจมตีรอด ฝ่ายรับตาย โล่หมด
                removeShield(row, col);
                board[newRow][newCol] = piece;
                owner[newRow][newCol] = player;
                board[row][col] = '.';
                owner[row][col] = 0;
            }
            else if (defenderShield && !attackerShield)
            {
                // ฝ่ายรับมีโล่ ผู้โจมตีไม่มี → ฝ่ายรับรอด ผู้โจมตีตาย โล่หมด
                removeShield(newRow, newCol);
                board[row][col] = '.';
                owner[row][col] = 0;
            }
            else if (!attackerShield && !defenderShield)
            {
                // ไม่มีโล่ทั้งคู่ → ตายทั้งคู่ หายไปจากกระดานเลย
                board[row][col] = '.';
                owner[row][col] = 0;
                board[newRow][newCol] = '.';
                owner[newRow][newCol] = 0;
            }
            else
            {
                // มีโล่ทั้งคู่ → รอดทั้งคู่ โล่ทั้งสองหมดพร้อมกัน
                removeShield(row, col);
                removeShield(newRow, newCol);
            }
        }
    }

    return 1; // เดินสำเร็จ
}

// ----------------------------

// เช็คสถานะเกมว่าจบหรือยัง
// คืนค่า 0 = ยังเล่นอยู่, 1 = ผู้เล่น 1 ชนะ, 2 = ผู้เล่น 2 ชนะ, 3 = เสมอ
int checkGame()
{
    int p1 = countPieces(1); // หมากที่เหลือของผู้เล่น 1
    int p2 = countPieces(2); // หมากที่เหลือของผู้เล่น 2

    if (p1 == 0 && p2 == 0)
        return 3; // ทั้งสองหมดหมากพร้อมกัน → เสมอ
    if (p1 == 0)
        return 2; // ผู้เล่น 1 หมดหมาก → ผู้เล่น 2 ชนะ
    if (p2 == 0)
        return 1; // ผู้เล่น 2 หมดหมาก → ผู้เล่น 1 ชนะ

    return 0; // ยังมีหมากทั้งสองฝ่าย → เล่นต่อ
}

// ----------------------------

// จุดเริ่มต้นของโปรแกรม
// ลำดับ: แสดง title → เตรียมกระดาน → วนรับเทิร์น → เช็คผล → สลับผู้เล่น
int main()
{
    printf("============================================\n");
    printf("          JKQ Board Game\n");
    printf("       Mak Ruk Yuttha Hatthi\n");
    printf("============================================\n");

    setupBoard(); // เตรียมกระดานก่อนเริ่มเกม

    while (1) // วนเกมไปเรื่อยๆ จนกว่าจะมีผู้ชนะหรือเสมอ
    {
        // ถ้า playTurn คืนค่า 0 (เดินผิดพลาด) ให้วนถามใหม่จนกว่าจะเดินได้
        while (!playTurn(currentPlayer))
            ;

        // หลังเดินเสร็จ ตรวจสอบว่าเกมจบหรือยัง
        int result = checkGame();

        if (result == 1)
        {
            printBoard();
            printf("============================================\n");
            printf("         *** Player 1 WINS! ***\n");
            printf("============================================\n");
            break; // ออกจาก loop หลัก เกมจบ
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

        // เกมยังไม่จบ → สลับเทิร์นให้ผู้เล่นอีกคน
        currentPlayer = (currentPlayer == 1) ? 2 : 1;
    }

    return 0;
}
