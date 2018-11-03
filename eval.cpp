#include <iostream>
#include <iomanip>
#include <locale>
#include <sstream>
#include <iostream>					// for cout etc.
#include <vector>					// for vector class
#include <string.h>					// for string class
#include <algorithm>				// for sort algorithm
#include <time.h>					// for random seed
#include <math.h>					// for abs()
#include "defs.h"
#include "data.h"
#include "prototypes.h"


#define DOUBLED_PAWN_PENALTY		10
#define ISOLATED_PAWN_PENALTY		20
#define BACKWARDS_PAWN_PENALTY		8
#define PASSED_PAWN_BONUS			20
#define ROOK_SEMI_OPEN_FILE_BONUS	10
#define ROOK_OPEN_FILE_BONUS		15
#define ROOK_ON_SEVENTH_BONUS		20


using namespace std;


/* the values of the pieces */
int piece_value[6] = {
	100, 300, 300, 500, 900, 0
};

int location = 0;

/* The "pcsq" arrays are piece/square tables. They're values
   added to the material value of the piece based on the
   location of the piece. */

int pawn_pcsq[64] = {
	  0,   0,   0,   0,   0,   0,   0,   0,
	  5,  10,  15,  20,  20,  15,  10,   5,
	  4,   8,  12,  16,  16,  12,   8,   4,
	  3,   6,   9,  12,  12,   9,   6,   3,
	  2,   4,   6,   8,   8,   6,   4,   2,
	  1,   2,   3, -10, -10,   3,   2,   1,
	  0,   0,   0, -40, -40,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0
};

int knight_pcsq[64] = {
	-10, -10, -10, -10, -10, -10, -10, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10, -30, -10, -10, -10, -10, -30, -10
};

int bishop_pcsq[64] = {
	-10, -10, -10, -10, -10, -10, -10, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10, -10, -20, -10, -10, -20, -10, -10
};

int queen_pcsq[64] = {
	-10, -10, -10, -10, -10, -10, -10, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10, -10, -10, -30, -10, -10, -10, -10
};

int king_pcsq[64] = {
	-40, -40, -40, -40, -40, -40, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40,
	-20, -20, -20, -20, -20, -20, -20, -20,
	  0,  20,  40, -20,   0, -20,  40,  20
};

int king_endgame_pcsq[64] = {
	  0,  10,  20,  30,  30,  20,  10,   0,
	 10,  20,  30,  40,  40,  30,  20,  10,
	 20,  30,  40,  50,  50,  40,  30,  20,
	 30,  40,  50,  60,  60,  50,  40,  30,
	 30,  40,  50,  60,  60,  50,  40,  30,
	 20,  30,  40,  50,  50,  40,  30,  20,
	 10,  20,  30,  40,  40,  30,  20,  10,
	  0,  10,  20,  30,  30,  20,  10,   0
};

/* The flip array is used to calculate the piece/square
   values for DARK pieces. The piece/square value of a
   LIGHT pawn is pawn_pcsq[sq] and the value of a DARK
   pawn is pawn_pcsq[flip[sq]] */
int flip[64] = {
	 56,  57,  58,  59,  60,  61,  62,  63,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 32,  33,  34,  35,  36,  37,  38,  39,
	 24,  25,  26,  27,  28,  29,  30,  31,
	 16,  17,  18,  19,  20,  21,  22,  23,
	  8,   9,  10,  11,  12,  13,  14,  15,
	  0,   1,   2,   3,   4,   5,   6,   7
};

/* pawn_rank[x][y] is the rank of the least advanced pawn of color x on file
   y - 1. There are "buffer files" on the left and right to avoid special-case
   logic later. If there's no pawn on a rank, we pretend the pawn is
   impossibly far advanced (0 for LIGHT and 7 for DARK). This makes it easy to
   test for pawns on a rank and it simplifies some pawn evaluation code. */
int pawn_rank[2][10];

int piece_mat[2];  /* the value of a side's pieces */
int pawn_mat[2];  /* the value of a side's pawns */

int eval()
{
	int i;
	int f;  /* file */
	int score[2];  /* each side's score */

	/* this is the first pass: set up pawn_rank, piece_mat, and pawn_mat. */
	for (i = 0; i < 10; ++i) {
		pawn_rank[LIGHT][i] = 0;
		pawn_rank[DARK][i] = 7;
	}
	piece_mat[LIGHT] = 0;
	piece_mat[DARK] = 0;
	pawn_mat[LIGHT] = 0;
	pawn_mat[DARK] = 0;
	for (i = 0; i < 64; ++i) {
		if (color[i] == EMPTY)
			continue;
		if (piece[i] == PAWN) {
			pawn_mat[color[i]] += piece_value[PAWN];
			f = COL(i) + 1;  /* add 1 because of the extra file in the array */
			if (color[i] == LIGHT) {
				if (pawn_rank[LIGHT][f] < ROW(i))
					pawn_rank[LIGHT][f] = ROW(i);
			}
			else {
				if (pawn_rank[DARK][f] > ROW(i))
					pawn_rank[DARK][f] = ROW(i);
			}
		}
		else
			piece_mat[color[i]] += piece_value[piece[i]];
	}

	/* this is the second pass: evaluate each piece */
	score[LIGHT] = piece_mat[LIGHT] + pawn_mat[LIGHT];
	score[DARK] = piece_mat[DARK] + pawn_mat[DARK];
	for (i = 0; i < 64; ++i) {
		if (color[i] == EMPTY)
			continue;
		if (color[i] == LIGHT) {
			switch (piece[i]) {
				case PAWN:
					score[LIGHT]=eval_light_pawn(i);
//					cout<<"light pawn square:"<<pawn_pcsq[i]<<endl;
					if(pawn_pcsq[i] < 0) 
					{
					location = -1*pawn_pcsq[i];
//					cout<<"light pawn:"<<location<<endl;
					score[LIGHT] -= GA(PAWN, location);
//					cout<<"light pawn score:"<<score[LIGHT]<<"\n \n";
					}
					else
					{
					location = pawn_pcsq[i];
//					cout<<"light pawn:"<<location<<endl;
					score[LIGHT] += GA(PAWN, location);
//					cout<<"light pawn score:"<<score[LIGHT]<<"\n \n";
					}					
					break;
					
				case KNIGHT:
//					cout<<"light knight square:"<<knight_pcsq[i]<<endl;
					if(knight_pcsq[i] < 0) 
					{
					location = -1*knight_pcsq[i];
//					cout<<"light knight:"<<location<<endl;
					score[LIGHT] -= GA(KNIGHT, location);
//					cout<<"light knight score:"<<score[LIGHT]<<"\n \n";
					}
					else
					{
					location = knight_pcsq[i];
//					cout<<"light knight:"<<location<<endl;
					score[LIGHT] += GA(KNIGHT, location);
//					cout<<"light knight score:"<<score[LIGHT]<<"\n \n";
					}					
					break;
				case BISHOP:
//					cout<<"light bishop square:"<<bishop_pcsq[i]<<endl;
					if(bishop_pcsq[i] < 0)
					{
					location = -1*bishop_pcsq[i];
//					cout<<"light bishop:"<<location<<endl;
					score[LIGHT] -= GA(BISHOP, location);
//					cout<<"light bishop score:"<<score[LIGHT]<<"\n \n";
					}
					else
					{
					location = bishop_pcsq[i];
//					cout<<"light bishop:"<<location<<endl;
					score[LIGHT] += GA(BISHOP, location);
//					cout<<"light bishop score:"<<score[LIGHT]<<"\n \n";
					}					
					break;
				case ROOK:
					if (pawn_rank[LIGHT][COL(i) + 1] == 0) {
						if (pawn_rank[DARK][COL(i) + 1] == 7)
							score[LIGHT] += ROOK_OPEN_FILE_BONUS;
						else
							score[LIGHT] += ROOK_SEMI_OPEN_FILE_BONUS;
					}
					if (ROW(i) == 1)
						score[LIGHT] += ROOK_ON_SEVENTH_BONUS;
					location = 0;
					score[LIGHT] += GA(ROOK, location);
//					cout<<"light rook score:"<<score[LIGHT]<<"\n \n";				
					break;
				case QUEEN:
//					cout<<"light queen square:"<<queen_pcsq[i]<<endl;
					if(queen_pcsq[i] < 0)
					{
					location = -1*queen_pcsq[i];
//					cout<<"light queen:"<<location<<endl;
					score[LIGHT] -= GA(QUEEN, location);
//					cout<<"light queen score:"<<score[LIGHT]<<"\n \n";
					}
					else
					{
					location = queen_pcsq[i];
//					cout<<"light queen:"<<location<<endl;
					score[LIGHT] += GA(QUEEN, location);
//					cout<<"light queen score:"<<score[LIGHT]<<"\n \n";
					}					
					break;
				case KING:
					if (piece_mat[DARK] <= 1200)
						score[LIGHT] += king_endgame_pcsq[i];
					else
						score[LIGHT] += eval_light_king(i);
//					cout<<"light king square:"<<king_pcsq[i]<<endl;
					if(king_pcsq < 0)
					{
					location = -1*king_pcsq[i];
//					cout<<"light king:"<<location<<endl;
					score[LIGHT] -= GA(KING, location);
//					cout<<"light king score:"<<score[LIGHT]<<"\n \n";
					}
					else
					{
					location = king_pcsq[i];
//					cout<<"light king:"<<location<<endl;
					score[LIGHT] += GA(KING, location);
//					cout<<"light king score:"<<score[LIGHT]<<"\n \n";
					}					
					break;
			}	
		}
		else {
			switch (piece[i]) {
				case PAWN:
					score[DARK]=eval_dark_pawn(flip[i]);
//					cout<<"dark pawn square:"<<pawn_pcsq[flip[i]]<<endl;
					if(pawn_pcsq[flip[i]] < 0)
					{
					location = -1*pawn_pcsq[flip[i]];
//					cout<<"dark pawn:"<<location<<endl;
					score[DARK] -= GA(PAWN, location);
//					cout<<"dark pawn score:"<<score[DARK]<<"\n \n";
					}
					else
					{
					location = pawn_pcsq[flip[i]];
//					cout<<"dark pawn:"<<location<<endl;
					score[DARK] += GA(PAWN, location);
//					cout<<"dark pawn score:"<<score[DARK]<<"\n \n";
					}					
					break;
				case KNIGHT:
//					cout<<"dark knight square:"<<knight_pcsq[flip[i]]<<endl;
					if(knight_pcsq[flip[i]] < 0)
					{
					location = -1*knight_pcsq[flip[i]];
//					cout<<"dark knight:"<<location<<endl;
					score[DARK] -= GA(KNIGHT, location);
//					cout<<"dark knight score:"<<score[DARK]<<"\n \n";
					}
					else
					{
					location = knight_pcsq[flip[i]];
//					cout<<"dark knight:"<<location<<endl;
					score[DARK] += GA(KNIGHT, location);
//					cout<<"dark knight score:"<<score[DARK]<<"\n \n";
					}					
					break;
				case BISHOP:
//					cout<<"dark bishop square:"<<bishop_pcsq[flip[i]]<<endl;
					if(bishop_pcsq[flip[i]] < 0)
					{
					location = -1*bishop_pcsq[flip[i]];
//					cout<<"dark bishop:"<<location<<endl;
					score[DARK] -= GA(BISHOP, location);
//					cout<<"dark bishop score:"<<score[DARK]<<"\n \n";
					}
					else
					{
					location = bishop_pcsq[flip[i]];
//					cout<<"dark bishop:"<<location<<endl;
					score[DARK] += GA(BISHOP, location);
//					cout<<"dark bishop score:"<<score[DARK]<<"\n \n";
					}					
					break;
				case ROOK:
					if (pawn_rank[DARK][COL(i) + 1] == 7) {
						if (pawn_rank[LIGHT][COL(i) + 1] == 0)
							score[DARK] += ROOK_OPEN_FILE_BONUS;
						else
							score[DARK] += ROOK_SEMI_OPEN_FILE_BONUS;
					}
					if (ROW(i) == 6)
						score[DARK] += ROOK_ON_SEVENTH_BONUS;
	
					location = 0;
					score[DARK] += GA(ROOK, location);
//					cout<<"dark rook score:"<<score[DARK]<<"\n \n";
					break;
				case QUEEN:
//					cout<<"dark queen square:"<<queen_pcsq[flip[i]]<<endl;
					if(queen_pcsq[flip[i]] < 0)
					{
					location = -1*queen_pcsq[flip[i]];
//					cout<<"dark queen:"<<location<<endl;
					score[DARK] -= GA(QUEEN, location);
//					cout<<"dark queen score:"<<score[DARK]<<"\n \n";
					}
					else
					{
					location = queen_pcsq[flip[i]];
//					cout<<"dark queen:"<<location<<endl;
					score[DARK] += GA(QUEEN, location);
//					cout<<"dark queen score:"<<score[DARK]<<"\n \n";
					}					
					break;
				case KING:
					if (piece_mat[LIGHT] <= 1200)
						score[DARK] += king_endgame_pcsq[flip[i]];
					else
						score[DARK] += eval_dark_king(i);
					
//					cout<<"dark king square:"<<king_pcsq[flip[i]]<<endl;
					if(king_pcsq[flip[i]] < 0)
					{
					location = -1*king_pcsq[flip[i]];
//					cout<<"dark king:"<<location<<endl;
					score[DARK] -= GA(KING, location);
//					cout<<"dark king score:"<<score[DARK]<<"\n \n";
					}
					else
					{
					location = king_pcsq[flip[i]];
//					cout<<"dark king:"<<location<<endl;
					score[DARK] += GA(KING, location);
//					cout<<"dark king score:"<<score[DARK]<<"\n \n";
					}					
					break;
			
			
			}
			
		}
	}

	/* the score[] array is set, now return the score relative
	   to the side to move */
	if (side == LIGHT)
		return score[LIGHT] - score[DARK];
	return score[DARK] - score[LIGHT];
}

int eval_light_pawn(int sq)
{
	int r;  /* the value to return */
	int f;  /* the pawn's file */

	r = 0;
	f = COL(sq) + 1;

//	r += pawn_pcsq[sq];

	/* if there's a pawn behind this one, it's doubled */
	if (pawn_rank[LIGHT][f] > ROW(sq))
		r -= DOUBLED_PAWN_PENALTY;

	/* if there aren't any friendly pawns on either side of
	   this one, it's isolated */
	if ((pawn_rank[LIGHT][f - 1] == 0) &&
			(pawn_rank[LIGHT][f + 1] == 0))
		r -= ISOLATED_PAWN_PENALTY;

	/* if it's not isolated, it might be backwards */
	else if ((pawn_rank[LIGHT][f - 1] < ROW(sq)) &&
			(pawn_rank[LIGHT][f + 1] < ROW(sq)))
		r -= BACKWARDS_PAWN_PENALTY;

	/* add a bonus if the pawn is passed */
	if ((pawn_rank[DARK][f - 1] >= ROW(sq)) &&
			(pawn_rank[DARK][f] >= ROW(sq)) &&
			(pawn_rank[DARK][f + 1] >= ROW(sq)))
		r += (7 - ROW(sq)) * PASSED_PAWN_BONUS;

	return r;
}

int eval_dark_pawn(int sq)
{
	int r;  /* the value to return */
	int f;  /* the pawn's file */

	r = 0;
	f = COL(sq) + 1;

//	r += pawn_pcsq[flip[sq]];

	/* if there's a pawn behind this one, it's doubled */
	if (pawn_rank[DARK][f] < ROW(sq))
		r -= DOUBLED_PAWN_PENALTY;

	/* if there aren't any friendly pawns on either side of
	   this one, it's isolated */
	if ((pawn_rank[DARK][f - 1] == 7) &&
			(pawn_rank[DARK][f + 1] == 7))
		r -= ISOLATED_PAWN_PENALTY;

	/* if it's not isolated, it might be backwards */
	else if ((pawn_rank[DARK][f - 1] > ROW(sq)) &&
			(pawn_rank[DARK][f + 1] > ROW(sq)))
		r -= BACKWARDS_PAWN_PENALTY;

	/* add a bonus if the pawn is passed */
	if ((pawn_rank[LIGHT][f - 1] <= ROW(sq)) &&
			(pawn_rank[LIGHT][f] <= ROW(sq)) &&
			(pawn_rank[LIGHT][f + 1] <= ROW(sq)))
		r += ROW(sq) * PASSED_PAWN_BONUS;

	return r;
}

int eval_light_king(int sq)
{
	int r;  /* the value to return */
	int i;

	r = king_pcsq[sq];

	/* if the king is castled, use a special function to evaluate the
	   pawns on the appropriate side */
	if (COL(sq) < 3) {
		r += eval_lkp(1);
		r += eval_lkp(2);
		r += eval_lkp(3) / 2;  /* problems with pawns on the c & f files
								  are not as severe */
	}
	else if (COL(sq) > 4) {
		r += eval_lkp(8);
		r += eval_lkp(7);
		r += eval_lkp(6) / 2;
	}

	/* otherwise, just assess a penalty if there are open files near
	   the king */
	else {
		for (i = COL(sq); i <= COL(sq) + 2; ++i)
			if ((pawn_rank[LIGHT][i] == 0) &&
					(pawn_rank[DARK][i] == 7))
				r -= 10;
	}

	/* scale the king safety value according to the opponent's material;
	   the premise is that your king safety can only be bad if the
	   opponent has enough pieces to attack you */
	r *= piece_mat[DARK];
	r /= 3100;

	return r;
}

/* eval_lkp(f) evaluates the Light King Pawn on file f */

int eval_lkp(int f)
{
	int r = 0;

	if (pawn_rank[LIGHT][f] == 6);  /* pawn hasn't moved */
	else if (pawn_rank[LIGHT][f] == 5)
		r -= 10;  /* pawn moved one square */
	else if (pawn_rank[LIGHT][f] != 0)
		r -= 20;  /* pawn moved more than one square */
	else
		r -= 25;  /* no pawn on this file */

	if (pawn_rank[DARK][f] == 7)
		r -= 15;  /* no enemy pawn */
	else if (pawn_rank[DARK][f] == 5)
		r -= 10;  /* enemy pawn on the 3rd rank */
	else if (pawn_rank[DARK][f] == 4)
		r -= 5;   /* enemy pawn on the 4th rank */

	return r;
}

int eval_dark_king(int sq)
{
	int r;
	int i;

	r = king_pcsq[flip[sq]];
	if (COL(sq) < 3) {
		r += eval_dkp(1);
		r += eval_dkp(2);
		r += eval_dkp(3) / 2;
	}
	else if (COL(sq) > 4) {
		r += eval_dkp(8);
		r += eval_dkp(7);
		r += eval_dkp(6) / 2;
	}
	else {
		for (i = COL(sq); i <= COL(sq) + 2; ++i)
			if ((pawn_rank[LIGHT][i] == 0) &&
					(pawn_rank[DARK][i] == 7))
				r -= 10;
	}
	r *= piece_mat[LIGHT];
	r /= 3100;
	return r;
}

int eval_dkp(int f)
{
	int r = 0;

	if (pawn_rank[DARK][f] == 1);
	else if (pawn_rank[DARK][f] == 2)
		r -= 10;
	else if (pawn_rank[DARK][f] != 7)
		r -= 20;
	else
		r -= 25;

	if (pawn_rank[LIGHT][f] == 0)
		r -= 15;
	else if (pawn_rank[LIGHT][f] == 2)
		r -= 10;
	else if (pawn_rank[LIGHT][f] == 3)
		r -= 5;

	return r;
}
