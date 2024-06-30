#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROWS 6
#define COLS 7
#define CELL_SIZE 100
#define WINDOW_WIDTH (COLS * CELL_SIZE)
#define WINDOW_HEIGHT (ROWS * CELL_SIZE + CELL_SIZE)

typedef enum
{
    NONE,
    RED,
    YELLOW
} Piece;

Piece board[ROWS][COLS];
int winningPositions[4][2];

bool isTie = false;

bool initSDL(SDL_Window **window, SDL_Renderer **renderer);
void closeSDL(SDL_Window *window, SDL_Renderer *renderer);

void drawBoard(SDL_Renderer *renderer);
void drawTurnIndicator(SDL_Renderer *renderer, Piece current, Piece winner);

bool handleEvent(SDL_Event *e, int *col);
bool dropTile(int col, Piece current);

Piece checkWin();
void highlightWinningPieces(SDL_Renderer *renderer);

int main(int argc, char *args[])
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (!initSDL(&window, &renderer))
    {
        return 1;
    }

    srand(time(NULL));
    Piece current = (rand() % 2 == 0) ? RED : YELLOW;

    SDL_Event e;
    Piece winner = NONE;
    bool quit = false;

    while (!quit)
    {
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
            else
            {
                int col;
                if (handleEvent(&e, &col))
                {
                    if (dropTile(col, current))
                    {
                        winner = checkWin();
                        if (winner != NONE || isTie)
                        {
                            while (!quit)
                            {
                                while (SDL_PollEvent(&e) != 0)
                                {
                                    if (e.type == SDL_QUIT)
                                    {
                                        quit = true;
                                    }
                                }
                                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                                SDL_RenderClear(renderer);
                                drawBoard(renderer);
                                highlightWinningPieces(renderer);
                                drawTurnIndicator(renderer, current, winner);
                                SDL_RenderPresent(renderer);
                            }
                        }
                        current = (current == RED) ? YELLOW : RED;
                    }
                }
            }
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        drawBoard(renderer);
        drawTurnIndicator(renderer, current, winner);
        SDL_RenderPresent(renderer);
    }

    closeSDL(window, renderer);
    return 0;
}

bool initSDL(SDL_Window **window, SDL_Renderer **renderer)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Erro ao iniciar SDL: %s\n", SDL_GetError());
        return false;
    }

    *window = SDL_CreateWindow("Connect Four", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (*window == NULL)
    {
        printf("Erro ao criar janela: %s\n", SDL_GetError());
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (*renderer == NULL)
    {
        printf("Erro ao criar renderer: %s\n", SDL_GetError());
        return false;
    }

    SDL_SetRenderDrawColor(*renderer, 255, 255, 255, 255);
    return true;
}

void closeSDL(SDL_Window *window, SDL_Renderer *renderer)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void drawBoard(SDL_Renderer *renderer)
{
    // Tabuleiro
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (int i = 0; i <= ROWS; ++i)
    {
        SDL_RenderDrawLine(renderer, 0, i * CELL_SIZE + CELL_SIZE, COLS * CELL_SIZE, i * CELL_SIZE + CELL_SIZE);
    }
    for (int j = 0; j <= COLS; ++j)
    {
        SDL_RenderDrawLine(renderer, j * CELL_SIZE, CELL_SIZE, j * CELL_SIZE, WINDOW_HEIGHT);
    }

    // Peças
    for (int i = 0; i < ROWS; ++i)
    {
        for (int j = 0; j < COLS; ++j)
        {
            if (board[i][j] != NONE)
            {
                SDL_Rect fillRect = {j * CELL_SIZE + 2, (i + 1) * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4};
                if (board[i][j] == RED)
                {
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                }
                else if (board[i][j] == YELLOW)
                {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                }
                SDL_RenderFillRect(renderer, &fillRect);
            }
        }
    }
}

void drawTurnIndicator(SDL_Renderer *renderer, Piece current, Piece winner)
{  
    SDL_Rect turnRect = {0, 0, WINDOW_WIDTH, CELL_SIZE};
    if (isTie)
    {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    }
    else if (winner == RED)
    {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    }
    else if (winner == YELLOW)
    {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    }
    else
    {
        if (current == RED)
        {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        }
        else if (current == YELLOW)
        {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        }
    }
    SDL_RenderFillRect(renderer, &turnRect);
}

bool handleEvent(SDL_Event *e, int *col)
{
    if (e->type == SDL_MOUSEBUTTONDOWN)
    {
        int x, y;
        SDL_GetMouseState(&x, &y);
        *col = x / CELL_SIZE;
        return true;
    }
    return false;
}

bool dropTile(int col, Piece current)
{
    if (col < 0 || col >= COLS)
    {
        return false;
    }
    for (int i = ROWS - 1; i >= 0; --i)
    {
        if (board[i][col] == NONE)
        {
            board[i][col] = current;
            return true;
        }
    }
    return false;
}

Piece checkWin()
{
    // Horizontal
    for (int i = 0; i < ROWS; ++i)
    {
        for (int j = 0; j < COLS - 3; ++j)
        {
            if (board[i][j] != NONE && board[i][j] == board[i][j + 1] && board[i][j] == board[i][j + 2] && board[i][j] == board[i][j + 3])
            {
                for (int k = 0; k < 4; ++k)
                {
                    winningPositions[k][0] = i;
                    winningPositions[k][1] = j + k;
                }
                return board[i][j];
            }
        }
    }

    // Vertical
    for (int i = 0; i < ROWS - 3; ++i)
    {
        for (int j = 0; j < COLS; ++j)
        {
            if (board[i][j] != NONE && board[i][j] == board[i + 1][j] && board[i][j] == board[i + 2][j] && board[i][j] == board[i + 3][j])
            {
                for (int k = 0; k < 4; ++k)
                {
                    winningPositions[k][0] = i + k;
                    winningPositions[k][1] = j;
                }
                return board[i][j];
            }
        }
    }

    // Ascendente
    for (int i = 3; i < ROWS; ++i)
    {
        for (int j = 0; j < COLS - 3; ++j)
        {
            if (board[i][j] != NONE && board[i][j] == board[i - 1][j + 1] && board[i][j] == board[i - 2][j + 2] && board[i][j] == board[i - 3][j + 3])
            {
                for (int k = 0; k < 4; ++k)
                {
                    winningPositions[k][0] = i - k;
                    winningPositions[k][1] = j + k;
                }
                return board[i][j];
            }
        }
    }

    // Descendente
    for (int i = 0; i < ROWS - 3; ++i)
    {
        for (int j = 0; j < COLS - 3; ++j)
        {
            if (board[i][j] != NONE && board[i][j] == board[i + 1][j + 1] && board[i][j] == board[i + 2][j + 2] && board[i][j] == board[i + 3][j + 3])
            {
                for (int k = 0; k < 4; ++k)
                {
                    winningPositions[k][0] = i + k;
                    winningPositions[k][1] = j + k;
                }
                return board[i][j];
            }
        }
    }

    // Empate
    bool full = true;
    for (int i = 0; i < ROWS; ++i)
    {
        for (int j = 0; j < COLS; ++j)
        {
            if (board[i][j] == NONE)
            {
                full = false;
                break;
            }
        }
        if (!full)
        {
            break;
        }
    }
    if (full)
    {
        isTie = true;
    }

    return NONE;
}

void highlightWinningPieces(SDL_Renderer *renderer)
{
    if (isTie)
    {
        return;
    }
    for (int i = 0; i < 4; ++i)
    {
        int row = winningPositions[i][0];
        int col = winningPositions[i][1];
        SDL_Rect fillRect = {col * CELL_SIZE + 2, (row + 1) * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4};
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &fillRect);
    }
}
