/* Solution to comp10002 Assignment 2, 2019 semester 2.

   Authorship Declaration:

   (1) I certify that the program contained in this submission is completely
   my own individual work, except where explicitly noted by comments that
   provide details otherwise.  I understand that work that has been developed
   by another student, or by me in collaboration with other students,
   or by non-students as a result of request, solicitation, or payment,
   may not be submitted for assessment in this subject.  I understand that
   submitting for assessment work developed by or in collaboration with
   other students or non-students constitutes Academic Misconduct, and
   may be penalized by mark deductions, or by other penalties determined
   via the University of Melbourne Academic Honesty Policy, as described
   at https://academicintegrity.unimelb.edu.au.

   (2) I also certify that I have not provided a copy of this work in either
   softcopy or hardcopy or any other form to any other student, and nor will
   I do so until after the marks are released. I understand that providing
   my work to other students, regardless of my intention or any undertakings
   made to me by that other student, is also Academic Misconduct.

   (3) I further understand that providing a copy of the assignment
   specification to any form of code authoring or assignment tutoring
   service, or drawing the attention of others to such services and code
   that may have been made available via such a service, may be regarded
   as Student General Misconduct (interfering with the teaching activities
   of the University and/or inciting others to commit Academic Misconduct).
   I understand that an allegation of Student General Misconduct may arise
   regardless of whether or not I personally make use of such solutions
   or sought benefit from such actions.

   Signed by: [Zi Ying Ng 1085130]
   Dated:     [13/10/2019]

*/

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <assert.h>

/* route format */
# define ARROW "->"
# define ROUTE_END "."

/* grid cell types */
# define INITIAL_CELL 'I'
# define GOAL_CELL 'G'
# define BLOCK_CELL '#'
# define PATH '*'
# define BLANK ' '
# define ROUTE '*'

/* delimit stages and parts */
# define LEFT_STAGEMARKER "==STAGE "
# define RIGHT_STAGEMARKER "=======================================\n"
# define PART_DELIM "------------------------------------------------\n"
# define END_DELIM "================================================\n"

/* route statuses */
# define STATUS_1 "Initial cell in the route is wrong!"
# define STATUS_2 "Goal cell in the route is wrong!"
# define STATUS_3 "There is an illegal move in this route!"
# define STATUS_4 "There is a block on this route!"
# define STATUS_5 "The route is valid!"
# define BROKEN_ROUTE "The route cannot be repaired!"

/* adjacent cell directions */
# define UP 0
# define DOWN 1
# define LEFT 2
# define RIGHT 3

/******************************************************************************/

/* function protoypes */

typedef struct {
    int x; 
    int y; 
} cell_t; 

typedef struct {
    cell_t cell;
    int count;
} qitem_t;

typedef struct node {
    cell_t cell;
    struct node *next;
} node_t;

typedef struct qnode {
    qitem_t qitem;
    struct qnode *next;
} qnode_t;

typedef struct {
    node_t *start;
    node_t *end;
} list_t;

typedef struct {
    qnode_t *start;
    qnode_t *end;
} queue_t;

/* create the data structures */
int make_grid(int nrows, int ncols, char ***grid);
list_t *make_empty_list(void);
queue_t *make_empty_queue(void);

/* read and insert data */
int grid_insert(char ***grid, cell_t *cell, char cell_type);
int get_cell(cell_t *cell);
int get_route_cell(cell_t *cell);
int get_blocks(int nrows, int ncols, char ***grid);
list_t *get_route(list_t *list);

/* manipulating lists and queue */
list_t *insert_at_tail(void *list, void *data);
list_t *insert_at_head(void *list, void *data);
queue_t *insert_queue(void *list, void *data);

/* process data */
int get_route_status(list_t *route, char ***grid);
int is_adjacent(cell_t *cell1, cell_t *cell2);

/* print the data */
int print_list(node_t *start);
int print_status(int status_num);
int print_grid(list_t *route, char ***grid, int nrows, int ncols);

/* fix the route */
int fix_route(list_t *route, char ***grid, int nrows, int ncols);
int meet_route(list_t *route, char ***grid, cell_t *cell, cell_t *route_start);
int in_queue(cell_t *cell, queue_t *queue);
int out_of_bounds(cell_t *cell, int nrows, int ncols);
cell_t find_adj(cell_t *curr_cell, int direction);
cell_t fix_seg(list_t *route, char ***grid, int nrows, int ncols);
list_t *insert_seg(list_t *route, list_t *segment);
list_t *make_fixed_segment(queue_t *queue, cell_t seg_s, int nrows, int ncols);

/* free memory */
void free_list(list_t *list);
void free_queue(queue_t *queue);
void free_grid(char ***grid, int nrows, int ncols);

/******************************************************************************/

/* main program controls the sequence of actions */

int
main(int argc, char *argv[]) {
    int stage=0, status=0, terminate=0;
    int nrows, ncols, nblocks=0;
    char **grid;
    cell_t init, goal;
    list_t *route;

    /* STAGE 0*/
    /* read the dimensions, as well as initial and goal cells */
    scanf("%dx%d\n", &nrows, &ncols);
    get_cell(&init);
    get_cell(&goal);

    /* create a 2D grid and insert initial, goal and block cells */
    make_grid(nrows, ncols, &grid);
    nblocks = get_blocks(nrows, ncols, &grid);
    grid_insert(&grid, &init, INITIAL_CELL);
    grid_insert(&grid, &goal, GOAL_CELL);

    /* read and evaluate the proposed route */
    route = make_empty_list(); 
    route = get_route(route);
    status = get_route_status(route, &grid);
    
    /* print stage 0 output */
    printf("%s%d%s", LEFT_STAGEMARKER, stage, RIGHT_STAGEMARKER); 
    printf("The grid has %d rows and %d columns.\n", nrows, ncols);
    printf("The grid has %d block(s).\n", nblocks);
    printf("The initial cell in the grid is [%d,%d].\n", init.x, init.y);
    printf("The goal cell in the grid is [%d,%d].\n", goal.x, goal.y);
    printf("The proposed route in the grid is:\n");
    print_list(route->start);
    print_status(status); 

    /* STAGE 1 */
    stage = 1;
    printf("%s%d%s", LEFT_STAGEMARKER, stage, RIGHT_STAGEMARKER); 
    print_grid(route, &grid, nrows, ncols);

    if (status != 4) {
        /* stop at stage 1 if invalid route */
        if (status != 5) {
            terminate = 1;
        }

    } else {
        /* fix the grid route at the first broken segment and evaluate*/
        fix_seg(route, &grid, nrows, ncols);
        status = get_route_status(route, &grid);

        /* print remaining stage 1 output */
        printf("%s", PART_DELIM);
        print_grid(route, &grid, nrows, ncols);
        printf("%s", PART_DELIM);
        print_list(route->start);
        print_status(status); 
    }
    
    /* STAGE 2 */
    char c;
    int more=0;
    more = scanf("%c\r", &c);

    /* no more input or invalid route */
    if (more!=1 || terminate==1){
        printf("%s", END_DELIM);
    }

    /* oo there's more input */
    else {
        stage=2;
        printf("%s%d%s", LEFT_STAGEMARKER, stage, RIGHT_STAGEMARKER); 

        /* while there are more blocks */
        do {    
            /* insert blocks, initial and goal cell into grid */
            get_blocks(nrows, ncols, &grid);
            grid_insert(&grid, &init, INITIAL_CELL);
            grid_insert(&grid, &goal, GOAL_CELL);

            /* print new grid with the route and check status */
            print_grid(route, &grid, nrows, ncols);           
            status = get_route_status(route, &grid);

            /* block on the route */
            if (status==4) {

                /* fix and evaluate route */
                fix_route(route, &grid, nrows, ncols);
                status = get_route_status(route, &grid);

                /* print grid with new route*/ 
                printf("%s", PART_DELIM);
                print_grid(route, &grid, nrows, ncols);
                printf("%s", PART_DELIM);

                if (status==5) {
                    /* print route and status if repaired */
                    print_list(route->start);
                    print_status(status);
                } 
                else {
                    /* route cannot be fixed */
                    printf("%s\n", BROKEN_ROUTE);
                }
            } 
            printf("%s", END_DELIM);

        } while ((more=scanf("%c\r", &c)) == 1);
    }

    /* free all memory */
    free_list(route);
    free_grid(&grid, nrows, ncols);

	return 0;
}

/******************************************************************************/

/* reads in the coordinates of a initial, goal and block cells 
 */

int
get_cell (cell_t *cell){
    int nvals=0, x, y;
    
    nvals = scanf("[%d,%d]\n", &x, &y);

    /* enter cell data if values read */
    if (nvals == 2) {
        cell->x = x;
        cell->y = y;

    /* look out for part delimiter */
    } else {
        return EOF;
    } 

    return 0;  
}

/******************************************************************************/

/* reads the coordinates of the block and adds it to the 2D grid 
 */

int
get_blocks(int nrows, int ncols, char ***grid) {
    cell_t block;
    int nblocks=0, i, j;

    /* clears the grid from previous blocks */
    for (i=0; i<nrows; i++) {
        for (j=0; j<ncols; j++) {
            (*grid)[i][j] = BLANK;
        }
    }

    /* inserts new blocks into the grid */
    while (get_cell(&block) != EOF) {
        grid_insert(grid, &block, BLOCK_CELL);
        nblocks++;
    }

    return nblocks;
}

/******************************************************************************/

/* reads in route cells 
 */

int
get_route_cell(cell_t *cell) {
    int nvals;
    
    nvals = scanf("[%d,%d]->", &cell->x, &cell->y);
    
    return nvals;
}

/******************************************************************************/

/* reads in the cells that form the route and make puts the coordinates into a
 * singly-linked list
 */

list_t
*get_route(list_t *route) {
    int nvals;
    int end_route=0;
    char c;
    cell_t cell;

    /* aligns stdin with read format */
    scanf("%c\r", &c); 

    /* read route cells until you reach the end */
    while (end_route == 0) {

        /* if cell read, add it to route list */
        if ((nvals=get_route_cell(&cell)) == 2) {
            route = insert_at_tail(route, &cell);

        } else if (((scanf("%c", &c)) == 1) && ((c=='\n') || (c=='\r'))) {
            /* keep reading in input */

        } else {
            end_route = 1;
        }
    }

    return route;
}

/******************************************************************************/

/* creates an 2D grid of pointers of specified dimensions 
 */

int
make_grid(int nrows, int ncols, char ***grid) {
    int i, j;

    /* allocate space fot the grid */
    *grid = (char**)malloc(sizeof(char*)*nrows);
    for (i=0; i<nrows; i++) {
        (*grid)[i] = (char*)malloc(sizeof(char)*ncols);
    }

    /* inserts blanks into the grid */
    for (i=0; i<nrows; i++) {
        for (j=0; j<ncols; j++) {
            (*grid)[i][j] = BLANK;
        }
    }

    return 0;
}

/******************************************************************************/

/* insert a cell into the grid
 */

int
grid_insert(char ***grid, cell_t *cell, char cell_type) {
    
    (*grid)[cell->x][cell->y] = cell_type;

    return 0;
}

/******************************************************************************/

/* creates an empty list, adapted from Programming, Problem Solving, and 
 * Abstraction by A. Moffat (2013), page 172
 */

list_t
*make_empty_list(void) {
    list_t *list;

    list = (list_t*)malloc(sizeof(*list));
    assert(list!=NULL);
    list->start = list->end = NULL;

    return list;
}

/******************************************************************************/

/* creates an empty queue
 */

queue_t
*make_empty_queue(void) {
    queue_t *queue;

    queue = (queue_t*)malloc(sizeof(*queue));
    assert(queue!=NULL);
    queue->start = queue->end = NULL;

    return queue;
}

/******************************************************************************/

/* inserts data into the head of the list, adapted from Programming, Problem 
 * Solving, and Abstraction by A. Moffat (2013), page 172
 */

list_t
*insert_at_head(void *list, void *data) {
    node_t *new;
    cell_t *curr_cell=data;
    list_t *route = list;

    new = (node_t*)malloc(sizeof(*new));
    assert(route!=NULL && new!=NULL);

    new->cell = *curr_cell;
    new->next = route->start;
    route->start = new;

    if (route->end==NULL) {
        /* first entry */
        route->end = new;
    }

    return route;
}

/******************************************************************************/
/* inserts the next cell of the route into the foot of the list, adapted from 
 * Programming, Problem Solving, and Abstraction by A. Moffat (2013), page 172
 */

list_t
*insert_at_tail(void *list, void *data) {
    node_t *new;
    cell_t *curr_cell=data;
    list_t *route = list;

    new = (node_t*)malloc(sizeof(*new));
    assert(route!=NULL && new!=NULL);
    new->cell = *curr_cell;
    new->next = NULL;

    if (route->end==NULL) {
        /* first entry */
        route->start = route->end = new;
    } else {
        route->end->next = new;
        route->end = new;
    }

    return route;
}

/******************************************************************************/

/* inserts the next item in queue into the end of the queue
 */

queue_t
*insert_queue(void *list, void *data) {
    qnode_t *new;
    qitem_t *curr=data;
    queue_t *queue = list;

    new = (qnode_t*)malloc(sizeof(*new));
    assert(queue!=NULL && new!=NULL);
    new->qitem = *curr;
    new->next = NULL;

    if (queue->end==NULL) {
        /* first entry */
        queue->start = queue->end = new;
    } else {
        queue->end->next = new;
        queue->end = new;
    }

    return queue;
}

/******************************************************************************/

/* frees the space allocated to a list, adapted from Programming, Problem 
 * Solving, and Abstraction by A. Moffat (2013), page 172 
 */

void
free_list(list_t *list) {
    node_t *curr, *prev;

    assert (list!=NULL);
    curr = list->start;

    while (curr) {
        /* traverse list and free each node */
        prev = curr;
        curr = curr->next;
        free(prev);
    }
    free(list);
}

/******************************************************************************/

/* frees the space allocated to a queue
 */

void 
free_queue(queue_t *queue) {
    qnode_t *curr, *prev;

    assert (queue!=NULL);
    curr = queue->start;

    while (curr) {
        /* go down queue and free each item */
        prev = curr;
        curr = curr->next;
        free(prev);
    }
    free(queue);
}

/******************************************************************************/

/* frees the space allocated to the grid 
 */

void 
free_grid(char ***grid, int nrows, int ncols) {
    int i;

    for (i=0; i< nrows; i++) {
        free((*grid)[i]);
    }
    free(*grid);
}

/******************************************************************************/

/* prints out a all elements of a list in the specified format of a route 
 */

int
print_list(node_t *start) {
    node_t *curr = start;
    int cell_count=0;
    
    /* traverse list while printing until you reach the end */
    while (curr!=NULL) {

        printf("[%d,%d]", curr->cell.x, curr->cell.y);
        cell_count++;
        
        /* print period after last cell and arrow after every other cell */
        if (curr->next == NULL) {
            printf("%s", ROUTE_END);
            /* avoid double printing newline */
            if (cell_count!=5) { 
                printf("\n");
            }
        } else {
            printf("%s", ARROW);
        }
        
        /* newline after every 5 cells printed */
        if (cell_count==5) {
            printf("\n");
            cell_count=0;
        }

        curr = curr->next;
    }

    return 0;
}

/******************************************************************************/

/* Takes in a proposed route and the initial, goal and block cells in the form 
 * of a 2D array and returns the status of the route as an integer status number
 */

int
get_route_status(list_t *route, char ***grid) {
    int status=0;
    node_t *curr;

    /* check status 1: wrong initial cell */
    if ((*grid)[route->start->cell.x][route->start->cell.y] != INITIAL_CELL) {
        status = 1;
        return status;
    } 

    /* check status 2: wrong goal cell */
    if ((*grid)[route->end->cell.x][route->end->cell.y] != GOAL_CELL) {
        status = 2;
        return status;
    }

    /* check status 3: illegal move */
    curr = route->start;
    while (curr->next!=NULL) {
        
        /* early exit if two route cells are not adjacent */
        if (is_adjacent(&curr->cell, &curr->next->cell)==1) {
            status = 3;
            return status;
        }
        curr = curr->next;
    }

    /* check status 4: block on the route */
    curr = route->start;
    while (curr!=NULL) {

        /* early exit there is a block on the route */
        if ((*grid)[curr->cell.x][curr->cell.y] == BLOCK_CELL) {
            status = 4;
            return status;
        }
        curr = curr->next;
    }

    /* route is valid if errors detected */
    status = 5;
    return status;
}

/******************************************************************************/

/* checks two cells if they are adjacent on the grid 
 */

int
is_adjacent(cell_t *cell1, cell_t *cell2) {
    
    if (abs(cell1->x - cell2->x) + abs(cell1->y - cell2->y) != 1) {
        /* if cells are more than one unit apart return 1 */
        return 1;
    }

    return 0;
}

/******************************************************************************/

/* prints the status of the route 
 */

int 
print_status(int status_num) {

    if (status_num==1) {
        printf("%s\n", STATUS_1);

    } else if (status_num==2) {
        printf("%s\n", STATUS_2);

    } else if (status_num==3) {
        printf("%s\n", STATUS_3);

    } else if (status_num==4) {
        printf("%s\n", STATUS_4);

    } else if (status_num==5) {
        printf("%s\n", STATUS_5);
    }

    return 0;
}

/******************************************************************************/

/* prints a visualization of the input route in the grid using ASCII characters
 */

int
print_grid(list_t *route, char ***grid, int nrows, int ncols) {
    int i, j;
    node_t *curr;

    /* adds the route to the grid */
    curr = route->start;
    while (curr!=NULL) {

        if ((*grid)[curr->cell.x][curr->cell.y] == BLANK) {
            grid_insert(grid, &curr->cell, ROUTE);
        }
        curr = curr->next;      
    }

    /* print out column labels */
    printf(" ");
    for (i=0; i<ncols; i++) {
        printf("%d", i % 10);
    }
    printf("\n");

    /* print grid line by line */
    for (i=0; i<nrows; i++) {
        printf("%d", i % 10);

        for (j=0; j<ncols; j++) {
            printf("%c", (*grid)[i][j]);
        }

        printf("\n");
    }

    /* clears the route from the grid */
    curr = route->start;
    while (curr!=NULL) {

        if ((*grid)[curr->cell.x][curr->cell.y] == ROUTE) {
            grid_insert(grid, &curr->cell, BLANK);
        }
        curr = curr->next;
    }

    return 0;
}

/******************************************************************************/

/* takes as input a grid and route with status 4 and returns a route fixed at 
 * the first broken segment of the route 
 */

cell_t
fix_seg(list_t *route, char ***grid, int nrows, int ncols) {
    node_t *curr;
    cell_t seg_s, add_cell;
    qitem_t qseg_s, qitem;
    qnode_t *qcurr;
    queue_t *queue;
    int count=0, fixed=0, direction=UP;

    /* find start of the first broken route segment */
    curr = route->start;
    while ((*grid)[curr->cell.x][curr->cell.y] != BLOCK_CELL) {

        if ((*grid)[curr->next->cell.x][curr->next->cell.y] == BLOCK_CELL) {
            seg_s = curr->cell;
        }
        curr = curr->next;
    }

    /* make queue and add to it the start cell of the broken segment with 
    count 0 */
    qseg_s.cell = seg_s;
    qseg_s.count = count;
    queue = make_empty_queue();
    queue = insert_queue(queue, &qseg_s);
    
    /* traverses the queue adding adjacent cells, until the last pair in the 
    queue is processed or a pair that contains a cell in the route that follows
    the start of the broken segment is added to the queue */
    qcurr = queue->start;
    count++;

    while (qcurr!=NULL) {

        add_cell = find_adj(&(qcurr->qitem.cell), direction);

        if (out_of_bounds(&add_cell, nrows, ncols) == 0 &&
            ((*grid)[add_cell.x][add_cell.y] == BLANK ||
            (*grid)[add_cell.x][add_cell.y] == GOAL_CELL) && 
            in_queue(&add_cell, queue)!=1) {
            
            /* add an adjcent cell which is on grid and blank and not already 
            in queue */
            qitem.cell = add_cell;
            qitem.count = count;
            queue = insert_queue(queue, &qitem);

            if (meet_route(route, grid, &add_cell, &seg_s) == 1) {
                /* found a repair */
                fixed = 1;
                break;
            }
        }

        direction++;
        /* next queue item every four directions */
        if ((direction = direction%4) == 0) {
            
            qcurr = qcurr->next;  
            if (&qcurr->qitem!=NULL) {
                count = qcurr->qitem.count;
                count++;
            }
        }
    }

    /* early exit if route could not be fixed */
    if (fixed==0) {
        return seg_s;
    }

    /* fix the broken segment and combine with original route */  
    list_t *fixed_seg = make_empty_list();
    fixed_seg = make_fixed_segment(queue, seg_s, nrows, ncols);
    route = insert_seg(route, fixed_seg);

    /* free all memory */
    free_queue(queue);

    return seg_s;
}

/******************************************************************************/

/* input a queue generated from the start of a broken segment to where it meets
 * the route and generates a fixed segment 
 */

list_t
*make_fixed_segment(queue_t *queue, cell_t seg_s, int nrows, int ncols) {
    char **temp;
    qnode_t *qcurr;
    cell_t curr_cell = queue->end->qitem.cell; 

    /* initialize fixed segment */
    list_t *fixed_segment = make_empty_list();
    fixed_segment = insert_at_tail(fixed_segment, &curr_cell);

    /* create a grid that is populated by the counts from the queue */
    make_grid(nrows, ncols, &temp);
    qcurr = queue->start;
    while (qcurr!=NULL) {
        temp[qcurr->qitem.cell.x][qcurr->qitem.cell.y] = 
            qcurr->qitem.count + '0';
        qcurr = qcurr->next;
    }

    /* track which adjacent cell has smallest count or highest priority */
    cell_t min_cell, add_cell;
    int min_count = queue->end->qitem.count; 
    char c; 
    int direction;
    
    /* until you reach the start of broken segment */
    while (fixed_segment->start->cell.x != seg_s.x ||
        fixed_segment->start->cell.y != seg_s.y) {
        
        direction = RIGHT; /* lowest priority */
        while (direction >= 0) {

            /* check the adjacent cells */
            add_cell = find_adj(&curr_cell, direction);

            if (out_of_bounds(&add_cell, nrows, ncols) == 0) {
                c = temp[add_cell.x][add_cell.y]; 

                if (c && c!=BLANK && c-'0' <= min_count) {
                /* new min cell has smaller count and higher priority*/

                    min_cell = add_cell;
                    min_count = c-'0';
                }
            }

            direction--;
        }       
        fixed_segment = insert_at_head(fixed_segment, &min_cell);
        curr_cell = min_cell;
    } 

    free_grid(&temp, nrows, ncols);

    return fixed_segment;
}

/******************************************************************************/

/* checks if the cell added is in the route that follows the start of the 
 * broken segment of the queue 
 */

int
meet_route(list_t *route, char ***grid, cell_t *add_cell, cell_t *route_start) {
    node_t *curr;

    /* identify the node where the broken segment begins */
    curr = route->start;
    while (curr->cell.x != route_start->x || curr->cell.y != route_start->y) {
        curr = curr->next;
    }
    curr = curr->next->next;
    
    /* check if added cell meets any cell in the route after broken segment*/
    while (curr!=NULL) {
        
        /* return 1 if added cell meets a cell in route that is not blocked */
        if (((*grid)[curr->cell.x][curr->cell.y] == BLANK || 
            (*grid)[curr->cell.x][curr->cell.y] == GOAL_CELL) && 
            curr->cell.x == add_cell->x &&
            curr->cell.y == add_cell->y) {
            
            return 1;
        }
        curr = curr->next;
    }

    return 0;
}

/******************************************************************************/

/* returns a cell that is adjacent to the cell provided in the direction 
 * specified 
 */

cell_t
find_adj(cell_t *curr_cell, int direction) {
    cell_t adj_cell;

    /* cell above */
    if (direction == UP) {
        adj_cell.x = curr_cell->x - 1;
        adj_cell.y = curr_cell->y;
    } 
    /* cell below */
    else if (direction == DOWN) {
        adj_cell.x = curr_cell->x + 1;
        adj_cell.y = curr_cell->y;
    }
    /* cell to the left */
    else if (direction == LEFT) {
        adj_cell.x = curr_cell->x;
        adj_cell.y = curr_cell->y - 1;
    }
    /* cell to the right */
    else if (direction == RIGHT) {
        adj_cell.x = curr_cell->x;
        adj_cell.y = curr_cell->y + 1;
    }

    return adj_cell;
}

/******************************************************************************/

/* checks if the cell is already in the queue 
 */

int
in_queue(cell_t *cell, queue_t *queue) {
    qnode_t *curr;

    curr = queue->start;
    while(curr!=NULL) {

        if (curr->qitem.cell.x == cell->x &&  curr->qitem.cell.y == cell->y) {
            /* return 1 if cell is found in queue */
            return 1;
        }
        curr = curr->next;
    }

    return 0;
}

/******************************************************************************/

/* returns 1 if the cell is not within bounds of the grid 
 */

int
out_of_bounds(cell_t *cell, int nrows, int ncols) {

    if (cell->x >= nrows || cell->x < 0 || cell->y >= ncols || cell->y < 0) {
        return 1;
    }

    return 0;
}

/******************************************************************************/

/* Takes in a list and a segment which at least two overlapping nodes and 
 * inserts the segment into the list
 */ 

list_t
*insert_seg(list_t *route, list_t *seg) {
    node_t *curr;

    /* append the end of the route to the end of the segment */
    curr = route->start;
    while (curr->cell.x != seg->end->cell.x || 
        curr->cell.y != seg->end->cell.y) {

        curr = curr->next;
    }
    seg->end->next = curr->next;

    /* append the start of the route to the start of the segment */
    curr = route->start;
    while (curr->cell.x != seg->start->cell.x || 
        curr->cell.y != seg->start->cell.y) {

        curr = curr->next;
    }
    curr->next = seg->start->next;
    
    return route;
}

/******************************************************************************/

/* fixes the route completely if it is fixable, returns 1 otherwise
 */

int
fix_route(list_t *route, char ***grid, int nrows, int ncols) {
    cell_t seg_s1, seg_s2; /*cell before first broken segment */
    seg_s1 = fix_seg(route, grid, nrows, ncols);
    
    /* fix segments until route is completely fixed */
    while (get_route_status(route, grid) != 5) {
        
        seg_s2 = fix_seg(route, grid, nrows, ncols);
        
        /* early exit the same segment is broken as last time */
        if (seg_s1.x==seg_s2.x && seg_s1.y==seg_s2.y) {
            return 1;
        } 
        else {
            seg_s1 = seg_s2;
        }
    }

    return 0;
}

/******************************************************************************/

/* algorithms are fun */
