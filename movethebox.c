/* find_moves { for each box b {
		moves = possible_moves(b)
		for each move m in moves {
			move(b, m)
			if is a solution
				print move
				return true
			else {
				steps++
				if steps < max_steps
					if(find_moves()) {
						return true
					}
			}
			unmove(b, m)
		}
	}
	return false
}
*/
#include <string.h>
#include <stdio.h>
#include <err.h>

/* !! QUEUE SIZE MUST BE THE GREATER NUMBER BETWEEN ROWS AND COLS !! */
#define QUEUESIZE 7
#define ROWS 7
#define COLS 7

#define EMPTY 0

/* !! DO NOT CHANGE THESE. BY DOING SO, YOU'RE GONNA SCREW movetostring() UP! !! */
#define RIGHT 0
#define LEFT  1
#define DOWN  2
#define UP    3

int
isbox(int cell)
{
	return cell > 0;
}

/* Returns in moves, all moves a box (row,col) may perform */
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

/* Performs a move on a box */
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

void right(int matrix[ROWS][COLS], int mask[ROWS][COLS], int row, int col);
void down (int matrix[ROWS][COLS], int mask[ROWS][COLS], int row, int col);

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

void
fix(int matrix[ROWS][COLS])
{
	int i, j, k;
	FIX:
	/* First, we make the boxes fall */
	for (i = 0; i < ROWS; i++)
		for(j = 0; j < COLS; j++) {
			int changed = 0;
			k = i;
			while(    isbox(matrix[k][j])  
			       && k < ROWS - 1 
				   && !isbox(matrix[k+1][j])) {
				   swap(matrix, k, j, k+1, j);
				   k++;
				   changed = 1;
			}
			if (changed)
				goto FIX;
		}

	/* Now we eliminate box that are joined together */
	if (erase(matrix))
		goto FIX;
}

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

const char*
movetostring(int move)
{
	char *moves[4] = { "RIGHT", "LEFT", "DOWN", "UP" };
	if (move < RIGHT || move > UP)
		errx(1, "movetostring: Invalid move (%d)", move);
	return moves[move];
}

int 
findmoves(int matrix[ROWS][COLS], int steps, int maxsteps) 
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
					printf("Step %d - Box (%d,%d) moved %s\n", 
						   steps, i, j, movetostring(moves[k]));
					return 1;
				} else if(    steps + 1 < maxsteps
						   && findmoves(changedmatrix, steps+1, maxsteps)) {
					printf("Step %d - Box (%d,%d) moved %s\n", 
						   steps, i, j, movetostring(moves[k]));
					return 1;
				}
			}
		}
	}
	return 0;
}

void
printmatrix(int matrix[ROWS][COLS])
{
	int i, j;
	for (i = 0; i < ROWS; i++) 
		for(j = 0; j < COLS; j++)  {
			printf("%d", matrix[i][j]);
			if (j == COLS - 1)
				printf("\n");
		}
}

int 
main(void)
{
	int matrix[ROWS][COLS];
	int i, j, max;
	char c;

	memset(matrix, 0, ROWS * COLS * sizeof(int));
	scanf("%d%c", &max, &c);
	for (i = 0; i < ROWS; i++) {
		for(j = 0; j < COLS; j++) {
			scanf("%c", &c);	
			matrix[i][j] = (int) (c - '0');
		}
		scanf("%c", &c);	
	}
	//printf("Matrix:\n");
	//printmatrix(matrix);

	findmoves(matrix, 0, max);
}
