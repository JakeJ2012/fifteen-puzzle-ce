#include <tice.h>
#include <graphx.h>
#include <keypadc.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

typedef enum gameState
{
    NOT_PLAYING = 0,
    PLAYING,
    EXITING,
    JUST_COMPLETED
} gameState;

void newShuffledBoard(uint8_t board[16]);
void tick(gameState *state, uint8_t board[16], int *selected, uint8_t keys[8], uint8_t lastKeys[8]);
int isPressed(uint8_t keys[8], int index, int mask);
int isNewPress(uint8_t keys[8], uint8_t lastKeys[8], int index, int mask);
int isHeldPress(uint8_t keys[8], uint8_t lastKeys[8], int index, int mask);
void newSolvedBoard(uint8_t board[16]);
void printCentered(char *text, int x, int y);
void renderBoard(uint8_t board[16], int selected, bool highlightSelected);
void swapBlankWith(int i, uint8_t board[16]);
bool validateBoard(uint8_t board[16]);

int main(void)
{

    gfx_SetDrawBuffer();

    srand(time(NULL));

    gfx_Begin();

    bool running = 1;

    gameState state = NOT_PLAYING;

    int selected = 0;

    uint8_t keys[8] = {0};
    uint8_t lastKeys[8] = {0};

    uint8_t board[16] = {0};
    newSolvedBoard(board);

    while (running)
    {
        kb_Scan();
        for (int i = 0; i < 8; i++)
        {
            lastKeys[i] = keys[i];
            keys[i] = kb_Data[i];
        }
        tick(&state, board, &selected, keys, lastKeys);
        if (state == EXITING)
        {
            running = false;
        }
        gfx_SwapDraw();
    }

    gfx_End();
    return 0;
}

void tick(gameState *state, uint8_t board[16], int *selected, uint8_t keys[8], uint8_t lastKeys[8])
{
    gfx_FillScreen(0);
    gfx_SetColor(43);

    gfx_FillRectangle(67, 5, 185, 185);
    gfx_FillRectangle(5, 195, 310, 40);

    gfx_SetColor(138);

    gfx_SetTextFGColor(0);
    gfx_SetTextBGColor(1);
    gfx_SetTextTransparentColor(1);

    gfx_PrintStringXY("[Enter] = play", 6, 196);
    gfx_PrintStringXY("[Clear] = exit/end game", 6, 206);
    gfx_PrintStringXY("Sort the tiles in order with the blank tile", 6, 216);
    gfx_PrintStringXY("at the bottom right", 6, 226);

    if (*state == PLAYING)
    {
        renderBoard(board, *selected, true);

        if (isNewPress(keys, lastKeys, 6, kb_Clear))
        {
            *state = NOT_PLAYING;
            newSolvedBoard(board);
            *selected = 0;
        }

        if (*state == PLAYING)
        {

            int dx = (int)isNewPress(keys, lastKeys, 7, kb_Right) - (int)isNewPress(keys, lastKeys, 7, kb_Left);
            int dy = (int)isNewPress(keys, lastKeys, 7, kb_Down) - (int)isNewPress(keys, lastKeys, 7, kb_Up);

            int y = *selected / 4;
            int x = *selected % 4;

            x += dx;
            y += dy;

            if (x < 0)
            {
                x = 0;
            }
            else if (x > 3)
            {
                x = 3;
            }

            if (y < 0)
            {
                y = 0;
            }
            else if (y > 3)
            {
                y = 3;
            }

            int i = 4 * y + x;

            *selected = i;

            if (isNewPress(keys, lastKeys, 6, kb_Enter) || isNewPress(keys, lastKeys, 1, kb_2nd))
            {
                swapBlankWith(*selected, board);
            }

            bool isSolved = validateBoard(board);

            if (isSolved)
            {
                *state = JUST_COMPLETED;
            }
        }
    }
    else if (*state == NOT_PLAYING)
    {
        renderBoard(board, *selected, false);

        if (isNewPress(keys, lastKeys, 6, kb_Enter))
        {
            newShuffledBoard(board);
            *selected = 0;
            *state = PLAYING;
        }
        if (isNewPress(keys, lastKeys, 6, kb_Clear))
        {
            *state = EXITING;
        }
    }
    else
    {
        printCentered("Completed!", 160, 50);
        printCentered("Press [Enter] to play again", 160, 120);
        printCentered("or press [Clear] to exit", 160, 130);

        if (isNewPress(keys, lastKeys, 6, kb_Clear))
        {
            *state = EXITING;
        }
        else if (isNewPress(keys, lastKeys, 6, kb_Enter))
        {
            newShuffledBoard(board);
            *selected = 0;
            *state = PLAYING;
        }
    }
}

void renderBoard(uint8_t board[16], int selected, bool highlightSelected)
{
    gfx_SetTextFGColor(0);
    gfx_SetTextBGColor(1);
    gfx_SetTextTransparentColor(1);
    char buf[3];
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            int index = y * 4 + x;
            uint8_t value = board[index];

            int tileX = 72 + x * 45;
            int tileY = 10 + y * 45;

            if (index == selected && highlightSelected)
            {
                gfx_SetColor(198);

                gfx_Rectangle(tileX - 1, tileY - 1, 42, 42);
            }

            if (value == 0)
            {
                continue;
            }

            if (value == index + 1)
            {
                gfx_SetColor(46);
            }
            else
            {
                gfx_SetColor(138);
            }

            gfx_FillRectangle(tileX, tileY, 40, 40);

            snprintf(buf, sizeof(buf), "%d", value);
            printCentered(buf, tileX + 20, tileY + 20 - 4);
        }
    }
}

void newShuffledBoard(uint8_t board[16])
{
    for (int i = 0; i < 16; i++)
    {
        board[i] = i;
    }

    for (int i = 15; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int temp = board[i];
        board[i] = board[j];
        board[j] = temp;
    }

    int blankRow;
    int inversions = 0;

    for (int i = 0; i < 16; i++)
    {
        uint8_t at = board[i];
        if (at == 0)
        {
            blankRow = i / 4;
        }

        for (int j = i + 1; j < 16; j++)
        {
            if (at == 0 || board[j] == 0)
            {
                continue;
            }
            if (board[j] < at)
            {
                inversions++;
            }
        }
    }

    int fromBottom = 3 - blankRow;

    if (((fromBottom + inversions) % 2))
    {
        int i1 = -1;
        int i2 = -1;

        for (int i = 0; i < 16; i++)
        {
            if (board[i] != 0)
            {
                if (i1 == -1)
                    i1 = i;
                else
                {
                    i2 = i;
                    break;
                }
            }
        }

        int tmp = board[i2];
        board[i2] = board[i1];
        board[i1] = tmp;
    }
}

int isNewPress(uint8_t keys[8], uint8_t lastKeys[8], int index, int mask)
{
    return (keys[index] & mask) && !(lastKeys[index] & mask);
}

int isPressed(uint8_t keys[8], int index, int mask)
{
    return (keys[index] & mask);
}

void newSolvedBoard(uint8_t board[16])
{
    for (int i = 0; i < 15; i++)
    {
        board[i] = i + 1;
    }

    board[15] = 0;
}

void printCentered(char *text, int x, int y)
{
    gfx_PrintStringXY(text, x - gfx_GetStringWidth(text) / 2, y);
}

void swapBlankWith(int i, uint8_t board[16])
{
    int blankIndex = -1;

    for (int j = 0; j < 16; j++)
    {
        if (!board[j])
        {
            blankIndex = j;
            break;
        }
    }

    if ((blankIndex - i) == 1 || (blankIndex - i) == -1 || (blankIndex - i) == 4 || (blankIndex - i) == -4)
    {
        board[blankIndex] = board[i];
        board[i] = 0;
    }
}

bool validateBoard(uint8_t board[16])
{
    for (int i = 0; i < 15; i++)
    {
        if (board[i] != (uint8_t)(i + 1))
        {
            return false;
        }
    }

    return board[15] == 0;
}