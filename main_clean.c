#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 5

char board[SIZE][SIZE];
int owner[SIZE][SIZE];

int trap_r[2] = {-1, -1};
int trap_c[2] = {-1, -1};
int trap_on[2] = {0, 0};

int shield_r[3];
int shield_c[3];
int used_shield[3];

int current_player = 1;
char piece_name[8];
char dir_input[8];

void init_board();
void show_board();
int count_pieces(int p);
int can_eat(char a, char b);
int in_bounds(int r, int c);
int has_shield_at(int r, int c, int p);
void remove_shield_at(int r, int c);
void move_shield(int r1, int c1, int r2, int c2);
int find_piece(char name, int player, int *out_r, int *out_c);
void do_trap(int r, int c, int player, int t);
int do_turn(int player);
int check_winner();
void clear_buf();

void init_board() {
  srand((unsigned int)time(NULL));

  int i, j;
  for (i = 0; i < SIZE; i++) {
    for (j = 0; j < SIZE; j++) {
      board[i][j] = '.';
      owner[i][j] = 0;
    }
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

  shield_r[1] = shield_c[1] = -1;
  shield_r[2] = shield_c[2] = -1;
  used_shield[1] = used_shield[2] = 0;

  int ranges[2][2] = {{1, 2}, {2, 3}};
  int t;
  for (t = 0; t < 2; t++) {
    trap_on[t] = 1;
    do {
      trap_r[t] = ranges[t][0] + rand() % (ranges[t][1] - ranges[t][0] + 1);
      trap_c[t] = rand() % SIZE;
    } while (board[trap_r[t]][trap_c[t]] != '.' ||
             (t == 1 && trap_r[1] == trap_r[0] && trap_c[1] == trap_c[0]));
  }

  printf("[Feature 1] 2 traps have been secretly placed!\n");
}

void show_board() {
  printf("\n  *** PLAYER 1 *** (pieces: %d)\n", count_pieces(1));

  printf("        ");
  int j;
  for (j = 0; j < SIZE; j++)
    printf("  %d   ", j);
  printf("\n");

  int i;
  for (i = 0; i < SIZE; i++) {
    printf("        ");
    for (j = 0; j < SIZE; j++)
      printf("+-----");
    printf("+\n");

    printf("   %d    ", i);
    for (j = 0; j < SIZE; j++) {
      if (board[i][j] == '.') {
        printf("|  .  ");
      } else {
        if (has_shield_at(i, j, owner[i][j]))
          printf("|[%c%d] ", board[i][j], owner[i][j]);
        else
          printf("| %c%d  ", board[i][j], owner[i][j]);
      }
    }
    printf("|\n");
  }

  printf("        ");
  for (j = 0; j < SIZE; j++)
    printf("+-----");
  printf("+\n");

  printf("  *** PLAYER 2 *** (pieces: %d)\n", count_pieces(2));

  int p;
  for (p = 1; p <= 2; p++) {
    if (shield_r[p] != -1)
      printf("  [Shield] Player %d: %c%d at (%d,%d) is shielded\n", p,
             board[shield_r[p]][shield_c[p]], p, shield_r[p], shield_c[p]);
  }
}

int count_pieces(int p) {
  int n = 0, i, j;
  for (i = 0; i < SIZE; i++)
    for (j = 0; j < SIZE; j++)
      if (owner[i][j] == p)
        n++;
  return n;
}

int can_eat(char a, char b) {
  if (a == 'K' && b == 'Q')
    return 1;
  if (a == 'Q' && b == 'J')
    return 1;
  if (a == 'J' && b == 'K')
    return 1;
  return 0;
}

int in_bounds(int r, int c) { return r >= 0 && r < SIZE && c >= 0 && c < SIZE; }

int has_shield_at(int r, int c, int p) {
  return (shield_r[p] == r && shield_c[p] == c);
}

void remove_shield_at(int r, int c) {
  int p;
  for (p = 1; p <= 2; p++)
    if (shield_r[p] == r && shield_c[p] == c)
      shield_r[p] = shield_c[p] = -1;
}

void move_shield(int r1, int c1, int r2, int c2) {
  int p;
  for (p = 1; p <= 2; p++)
    if (shield_r[p] == r1 && shield_c[p] == c1) {
      shield_r[p] = r2;
      shield_c[p] = c2;
    }
}

int find_piece(char name, int player, int *out_r, int *out_c) {
  int i, j;
  for (i = 0; i < SIZE; i++)
    for (j = 0; j < SIZE; j++)
      if (owner[i][j] == player && board[i][j] == name) {
        *out_r = i;
        *out_c = j;
        return 1;
      }
  return 0;
}

void clear_buf() {
  int c;
  while ((c = getchar()) != '\n' && c != EOF)
    ;
}

void do_trap(int r, int c, int player, int t) {
  printf("\n");
  printf("  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
  printf("  !!  TRAP triggered at (%d,%d)! !!\n", r, c);
  printf("  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

  int dr = (player == 1) ? 1 : -1;
  int fwd_r = r + dr;
  int bk_r = r - dr;

  if (in_bounds(fwd_r, c) && board[fwd_r][c] == '.') {
    printf("  >> Trap: pushed FORWARD -> (%d,%d)\n", fwd_r, c);
    board[fwd_r][c] = board[r][c];
    owner[fwd_r][c] = owner[r][c];
    move_shield(r, c, fwd_r, c);
    board[r][c] = '.';
    owner[r][c] = 0;
  } else if (in_bounds(bk_r, c) && board[bk_r][c] == '.') {
    printf("  >> Trap: pushed BACKWARD -> (%d,%d)\n", bk_r, c);
    board[bk_r][c] = board[r][c];
    owner[bk_r][c] = owner[r][c];
    move_shield(r, c, bk_r, c);
    board[r][c] = '.';
    owner[r][c] = 0;
  } else {
    printf("  >> Trap: no room to move, piece stays.\n");
  }

  trap_on[t] = 0;
  printf("  >> Trap is now gone.\n");
}

int do_turn(int player) {
  show_board();
  printf("\n=== Player %d's Turn ===\n", player);

  if (!used_shield[player]) {
    int use = -1;
    while (use != 0 && use != 1) {
      printf("  [Feature 2] Use Shield this turn? (1=Yes / 0=No): ");
      if (scanf("%d", &use) != 1) {
        clear_buf();
        use = -1;
        continue;
      }
      clear_buf();
      if (use != 0 && use != 1)
        printf("  ! Please enter 1 or 0.\n");
    }

    if (use == 1) {
      while (1) {
        printf("  Select piece to shield (J / K / Q) or '0' to cancel: ");
        char sname[8];
        if (scanf("%7s", sname) != 1) {
          clear_buf();
          continue;
        }
        clear_buf();

        if (sname[0] == '0') {
          printf("  Shield cancelled.\n");
          break;
        }

        char skey = sname[0];
        if (skey >= 'a' && skey <= 'z')
          skey = skey - 32;

        if (skey != 'J' && skey != 'K' && skey != 'Q') {
          printf("  ! Use J, K, or Q only.\n");
          continue;
        }

        int sr, sc;
        if (!find_piece(skey, player, &sr, &sc)) {
          printf("  ! You don't have '%c' on the board.\n", skey);
          continue;
        }

        shield_r[player] = sr;
        shield_c[player] = sc;
        used_shield[player] = 1;
        printf("  >> Shield applied to '%c' at (%d,%d)\n", board[sr][sc], sr,
               sc);
        show_board();
        break;
      }
    }
  }

  printf("  Select piece to move (J / K / Q): ");
  if (scanf("%7s", piece_name) != 1) {
    clear_buf();
    printf("  ! Invalid input.\n");
    return 0;
  }
  clear_buf();

  char mkey = piece_name[0];
  if (mkey >= 'a' && mkey <= 'z')
    mkey = mkey - 32;

  if (mkey != 'J' && mkey != 'K' && mkey != 'Q') {
    printf("  ! Use J, K, or Q only.\n");
    return 0;
  }

  int r1, c1;
  if (!find_piece(mkey, player, &r1, &c1)) {
    printf("  ! You don't have '%c' on the board.\n", mkey);
    return 0;
  }

  printf("  >> Selected '%c' at (%d,%d)%s\n", board[r1][c1], r1, c1,
         has_shield_at(r1, c1, player) ? "  [** SHIELDED **]" : "");

  printf("  Direction (W=Up / S=Down / A=Left / D=Right): ");
  if (scanf("%7s", dir_input) != 1) {
    clear_buf();
    return 0;
  }
  clear_buf();

  char key = dir_input[0];
  if (key >= 'a' && key <= 'z')
    key = key - 32;

  int dr = 0, dc = 0;
  if (key == 'W')
    dr = -1;
  else if (key == 'S')
    dr = 1;
  else if (key == 'A')
    dc = -1;
  else if (key == 'D')
    dc = 1;
  else {
    printf("  ! Invalid key. Use W/A/S/D.\n");
    return 0;
  }

  int r2 = r1 + dr;
  int c2 = c1 + dc;

  if (!in_bounds(r2, c2)) {
    printf("  ! Out of bounds.\n");
    return 0;
  }

  if (owner[r2][c2] == player) {
    printf("  ! Cannot move onto your own piece.\n");
    return 0;
  }

  char attacker = board[r1][c1];
  char defender = board[r2][c2];

  int is_trap = -1;
  int t;
  for (t = 0; t < 2; t++)
    if (trap_on[t] && r2 == trap_r[t] && c2 == trap_c[t])
      is_trap = t;

  if (defender == '.' || is_trap >= 0) {
    board[r2][c2] = attacker;
    owner[r2][c2] = player;
    move_shield(r1, c1, r2, c2);
    board[r1][c1] = '.';
    owner[r1][c1] = 0;

    if (is_trap >= 0)
      do_trap(r2, c2, player, is_trap);
    else
      printf("  >> Moved '%c' to (%d,%d)\n", attacker, r2, c2);

  } else {
    int def_player = owner[r2][c2];
    int def_has_shield = has_shield_at(r2, c2, def_player);
    int atk_has_shield = has_shield_at(r1, c1, player);

    if (def_has_shield && def_player != player) {
      printf("  *** Player %d's '%c' at (%d,%d) is SHIELDED! "
             "Shield absorbs the attack — your '%c' is destroyed! ***\n",
             def_player, defender, r2, c2, attacker);
      remove_shield_at(r2, c2);
      board[r1][c1] = '.';
      owner[r1][c1] = 0;
      printf("  (Shield used up. '%c' (Player %d) remains at (%d,%d).)\n",
             defender, def_player, r2, c2);

    } else if (can_eat(attacker, defender)) {
      printf("  >> '%c' captures '%c' from Player %d!\n", attacker, defender,
             def_player);
      remove_shield_at(r2, c2);
      board[r2][c2] = attacker;
      owner[r2][c2] = player;
      move_shield(r1, c1, r2, c2);
      board[r1][c1] = '.';
      owner[r1][c1] = 0;

    } else if (can_eat(defender, attacker)) {
      if (atk_has_shield) {
        printf(
            "  *** Your '%c' had a SHIELD and survived! Shield used up. ***\n",
            attacker);
        remove_shield_at(r1, c1);
      } else {
        printf("  >> Your '%c' was captured by '%c' from Player %d!\n",
               attacker, defender, def_player);
        board[r1][c1] = '.';
        owner[r1][c1] = 0;
      }

    } else {
      if (atk_has_shield && !def_has_shield) {
        printf("  >> Same type! '%c' (Player %d) had a shield and SURVIVED! "
               "'%c' (Player %d) is removed!\n",
               attacker, player, defender, def_player);
        remove_shield_at(r1, c1);
        board[r2][c2] = attacker;
        owner[r2][c2] = player;
        board[r1][c1] = '.';
        owner[r1][c1] = 0;

      } else if (def_has_shield && !atk_has_shield) {
        printf("  >> Same type! '%c' (Player %d) had a shield and SURVIVED! "
               "'%c' (Player %d) is removed!\n",
               defender, def_player, attacker, player);
        remove_shield_at(r2, c2);
        board[r1][c1] = '.';
        owner[r1][c1] = 0;

      } else if (!atk_has_shield && !def_has_shield) {
        printf("  >> '%c' vs '%c' - same type! Both removed!\n", attacker,
               defender);
        board[r1][c1] = '.';
        owner[r1][c1] = 0;
        board[r2][c2] = '.';
        owner[r2][c2] = 0;

      } else {
        printf("  >> Same type! Both had shields - BOTH SURVIVED! Shields used "
               "up.\n");
        remove_shield_at(r1, c1);
        remove_shield_at(r2, c2);
      }
    }
  }

  return 1;
}

int check_winner() {
  int p1 = count_pieces(1);
  int p2 = count_pieces(2);

  if (p1 == 0 && p2 == 0)
    return 3;
  if (p1 == 0)
    return 2;
  if (p2 == 0)
    return 1;
  return 0;
}

int main() {
  printf("============================================\n");
  printf("          JKQ Board Game\n");
  printf("       Mak Ruk Yuttha Hatthi\n");
  printf("============================================\n");

  init_board();

  while (1) {
    while (!do_turn(current_player))
      ;

    int w = check_winner();

    if (w == 1) {
      show_board();
      printf("=========================================================\n");
      printf("\n*** Player 1 WINS! All opponent pieces eliminated! ***\n");
      printf("\n=========================================================\n");
      break;
    } else if (w == 2) {
      show_board();
      printf("=========================================================\n");
      printf("\n*** Player 2 WINS! All opponent pieces eliminated! ***\n");
      printf("\n=========================================================\n");
      break;
    } else if (w == 3) {
      show_board();
      printf("=========================================================\n");
      printf("\n*** DRAW! Both players lost all pieces simultaneously! ***\n");
      printf("\n=========================================================\n");
      break;
    }

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
