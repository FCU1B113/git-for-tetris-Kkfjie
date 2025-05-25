#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <windows.h>

#define CANVAS_WIDTH 10
#define CANVAS_HEIGHT 20

#define FALL_DELAY 500
#define RENDER_DELAY 100

// 按鍵定義
#define LEFT_KEY 0x25
#define RIGHT_KEY 0x27 
#define ROTATE_KEY 0x26 
#define DOWN_KEY 0x28 
#define FALL_KEY 0x20 

// 判斷按鍵是否被按下的函數
#define LEFT_FUNC() GetAsyncKeyState(LEFT_KEY) & 0x8000
#define RIGHT_FUNC() GetAsyncKeyState(RIGHT_KEY) & 0x8000
#define ROTATE_FUNC() GetAsyncKeyState(ROTATE_KEY) & 0x8000
#define DOWN_FUNC() GetAsyncKeyState(DOWN_KEY) & 0x8000
#define FALL_FUNC() GetAsyncKeyState(FALL_KEY) & 0x8000

typedef enum
{
    RED = 41,
    GREEN,
    YELLOW,
    BLUE,
    PURPLE,
    CYAN,
    WHITE,
    BLACK = 0,
} Color;

typedef enum
{
    EMPTY = -1,
    I,
    J,
    L,
    O,
    S,
    T,
    Z
} ShapeId;

typedef enum
{
    SEEDS = 0,
    PLANT = 1,
    FLOWER = 2
} PlantStage;

typedef struct
{
    ShapeId shape;
    Color color;
    int size;
    char rotates[4][4][4];
} Shape;

typedef struct
{
    int x;
    int y;
    int score;
    int rotate;
    int fallTime;
    ShapeId queue[4];
    PlantStage plantStage;
} State;

typedef struct
{
    Color color;
    ShapeId shape;
    bool current;
} Block;

Shape shapes[7] = {
    {.shape = I,
     .color = CYAN,
     .size = 4,
     .rotates =
         {
             {{0, 0, 0, 0},
              {1, 1, 1, 1},
              {0, 0, 0, 0},
              {0, 0, 0, 0}},
             {{0, 0, 1, 0},
              {0, 0, 1, 0},
              {0, 0, 1, 0},
              {0, 0, 1, 0}},
             {{0, 0, 0, 0},
              {0, 0, 0, 0},
              {1, 1, 1, 1},
              {0, 0, 0, 0}},
             {{0, 1, 0, 0},
              {0, 1, 0, 0},
              {0, 1, 0, 0},
              {0, 1, 0, 0}}}},
    {.shape = J,
     .color = BLUE,
     .size = 3,
     .rotates =
         {
             {{1, 0, 0},
              {1, 1, 1},
              {0, 0, 0}},
             {{0, 1, 1},
              {0, 1, 0},
              {0, 1, 0}},
             {{0, 0, 0},
              {1, 1, 1},
              {0, 0, 1}},
             {{0, 1, 0},
              {0, 1, 0},
              {1, 1, 0}}}},
    {.shape = L,
     .color = YELLOW,
     .size = 3,
     .rotates =
         {
             {{0, 0, 1},
              {1, 1, 1},
              {0, 0, 0}},
             {{0, 1, 0},
              {0, 1, 0},
              {0, 1, 1}},
             {{0, 0, 0},
              {1, 1, 1},
              {1, 0, 0}},
             {{1, 1, 0},
              {0, 1, 0},
              {0, 1, 0}}}},
    {.shape = O,
     .color = WHITE,
     .size = 2,
     .rotates =
         {
             {{1, 1},
              {1, 1}},
             {{1, 1},
              {1, 1}},
             {{1, 1},
              {1, 1}},
             {{1, 1},
              {1, 1}}}},
    {.shape = S,
     .color = GREEN,
     .size = 3,
     .rotates =
         {
             {{0, 1, 1},
              {1, 1, 0},
              {0, 0, 0}},
             {{0, 1, 0},
              {0, 1, 1},
              {0, 0, 1}},
             {{0, 0, 0},
              {0, 1, 1},
              {1, 1, 0}},
             {{1, 0, 0},
              {1, 1, 0},
              {0, 1, 0}}}},
    {.shape = T,
     .color = PURPLE,
     .size = 3,
     .rotates =
         {
             {{0, 1, 0},
              {1, 1, 1},
              {0, 0, 0}},

             {{0, 1, 0},
              {0, 1, 1},
              {0, 1, 0}},
             {{0, 0, 0},
              {1, 1, 1},
              {0, 1, 0}},
             {{0, 1, 0},
              {1, 1, 0},
              {0, 1, 0}}}},
    {.shape = Z,
     .color = RED,
     .size = 3,
     .rotates =
         {
             {{1, 1, 0},
              {0, 1, 1},
              {0, 0, 0}},
             {{0, 0, 1},
              {0, 1, 1},
              {0, 1, 0}},
             {{0, 0, 0},
              {1, 1, 0},
              {0, 1, 1}},
             {{0, 1, 0},
              {1, 1, 0},
              {1, 0, 0}}}},
};

void setBlock(Block* block, Color color, ShapeId shape, bool current)
{
    block->color = color;
    block->shape = shape;
    block->current = current;
}

void resetBlock(Block* block)
{
    block->color = BLACK;
    block->shape = EMPTY;
    block->current = false;
}

void printPlant(PlantStage stage, int row, int col)
{
    switch (stage)
    {
    case SEEDS:
        printf("\033[%d;%dH\033[33m", row, col);     // Yellow color for seeds
        printf("  o o  ");
        printf("\033[%d;%dH", row + 1, col);
        printf(" o o o ");
        printf("\033[%d;%dH", row + 2, col);
        printf("_______");
        break;

    case PLANT:
        printf("\033[%d;%dH\033[32m", row, col);     // Green color for plant
        printf("  \\|/  ");
        printf("\033[%d;%dH", row + 1, col);
        printf("   |   ");
        printf("\033[%d;%dH", row + 2, col);
        printf("___|___");
        break;

    case FLOWER:
        printf("\033[%d;%dH\033[35m", row, col);     // Magenta color for flower
        printf(" \\o|o/ ");
        printf("\033[%d;%dH\033[32m", row + 1, col); // Green stem
        printf("   |   ");
        printf("\033[%d;%dH", row + 2, col);
        printf("___|___");
        break;
    }
    printf("\033[0m"); // Reset color
}

void printCanvas(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], State* state)
{
    printf("\033[0;0H\n");
    for (int i = 0; i < CANVAS_HEIGHT; i++)
    {
        printf("|");
        for (int j = 0; j < CANVAS_WIDTH; j++)
        {
            printf("\033[%dm\u3000", canvas[i][j].color);
        }
        printf("\033[0m|\n");
    }

    // 顯示Score
    printf("\033[%d;%dH\033[36mScore: %d\033[0m", 1, CANVAS_WIDTH * 2 + 5, state->score);

    // 顯示Plant Stage
    printf("\033[%d;%dH\033[36mPlant Stage:", 2, CANVAS_WIDTH * 2 + 5);
    const char* stageNames[] = { "Seeds", "Plant", "Flower" };
    printf("\033[%d;%dH\033[36m%s\033[0m", 3, CANVAS_WIDTH * 2 + 5, stageNames[state->plantStage]);

    // 顯示植物ASCII圖案
    printPlant(state->plantStage, 4, CANVAS_WIDTH * 2 + 5);

    // 顯示Next:
    printf("\033[%d;%dH\033[36mNext:\033[0m", 8, CANVAS_WIDTH * 2 + 5);
    // 顯示接下來的方塊
    for (int i = 1; i <= 3; i++)
    {
        Shape shapeData = shapes[state->queue[i]];
        for (int j = 0; j < 4; j++)
        {
            printf("\033[%d;%dH", 8 + i * 4 + j, CANVAS_WIDTH * 2 + 15);
            for (int k = 0; k < 4; k++)
            {
                if (j < shapeData.size && k < shapeData.size && shapeData.rotates[0][j][k])
                {
                    printf("\x1b[%dm  ", shapeData.color);
                }
                else
                {
                    printf("\x1b[0m  ");
                }
            }
        }
    }
    return;
}

bool move(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], int originalX, int originalY, int originalRotate, int newX, int newY, int newRotate, ShapeId shapeId)
{
    Shape shapeData = shapes[shapeId];
    int size = shapeData.size;

    // 判斷新位置是否合法
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if (shapeData.rotates[newRotate][i][j])
            {
                // 判斷是否超出邊界
                if (newX + j < 0 || newX + j >= CANVAS_WIDTH || newY + i < 0 || newY + i >= CANVAS_HEIGHT)
                {
                    return false;
                }
                // 判斷是否碰到其他方塊
                if (!canvas[newY + i][newX + j].current && canvas[newY + i][newX + j].shape != EMPTY)
                {
                    return false;
                }
            }
        }
    }

    // 清除原來的位置
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if (shapeData.rotates[originalRotate][i][j])
            {
                resetBlock(&canvas[originalY + i][originalX + j]);
            }
        }
    }

    // 設置新的位置
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if (shapeData.rotates[newRotate][i][j])
            {
                setBlock(&canvas[newY + i][newX + j], shapeData.color, shapeId, true);
            }
        }
    }

    return true;
}

int clearLine(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], State* state)
{
    for (int i = 0; i < CANVAS_HEIGHT; i++)
    {
        for (int j = 0; j < CANVAS_WIDTH; j++)
        {
            if (canvas[i][j].current)
            {
                canvas[i][j].current = false;
            }
        }
    }

    int linesCleared = 0;
    for (int i = CANVAS_HEIGHT - 1; i >= 0; i--)
    {
        bool isFull = true;
        for (int j = 0; j < CANVAS_WIDTH; j++)
        {
            if (canvas[i][j].shape == EMPTY) {
                isFull = false;
                break;
            }
        }
        if (isFull) {
            linesCleared += 1;

            for (int j = i; j > 0; j--)
            {
                for (int k = 0; k < CANVAS_WIDTH; k++)
                {
                    setBlock(&canvas[j][k], canvas[j - 1][k].color, canvas[j - 1][k].shape, false);
                    resetBlock(&canvas[j - 1][k]);
                }
            }
            i++;
        }
    }

    // Handle scoring and plant growth for each line cleared
    for (int i = 0; i < linesCleared; i++)
    {
        // Add 5 points for each line cleared
        state->score += 5;

        // Advance plant stage
        if (state->plantStage == SEEDS)
        {
            state->plantStage = PLANT;
        }
        else if (state->plantStage == PLANT)
        {
            state->plantStage = FLOWER;
        }
        else if (state->plantStage == FLOWER)
        {
            // Flower stage: add 10 bonus points and reset to seeds
            state->score += 10;
            state->plantStage = SEEDS;
        }
    }

    return linesCleared;
}

void logic(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], State* state)
{
    if (ROTATE_FUNC())
    {
        int newRotate = (state->rotate + 1) % 4;
        if (move(canvas, state->x, state->y, state->rotate, state->x, state->y, newRotate, state->queue[0]))
        {
            state->rotate = newRotate;
        }
    }
    else if (LEFT_FUNC())
    {
        if (move(canvas, state->x, state->y, state->rotate, state->x - 1, state->y, state->rotate, state->queue[0]))
        {
            state->x -= 1;
        }
    }
    else if (RIGHT_FUNC())
    {
        if (move(canvas, state->x, state->y, state->rotate, state->x + 1, state->y, state->rotate, state->queue[0]))
        {
            state->x += 1;
        }
    }
    else if (DOWN_FUNC())
    {
        state->fallTime = FALL_DELAY;
    }
    else if (FALL_FUNC())
    {
        state->fallTime += FALL_DELAY * CANVAS_HEIGHT;
    }

    state->fallTime += RENDER_DELAY;

    while (state->fallTime >= FALL_DELAY)
    {
        state->fallTime -= FALL_DELAY;
        if (move(canvas, state->x, state->y, state->rotate, state->x, state->y + 1, state->rotate, state->queue[0]))
        {
            state->y++;
        }
        else
        {
            clearLine(canvas, state);

            state->x = CANVAS_WIDTH / 2;
            state->y = 0;
            state->rotate = 0;
            state->fallTime = 0;
            state->queue[0] = state->queue[1];
            state->queue[1] = state->queue[2];
            state->queue[2] = state->queue[3];
            state->queue[3] = rand() % 7;

            //檢查遊戲結束
            if (!move(canvas, state->x, state->y, state->rotate, state->x, state->y, state->rotate, state->queue[0]))
            {
                printf("\033[%d;%dH\x1b[41m GAME OVER \x1b[0m", CANVAS_HEIGHT - 3, CANVAS_WIDTH * 2 + 5);
                printf("\033[%d;%dH\x1b[36mFinal Score: %d\x1b[0m", CANVAS_HEIGHT - 2, CANVAS_WIDTH * 2 + 5, state->score);
                printf("\033[%d;%dH", CANVAS_HEIGHT + 5, 0);
                exit(0);//結束程式
            }
        }
    }
    return;
}

int main()
{
    srand(time(NULL));
    State state = {
        .x = CANVAS_WIDTH / 2,
        .y = 0,
        .score = 0,
        .rotate = 0,
        .fallTime = 0,
        .plantStage = SEEDS };

    for (int i = 0; i < 4; i++)
    {
        state.queue[i] = rand() % 7;
    }

    Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH];
    for (int i = 0; i < CANVAS_HEIGHT; i++)
    {
        for (int j = 0; j < CANVAS_WIDTH; j++)
        {
            resetBlock(&canvas[i][j]);
        }
    }

    Shape shapeData = shapes[state.queue[0]];

    for (int i = 0; i < shapeData.size; i++)
    {
        for (int j = 0; j < shapeData.size; j++)
        {
            if (shapeData.rotates[0][i][j])
            {
                setBlock(&canvas[state.y + i][state.x + j], shapeData.color, state.queue[0], true);
            }
        }
    }

    while (1)
    {
        printCanvas(canvas, &state);
        logic(canvas, &state);
        Sleep(100);
    }

    return 0;
}