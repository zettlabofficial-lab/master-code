/*
 * ============================================================
 *   JKQ Board Game — Mak Ruk Yuttha Hatthi
 * ============================================================
 *
 *  [กติกาหลัก]
 *   - กระดาน 5x5  ผู้เล่น 2 คน
 *   - หมาก 3 ชนิด : J (จั๊ก), K (คิง), Q (ควีน)
 *   - กินกัน : K กิน Q / Q กิน J / J กิน K  (วนแบบ Rock-Paper-Scissors)
 *   - ชนิดเดียวกันชนกัน → ตายทั้งคู่
 *
 *  [Feature พิเศษ]
 *   1. กับดัก (Trap)   — ซ่อนบนกระดาน 2 อัน เหยียบแล้วถูกผลัก
 *   2. โล่ป้องกัน (Shield) — ใช้ได้คนละ 1 ครั้ง ป้องกันการโจมตีได้ 1 ครั้ง
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BOARD_SIZE 5 // ขนาดกระดาน (5 x 5)

/* ------------------------------------------------------------
 *  SECTION 1 : ตัวแปร (Variables)
 *  — เก็บข้อมูลทุกอย่างของเกมไว้ที่นี่
 * ------------------------------------------------------------ */

char game_board[BOARD_SIZE][BOARD_SIZE]; // กระดาน — แต่ละช่องเก็บตัวหมาก ('J','K','Q') หรือ '.' ถ้าว่าง
int  piece_owner[BOARD_SIZE][BOARD_SIZE]; // เจ้าของแต่ละช่อง (0=ว่าง  1=Player1  2=Player2)

// กับดัก — มี 2 อัน เก็บแถว/คอลัมน์ และสถานะว่ายังอยู่บนกระดานไหม
int trap_row[2]    = {-1, -1}; // แถวของกับดักแต่ละอัน
int trap_col[2]    = {-1, -1}; // คอลัมน์ของกับดักแต่ละอัน
int trap_active[2] = {0, 0};   // 1=ยังซ่อนอยู่  0=หายไปแล้ว

// โล่ป้องกัน — เก็บว่าหมากตัวไหนมีโล่อยู่ (index 1=p1, 2=p2)
int shield_row[3];   // แถวของหมากที่ได้รับโล่
int shield_col[3];   // คอลัมน์ของหมากที่ได้รับโล่
int shield_used[3];  // 1=เคยใช้โล่ไปแล้ว  0=ยังไม่เคยใช้

// ตัวแปรทั่วไป
int  current_player   = 1;  // ตอนนี้ถึงเทิร์นใคร
char piece_input[8];         // รับชื่อหมากจากผู้เล่น เช่น "J" "K" "Q"
char direction_input[8];     // รับทิศทางจากผู้เล่น เช่น "W" "A" "S" "D"

/* ------------------------------------------------------------
 *  SECTION 2 : ประกาศฟังก์ชัน (Function Declarations)
 *  — บอก compiler ล่วงหน้าว่ามีฟังก์ชันอะไรบ้างในโปรแกรม
 * ------------------------------------------------------------ */

void setup_board();                                                          // เตรียมกระดานก่อนเริ่มเกม
void print_board();                                                          // แสดงกระดานบนหน้าจอ
int  count_remaining_pieces(int target_player);                             // นับหมากที่เหลืออยู่ของผู้เล่น
int  can_capture(char attacking_type, char defending_type);                 // เช็คว่าหมาก a กินหมาก b ได้ไหม
int  is_in_bounds(int row, int col);                                        // เช็คว่าตำแหน่งอยู่ในกระดานไหม
int  piece_has_shield(int row, int col, int player);                        // เช็คว่าหมากที่ (row,col) มีโล่ไหม
void remove_shield(int row, int col);                                       // เอาโล่ออกจากตำแหน่ง
void move_shield_with_piece(int from_row, int from_col, int to_row, int to_col); // ย้ายโล่ตามหมากเวลาเดิน
int  search_piece(char piece_name, int player, int *result_row, int *result_col); // ค้นหาหมากบนกระดาน
void trigger_trap(int row, int col, int player, int trap_index);           // จัดการกับดักที่ถูกเหยียบ
int  play_turn(int player);                                                 // จัดการ 1 เทิร์นของผู้เล่น
int  get_game_result();                                                     // เช็คว่าเกมจบหรือยัง
void clear_input_buffer();                                                  // ล้าง input buffer

/* ------------------------------------------------------------
 *  SECTION 3 : เตรียมกระดาน (setup_board)
 *  — ทำครั้งเดียวตอนเริ่มเกม
 *    วางหมากทั้งสองฝ่าย, รีเซ็ตโล่, สุ่มวางกับดัก
 * ------------------------------------------------------------ */

void setup_board() {
  srand((unsigned int)time(NULL)); // ตั้งค่าเมล็ดสำหรับการสุ่ม

  // ล้างทุกช่องบนกระดานให้ว่าง
  int row, col;
  for (row = 0; row < BOARD_SIZE; row++) {
    for (col = 0; col < BOARD_SIZE; col++) {
      game_board[row][col]  = '.'; // '.' หมายถึงช่องว่าง
      piece_owner[row][col] = 0;
    }
  }

  // วางหมาก Player 1 ที่แถวบนสุด (row 0)
  game_board[0][1]  = 'J';
  piece_owner[0][1] = 1;
  game_board[0][2]  = 'K';
  piece_owner[0][2] = 1;
  game_board[0][3]  = 'Q';
  piece_owner[0][3] = 1;

  // วางหมาก Player 2 ที่แถวล่างสุด (row 4)
  game_board[4][1]  = 'J';
  piece_owner[4][1] = 2;
  game_board[4][2]  = 'K';
  piece_owner[4][2] = 2;
  game_board[4][3]  = 'Q';
  piece_owner[4][3] = 2;

  // รีเซ็ตโล่ — ยังไม่มีใครได้โล่ตอนเริ่มเกม
  shield_row[1] = shield_col[1] = -1;
  shield_row[2] = shield_col[2] = -1;
  shield_used[1] = shield_used[2] = 0;

  // สุ่มวางกับดัก 2 อัน
  // อันที่ 0 อยู่ในแถว 1-2 (ใกล้ฝั่ง Player 1)
  // อันที่ 1 อยู่ในแถว 2-3 (ใกล้ฝั่ง Player 2)
  int row_ranges[2][2] = {{1, 2}, {2, 3}};
  int trap_index;
  for (trap_index = 0; trap_index < 2; trap_index++) {
    trap_active[trap_index] = 1;
    do {
      trap_row[trap_index] = row_ranges[trap_index][0]
                           + rand() % (row_ranges[trap_index][1] - row_ranges[trap_index][0] + 1);
      trap_col[trap_index] = rand() % BOARD_SIZE;
    } while (game_board[trap_row[trap_index]][trap_col[trap_index]] != '.' || // ต้องเป็นช่องว่าง
             (trap_index == 1 && trap_row[1] == trap_row[0]
              && trap_col[1] == trap_col[0])); // ห้ามซ้อนกัน
  }

  printf("[Feature 1] 2 traps have been secretly placed!\n");
}

/* ------------------------------------------------------------
 *  SECTION 4 : แสดงกระดาน (print_board)
 *  — วาดกระดานบนหน้าจอ พร้อมโชว์จำนวนหมากและสถานะโล่
 *  — หมากที่มีโล่จะแสดงเป็น [J1] แทน J1
 * ------------------------------------------------------------ */

void print_board() {
  printf("\n  *** PLAYER 1 *** (pieces: %d)\n", count_remaining_pieces(1));

  // พิมพ์หมายเลขคอลัมน์ด้านบน
  printf("        ");
  int col;
  for (col = 0; col < BOARD_SIZE; col++)
    printf("  %d   ", col);
  printf("\n");

  // พิมพ์แต่ละแถวของกระดาน
  int row;
  for (row = 0; row < BOARD_SIZE; row++) {
    // เส้นแบ่งแนวนอน
    printf("        ");
    for (col = 0; col < BOARD_SIZE; col++)
      printf("+-----");
    printf("+\n");

    // แถวหมาก — แสดงเลขแถวด้านซ้าย
    printf("   %d    ", row);
    for (col = 0; col < BOARD_SIZE; col++) {
      if (game_board[row][col] == '.') {
        printf("|  .  "); // ช่องว่าง
      } else {
        if (piece_has_shield(row, col, piece_owner[row][col]))
          printf("|[%c%d] ", game_board[row][col], piece_owner[row][col]); // หมากมีโล่ เช่น [K1]
        else
          printf("| %c%d  ", game_board[row][col], piece_owner[row][col]); // หมากปกติ เช่น K1
      }
    }
    printf("|\n");
  }

  // เส้นปิดด้านล่าง
  printf("        ");
  for (col = 0; col < BOARD_SIZE; col++)
    printf("+-----");
  printf("+\n");

  printf("  *** PLAYER 2 *** (pieces: %d)\n", count_remaining_pieces(2));

  // แสดงสถานะโล่ถ้ามีอยู่
  int player_num;
  for (player_num = 1; player_num <= 2; player_num++) {
    if (shield_row[player_num] != -1)
      printf("  [Shield] Player %d: %c%d at (%d,%d) is shielded\n",
             player_num,
             game_board[shield_row[player_num]][shield_col[player_num]],
             player_num, shield_row[player_num], shield_col[player_num]);
  }
}

/* ------------------------------------------------------------
 *  SECTION 5 : ฟังก์ชันช่วย (Helper Functions)
 *  — ฟังก์ชันเล็กๆ ที่ถูกเรียกใช้ซ้ำหลายที่
 * ------------------------------------------------------------ */

// นับหมากที่เหลืออยู่ของผู้เล่น
int count_remaining_pieces(int target_player) {
  int count = 0, row, col;
  for (row = 0; row < BOARD_SIZE; row++)
    for (col = 0; col < BOARD_SIZE; col++)
      if (piece_owner[row][col] == target_player)
        count++;
  return count;
}

// เช็คว่าหมาก attacking_type กินหมาก defending_type ได้ไหม (K>Q, Q>J, J>K)
int can_capture(char attacking_type, char defending_type) {
  if (attacking_type == 'K' && defending_type == 'Q')
    return 1;
  if (attacking_type == 'Q' && defending_type == 'J')
    return 1;
  if (attacking_type == 'J' && defending_type == 'K')
    return 1;
  return 0;
}

// เช็คว่า (row, col) อยู่ในกระดานไหม
int is_in_bounds(int row, int col) {
  return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

// เช็คว่าหมากที่ (row, col) ของผู้เล่น player มีโล่ปกป้องอยู่ไหม
int piece_has_shield(int row, int col, int player) {
  return (shield_row[player] == row && shield_col[player] == col);
}

// เอาโล่ออกจากตำแหน่ง (row, col) — ใช้ตอนโล่ถูกใช้ไปแล้ว
void remove_shield(int row, int col) {
  int player_index;
  for (player_index = 1; player_index <= 2; player_index++)
    if (shield_row[player_index] == row && shield_col[player_index] == col)
      shield_row[player_index] = shield_col[player_index] = -1;
}

// อัปเดตตำแหน่งโล่เมื่อหมากย้ายจาก (from_row, from_col) ไป (to_row, to_col)
void move_shield_with_piece(int from_row, int from_col, int to_row, int to_col) {
  int player_index;
  for (player_index = 1; player_index <= 2; player_index++)
    if (shield_row[player_index] == from_row && shield_col[player_index] == from_col) {
      shield_row[player_index] = to_row;
      shield_col[player_index] = to_col;
    }
}

// ค้นหาหมากชื่อ piece_name ของผู้เล่น player บนกระดาน
// คืน 1 ถ้าเจอ และใส่ตำแหน่งกลับใน result_row, result_col
// คืน 0 ถ้าหาไม่เจอ
int search_piece(char piece_name, int player, int *result_row, int *result_col) {
  int row, col;
  for (row = 0; row < BOARD_SIZE; row++)
    for (col = 0; col < BOARD_SIZE; col++)
      if (piece_owner[row][col] == player && game_board[row][col] == piece_name) {
        *result_row = row;
        *result_col = col;
        return 1;
      }
  return 0;
}

// ล้าง input buffer — ป้องกันตัวอักษรเก่าค้างอยู่ใน buffer
void clear_input_buffer() {
  int char_read;
  while ((char_read = getchar()) != '\n' && char_read != EOF)
    ;
}

/* ------------------------------------------------------------
 *  SECTION 6 : กับดัก (trigger_trap)
 *  — เรียกใช้เมื่อหมากเหยียบช่องที่มีกับดักซ่อนอยู่
 *  — ผลัก : ไปข้างหน้า 1 ช่อง → ถ้าไม่ได้ก็ถอยหลัง 1 ช่อง
 *  — กับดักหายไปหลังถูกเหยียบ
 * ------------------------------------------------------------ */

void trigger_trap(int row, int col, int player, int trap_index) {
  printf("\n");
  printf("  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
  printf("  !!  TRAP triggered at (%d,%d)! !!\n", row, col);
  printf("  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

  // Player 1 เดินลง (push_direction=+1), Player 2 เดินขึ้น (push_direction=-1)
  int push_direction = (player == 1) ? 1 : -1;
  int forward_row    = row + push_direction; // ช่องข้างหน้า
  int backward_row   = row - push_direction; // ช่องข้างหลัง

  if (is_in_bounds(forward_row, col) && game_board[forward_row][col] == '.') {
    // ผลักไปข้างหน้าได้
    printf("  >> Trap: pushed FORWARD -> (%d,%d)\n", forward_row, col);
    game_board[forward_row][col]  = game_board[row][col];
    piece_owner[forward_row][col] = piece_owner[row][col];
    move_shield_with_piece(row, col, forward_row, col);
    game_board[row][col]  = '.';
    piece_owner[row][col] = 0;
  } else if (is_in_bounds(backward_row, col) && game_board[backward_row][col] == '.') {
    // ข้างหน้าไม่ว่าง → ผลักถอยหลังแทน
    printf("  >> Trap: pushed BACKWARD -> (%d,%d)\n", backward_row, col);
    game_board[backward_row][col]  = game_board[row][col];
    piece_owner[backward_row][col] = piece_owner[row][col];
    move_shield_with_piece(row, col, backward_row, col);
    game_board[row][col]  = '.';
    piece_owner[row][col] = 0;
  } else {
    // ทั้งสองทางมีคนอยู่ → อยู่เดิม
    printf("  >> Trap: no room to move, piece stays.\n");
  }

  trap_active[trap_index] = 0; // กับดักหายไปแล้ว
  printf("  >> Trap is now gone.\n");
}

/* ------------------------------------------------------------
 *  SECTION 7 : จัดการ 1 เทิร์น (play_turn)
 *  — ฟังก์ชันหลักของแต่ละรอบ ทำงานตามขั้นตอนนี้:
 *    1) แสดงกระดาน       → ผู้เล่นเห็นสถานะปัจจุบัน
 *    2) ถามใช้โล่ไหม     → ถ้าตอบ Yes ให้เลือกหมากที่จะสวมโล่
 *    3) เลือกหมากที่จะเดิน (J / K / Q)
 *    4) เลือกทิศทาง      (W=ขึ้น / S=ลง / A=ซ้าย / D=ขวา)
 *    5) ตรวจกติกา + เดิน
 *    6) ถ้าไปเจอกับดัก   → ทำการผลัก
 *    7) ถ้าไปชนหมาก      → ตัดสินผลการปะทะ
 *  — คืน 1 ถ้าเดินสำเร็จ, คืน 0 ถ้าเดินผิดพลาด (จะถามใหม่)
 * ------------------------------------------------------------ */

int play_turn(int player) {
  print_board();
  printf("\n=== Player %d's Turn ===\n", player);

  /* ---- ขั้นที่ 1 : ถามใช้โล่ไหม (ถ้ายังไม่เคยใช้) ---- */
  if (!shield_used[player]) {
    int want_shield = -1;
    while (want_shield != 0 && want_shield != 1) {
      printf("  [Feature 2] Use Shield this turn? (1=Yes / 0=No): ");
      if (scanf("%d", &want_shield) != 1) {
        clear_input_buffer();
        want_shield = -1;
        continue;
      }
      clear_input_buffer();
      if (want_shield != 0 && want_shield != 1)
        printf("  ! Please enter 1 or 0.\n");
    }

    if (want_shield == 1) {
      // วนถามจนกว่าจะพิมชื่อหมากถูก หรือกด 0 เพื่อยกเลิก
      while (1) {
        printf("  Select piece to shield (J / K / Q) or '0' to cancel: ");
        char shield_input[8];
        if (scanf("%7s", shield_input) != 1) {
          clear_input_buffer();
          continue;
        }
        clear_input_buffer();

        if (shield_input[0] == '0') {
          printf("  Shield cancelled.\n");
          break;
        }

        // แปลง lowercase -> uppercase เผื่อผู้เล่นพิมพ์ตัวเล็ก
        char shield_piece = shield_input[0];
        if (shield_piece >= 'a' && shield_piece <= 'z')
          shield_piece = shield_piece - 32;

        if (shield_piece != 'J' && shield_piece != 'K' && shield_piece != 'Q') {
          printf("  ! Use J, K, or Q only.\n");
          continue;
        }

        int shield_piece_row, shield_piece_col;
        if (!search_piece(shield_piece, player, &shield_piece_row, &shield_piece_col)) {
          printf("  ! You don't have '%c' on the board.\n", shield_piece);
          continue;
        }

        // ใส่โล่ให้หมากตัวนั้น
        shield_row[player]  = shield_piece_row;
        shield_col[player]  = shield_piece_col;
        shield_used[player] = 1;
        printf("  >> Shield applied to '%c' at (%d,%d)\n",
               game_board[shield_piece_row][shield_piece_col],
               shield_piece_row, shield_piece_col);
        print_board(); // refresh กระดานให้เห็น [ ] ทันทีก่อนเดิน
        break;
      }
    }
  }

  /* ---- ขั้นที่ 2 : เลือกหมากที่จะเดิน ---- */
  printf("  Select piece to move (J / K / Q): ");
  if (scanf("%7s", piece_input) != 1) {
    clear_input_buffer();
    printf("  ! Invalid input.\n");
    return 0;
  }
  clear_input_buffer();

  // แปลง lowercase -> uppercase
  char move_piece = piece_input[0];
  if (move_piece >= 'a' && move_piece <= 'z')
    move_piece = move_piece - 32;

  if (move_piece != 'J' && move_piece != 'K' && move_piece != 'Q') {
    printf("  ! Use J, K, or Q only.\n");
    return 0;
  }

  int from_row, from_col;
  if (!search_piece(move_piece, player, &from_row, &from_col)) {
    printf("  ! You don't have '%c' on the board.\n", move_piece);
    return 0;
  }

  // แสดงหมากที่เลือก พร้อมบอกถ้ามีโล่อยู่
  printf("  >> Selected '%c' at (%d,%d)%s\n",
         game_board[from_row][from_col], from_row, from_col,
         piece_has_shield(from_row, from_col, player) ? "  [** SHIELDED **]" : "");

  /* ---- ขั้นที่ 3 : รับทิศทาง WASD ---- */
  printf("  Direction (W=Up / S=Down / A=Left / D=Right): ");
  if (scanf("%7s", direction_input) != 1) {
    clear_input_buffer();
    return 0;
  }
  clear_input_buffer();

  char direction_key = direction_input[0];
  if (direction_key >= 'a' && direction_key <= 'z')
    direction_key = direction_key - 32;

  // แปลงทิศทางเป็นค่า row_change, col_change สำหรับบวกกับตำแหน่ง
  int row_change = 0, col_change = 0;
  if (direction_key == 'W')
    row_change = -1; // ขึ้น → แถวลดลง
  else if (direction_key == 'S')
    row_change = 1;  // ลง  → แถวเพิ่มขึ้น
  else if (direction_key == 'A')
    col_change = -1; // ซ้าย → คอลัมน์ลดลง
  else if (direction_key == 'D')
    col_change = 1;  // ขวา → คอลัมน์เพิ่มขึ้น
  else {
    printf("  ! Invalid key. Use W/A/S/D.\n");
    return 0;
  }

  // คำนวณตำแหน่งปลายทาง
  int to_row = from_row + row_change;
  int to_col = from_col + col_change;

  /* ---- ขั้นที่ 4 : ตรวจกติกาก่อนเดิน ---- */

  // ห้ามออกนอกกระดาน
  if (!is_in_bounds(to_row, to_col)) {
    printf("  ! Out of bounds.\n");
    return 0;
  }

  // ห้ามทับหมากตัวเอง
  if (piece_owner[to_row][to_col] == player) {
    printf("  ! Cannot move onto your own piece.\n");
    return 0;
  }

  char attacking_piece = game_board[from_row][from_col]; // ตัวที่กำลังเดิน
  char defending_piece = game_board[to_row][to_col];     // ตัวที่อยู่ที่ปลายทาง (ถ้ามี)

  // เช็คว่าปลายทางมีกับดักซ่อนอยู่ไหม
  int hit_trap_index = -1;
  int trap_index;
  for (trap_index = 0; trap_index < 2; trap_index++)
    if (trap_active[trap_index] && to_row == trap_row[trap_index] && to_col == trap_col[trap_index])
      hit_trap_index = trap_index;

  /* ---- ขั้นที่ 5 : เดิน ---- */

  if (defending_piece == '.' || hit_trap_index >= 0) {
    // ช่องว่าง (หรือมีกับดักซ่อน) → ย้ายหมากไป
    game_board[to_row][to_col]  = attacking_piece;
    piece_owner[to_row][to_col] = player;
    move_shield_with_piece(from_row, from_col, to_row, to_col); // โล่ติดตามหมากไปด้วย
    game_board[from_row][from_col]  = '.';
    piece_owner[from_row][from_col] = 0;

    if (hit_trap_index >= 0)
      trigger_trap(to_row, to_col, player, hit_trap_index); // โดนกับดัก!
    else
      printf("  >> Moved '%c' to (%d,%d)\n", attacking_piece, to_row, to_col);

  } else {
    /* ---- ขั้นที่ 6 : ปะทะกัน ---- */
    int defending_player   = piece_owner[to_row][to_col];
    int defender_has_shield = piece_has_shield(to_row, to_col, defending_player); // ฝ่ายตั้งรับมีโล่ไหม
    int attacker_has_shield = piece_has_shield(from_row, from_col, player);       // ฝ่ายโจมตีมีโล่ไหม

    if (defender_has_shield && defending_player != player) {
      // ---- กรณี: ฝ่ายตั้งรับมีโล่ → โล่ดูดซับการโจมตี ผู้โจมตีตาย ----
      printf("  *** Player %d's '%c' at (%d,%d) is SHIELDED! "
             "Shield absorbs the attack — your '%c' is destroyed! ***\n",
             defending_player, defending_piece, to_row, to_col, attacking_piece);
      remove_shield(to_row, to_col); // โล่ถูกใช้หมดแล้ว
      game_board[from_row][from_col]  = '.';
      piece_owner[from_row][from_col] = 0;
      printf("  (Shield used up. '%c' (Player %d) remains at (%d,%d).)\n",
             defending_piece, defending_player, to_row, to_col);

    } else if (can_capture(attacking_piece, defending_piece)) {
      // ---- กรณี: ผู้โจมตีกินฝ่ายตั้งรับได้ ----
      printf("  >> '%c' captures '%c' from Player %d!\n",
             attacking_piece, defending_piece, defending_player);
      remove_shield(to_row, to_col);
      game_board[to_row][to_col]  = attacking_piece;
      piece_owner[to_row][to_col] = player;
      move_shield_with_piece(from_row, from_col, to_row, to_col);
      game_board[from_row][from_col]  = '.';
      piece_owner[from_row][from_col] = 0;

    } else if (can_capture(defending_piece, attacking_piece)) {
      // ---- กรณี: ฝ่ายตั้งรับกินผู้โจมตีได้ ----
      if (attacker_has_shield) {
        // แต่ผู้โจมตีมีโล่ → รอดชีวิต โล่หมด
        printf("  *** Your '%c' had a SHIELD and survived! Shield used up. ***\n",
               attacking_piece);
        remove_shield(from_row, from_col);
      } else {
        // ไม่มีโล่ → ตายตามปกติ
        printf("  >> Your '%c' was captured by '%c' from Player %d!\n",
               attacking_piece, defending_piece, defending_player);
        game_board[from_row][from_col]  = '.';
        piece_owner[from_row][from_col] = 0;
      }

    } else {
      // ---- กรณี: หมากชนิดเดียวกันชนกัน ----
      if (attacker_has_shield && !defender_has_shield) {
        // ผู้โจมตีมีโล่ → รอด ฝ่ายตรงข้ามตาย
        printf("  >> Same type! '%c' (Player %d) had a shield and SURVIVED! "
               "'%c' (Player %d) is removed!\n",
               attacking_piece, player, defending_piece, defending_player);
        remove_shield(from_row, from_col);
        game_board[to_row][to_col]      = attacking_piece;
        piece_owner[to_row][to_col]     = player;
        game_board[from_row][from_col]  = '.';
        piece_owner[from_row][from_col] = 0;

      } else if (defender_has_shield && !attacker_has_shield) {
        // ฝ่ายตั้งรับมีโล่ → รอด ผู้โจมตีตาย
        printf("  >> Same type! '%c' (Player %d) had a shield and SURVIVED! "
               "'%c' (Player %d) is removed!\n",
               defending_piece, defending_player, attacking_piece, player);
        remove_shield(to_row, to_col);
        game_board[from_row][from_col]  = '.';
        piece_owner[from_row][from_col] = 0;

      } else if (!attacker_has_shield && !defender_has_shield) {
        // ไม่มีโล่เลย → ตายทั้งคู่
        printf("  >> '%c' vs '%c' - same type! Both removed!\n",
               attacking_piece, defending_piece);
        game_board[from_row][from_col]  = '.';
        piece_owner[from_row][from_col] = 0;
        game_board[to_row][to_col]      = '.';
        piece_owner[to_row][to_col]     = 0;

      } else {
        // มีโล่ทั้งคู่ → รอดทั้งคู่ โล่ทั้งสองหมด
        printf("  >> Same type! Both had shields - BOTH SURVIVED! Shields used up.\n");
        remove_shield(from_row, from_col);
        remove_shield(to_row, to_col);
      }
    }
  }

  return 1; // เดินสำเร็จ
}

/* ------------------------------------------------------------
 *  SECTION 8 : เช็คผู้ชนะ (get_game_result)
 *  — นับหมากที่เหลือของแต่ละฝั่ง
 *  — คืน 0 = ยังเล่นอยู่, 1 = P1 ชนะ, 2 = P2 ชนะ, 3 = เสมอ
 * ------------------------------------------------------------ */

int get_game_result() {
  int player1_count = count_remaining_pieces(1);
  int player2_count = count_remaining_pieces(2);

  if (player1_count == 0 && player2_count == 0)
    return 3; // หมดพร้อมกัน → เสมอ
  if (player1_count == 0)
    return 2; // Player 1 หมดหมาก → Player 2 ชนะ
  if (player2_count == 0)
    return 1; // Player 2 หมดหมาก → Player 1 ชนะ
  return 0;   // ยังมีหมากทั้งสองฝ่าย → เล่นต่อ
}

/* ------------------------------------------------------------
 *  SECTION 9 : main — จุดเริ่มต้นของโปรแกรม
 *  — แสดง Title → เริ่มเกม → วนเทิร์น → เช็คผล → สลับผู้เล่น
 * ------------------------------------------------------------ */

int main() {
  printf("============================================\n");
  printf("          JKQ Board Game\n");
  printf("       Mak Ruk Yuttha Hatthi\n");
  printf("============================================\n");

  setup_board(); // เตรียมกระดานก่อนเริ่ม

  while (1) {
    // วนถามซ้ำถ้าผู้เล่นเดินผิดกติกา (return 0)
    while (!play_turn(current_player))
      ;

    // หลังเดินแล้วเช็คว่าเกมจบหรือยัง
    int game_result = get_game_result();

    if (game_result == 1) {
      print_board();
      printf("=========================================================\n");
      printf("\n*** Player 1 WINS! All opponent pieces eliminated! ***\n");
      printf("\n=========================================================\n");
      break;
    } else if (game_result == 2) {
      print_board();
      printf("=========================================================\n");
      printf("\n*** Player 2 WINS! All opponent pieces eliminated! ***\n");
      printf("\n=========================================================\n");
      break;
    } else if (game_result == 3) {
      print_board();
      printf("=========================================================\n");
      printf("\n*** DRAW! Both players lost all pieces simultaneously! ***\n");
      printf("\n=========================================================\n");
      break;
    }

    // สลับเทิร์น
    if (current_player == 1)
      current_player = 2;
    else
      current_player = 1;
  }

  printf("============================================\n");
  printf("\n          Thanks for playing!\n");
  printf("\n============================================\n");
  return 0;
}