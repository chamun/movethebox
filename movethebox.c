#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* !! QUEUE SIZE MUST BE THE GREATER NUMBER BETWEEN ROWS AND COLS !! */
#define QUEUESIZE 8
#define ROWS      8
#define COLS      7

#define EMPTY 0

/* !! DO NOT CHANGE THESE. BY DOING SO, YOU'RE GONNA SCREW movetostring() UP! !! */
#define RIGHT 0
#define LEFT  1
#define DOWN  2
#define UP    3

struct queue {
	int list[QUEUESIZE];
	int index;
};

int
queuesize(struct queue *q)
{
	return q->index;
}

void
queueinit(struct queue *q)
{
	q->index = 0;
	memset(q->list, 0, QUEUESIZE * sizeof(int));
}

void
enqueue(struct queue *q, int elem)
{
	q->list[q->index] = elem;
	q->index++;
}

int
isbox(int cell)
{
	return cell > 0;
}

/* Returns in "moves", all moves a box (row,col) may perform */

int
possiblemoves(int boxabove, int row, int col, int *moves)
{
	int index;
	index = 0;
	if (col < COLS)
		moves[index++] = RIGHT;
	if (col > 0)
		moves[index++] = LEFT;
	if (row > 0 && isbox(boxabove))
		moves[index++] = UP;
	if (row < ROWS - 1)
		moves[index++] = DOWN;
	return index;
}

/* Swaps box (row1, col1) with box(row2, col2) */

void
swap(int matrix[ROWS][COLS], int row1, int col1, int row2, int col2)
{
	int aux = matrix[row2][col2];
	matrix[row2][col2] = matrix[row1][col1];
	matrix[row1][col1] = aux;
}

/* Moves a box */
void
move(int matrix[ROWS][COLS], int row, int col, int move)
{
	int aux;
	switch(move) {
		case UP:
			swap(matrix, row, col, row-1, col);
			break;
		case DOWN:
			swap(matrix, row, col, row+1, col);
			break;
		case LEFT:
			swap(matrix, row, col, row, col-1);
			break;
		case RIGHT:
			swap(matrix, row, col, row, col+1);
			break;
	}
} 

/* 
 * Checks whether there are two or more consecutives boxes on the right of the
 * box (row,col). If there are, mark them in "mask" so
 * they can be erased later. 
 */

void
right(int matrix[ROWS][COLS], int mask[ROWS][COLS], int row, int col) 
{
	struct queue q;
	int ci, i;

	queueinit(&q);
	enqueue(&q, col);
	ci = col+1;

	while( ci < COLS && matrix[row][ci] == matrix[row][col] ) {
		enqueue(&q, ci);
		ci++;
	}

	if (queuesize(&q) >= 3) {
		for (i = 1; i < queuesize(&q); i++) {
			ci = q.list[i];
			mask[row][ci] = 1;
		}
		mask[row][col] = 1;
	}
}

/*
 * Same thing as right, but checks for consecutives boxes below the box
 * (row,col)
 */

void
down (int matrix[ROWS][COLS], int mask[ROWS][COLS], int row, int col)
{
	struct queue q;
	int cr, i;

	queueinit(&q);
	enqueue(&q, row);
	cr = row+1;

	while( cr < ROWS && matrix[cr][col] == matrix[row][col] ) {
		enqueue(&q, cr);
		cr++;
	}

	if (queuesize(&q) >= 3) {
		for (i = 1; i < queuesize(&q); i++) {
			cr = q.list[i];
			mask[cr][col] = 1;
		}
		mask[row][col] = 1;
	}
}

/* 
 * Looks for boxes with the same shape that are grouped together and erase them
 * from the matrix 
 */

int
erase(int matrix[ROWS][COLS])
{
	int mask[ROWS][COLS];
	int i, j, ret;

	memset(mask, 0, ROWS * COLS * sizeof(int));
	ret = 0;

	for (i = 0; i < ROWS; i++)
		for (j = 0; j < COLS; j++) 
			if (isbox(matrix[i][j])) {
				down(matrix, mask, i, j);
				right(matrix, mask, i, j);
			}

	for (i = 0; i < ROWS; i++)
		for (j = 0; j < COLS; j++) 
			if (mask[i][j]) {
				ret = 1;
				matrix[i][j] = EMPTY;
			}
	
	return ret;
}

/* Moves boxes down if there are no boxes under them */

void
fall (int matrix[ROWS][COLS], int bottom, int top, int to, int col)
{
	int cto, cbottom;
	
	cto = to;
	cbottom = bottom;
	while (cbottom > top) {
		matrix[cto][col] = matrix[cbottom][col];
		matrix[cbottom][col] = EMPTY;
		cto--;
		cbottom--;
	}
	
}

/*
 * Eliminates boxes that look tha same, and places boxes in the right place if
 * there are no boxes under them
 */
void
fix(int matrix[ROWS][COLS])
{
	int i, j;
	/* First, we make the boxes fall */
	for (i = 0; i < ROWS - 1; i++)
		for(j = 0; j < COLS; j++) {
			if(!(isbox(matrix[i][j]) && !isbox(matrix[i+1][j])))
				continue;
			int to, top;
			for (to  = i+1; to  < ROWS && !isbox(matrix[to ][j]); to++);
			for (top = i-1; top >= 0   &&  isbox(matrix[top][j]); top--);
			/* "to" is the row position of the next box below the current box.
			 * We decrement 1 so it will be the position above the next box */
			fall(matrix, i, top, to-1, j);
		}

	/* Now we eliminate boxes that are joined together */
	if (erase(matrix))
		fix(matrix);
}

/* Checks if the current matrix have no boxes */
int
issolution(int matrix[ROWS][COLS])
{
	int i, j;
	for (i = 0; i < ROWS; i++)
		for(j = 0; j < COLS; j++)
			if (isbox(matrix[i][j]))
				return 0;
	return 1;
}

/* Returns the corresponding string of the integer "move" */
const char*
movetostring(int move)
{
	char *moves[4] = { "RIGHT", "LEFT", "DOWN", "UP" };
	if (move < RIGHT || move > UP)
		errx(1, "movetostring: Invalid move (%d)", move);
	return moves[move];
}

struct step { int row, col, move; }; 
void
initstep(struct step *s, int row, int col, int move)
{
	s->row = row;
	s->col = col;
	s->move = move;
}

/* Finds the moves we need to perform to solve the game */
int 
findmoves(int matrix[ROWS][COLS], int steps, int maxsteps, struct step **solution) 
{
	int i, j, k;
	int moves[4];
	int changedmatrix[ROWS][COLS];

	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLS; j++) {
			if(!isbox(matrix [i][j])) 
				continue;

			int lastmove;
			int above = i > 0 ? matrix[i-1][j] : EMPTY;
			lastmove = possiblemoves(above, i, j, moves);
			for (k = 0; k < lastmove; k++) {
				memcpy(changedmatrix, matrix, ROWS * COLS * sizeof(int));
				move(changedmatrix, i, j, moves[k]);
				fix(changedmatrix);
				if (issolution(changedmatrix)) {
					initstep(solution[steps], i, j, moves[k]);
					return 1;
				} else if(    steps + 1 < maxsteps
						   && findmoves(changedmatrix, steps+1, maxsteps, solution)) {
					initstep(solution[steps], i, j, moves[k]);
					return 1;
				}
			}
		}
	}
	return 0;
}

/* 
 * Print a matrix. row and col are the position of a box that should be in
 * evidence
 */
void
printmatrix(int matrix[ROWS][COLS], int row, int col)
{
	int i, j;
	char symbols[11] = { ' ', '@', '#', '$', '%', '&', '*', 'D', 'O', 'U', '?'};
	char symbol;
	for (i = 0; i < ROWS; i++) {
		printf("%d ", i+1);
		for(j = 0; j < COLS; j++)  {
			if (matrix[i][j] > 10)
				errx(1, "printmatrix: unexpected code for a box (!= [0-9])");
			symbol = (i == row && j == col) ? symbols[10] : symbols[matrix[i][j]];
			printf("%c", symbol);
			if (j == COLS - 1)
				printf("\n");
		}
	}

	printf("  ");
	for(j = 0; j < COLS; j++) 
		printf("%d", j+1);
	printf("\n");
}

int 
main(void)
{
	int matrix[ROWS][COLS];
	int i, j, max;
	char c;
	struct step **solution;


	memset(matrix, 0, ROWS * COLS * sizeof(int));
	scanf("%d%c", &max, &c);
	for (i = 0; i < ROWS; i++) {
		for(j = 0; j < COLS; j++) {
			scanf("%c", &c);	
			matrix[i][j] = (int) (c - '0');
		}
		scanf("%c", &c);	
	}

	solution = malloc(sizeof(struct step *) * max);
	for (i = 0; i < max; i++)
		solution[i] = malloc(sizeof (struct step));

	findmoves(matrix, 0, max, solution);
	for (i = 0; i < max; i++) {
		printf("STEP %d\n", i+1);
		printmatrix(matrix, solution[i]->row, solution[i]->col);
		printf("Move box (%d,%d) %s\n",
				solution[i]->row+1, solution[i]->col+1,
				movetostring(solution[i]->move));
		move(matrix, solution[i]->row, solution[i]->col, solution[i]->move);
		fix(matrix);
	}

	for (i = 0; i < max; i++)
		free(solution[i]);
	free (solution);
}
