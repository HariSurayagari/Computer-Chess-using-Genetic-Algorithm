/*
 *	MAIN.C
 *	Tom Kerrigan's Simple Chess Program (TSCP)
 *
 *	Copyright 1997 Tom Kerrigan
 */

#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "defs.h"
#include "data.h"
#include "prototypes.h"

using namespace std;


/* get_ms() returns the milliseconds elapsed since midnight,
   January 1, 1970. */

#include <sys/timeb.h>
BOOL ftime_ok = FALSE;  /* does ftime return milliseconds? */
int get_ms()
{
	struct timeb timebuffer;
	ftime(&timebuffer);
	if (timebuffer.millitm != 0)
		ftime_ok = TRUE;
	return (timebuffer.time * 1000) + timebuffer.millitm;
}


/* main() is basically an infinite loop that either calls
   think() when it's the computer's turn to move or prompts
   the user for a command (and deciphers it). */

int main()
{
	int computer_side;
	char s[256];
	int m;

	cout<<"\n";
	cout<<"Tom Kerrigan's Simple Chess Program (TSCP)\n";
	cout<<"version 1.81, 2/5/03\n";
	cout<<"Copyright 1997 Tom Kerrigan\n";
	cout<<"\n";
	cout<<"\"help\" displays a list of commands.\n";
	cout<<"\n";

	init_hash();
	init_board();
	open_book();
	gen();
	computer_side = EMPTY;
	max_time = 1 << 25;
	max_depth = 2;
	for (;;) {
		if (side == computer_side) {  /* computer's turn */
			
			/* think about the move and make it */
			think(1);
			if (!pv[0][0].u) {
				cout << "(no legal moves)\n";
				computer_side = EMPTY;
				continue;
			}
			cout << "Computer's move: "<< move_str(pv[0][0].b) << "\n";
			makemove(pv[0][0].b);
			ply = 0;
			gen();
			print_result();
			continue;
		}

		/* get user input */
		cout << "tscp> ";
		cin >> s;
		if (!strcmp(s, "on")) {
			computer_side = side;
			continue;
		}
		if (!strcmp(s, "off")) {
			computer_side = EMPTY;
			continue;
		}
		if (!strcmp(s, "st")) {
			scanf("%d", &max_time);
			max_time *= 1000;
			max_depth = 32;
			continue;
		}
		if (!strcmp(s, "sd")) {
			scanf("%d", &max_depth);
			max_time = 1 << 25;
			continue;
		}
		if (!strcmp(s, "undo")) {
			if (!hply)
				continue;
			computer_side = EMPTY;
			takeback();
			ply = 0;
			gen();
			continue;
		}
		if (!strcmp(s, "new")) {
			computer_side = EMPTY;
			init_board();
			gen();
			continue;
		}
		if (!strcmp(s, "d")) {
			print_board();
			continue;
		}
		if (!strcmp(s, "bye")) {
			cout << "Share and enjoy!\n";
			break;
		}
		
		if (!strcmp(s, "help")) {
			cout <<"on - computer plays for the side to move\n";
			cout <<"off - computer stops playing\n";
			cout <<"st n - search for n seconds per move\n";
			cout <<"sd n - search n ply per move\n";
			cout <<"undo - takes back a move\n";
			cout <<"new - starts a new game\n";
			cout <<"d - display the board\n";
			cout <<"bye - exit the program\n";
			cout <<"Enter moves in coordinate notation, e.g., e2e4, e7e8Q\n";
			continue;
		}

		/* maybe the user entered a move? */
		m = parse_move(s);
		if (m == -1 || !makemove(gen_dat[m].m.b))
			cout <<"Illegal move.\n";
		else {
			ply = 0;
			gen();
			print_result();
		}
	}

	close_book();
	return 0;
}


/* parse the move s (in coordinate notation) and return the move's
   index in gen_dat, or -1 if the move is illegal */

int parse_move(char *s)
{
	int from, to, i;

	/* make sure the string looks like a move */
	if (s[0] < 'a' || s[0] > 'h' ||
			s[1] < '0' || s[1] > '9' ||
			s[2] < 'a' || s[2] > 'h' ||
			s[3] < '0' || s[3] > '9')
		return -1;

	from = s[0] - 'a';
	from += 8 * (8 - (s[1] - '0'));
	to = s[2] - 'a';
	to += 8 * (8 - (s[3] - '0'));

	for (i = 0; i < first_move[1]; ++i)
		if (gen_dat[i].m.b.from == from && gen_dat[i].m.b.to == to) {

			/* if the move is a promotion, handle the promotion piece;
			   assume that the promotion moves occur consecutively in
			   gen_dat. */
			if (gen_dat[i].m.b.bits & 32)
				switch (s[4]) {
					case 'N':
						return i;
					case 'B':
						return i + 1;
					case 'R':
						return i + 2;
					default:  /* assume it's a queen */
						return i + 3;
				}
			return i;
		}

	/* didn't find the move */
	return -1;
}


/* move_str returns a string with move m in coordinate notation */

char *move_str(move_bytes m)
{
	static char str[6];

	char c;

	if (m.bits & 32) {
		switch (m.promote) {
			case KNIGHT:
				c = 'n';
				break;
			case BISHOP:
				c = 'b';
				break;
			case ROOK:
				c = 'r';
				break;
			default:
				c = 'q';
				break;
		}
		sprintf(str, "%c%d%c%d%c",
				COL(m.from) + 'a',
				8 - ROW(m.from),
				COL(m.to) + 'a',
				8 - ROW(m.to),
				c);
	}
	else
		sprintf(str, "%c%d%c%d",
				COL(m.from) + 'a',
				8 - ROW(m.from),
				COL(m.to) + 'a',
				8 - ROW(m.to));
	return str;
}


/* print_board() prints the board */

void print_board()
{
	int i;
	
	cout << "\n8 ";
	for (i = 0; i < 64; ++i) {
		switch (color[i]) {
			case EMPTY:
				cout << " .";
				break;
			case LIGHT:
				cout <<" " << light_piece_char[piece[i]];
				break;
			case DARK:
				cout <<" "<< dark_piece_char[piece[i]];
				break;
		}
		if ((i + 1) % 8 == 0 && i != 63)
			cout <<"\n"<< 7 - ROW(i) << " ";
	}
	cout << "\n\n   a b c d e f g h\n\n";
}



/* print_result() checks to see if the game is over, and if so,
   prints the result. */

void print_result()
{
	int i;

	/* is there a legal move? */
	for (i = 0; i < first_move[1]; ++i)
		if (makemove(gen_dat[i].m.b)) {
			takeback();
			break;
		}
	if (i == first_move[1]) {
		if (in_check(side)) {
			if (side == LIGHT)
				cout <<"0-1 {Black mates}\n";
			else
				cout <<"1-0 {White mates}\n";
		}
		else
			cout <<"1/2-1/2 {Stalemate}\n";
	}
	else if (reps() == 3)
		cout <<"1/2-1/2 {Draw by repetition}\n";
	else if (fifty >= 100)
		cout <<"1/2-1/2 {Draw by fifty move rule}\n";
}

