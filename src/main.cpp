#include <stdlib.h>
#include <SDL.h>

// This is a simple implementation of Conway's Game of Life.  It uses SDL2 as a
// platform layer and provides a simple interface to manipulate the simulation.
//
// Controls
// ========
//
// * Spacebar or right clicking the window pauses the simulation and puts it
//   into seed mode.
// * When in seed mode a grid is displayed and one can left click to toggle
//   the state of the cell to be either live or dead.
// * The 'f' key speeds up the simulation ('f' for faster).  This is capped
//   to be no faster than 1/10th of a second.
// * The 's' key slows down the simulation ('s' for slower).  This is capped
//   to be no slower than once per second.
//
//
// Architecturally, a bit board is used to maintain the cell world (see CellBoard for
// more).  The edges wrap to the other side, so any group of cells moving off to the
// right side of the screen will wrap around to the left, while cells moving off the
// bottom will wrap around to the top.  This works in the other direction as well.
//
// The simulation updates at twice per second unless the user manipulates the
// speed with the controls described above.  Speed changes occur in 0.1 second
// increments and are capped to the range of [0.1, 1.0].
//
// Seed mode allows the user to manipulate the board by toggling the states of
// of cells.  While in seed mode no updates take place and the simulation remains
// in a stand still.  Seed mode becomes apparent as a grid is drawn on the window.
//

// CellBoard represents the board used in Conways's Game of Life.  In represents
// infinite edges by wrapping the left border to to the right, wrapping the top to
// the bottom, and vice-a-versa for both directions.
//
// CellBoard uses a coordinate system much like a drawing buffer on a screen with
// (0, 0) coordinate being the top left of the board.  An 'x' coordinate is along one
// of the board's columns, while a 'y' coordinate is along one of the board's rows.
//
// Every cell within the board can have a value of 0 or 1.  In essence, CellBoard
// is a bit board but also resembles a rendering architecture in that it uses double
// buffering to maintain the board.  Changes made with ChangeCellTo can't be read
// with calls to CellAt until a call to SwitchBuffer is made.
struct CellBoard {
    static const int EDGE_SZ = 64;

    Uint8 board[(EDGE_SZ * EDGE_SZ / 8) + 1] = {0};
    Uint8 backbuf[(EDGE_SZ * EDGE_SZ / 8) + 1] = {0};

    int Rows()    { return EDGE_SZ; }
    int Columns() { return EDGE_SZ; }

    int adjust(int val, int max) {
        auto v = val % max;
        if (v < 0) {
            v += max;
        }
        return v;
    }

    int  CellAt(int x, int y);
    void ChangeCellTo(int x, int y, int newValue);
    void CopyBuffer();
    void SwitchBuffer();
};


// CellAt returns the value at the (x, y) coordinate of the cell
// board.  X can be interpreted as the column that starts at zero and
// goes to Columns() - 1.  Y can be interpreted as the row that starts
// at zero and goes to Rows() - 1.  If x or y are out of these bounds
// it will wrap around to the start or end.
int CellBoard::CellAt(int x, int y) {
    auto xr = adjust(x, Columns());
    auto yr = adjust(y, Rows());
    auto off = xr * EDGE_SZ + yr;
    auto idx = off / 8;
    auto bit = off % 8;
    auto val = board[idx] & (1UL << bit);

    return val > 0 ? 1 : 0;
}

// ChangeCellTo changes the value at the (x, y) coordinate to value.
// Note this change is written to a back buffer and isn't reflected,
// such as a call from CellAt, until SwitchBuffer is called.
void CellBoard::ChangeCellTo(int x, int y, int value) {
    auto xr  = adjust(x, Columns());
    auto yr  = adjust(y, Rows());
    auto off = xr * EDGE_SZ + yr;
    auto idx = off / 8;
    auto bit = off % 8;

    if (value == 0) {
        backbuf[idx] &= ~(1UL << bit);
    } else {
        backbuf[idx] |= 1UL << bit;
    }
}

// CopyBuffer copies the current board to the back buffer of the
// board.  Any previous to the back buffer with calls to ChangeCellTo
// will be overwritten.
void CellBoard::CopyBuffer() {
    auto len = sizeof(backbuf) / sizeof(backbuf[0]);
    for (int i = 0; i < len; i++) {
        backbuf[i] = board[i];
    }
}

// SwitchBuffer copies the back buffer to the CellBoard's main buffer.
// The back buffer is zeroed out duing this process.
void CellBoard::SwitchBuffer() {
    auto len = sizeof(backbuf) / sizeof(backbuf[0]);
    for (int i = 0; i < len; i++) {
        board[i] = backbuf[i];
        backbuf[i] = 0;
    }
}
        
// Life maintains the state for Conway's Game of Life.  It has two purposes:
// to update the cell simulation and to render the display.
struct Life {
    bool          haveErr   = false;   // Error encontered when manipulating the app.  Resets at every render call.
    bool          seed_mode = false;   // Whether the user can seed the board.
    int           width     = 0;       // Current draw width on the screen.
    int           height    = 0;       // Current draw height on the screen.
    SDL_Renderer *renderer  = nullptr; // Screen renderer.

    CellBoard board;

    const static int CELL_SZ = 12;

    Life(SDL_Renderer *r) : renderer(r) {
        auto x = board.Columns() / 2;
        auto y = board.Rows() / 2;

        board.ChangeCellTo(x, y-1, 1);
        board.ChangeCellTo(x, y,   1);
        board.ChangeCellTo(x, y+1, 1);
    }

    // Simulation interface

    void Update(double t, double dt);
    void ToggleSeed() { seed_mode = !seed_mode; }
    void SeedCell(int x, int y);

    // Rendering interface

    void Render();
    void SetDrawColor(int r, int g, int b, int alpha);
    void RenderClear();
    void FillRect(int x, int y, int w, int h);
    void DrawLine(int x1, int y1, int x2, int y2);
    void DrawGrid();
};

// SeedCell toggles the cell at the (x, y) coordinates on the
// cell board.  This method is a no op if seed mode is not active.
void Life::SeedCell(int x, int y) {
    if (!seed_mode) {
        return;
    }

    auto x_ = x / CELL_SZ;
    auto y_ = y / CELL_SZ;
    auto c  = board.CellAt(x_, y_);

    if (c > 0) {
        c = 0;
    } else {
        c = 1;
    }
    
    board.CopyBuffer();
    board.ChangeCellTo(x_, y_, c);
    board.SwitchBuffer();
}

// Update updates the simulation by examining every cell in the world
// and changes their state based off rules from Conway's Game of Life,
// which is described as follows:
//
// 1)  If a cell is live and it has 2 or 3 live cell neighbors then the
//     cell continues to live.
// 2)  If a cell is dead and it has 3 live cell neighbors then the cell
//     becomes live.
// 3)  Otherwise, the cell dies.
//
// Note that a neighbor is described as any cell that is immediately to
// the left, right, top, bottom, or either of the top or bottom diagonal
// positions to the cell.  In our world, the edges wrap, so a cell on the
// far right of the screen has neighbor on the far left side of the screen
// along the same axis.
//
// This method is a no-op and immediately returns if seed mode is active.
void Life::Update(double t, double dt) {
    if (seed_mode) {
        return;
    }

    for (int x = 0; x < board.Columns(); x++) {
        for (int y = 0; y < board.Rows(); y++) {
            auto cnt = 0;
            if (board.CellAt(x-1, y-1) != 0) cnt++;
            if (board.CellAt(x,   y-1) != 0) cnt++;
            if (board.CellAt(x+1, y-1) != 0) cnt++;
            if (board.CellAt(x-1, y)   != 0) cnt++;
            if (board.CellAt(x+1, y)   != 0) cnt++;
            if (board.CellAt(x-1, y+1) != 0) cnt++;
            if (board.CellAt(x,   y+1) != 0) cnt++;
            if (board.CellAt(x+1, y+1) != 0) cnt++;

            auto v = board.CellAt(x, y);
            // Live cell with 2 or 3 live neighbors survives.
            if (v == 1 && (cnt == 2 || cnt == 3)) {
                board.ChangeCellTo(x, y, 1);
            } else if (v == 0 && cnt == 3) {
                board.ChangeCellTo(x, y, 1);
            }
        }
    }
    board.SwitchBuffer();
}

// Render draws the current state of the simulation.  Every live
// cell is rendered to the screen.  If seed mode is active then a
// cell grid is rendered as well.
void Life::Render() {
    if (SDL_GetRendererOutputSize(renderer, &width, &height) < 0) {
        SDL_Log("ERROR: SDL_GetRendererOutputSize: %s\n", SDL_GetError());
        return;
    }
    haveErr = false;

    SetDrawColor(255, 255, 255, 255);
    RenderClear();
    SetDrawColor(0, 0, 0, 255);

    for (int x = 0; x < board.Columns(); x++) {
        for (int y = 0; y < board.Rows(); y++) {
            if (board.CellAt(x, y) != 0) {
                auto wd = CELL_SZ - 2;
                FillRect(x * CELL_SZ + 1, y * CELL_SZ + 1, wd, wd);
            }
        }
    }

    if (seed_mode) {
        DrawGrid();
    }
    SDL_RenderPresent(renderer);
}

// DrawGrid is a render sub-routine to draw a cell grid on the display.
void Life::DrawGrid() {
    for (int y = 0; y < board.Rows() + 1; y++) {
        auto y1 = y * CELL_SZ;
        auto x1 = 0;
        auto x2 = CELL_SZ * board.Columns();
        DrawLine(x1, y1, x2, y1);
    }
    for (int x = 0; x < board.Columns() + 1; x++) {
        auto x1 = x * CELL_SZ;
        auto y1 = 0;
        auto y2 = CELL_SZ * board.Rows();
        DrawLine(x1, y1, x1, y2);
    }
}

// SetDrawColor sets the color for the next rendering command, such as
// DrawGrid, FillRect, or DrawLine.
void Life::SetDrawColor(int r, int g, int b, int alpha) {
    if (haveErr)
        return;
    if (SDL_SetRenderDrawColor(renderer, r, g, b, alpha) < 0) {
        SDL_Log("ERROR: SDL_SetRenderDrawColor: %s\n", SDL_GetError());
        haveErr = true;
    }    
}

// RenderClear completely fills the entire render buffer with the last
// color set by SetDrawColor.
void Life::RenderClear() {
    if (haveErr)
        return;
    if (SDL_RenderClear(renderer) < 0) {
        SDL_Log("ERROR: SDL_ClearRenderer: %s\n", SDL_GetError());
        haveErr = true;
    }
}

// FillRect draws a rect at coordinates (x, y) with a width of w and
// a height of h.  The rect is completely filled in with the color
// set by SetDrawColor.
void Life::FillRect(int x, int y, int w, int h) {
    if (haveErr)
        return;

    auto rect = SDL_Rect{x, y, w, h};
    if (SDL_RenderFillRect(renderer, &rect)) {
        SDL_Log("ERROR: SDL_RenderFillRect: %s\n", SDL_GetError());
        haveErr = true;
    }
}

// DrawLine draws a line that starts at (x1, y1) and ends at (x2, y2).
// The line's color is set with a call to SetDrawColor.
void Life::DrawLine(int x1, int y1, int x2, int y2) {
    if (haveErr)
        return;

    if (SDL_RenderDrawLine(renderer, x1, y1, x2, y2)) {
        SDL_Log("ERROR: SDL_RenderDrawLine: %s\n", SDL_GetError());
        haveErr = true;
    }
}

// TimeKeeper tracks the amount of time that has elapsed between calls
// to SDL_GetPerformanceFrequency.
struct TimeKeeper {
    Uint64 last = 0;

    TimeKeeper() {
        last = SDL_GetPerformanceCounter();
    }

    // ElapsedTimeInSeconds returns the elapsed time in seconds since
    // the last call to ElapsedTimeInSeconds.  The first call to this
    // method returns the elapsed time since the TimeKeeper was created.
    double ElapsedTimeInSeconds() {
        auto now  = SDL_GetPerformanceCounter();
        auto freq = (double)SDL_GetPerformanceFrequency();
        auto diff = (double)(now - last);

        last = now;

        return diff / freq;
    }
};

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }
    atexit(SDL_Quit);

    auto win_sz = CellBoard::EDGE_SZ * Life::CELL_SZ;
    auto window = SDL_CreateWindow(
        "cgol",                  // window title
        SDL_WINDOWPOS_UNDEFINED, // initial x position
        SDL_WINDOWPOS_UNDEFINED, // initial y position
        win_sz,                  // width, in pixels
        win_sz,                  // height, in pixels
        0
    );
    if (window == NULL) {
        SDL_Log("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    auto renderer = SDL_CreateRenderer(
        window,                  // Renderer's parent window.
        -1,                      // Index of rendering driver to initialize, -1 to use flags
        SDL_RENDERER_ACCELERATED // Flags for type of renderer
    );
    if (renderer == NULL) {
        SDL_Log("Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        return 1;
    }

    SDL_ShowWindow(window);
    SDL_RaiseWindow(window);

    Life       life(renderer);
    TimeKeeper timeKeeper;

    // Variables to used for time keeping that updates the simulation
    // independent of the framerate.
    double dt              = 0.5; // seconds
    double t               = 0.0;
    double accumulatedTime = 0.0;

    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // Application is closed if the user closes the window through the
            // standard UI window means or hitting the escape key.
            switch (event.type) {

            case SDL_QUIT:
                goto EndMainLoop;

            case SDL_KEYDOWN: {
                auto key = event.key;
                if (key.state == SDL_PRESSED) {
                    switch (key.keysym.sym) {
                    case SDLK_ESCAPE:
                        goto EndMainLoop;
                    case SDLK_SPACE:
                        life.ToggleSeed();
                        break;
                    case SDLK_s: {
                        dt += 0.1;
                        if (dt > 1.0) dt = 1.0;
                    } break;
                    case SDLK_f: {
                        dt -= 0.1;
                        if (dt < 0.1) dt = 0.1;
                    } break;
                    }
                }
            } break;

            case SDL_MOUSEBUTTONDOWN: {
                auto btn = event.button;
                if (btn.state == SDL_PRESSED) {
                    switch (btn.button) {
                    case SDL_BUTTON_LEFT:
                        life.SeedCell(btn.x, btn.y);
                        break;

                    case SDL_BUTTON_RIGHT:
                        life.ToggleSeed();
                        break;
                    }
                }
            } break;

            }
        }

        // Update simulation at a fixed time step.
        accumulatedTime += timeKeeper.ElapsedTimeInSeconds();
        while (accumulatedTime >= dt) {
            life.Update(t, dt);
            accumulatedTime -= dt;
            t += dt;
        }

        life.Render();
    }
EndMainLoop:

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}
