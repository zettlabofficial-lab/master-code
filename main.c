// บรรทัดนี้บอกโปรแกรมว่า "ขอยืมชุดคำสั่งพิมพ์ข้อความและรับค่าจากคีย์บอร์ดมาใช้"
// (printf, scanf อยู่ในนี้ ถ้าไม่มีบรรทัดนี้ โปรแกรมจะพิมพ์อะไรไม่ได้เลย)
#include <stdio.h>

// บรรทัดนี้ขอยืมชุดคำสั่งสุ่มเลข (rand) มาใช้
#include <stdlib.h>

// บรรทัดนี้ขอยืมชุดคำสั่งดึงเวลาปัจจุบันของเครื่อง (time) มาใช้
// เอาไว้ทำให้การสุ่มกับดักได้ตำแหน่งต่างกันทุกครั้ง
#include <time.h>

// กำหนดขนาดกระดานว่ากว้างและยาวเท่าไหร่ — ตั้งไว้ที่ 5 (กระดาน 5x5)
// ถ้าอยากเปลี่ยนขนาดกระดาน แก้เลข 5 ตรงนี้ที่เดียวพอ
int BOARD_SIZE = 5;

/* ============================================================
 *  ส่วนที่ 1 : กล่องเก็บข้อมูลของเกม
 *
 *  เปรียบเหมือนกระดาษจดบันทึก ก่อนเริ่มเกมจะต้องเตรียม
 *  กล่องพวกนี้ไว้รับข้อมูลตลอดการเล่น ทั้งตำแหน่งหมาก
 *  กับดัก โล่ และว่าตอนนี้ถึงเทิร์นของใคร
 * ============================================================ */

// ===== กระดาน =====

// ตารางขนาด 5 แถว x 5 คอลัมน์ สำหรับเก็บว่าแต่ละช่องมีหมากอะไร
// ค่าที่เป็นไปได้: 'J' = จั๊ก, 'K' = คิง, 'Q' = ควีน, '.' = ช่องว่าง
char board[5][5];

// ตารางขนาดเดียวกัน สำหรับเก็บว่าช่องนั้น "เป็นของใคร"
// 0 = ไม่มีเจ้าของ (ว่าง)   1 = ผู้เล่น 1   2 = ผู้เล่น 2
int owner[5][5];

// ===== กับดัก =====

// เก็บว่ากับดักอยู่ที่ "แถว" ไหน — [2] หมายถึงมีกับดัก 2 อัน (index 0 และ 1)
int trapRow[2];
// เก็บว่ากับดักอยู่ที่ "คอลัมน์" ไหน
int trapCol[2];
// เก็บว่ากับดักแต่ละอัน "ยังซ่อนอยู่" หรือ "ถูกเหยียบไปแล้ว"
// 1 = ยังอยู่บนกระดาน   0 = ถูกเหยียบแล้ว หายไปแล้ว
int trapActive[2];

// ===== โล่ป้องกัน =====

// เก็บว่าโล่ของผู้เล่นแต่ละคนอยู่ที่ "แถว" ไหน
// ใช้ขนาด [3] เพราะเข้าถึงด้วยหมายเลขผู้เล่น 1 และ 2 ตรงๆ (ช่อง [0] เว้นไว้ไม่ใช้)
int shieldRow[3];
// เก็บว่าโล่อยู่ที่ "คอลัมน์" ไหน
int shieldCol[3];
// เก็บว่าผู้เล่นคนนั้น "เคยใช้โล่แล้วหรือยัง"
// 0 = ยังไม่ได้ใช้เลย   1 = ใช้ไปแล้ว (โล่ใช้ได้คนละครั้งเดียวตลอดเกม)
int shieldUsed[3];

// ===== ตัวแปรเกม =====

// เก็บว่าตอนนี้เป็นเทิร์นของผู้เล่นคนไหน — ตั้งต้นที่ผู้เล่น 1 ก่อนเสมอ
int currentPlayer = 1;

// กล่องรับข้อความที่ผู้เล่นพิมพ์ตอนเลือกหมาก เช่น "J", "K", "Q"
// ขนาด [8] = รับได้สูงสุด 8 ตัวอักษร เพื่อป้องกันการพิมพ์ยาวเกิน
char pieceInput[8];

// กล่องรับข้อความที่ผู้เล่นพิมพ์ตอนเลือกทิศทาง เช่น "W", "A", "S", "D"
char dirInput[8];

/* ============================================================
 *  ส่วนที่ 2 : เครื่องมือพื้นฐาน
 *
 *  ฟังก์ชันเล็กๆ ที่ถูกเรียกใช้ซ้ำหลายที่ตลอดโปรแกรม
 *  เปรียบเหมือนอุปกรณ์ในกล่องเครื่องมือ เช่น
 *  ไขควง ค้อน ที่ใช้กับงานหลายๆ อย่าง
 * ============================================================ */

// ฟังก์ชันล้างข้อความค้างใน "คิว" รับข้อมูล
// ปัญหา: หลัง scanf รับค่า มักมีตัวอักษรเก่าหลงเหลืออยู่
// ถ้าไม่ล้างออก การรับค่าครั้งต่อไปอาจดึงเอาค่าเก่านั้นมาใช้แทน
void clearInputBuffer() {
  int ch; // ตัวแปรชั่วคราวเก็บตัวอักษรที่ดึงออกมา

  // วนดึงตัวอักษรออกมาทิ้งทีละตัว
  // หยุดเมื่อเจอ Enter ('\n') หรือสัญญาณสิ้นสุดข้อมูล (EOF)
  while ((ch = getchar()) != '\n' && ch != EOF)
    ; // ไม่ต้องทำอะไรในลูปนี้ แค่ดึงทิ้งทีละตัวก็พอ
}

// ตรวจสอบว่าตำแหน่ง (row=แถว, col=คอลัมน์) อยู่บนกระดานหรือเปล่า
// คืนค่า 1 = อยู่ในกระดาน   คืนค่า 0 = อยู่นอกกระดาน
int inBoard(int row, int col) {
  // ต้องผ่านทั้ง 4 เงื่อนไขพร้อมกัน:
  // แถวต้องไม่ติดลบ AND แถวต้องน้อยกว่าขนาดกระดาน AND คอลัมน์เช่นกัน
  return (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE);
}

// นับจำนวนหมากที่ผู้เล่นคนนั้นยังเหลืออยู่บนกระดาน
int countPieces(int player) {
  int count = 0; // เริ่มนับจาก 0

  // วนสแกนทุกช่องบนกระดาน จากแถว 0 ไป 4, คอลัมน์ 0 ไป 4
  for (int row = 0; row < BOARD_SIZE; row++)
    for (int col = 0; col < BOARD_SIZE; col++)
      // ถ้าช่องนี้เป็นของผู้เล่นที่ต้องการ ให้นับเพิ่ม 1
      if (owner[row][col] == player)
        count++;

  return count; // ส่งจำนวนที่นับได้กลับออกไป
}

// ตรวจสอบว่าหมาก attacker กินหมาก defender ได้ไหม ตามกฎ K>Q>J>K (เป่ายิ้งฉุบ)
// คืนค่า 1 = กินได้   คืนค่า 0 = กินไม่ได้
int canCapture(char attacker, char defender) {
  if (attacker == 'K' && defender == 'Q') // K กิน Q ได้
    return 1;
  if (attacker == 'Q' && defender == 'J') // Q กิน J ได้
    return 1;
  if (attacker == 'J' && defender == 'K') // J กิน K ได้
    return 1;
  return 0; // คู่อื่นๆ กินกันไม่ได้
}

// ค้นหาหมากชนิด piece ของผู้เล่น player ว่าอยู่ช่องไหนบนกระดาน
// ถ้าเจอ: เขียนตำแหน่งลงใน *row และ *col แล้วคืนค่า 1
// ถ้าไม่เจอ: คืนค่า 0
int findPiece(char piece, int player, int *row, int *col) {
  // วนสแกนทุกช่อง
  for (int i = 0; i < BOARD_SIZE; i++)
    for (int j = 0; j < BOARD_SIZE; j++)
      // ถ้าชนิดหมากตรงกัน และเจ้าของตรงกัน → เจอแล้ว
      if (board[i][j] == piece && owner[i][j] == player) {
        *row = i; // บันทึกแถวที่เจอ กลับผ่าน pointer
        *col = j; // บันทึกคอลัมน์ที่เจอ
        return 1;
      }
  return 0; // วนครบแล้วยังไม่เจอ
}

// ตรวจว่าหมากที่ตำแหน่ง (row, col) ของผู้เล่น player มีโล่ปกป้องอยู่ไหม
// คืนค่า 1 = มีโล่   คืนค่า 0 = ไม่มีโล่
int hasShield(int row, int col, int player) {
  // เปรียบเทียบว่าตำแหน่งที่ถามตรงกับตำแหน่งที่โล่ของผู้เล่นคนนั้นอยู่ไหม
  return (shieldRow[player] == row && shieldCol[player] == col);
}

// เอาโล่ออกจากหมากที่ตำแหน่ง (row, col)
// เรียกใช้ตอนโล่ถูกใช้งาน หรือหมากที่ถือโล่ตาย
void removeShield(int row, int col) {
  // วนเช็คทั้งผู้เล่น 1 และ 2 ว่าใครเป็นเจ้าของโล่ที่ตำแหน่งนี้
  for (int i = 1; i <= 2; i++)
    if (shieldRow[i] == row && shieldCol[i] == col) {
      shieldRow[i] = -1; // รีเซ็ตเป็น -1 = "ไม่มีโล่แล้ว"
      shieldCol[i] = -1; // (พิกัดจริงบนกระดานจะไม่ติดลบ จึงใช้ -1 เป็นสัญญาณ)
    }
}

// ย้ายโล่ตามหมากไปด้วยทุกครั้งที่หมากเดิน
// เพราะโล่ต้องอยู่กับหมากที่รับโล่เสมอ ไม่ใช่ตรึงอยู่กับที่
void moveShield(int fromRow, int fromCol, int toRow, int toCol) {
  // วนเช็คทั้งสองผู้เล่น
  for (int i = 1; i <= 2; i++)
    // ถ้าโล่ของผู้เล่น i อยู่ที่ช่องเดิม (fromRow, fromCol) ของหมาก
    if (shieldRow[i] == fromRow && shieldCol[i] == fromCol) {
      shieldRow[i] = toRow; // อัปเดตให้โล่ตามไปอยู่ที่ช่องใหม่
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

void resolvePush(int fromRow, int fromCol, int toRow, int toCol, int player) {
  char attacker = board[fromRow][fromCol]; // ชนิดหมากที่ถูกผลัก
  char defender = board[toRow][toCol];     // ชนิดหมากที่อยู่ที่ปลายทาง (ถ้ามี)

  if (defender == '.') {
    // ===== ปลายทางว่าง → ย้ายหมากไปเลย ไม่มีการปะทะ =====

    board[toRow][toCol] = attacker;             // วางหมากในช่องใหม่
    owner[toRow][toCol] = player;               // บันทึกว่าช่องใหม่เป็นของใคร
    moveShield(fromRow, fromCol, toRow, toCol); // โล่ตามหมากไปด้วย (ถ้ามี)
    board[fromRow][fromCol] = '.';              // ช่องเดิมกลายเป็นว่าง
    owner[fromRow][fromCol] = 0;                // ช่องเดิมไม่มีเจ้าของแล้ว
    printf("  >> Trap: pushed to (%d,%d)\n", toRow, toCol);

  } else {
    // ===== ปลายทางมีหมากข้าศึก → เกิดการปะทะตามกฎปกติ =====

    int defenderPlayer = owner[toRow][toCol]; // ดึงว่าหมากปลายทางเป็นของผู้เล่นคนไหน
    int attackerShield =
        hasShield(fromRow, fromCol, player); // หมากที่ถูกผลักมีโล่ไหม
    int defenderShield =
        hasShield(toRow, toCol, defenderPlayer); // หมากปลายทางมีโล่ไหม

    printf("  >> Trap: pushed into enemy '%c' at (%d,%d)!\n", defender, toRow,
           toCol);

    if (defenderShield) {
      // --- ฝ่ายรับมีโล่ → โล่ดูดซับ หมากที่ถูกผลักตาย ---
      printf("  *** Defender shielded! Pushed piece is destroyed! ***\n");
      removeShield(toRow, toCol);    // โล่ถูกใช้ไปแล้ว เอาออก
      board[fromRow][fromCol] = '.'; // หมากที่ถูกผลักหายจากกระดาน
      owner[fromRow][fromCol] = 0;

    } else if (canCapture(attacker, defender)) {
      // --- หมากที่ถูกผลักกินหมากปลายทางได้ตามกฎ ---
      printf("  >> '%c' captures '%c'!\n", attacker, defender);
      removeShield(toRow, toCol);     // เอาโล่ฝ่ายรับออก (ถ้ามี)
      board[toRow][toCol] = attacker; // หมากที่ถูกผลักเข้าแทนที่
      owner[toRow][toCol] = player;
      moveShield(fromRow, fromCol, toRow, toCol); // โล่ตามไปด้วย
      board[fromRow][fromCol] = '.';              // ช่องเดิมกลายเป็นว่าง
      owner[fromRow][fromCol] = 0;

    } else if (canCapture(defender, attacker)) {
      // --- หมากปลายทางแข็งแกร่งกว่า หมากที่ถูกผลักควรตาย ---
      if (attackerShield) {
        // แต่มีโล่ → โล่ดูดซับ รอดชีวิต โล่หมด
        printf("  *** Shield saved the pushed piece! ***\n");
        removeShield(fromRow, fromCol); // โล่ถูกใช้ไปแล้ว
      } else {
        // ไม่มีโล่ → ตายตามปกติ
        printf("  >> Pushed piece '%c' was captured!\n", attacker);
        board[fromRow][fromCol] = '.';
        owner[fromRow][fromCol] = 0;
      }

    } else {
      // --- หมากชนิดเดียวกันชนกัน → ผลขึ้นอยู่กับโล่ ---

      if (attackerShield && !defenderShield) {
        // ผู้ถูกผลักมีโล่ ฝ่ายรับไม่มี → ผู้ถูกผลักรอด ฝ่ายรับตาย โล่หมด
        removeShield(fromRow, fromCol); // โล่ถูกใช้ไป
        board[toRow][toCol] = attacker; // หมากที่ถูกผลักครองช่องใหม่
        owner[toRow][toCol] = player;
        board[fromRow][fromCol] = '.'; // ช่องเดิมว่าง
        owner[fromRow][fromCol] = 0;

      } else if (defenderShield && !attackerShield) {
        // ฝ่ายรับมีโล่ ผู้ถูกผลักไม่มี → ฝ่ายรับรอด ผู้ถูกผลักตาย โล่หมด
        removeShield(toRow, toCol);    // โล่ฝ่ายรับถูกใช้ไป
        board[fromRow][fromCol] = '.'; // หมากที่ถูกผลักหายไป
        owner[fromRow][fromCol] = 0;

      } else if (!attackerShield && !defenderShield) {
        // ไม่มีโล่เลยทั้งคู่ → ตายทั้งคู่ หายจากกระดานพร้อมกัน
        printf("  >> Same type collision! Both removed!\n");
        board[fromRow][fromCol] = '.';
        owner[fromRow][fromCol] = 0;
        board[toRow][toCol] = '.';
        owner[toRow][toCol] = 0;

      } else {
        // มีโล่ทั้งคู่ → รอดทั้งคู่ แต่โล่ทั้งสองหมดพร้อมกัน
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

void setupBoard() {
  // ตั้งค่าเมล็ดสุ่มโดยใช้เวลาปัจจุบันของเครื่อง
  // ทำให้ตำแหน่งกับดักต่างกันทุกครั้งที่รันโปรแกรมใหม่
  srand((unsigned int)time(NULL));

  // วนล้างทุกช่องบนกระดานให้ว่างก่อน
  for (int row = 0; row < BOARD_SIZE; row++)
    for (int col = 0; col < BOARD_SIZE; col++) {
      board[row][col] = '.'; // '.' = ช่องว่าง ไม่มีหมาก
      owner[row][col] = 0;   // 0 = ไม่มีเจ้าของ
    }

  // วางหมากผู้เล่น 1 ที่แถวบนสุด (แถว 0) คอลัมน์ 1, 2, 3
  board[0][1] = 'J';
  owner[0][1] = 1;
  board[0][2] = 'K';
  owner[0][2] = 1;
  board[0][3] = 'Q';
  owner[0][3] = 1;

  // วางหมากผู้เล่น 2 ที่แถวล่างสุด (แถว 4) คอลัมน์ 1, 2, 3
  board[4][1] = 'J';
  owner[4][1] = 2;
  board[4][2] = 'K';
  owner[4][2] = 2;
  board[4][3] = 'Q';
  owner[4][3] = 2;

  // รีเซ็ตโล่ทั้งสองผู้เล่น — ตอนเริ่มยังไม่มีใครมีโล่เลย
  for (int i = 1; i <= 2; i++) {
    shieldRow[i] = shieldCol[i] = -1; // -1 = ไม่มีโล่
    shieldUsed[i] = 0;                // 0 = ยังไม่เคยใช้
  }

  // กำหนดว่ากับดักแต่ละอันจะสุ่มอยู่ในช่วงแถวไหน
  // กับดัก [0] สุ่มในแถว 1-2 (โซนกลางค่อนบน)
  // กับดัก [1] สุ่มในแถว 2-3 (โซนกลางค่อนล่าง)
  int range[2][2] = {{1, 2}, {2, 3}};

  for (int i = 0; i < 2; i++) {
    trapActive[i] = 1; // กับดักนี้ยังซ่อนอยู่บนกระดาน

    // วนสุ่มตำแหน่งซ้ำจนกว่าจะได้ตำแหน่งที่ใช้ได้
    do {
      // สุ่มแถวในช่วงที่กำหนด
      trapRow[i] = range[i][0] + rand() % (range[i][1] - range[i][0] + 1);
      // สุ่มคอลัมน์ใดก็ได้ตั้งแต่ 0 ถึง 4
      trapCol[i] = rand() % BOARD_SIZE;
    }
    // สุ่มใหม่ถ้า: ช่องที่ได้ไม่ว่าง หรือ กับดักอันที่ 2 ไปทับกับอันแรก
    while (board[trapRow[i]][trapCol[i]] != '.' ||
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

void printBoard() {
  // แสดงจำนวนหมากที่เหลือของผู้เล่น 1 ด้านบนกระดาน
  printf("\n  *** PLAYER 1 *** (pieces: %d)\n", countPieces(1));

  // พิมพ์ระยะเยื้องก่อน แล้วพิมพ์หมายเลขคอลัมน์ 0-4 ด้านบน
  printf("        ");
  for (int col = 0; col < BOARD_SIZE; col++)
    printf("  %d   ", col); // พิมพ์ 0, 1, 2, 3, 4 เว้นระยะให้ตรงช่อง
  printf("\n");

  // วนพิมพ์ทีละแถว จากแถว 0 (บน) ลงมาถึงแถว 4 (ล่าง)
  for (int row = 0; row < BOARD_SIZE; row++) {

    // พิมพ์เส้นขอบแนวนอน "+-----+-----+..." ก่อนแต่ละแถว
    printf("        ");
    for (int col = 0; col < BOARD_SIZE; col++)
      printf("+-----");
    printf("+\n"); // ปิดเส้นขวาสุด ขึ้นบรรทัดใหม่

    // พิมพ์เลขแถวทางซ้าย แล้วพิมพ์หมากในแต่ละช่อง
    printf("   %d    ", row);

    for (int col = 0; col < BOARD_SIZE; col++) {
      if (board[row][col] == '.')
        printf("|  .  "); // ช่องว่าง
      else {
        if (hasShield(row, col, owner[row][col]))
          printf("|[%c%d] ", board[row][col],
                 owner[row][col]); // มีโล่ → เช่น [K1]
        else
          printf("| %c%d  ", board[row][col],
                 owner[row][col]); // ไม่มีโล่ → เช่น K1
      }
    }
    printf("|\n"); // ปิดขอบขวา ขึ้นบรรทัดใหม่
  }

  // พิมพ์เส้นปิดด้านล่างสุดของกระดาน
  printf("        ");
  for (int col = 0; col < BOARD_SIZE; col++)
    printf("+-----");
  printf("+\n");

  // แสดงจำนวนหมากที่เหลือของผู้เล่น 2 ด้านล่างกระดาน
  printf("  *** PLAYER 2 *** (pieces: %d)\n", countPieces(2));

  // วนเช็คทั้งสองผู้เล่น ถ้ามีโล่อยู่ให้แสดงข้อมูล
  for (int i = 1; i <= 2; i++)
    if (shieldRow[i] != -1) // -1 = ไม่มีโล่ → ข้ามไป
      printf("  [Shield] Player %d: %c%d at (%d,%d) is shielded\n", i,
             board[shieldRow[i]][shieldCol[i]], i, shieldRow[i], shieldCol[i]);
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

void triggerTrap(int row, int col, int player, int index) {
  // แสดงข้อความแจ้งเตือนว่าโดนกับดัก
  printf("\n");
  printf("  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
  printf("  !!  TRAP triggered at (%d,%d)! !!\n", row, col);
  printf("  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

  // กำหนดทิศผลัก: ผู้เล่น 1 เดินลง (+1), ผู้เล่น 2 เดินขึ้น (-1)
  int direction = (player == 1) ? 1 : -1;

  int forwardRow = row + direction;  // แถวข้างหน้า (ทิศที่หมากกำลังเดินมา)
  int backwardRow = row - direction; // แถวข้างหลัง (ทิศตรงข้าม)

  // ลองผลักไปข้างหน้าก่อน
  // เงื่อนไข: อยู่ในกระดาน AND ช่องนั้นไม่ใช่หมากตัวเอง (ว่างหรือข้าศึกก็ได้)
  if (inBoard(forwardRow, col) && owner[forwardRow][col] != player) {
    printf("  >> Trap: pushed FORWARD -> (%d,%d)\n", forwardRow, col);
    resolvePush(row, col, forwardRow, col, player); // ส่งต่อให้จัดการผลการผลัก

    // ข้างหน้าไม่ได้ → ลองผลักถอยหลัง
  } else if (inBoard(backwardRow, col) && owner[backwardRow][col] != player) {
    printf("  >> Trap: pushed BACKWARD -> (%d,%d)\n", backwardRow, col);
    resolvePush(row, col, backwardRow, col, player);

  } else {
    // ทั้งสองทางมีหมากตัวเองขวางอยู่ → ขยับไม่ได้เลย
    printf("  >> Trap: no room to move, piece stays.\n");
  }

  // กับดักถูกเหยียบแล้ว → ปิดการทำงาน (0 = หายไปแล้ว)
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

int playTurn(int player) {
  printBoard(); // แสดงกระดานให้ผู้เล่นดูสถานะปัจจุบันก่อน
  printf("\n=== Player %d's Turn ===\n", player);

  // ===== ถามว่าจะใช้โล่ไหม (เฉพาะคนที่ยังไม่เคยใช้) =====
  if (!shieldUsed[player]) { // ถ้า shieldUsed = 0 (ยังไม่ได้ใช้) จะเข้าบล็อกนี้
    int use = -1;            // ตั้งต้นที่ -1 = ยังไม่ได้ตอบ

    // วนถามซ้ำจนกว่าจะได้คำตอบที่ถูก (0 หรือ 1 เท่านั้น)
    while (use != 0 && use != 1) {
      printf("  [Feature 2] Use Shield this turn? (1=Yes / 0=No): ");

      if (scanf("%d", &use) != 1) // ถ้ารับค่าไม่ได้ (เช่น พิมพ์ตัวอักษร)
        use = -1;                 // รีเซ็ตให้วนถามใหม่

      clearInputBuffer(); // ล้างค่าค้างใน buffer ทุกกรณี

      if (use != 0 && use != 1) // ถ้าได้เลขแต่ไม่ใช่ 0 หรือ 1 เช่น "5"
        printf("  ! Please enter 1 or 0 only.\n");
    }

    if (use == 1) { // ผู้เล่นเลือกจะใช้โล่
      while (1) {   // วนถามจนกว่าจะเลือกหมากได้ถูก หรือยกเลิก
        printf("  Select piece to shield (J / K / Q) or '0' to cancel: ");
        scanf("%s", pieceInput); // รับชื่อหมากจากผู้เล่น

        if (pieceInput[0] == '0') // ถ้าพิมพ์ '0' คือยกเลิก
          break;                  // ออกจาก loop นี้

        char piece = pieceInput[0];       // ดึงตัวอักษรแรกมาใช้
        if (piece >= 'a' && piece <= 'z') // ถ้าเป็นตัวพิมพ์เล็ก
          piece -= 32; // ลบ 32 = แปลงเป็นตัวพิมพ์ใหญ่ (ตามรหัส ASCII)

        int row, col;
        if (!findPiece(piece, player, &row, &col)) { // ถ้าหาหมากนั้นไม่เจอบนกระดาน
          printf("  ! You don't have '%c'\n", piece);
          continue; // วนถามใหม่
        }

        // บันทึกว่าโล่ของผู้เล่นคนนี้ไปอยู่ที่หมากตัวไหน
        shieldRow[player] = row;
        shieldCol[player] = col;
        shieldUsed[player] = 1; // ทำเครื่องหมายว่าใช้โล่แล้ว (ใช้ได้ครั้งเดียว)

        printf("  >> Shield applied to '%c' at (%d,%d)\n", board[row][col], row,
               col);
        printBoard(); // แสดงกระดานใหม่ให้เห็น [K1] ก่อนเดิน
        break;
      }
    }
  }

  // ===== เลือกหมากที่จะเดิน =====
  printf("  Select piece to move (J / K / Q): ");
  scanf("%s", pieceInput); // รับชื่อหมากจากผู้เล่น

  char piece = pieceInput[0]; // ดึงตัวอักษรแรกมาใช้
  if (piece >= 'a' && piece <= 'z')
    piece -= 32; // แปลงเป็นพิมพ์ใหญ่

  int row, col;
  if (!findPiece(piece, player, &row, &col)) { // ถ้าหาหมากไม่เจอ
    printf("  ! You don't have '%c'\n", piece);
    return 0; // คืน 0 = เทิร์นนี้ใช้ไม่ได้ ให้วนถามใหม่
  }

  // แสดงหมากที่เลือก ถ้ามีโล่จะแสดง [** SHIELDED **] ด้วย
  printf("  >> Selected '%c' at (%d,%d)%s\n", board[row][col], row, col,
         hasShield(row, col, player) ? "  [** SHIELDED **]" : "");

  // ===== เลือกทิศทาง WASD =====
  printf("  Direction (W=Up / S=Down / A=Left / D=Right): ");
  scanf("%s", dirInput); // รับทิศทางจากผู้เล่น

  char direction = dirInput[0]; // ดึงตัวอักษรแรก
  if (direction >= 'a' && direction <= 'z')
    direction -= 32; // แปลงเป็นพิมพ์ใหญ่

  // แปลงปุ่มที่กดเป็นค่าเปลี่ยนแถว (dRow) และคอลัมน์ (dCol)
  int dRow = 0, dCol = 0; // เริ่มต้นที่ไม่เปลี่ยนทั้งคู่
  if (direction == 'W')
    dRow = -1; // ขึ้นบน = แถวลดลง 1
  else if (direction == 'S')
    dRow = 1; // ลงล่าง = แถวเพิ่มขึ้น 1
  else if (direction == 'A')
    dCol = -1; // ซ้าย = คอลัมน์ลดลง 1
  else if (direction == 'D')
    dCol = 1; // ขวา = คอลัมน์เพิ่มขึ้น 1
  else {
    printf("  ! Invalid key.\n");
    return 0; // กดปุ่มผิด → ให้เดินใหม่
  }

  // คำนวณตำแหน่งปลายทางที่หมากจะไป
  int newRow = row + dRow;
  int newCol = col + dCol;

  // ตรวจว่าปลายทางอยู่ในกระดานไหม
  if (!inBoard(newRow, newCol)) {
    printf("  ! Out of bounds.\n");
    return 0; // ออกนอกกระดาน → ให้เดินใหม่
  }

  // ตรวจว่าปลายทางไม่ใช่หมากของตัวเอง (ห้ามเดินทับหมากตัวเอง)
  if (owner[newRow][newCol] == player) {
    printf("  ! Cannot move onto your own piece.\n");
    return 0;
  }

  // เช็คว่าปลายทางมีกับดักซ่อนอยู่ไหม
  int hitTrap = -1; // -1 = ไม่เจอกับดัก
  for (int i = 0; i < 2; i++)
    if (trapActive[i] && trapRow[i] == newRow && trapCol[i] == newCol)
      hitTrap = i; // บันทึกว่าเจอกับดักอันที่ i

  char defender = board[newRow][newCol]; // หมากที่อยู่ที่ปลายทาง (ถ้ามี)

  // ===== เดินหมาก / ปะทะ =====
  if (defender == '.' || hitTrap != -1) {
    // ปลายทางว่าง หรือมีกับดักซ่อน → ย้ายหมากไปก่อน
    board[newRow][newCol] = board[row][col]; // วางหมากที่ช่องใหม่
    owner[newRow][newCol] = player;
    moveShield(row, col, newRow, newCol); // โล่ตามหมากไปด้วย
    board[row][col] = '.';                // ช่องเดิมกลายเป็นว่าง
    owner[row][col] = 0;

    if (hitTrap != -1)
      triggerTrap(newRow, newCol, player, hitTrap); // โดนกับดัก → จัดการต่อ
    else
      printf("  >> Moved '%c' to (%d,%d)\n", piece, newRow, newCol);

  } else {
    // ปลายทางมีหมากข้าศึก → เกิดการปะทะ
    int defenderPlayer = owner[newRow][newCol]; // เจ้าของหมากฝ่ายรับ
    int defenderShield =
        hasShield(newRow, newCol, defenderPlayer);    // ฝ่ายรับมีโล่ไหม
    int attackerShield = hasShield(row, col, player); // ฝ่ายโจมตีมีโล่ไหม

    if (defenderShield) {
      // ฝ่ายรับมีโล่ → โล่ดูดซับ ผู้โจมตีตาย
      printf("  *** Defender shielded! Your piece is destroyed! ***\n");
      removeShield(newRow, newCol); // โล่หมด
      board[row][col] = '.';        // หมากโจมตีตาย
      owner[row][col] = 0;

    } else if (canCapture(piece, defender)) {
      // ผู้โจมตีกินฝ่ายรับได้
      printf("  >> '%c' captures '%c'!\n", piece, defender);
      removeShield(newRow, newCol);  // เอาโล่ฝ่ายรับออก (ถ้ามี)
      board[newRow][newCol] = piece; // หมากโจมตีเข้าแทนที่
      owner[newRow][newCol] = player;
      moveShield(row, col, newRow, newCol); // โล่ตามไปด้วย
      board[row][col] = '.';
      owner[row][col] = 0;

    } else if (canCapture(defender, piece)) {
      // ฝ่ายรับกินผู้โจมตีได้
      if (attackerShield) {
        // ผู้โจมตีมีโล่ → รอด โล่หมด
        printf("  *** Shield saved your piece! ***\n");
        removeShield(row, col);
      } else {
        // ไม่มีโล่ → ตาย
        printf("  >> Your '%c' was captured!\n", piece);
        board[row][col] = '.';
        owner[row][col] = 0;
      }

    } else {
      // หมากชนิดเดียวกันชนกัน → ผลขึ้นอยู่กับโล่
      if (attackerShield && !defenderShield) {
        // ผู้โจมตีมีโล่ ฝ่ายรับไม่มี → ผู้โจมตีรอด ฝ่ายรับตาย
        removeShield(row, col);
        board[newRow][newCol] = piece;
        owner[newRow][newCol] = player;
        board[row][col] = '.';
        owner[row][col] = 0;

      } else if (defenderShield && !attackerShield) {
        // ฝ่ายรับมีโล่ ผู้โจมตีไม่มี → ฝ่ายรับรอด ผู้โจมตีตาย
        removeShield(newRow, newCol);
        board[row][col] = '.';
        owner[row][col] = 0;

      } else if (!attackerShield && !defenderShield) {
        // ไม่มีโล่ทั้งคู่ → ตายทั้งคู่
        board[row][col] = '.';
        owner[row][col] = 0;
        board[newRow][newCol] = '.';
        owner[newRow][newCol] = 0;

      } else {
        // มีโล่ทั้งคู่ → รอดทั้งคู่ โล่หมดทั้งสอง
        removeShield(row, col);
        removeShield(newRow, newCol);
      }
    }
  }

  return 1; // เดินสำเร็จ เทิร์นผ่านไปได้
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

int checkGame() {
  int p1 = countPieces(1); // นับหมากที่เหลือของผู้เล่น 1
  int p2 = countPieces(2); // นับหมากที่เหลือของผู้เล่น 2

  if (p1 == 0 && p2 == 0)
    return 3; // ทั้งสองหมดพร้อมกัน → เสมอ
  if (p1 == 0)
    return 2; // ผู้เล่น 1 หมดก่อน → ผู้เล่น 2 ชนะ
  if (p2 == 0)
    return 1; // ผู้เล่น 2 หมดก่อน → ผู้เล่น 1 ชนะ
  return 0;   // ยังมีหมากทั้งสองฝ่าย → เล่นต่อ
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

int main() {
  // แสดงหน้าจอเริ่มต้น
  printf("============================================\n");
  printf("          JKQ Board Game\n");
  printf("       Mak Ruk Yuttha Hatthi\n");
  printf("============================================\n");

  setupBoard(); // จัดโต๊ะ วางหมาก ซ่อนกับดัก ก่อนเริ่มเกม

  while (1) { // วนเกมไปเรื่อยๆ จนกว่าจะมีผู้ชนะหรือเสมอ

    // ถ้า playTurn คืนค่า 0 (เดินผิดกติกา) ให้วนถามใหม่ทันที
    while (!playTurn(currentPlayer))
      ;

    // หลังเดินเสร็จ ตรวจสอบว่าเกมจบหรือยัง
    int result = checkGame();

    if (result == 1) {
      printBoard(); // แสดงกระดานสุดท้าย
      printf("============================================\n");
      printf("         *** Player 1 WINS! ***\n");
      printf("============================================\n");
      break; // ออกจากลูปหลัก เกมจบ
    } else if (result == 2) {
      printBoard();
      printf("============================================\n");
      printf("        *** Player 2 WINS! ***\n");
      printf("============================================\n");
      break;
    } else if (result == 3) {
      printBoard();
      printf("============================================\n");
      printf("            *** DRAW! ***\n");
      printf("============================================\n");
      break;
    }

    // เกมยังไม่จบ → สลับเทิร์น: ถ้าเป็นผู้เล่น 1 ให้เปลี่ยนเป็น 2 และกลับกัน
    currentPlayer = (currentPlayer == 1) ? 2 : 1;
  }

  return 0; // บอกระบบว่าโปรแกรมจบแบบปกติ ไม่มีข้อผิดพลาด
}
