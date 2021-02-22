#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LINE_LENGTH 1025
#define SIZE_EDITABLE 500000
#define INSTRUCTION_LENGTH 50
#define SIZE_TO_ADD 50000
#define NUMBER_INSTRUCTIONS 30
#define INSTRUCTIONS_TO_ADD 10
#define UNIVERSE_DIM 500000
#define DIM_STACK 500000
#define BACKUP_INDEX 50

typedef enum {CHANGE, APPEND, DELETE, USELESS, FROZEN} mode;
typedef enum {FALSE, TRUE} boolean;

struct universe{
    int top;
    int size;
    char **text;
};

struct stack_universe{
    int top;
    int size;
    struct universe *stack;
};

struct node{
    int address;
    mode mode_instruction;
    char *line;
};

struct instruction{
    int size;
    int top;
    struct node *nodes;
};

struct stack{
    int size;
    int top;
    struct instruction *instructions;
};

void redo(struct stack *undo_stack, struct stack *redo_stack, struct stack_universe *universes, int tot) // ############################################# REDO ###########################################
{
    int index = redo_stack->top - 1;
    struct instruction *temp_undo;
    struct instruction *temp_redo;
    struct universe *temp_universe;
    char *temp_string;
    temp_universe = &universes->stack[universes->top];
    for(; tot > 0; tot --)
    {
        temp_redo = &redo_stack->instructions[index];
        temp_undo = &undo_stack->instructions[undo_stack->top];
        temp_undo->top = temp_redo->top;
        temp_undo->size = temp_redo->size;
        if (temp_redo->nodes[0].mode_instruction == USELESS) // REDO OF USELESS INSTRUCTION
        {
            temp_undo->nodes = temp_redo->nodes;
        }
        else if(temp_redo->nodes[0].mode_instruction != DELETE) // REDO OF CHANGE && APPEND
        {
            temp_undo->nodes = temp_redo->nodes;
            int index_node;
            for(index_node = 0; index_node < temp_redo->top && temp_redo->nodes[index_node].mode_instruction == CHANGE; index_node ++)
            {
                temp_string = temp_redo->nodes[index_node].line;
                temp_undo->nodes[index_node].line = temp_universe->text[temp_redo->nodes[index_node].address];
                temp_universe->text[temp_redo->nodes[index_node].address] = temp_string;
            }
            while(index_node < temp_redo->top)
            {
                temp_universe->top = temp_universe->top + 1;
                temp_universe->text[temp_redo->nodes[index_node].address] = temp_redo->nodes[index_node].line;
                index_node ++;
            }
        }
        else // REDO OF DELETE
        {
            undo_stack->instructions[undo_stack->top].nodes = temp_redo->nodes;
            universes->top = universes->top + 1;
            temp_universe = &universes->stack[universes->top];
        }
        undo_stack->top = undo_stack->top + 1;
        redo_stack->top = redo_stack->top - 1;
        index --;
    }
}

//######################################################################################## END REDO ############################################################################################

void undo(struct stack *undo_stack, struct stack *redo_stack, struct stack_universe *universes, int tot) ///######################################### UNDO ######################################/////////
{
    int index = undo_stack->top - 1;
    struct instruction *temp_undo;
    struct instruction *temp_redo;
    struct universe *temp_universe;
    char *temp_line;
    temp_universe = &universes->stack[universes->top];
    for (; tot > 0; tot --)
    {
        if (redo_stack->top == redo_stack->size)
        {
            redo_stack->size = redo_stack->size + SIZE_TO_ADD;
            redo_stack->instructions = realloc(redo_stack->instructions, redo_stack->size * sizeof(struct instruction));
        }
        temp_undo = &undo_stack->instructions[index];
        temp_redo = &redo_stack->instructions[redo_stack->top];
        temp_redo->top = temp_undo->top;
        temp_redo->size = temp_undo->size;
        if(temp_undo->nodes[0].mode_instruction == USELESS) // UNDO OF A USELESS INSTRUCTION
        {
            temp_redo->nodes = temp_undo->nodes;
        }
        else if(temp_undo->nodes[0].mode_instruction != DELETE)  // UNDO OF CHANGE && APPEND
        {
            redo_stack->instructions[redo_stack->top].nodes = temp_undo->nodes;
            int index_node = temp_undo->top - 1; // TOP points to an uninitialized cell, I need to point to its left cell
            while(index_node >= 0 && temp_undo->nodes[index_node].mode_instruction == APPEND)
            {
                temp_universe->top = temp_universe->top - 1; //DECREASE INDEX OF TEXT INSTEAD OF FREEING TEXT
                index_node --;
            }
            while(index_node >= 0)
            {
                temp_line = temp_redo->nodes[index_node].line;
                temp_redo->nodes[index_node].line = temp_universe->text[temp_undo->nodes[index_node].address];
                temp_universe->text[temp_undo->nodes[index_node].address] = temp_line;
                index_node --;
            }
        }
        else // UNDO OF DELETES
        {
            redo_stack->instructions[redo_stack->top].nodes = temp_undo->nodes;
            universes->top = universes->top - 1;
            temp_universe = &universes->stack[universes->top];
        }
        index --;
        undo_stack->top = undo_stack->top - 1;
        redo_stack->top = redo_stack->top + 1;
    }
}
//############################################################################################ END UNDO ########################################################################################

int main()
{
    struct stack_universe universe;
    char buffer[LINE_LENGTH];
    char * endPointer;
    int add1, add2, index_instruction, address, index_raw, differece, instructions, temp_undo, fake_instructions, fake_top_redo, edge, size;
    boolean undo_flag = FALSE;
    char raw_add1[INSTRUCTION_LENGTH];
    char raw_add2[INSTRUCTION_LENGTH];
    char instruction[INSTRUCTION_LENGTH];
    char point[3];
    struct stack stack_undo;
    struct stack stack_redo;
    struct instruction *temp;
    struct universe *temp_universe;
    struct universe *last_universe;
    universe.stack = malloc(UNIVERSE_DIM * sizeof(struct universe));
    universe.size = UNIVERSE_DIM;
    universe.top = 0;
    universe.stack[0].text = malloc(SIZE_EDITABLE *(sizeof(char *)));
    universe.stack[0].top = 0;
    universe.stack[0].size = SIZE_EDITABLE;
    stack_undo.instructions = malloc(DIM_STACK * sizeof(struct instruction));
    stack_undo.size = DIM_STACK;
    stack_undo.top = 0;
    stack_redo.instructions = malloc(DIM_STACK * sizeof(struct instruction));
    stack_redo.size = DIM_STACK;
    stack_redo.top = 0;
    instructions = 0;
    temp_undo = 0;
    fake_top_redo = 0;
    fake_instructions = 0;
    temp_universe = &(universe.stack[0]);
    /*######################### MAIN LOOP ########################################*/

    do
    {
        fgets(instruction, INSTRUCTION_LENGTH, stdin);
        instruction[strlen(instruction) - 1] = '\0';
        add1 = strtol(instruction, &endPointer, 10);
        if(*endPointer == ',')
        {
            endPointer ++;
            add2 = strtol(endPointer, &endPointer, 10);
            if (*endPointer == 'p') //PRINT OPERATION
            {
                if(undo_flag == TRUE)
                {
                    if (temp_undo > 0)
                    {
                        undo(&stack_undo, &stack_redo, &universe, temp_undo);
                        temp_universe = &universe.stack[universe.top];
                        temp_undo = 0;
                        instructions = fake_instructions;
                        fake_top_redo = stack_redo.top;
                    }
                    else if(temp_undo < 0)
                    {
                        temp_undo = - temp_undo;
                        redo(&stack_undo, &stack_redo, &universe, temp_undo);
                        temp_universe = &universe.stack[universe.top];
                        temp_undo = 0;
                        instructions = fake_instructions;
                        fake_top_redo = stack_redo.top;
                    }
                }
                for(address = add1 - 1; address < add2; address ++)
                {
                    if(address >= temp_universe->top || address < 0) fputs(".\n", stdout);
                    else fputs(temp_universe->text[address], stdout);
                }
            }
            else if (*endPointer == 'd') // DELETE OPERATION
            {
                if (undo_flag == TRUE)
                {
                    if (temp_undo > 0)
                    {
                        undo(&stack_undo, &stack_redo, &universe, temp_undo);
                        temp_universe = &universe.stack[universe.top];
                        temp_undo = 0;
                        instructions = fake_instructions;
                    }
                    else if(temp_undo < 0)
                    {
                        temp_undo = - temp_undo;
                        redo(&stack_undo, &stack_redo, &universe, temp_undo);
                        temp_universe = &universe.stack[universe.top];
                        temp_undo = 0;
                        instructions = fake_instructions;
                    }
                    stack_redo.instructions = malloc(SIZE_EDITABLE * sizeof(struct instruction));
                    stack_redo.size = SIZE_EDITABLE;
                    stack_redo.top = 0;
                    undo_flag = FALSE;
                    fake_top_redo = 0;
                }
                (add1 < 1) ? (add1 = 0) : (add1 --);
                if(add2 >= temp_universe->top && add1 < temp_universe->top) //delete the last lines.
                {
                    if(stack_undo.size == stack_undo.top)
                    {
                        stack_undo.size = stack_undo.size + SIZE_TO_ADD;
                        stack_undo.instructions = realloc(stack_undo.instructions, stack_undo.size * sizeof(struct instruction));
                    }
                    stack_undo.instructions[stack_undo.top].nodes = malloc(sizeof(struct node));
                    temp = &stack_undo.instructions[stack_undo.top];
                    temp->nodes[0].mode_instruction = DELETE;
                    if(universe.size - 1 == universe.top)
                    {
                        universe.size = universe.size + UNIVERSE_DIM;
                        universe.stack = realloc(universe.stack, universe.size * sizeof(struct universe));
                    }
                    last_universe = temp_universe;
                    universe.top = universe.top + 1;
                    temp_universe = &universe.stack[universe.top];
                    temp_universe->size = last_universe->size;
                    temp_universe->top = last_universe->top;
                    temp_universe->text = malloc(temp_universe->size * sizeof(char *));
                    differece = temp_universe->top - add1 ;
                    for(address = 0; address < add1; address ++)
                    {
                        temp_universe->text[address] = last_universe->text[address];
                    }
                    temp_universe->top = temp_universe->top - differece;
                }
                else if(add1 >= temp_universe->top || add2 < 1) //USELESS INSTRUCTION
                {
                    if(stack_undo.size == stack_undo.top)
                    {
                        stack_undo.size = stack_undo.size + SIZE_TO_ADD;
                        stack_undo.instructions = realloc(stack_undo.instructions, stack_undo.size * sizeof(struct instruction));
                    }
                    stack_undo.instructions[stack_undo.top].nodes = malloc(sizeof(struct node));
                    temp = &stack_undo.instructions[stack_undo.top];
                    temp->nodes[0].mode_instruction = USELESS;
                }
                else if(add2 >= 1)
                {
                    if(stack_undo.size == stack_undo.top)
                    {
                        stack_undo.size = stack_undo.size + SIZE_TO_ADD;
                        stack_undo.instructions = realloc(stack_undo.instructions, stack_undo.size * sizeof(struct instruction));
                    }
                    stack_undo.instructions[stack_undo.top].nodes = malloc(sizeof(struct node));
                    temp = &stack_undo.instructions[stack_undo.top];
                    temp->nodes[0].mode_instruction = DELETE;
                    differece = add2 - add1;
                    if(universe.size - 1 == universe.top)
                    {
                        universe.size = universe.size + UNIVERSE_DIM;
                        universe.stack = realloc(universe.stack, universe.size * sizeof(struct universe));
                    }
                    last_universe = temp_universe;
                    universe.top = universe.top + 1;
                    temp_universe = &universe.stack[universe.top];
                    temp_universe->size = last_universe->size;
                    temp_universe->top = last_universe->top;
                    temp_universe->text = malloc(temp_universe->size * sizeof(char *));
                    for(address = 0; address < add1; address ++)
                    {
                        temp_universe->text[address] = last_universe->text[address];
                    }
                    edge = last_universe->top;
                    for(; add2 < edge; add2 ++)
                    {
                        temp_universe->text[address] = last_universe->text[add2];
                        address ++;
                    }
                    temp_universe->top = temp_universe->top - differece;
                }
                stack_undo.top = stack_undo.top + 1;
                instructions ++;
                fake_instructions ++;
            }
            else //CHANGE OPERATION
            {
                if (undo_flag == TRUE)
                {
                    if (temp_undo > 0)
                    {
                        undo(&stack_undo, &stack_redo, &universe, temp_undo);
                        temp_universe = &universe.stack[universe.top];
                        temp_undo = 0;
                        instructions = fake_instructions;
                    }
                    else if(temp_undo < 0)
                    {
                        temp_undo = - temp_undo;
                        redo(&stack_undo, &stack_redo, &universe, temp_undo);
                        temp_universe = &universe.stack[universe.top];
                        temp_undo = 0;
                        instructions = fake_instructions;
                    }
                    stack_redo.instructions = malloc(SIZE_EDITABLE * sizeof(struct instruction));
                    stack_redo.size = SIZE_EDITABLE;
                    stack_redo.top = 0;
                    undo_flag = FALSE;
                    fake_top_redo = 0;
                }
                if(stack_undo.size == stack_undo.top)
                {
                    stack_undo.size = stack_undo.size + SIZE_TO_ADD;
                    stack_undo.instructions = realloc(stack_undo.instructions, stack_undo.size * sizeof(struct instruction));
                }
                if (instructions % BACKUP_INDEX == 0)
                {
                    if(universe.size - 1 == universe.top)
                    {
                        universe.size = universe.size + UNIVERSE_DIM;
                        universe.stack = realloc(universe.stack, universe.size * sizeof(struct universe));
                    }
                    last_universe = temp_universe;
                    universe.top = universe.top + 1;
                    temp_universe = &universe.stack[universe.top];
                    temp_universe->size = last_universe->size;
                    temp_universe->top = last_universe->top;
                    temp_universe->text = malloc(temp_universe->size * sizeof(char *));
                    for(address = 0; address < temp_universe->top; address ++)
                    {

                    }
                }
                else {
                    stack_undo.instructions[stack_undo.top].nodes = malloc(NUMBER_INSTRUCTIONS * sizeof(struct node));
                    temp = &stack_undo.instructions[stack_undo.top];
                    temp->size = NUMBER_INSTRUCTIONS;
                    temp->top = 0;
                    for (address = add1 - 1; address < temp_universe->top && address < add2; address++)// CHANGE AN EXISTING LINE .... address is the address of the array, it is not the number of the line.
                    {
                        if (temp->size == temp->top)
                        {
                            temp->size = temp->size + INSTRUCTIONS_TO_ADD;
                            temp->nodes = realloc(temp->nodes, temp->size * sizeof(struct node));
                        }
                        temp->nodes[temp->top].address = address;
                        temp->nodes[temp->top].line = temp_universe->text[address];
                        temp->nodes[temp->top].mode_instruction = CHANGE;
                        fgets(buffer, LINE_LENGTH, stdin);
                        size = strlen(buffer) + 1;
                        temp_universe->text[address] = malloc(size);
                        memcpy(temp_universe->text[address], buffer, size);
                        temp->top = temp->top + 1;
                    }
                    if (add2 > temp_universe->top)
                    {
                        for (address = temp_universe->top; address < add2; address++)// ADD A NEW LINE
                        {
                            temp_universe->top++;
                            if (temp_universe->top > temp_universe->size) {
                                temp_universe->size = temp_universe->size + SIZE_TO_ADD;
                                temp_universe->text = realloc(temp_universe->text, temp_universe->size * sizeof(char *));
                            }
                            if (temp->size == temp->top) {
                                temp->size = temp->size + INSTRUCTIONS_TO_ADD;
                                temp->nodes = realloc(temp->nodes, temp->size * sizeof(struct node));
                            }
                            fgets(buffer, LINE_LENGTH, stdin);
                            size = strlen(buffer) + 1;
                            temp_universe->text[address] = malloc(size);
                            memcpy(temp_universe->text[address], buffer, size);
                            temp->nodes[temp->top].line = temp_universe->text[address];
                            temp->nodes[temp->top].address = address;
                            temp->nodes[temp->top].mode_instruction = APPEND;
                            temp->top = temp->top + 1;
                        }
                    }
                }
                instructions ++;
                fake_instructions ++;
                stack_undo.top = stack_undo.top + 1;
                getchar_unlocked();
            }
        }
        else if(*endPointer == 'u') //UNDO OPERATION
        {
            if (fake_instructions != 0)
            {
                if (add1 > fake_instructions) add1 = fake_instructions;
                temp_undo = temp_undo + add1;
                fake_instructions = fake_instructions - add1;
                fake_top_redo = fake_top_redo + add1;
            }
            undo_flag = TRUE;
        }
        else if(*endPointer == 'r')   //REDO OPERATION
        {
            if (fake_top_redo != 0)
            {
                if (add1 > fake_top_redo) add1 = fake_top_redo;
                temp_undo = temp_undo - add1;
                fake_instructions = fake_instructions + add1;
                fake_top_redo = fake_top_redo - add1;
            }
            undo_flag = TRUE;
        }
    } while(strcmp(instruction, "q") != 0); //QUIT
    return 0;
}