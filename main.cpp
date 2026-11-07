// Terminal Bomberman (ncurses). Controls: arrows/WASD move, SPACE bomb, Q quit.
#include <ncurses.h>
#include <algorithm>
#include <random>
#include <string>
#include <utility>
#include <vector>

constexpr int W = 31, H = 13;
constexpr int BOMB_TICKS = 25;
constexpr int FLAME_TICKS = 6;
constexpr int TICK_MS = 80;

struct Bomb { int x, y, t; };
struct Enemy { int x, y; };

static std::mt19937 rng{std::random_device{}()};
static std::vector<std::string> grid;  // '#' wall, '+' brick, ' ' empty
static std::vector<std::vector<int>> flame;
static std::vector<Bomb> bombs;
static std::vector<Enemy> enemies;
static int px, py, score, level = 1, lives = 3, range = 2;

static bool bombAt(int x, int y) {
    for (auto &b : bombs) if (b.x == x && b.y == y) return true;
    return false;
}

static void initLevel() {
    grid.assign(H, std::string(W, ' '));
    flame.assign(H, std::vector<int>(W, 0));
    bombs.clear();
    enemies.clear();
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) {
            if (y == 0 || x == 0 || y == H - 1 || x == W - 1 || (x % 2 == 0 && y % 2 == 0))
                grid[y][x] = '#';
            else if (rng() % 100 < 40)
                grid[y][x] = '+';
        }
    px = py = 1;
    grid[1][1] = grid[1][2] = grid[2][1] = ' ';
    int n = 2 + level;
    while ((int)enemies.size() < n) {
        int x = 1 + rng() % (W - 2), y = 1 + rng() % (H - 2);
        if (grid[y][x] == ' ' && x + y > 8) enemies.push_back({x, y});
    }
}

static void explode(const Bomb &b) {
    flame[b.y][b.x] = FLAME_TICKS;
    const int dx[] = {1, -1, 0, 0}, dy[] = {0, 0, 1, -1};
    for (int d = 0; d < 4; d++)
        for (int r = 1; r <= range; r++) {
            int x = b.x + dx[d] * r, y = b.y + dy[d] * r;
            if (grid[y][x] == '#') break;
            flame[y][x] = FLAME_TICKS;
            if (grid[y][x] == '+') { grid[y][x] = ' '; score += 10; break; }
        }
}

static void update(int tick) {
    for (auto &b : bombs) b.t--;
    for (bool again = true; again;) {  // chain reactions
        again = false;
        for (size_t i = 0; i < bombs.size(); i++) {
            if (bombs[i].t <= 0 || flame[bombs[i].y][bombs[i].x] > 0) {
                Bomb b = bombs[i];
                bombs.erase(bombs.begin() + i);
                explode(b);
                again = true;
                break;
            }
        }
    }
    if (tick % 4 == 0) {
        const int dx[] = {1, -1, 0, 0}, dy[] = {0, 0, 1, -1};
        for (auto &e : enemies)
            for (int tries = 0; tries < 6; tries++) {
                int d = rng() % 4, nx = e.x + dx[d], ny = e.y + dy[d];
                if (grid[ny][nx] == ' ' && !bombAt(nx, ny)) { e.x = nx; e.y = ny; break; }
            }
    }
    size_t before = enemies.size();
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
                  [](const Enemy &e) { return flame[e.y][e.x] > 0; }), enemies.end());
    score += 100 * (int)(before - enemies.size());
    bool hurt = flame[py][px] > 0;
    for (auto &e : enemies) if (e.x == px && e.y == py) hurt = true;
    if (hurt) {
        lives--;
        px = py = 1;
        bombs.clear();
    }
    for (auto &row : flame) for (auto &f : row) if (f > 0) f--;
}

static void put(int y, int x, const char *s, int pair, bool bold = false) {
    attron(COLOR_PAIR(pair) | (bold ? A_BOLD : 0));
    mvaddstr(y + 2, x * 2, s);
    attroff(COLOR_PAIR(pair) | A_BOLD);
}

static void draw() {
    erase();
    mvprintw(0, 0, "BOMBERMAN  Level:%d  Score:%d  Lives:%d  Range:%d", level, score, lives, range);
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) {
            if (grid[y][x] == '#') put(y, x, "##", 1);
            else if (grid[y][x] == '+') put(y, x, "%%", 2);
            else if (flame[y][x] > 0) put(y, x, "**", 3, true);
        }
    for (auto &b : bombs) put(b.y, b.x, "()", 4, true);
    for (auto &e : enemies) put(e.y, e.x, "XX", 5, true);
    put(py, px, "@@", 6, true);
    mvprintw(H + 3, 0, "Arrows/WASD: move   SPACE: bomb   Q: quit");
    refresh();
}

static void message(const char *msg) {
    mvprintw(H + 4, 0, "%s  (press any key)", msg);
    refresh();
    nodelay(stdscr, FALSE);
    getch();
    nodelay(stdscr, TRUE);
}

int main() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(TICK_MS);
    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_WHITE, COLOR_BLUE);
        init_pair(2, COLOR_YELLOW, COLOR_RED);
        init_pair(3, COLOR_YELLOW, COLOR_YELLOW);
        init_pair(4, COLOR_WHITE, -1);
        init_pair(5, COLOR_MAGENTA, -1);
        init_pair(6, COLOR_GREEN, -1);
    }
    initLevel();
    for (int tick = 0;; tick++) {
        draw();
        int ch = getch();
        int dx = 0, dy = 0;
        switch (ch) {
            case 'q': case 'Q': endwin(); return 0;
            case KEY_UP: case 'w': case 'W': dy = -1; break;
            case KEY_DOWN: case 's': case 'S': dy = 1; break;
            case KEY_LEFT: case 'a': case 'A': dx = -1; break;
            case KEY_RIGHT: case 'd': case 'D': dx = 1; break;
            case ' ':
                if (!bombAt(px, py) && bombs.size() < 2) bombs.push_back({px, py, BOMB_TICKS});
                break;
        }
        if ((dx || dy) && grid[py + dy][px + dx] == ' ' && !bombAt(px + dx, py + dy)) {
            px += dx;
            py += dy;
        }
        update(tick);
        if (lives <= 0) {
            draw();
            message("GAME OVER");
            break;
        }
        if (enemies.empty()) {
            draw();
            level++;
            if (level % 2 == 0 && range < 5) range++;
            message("LEVEL CLEARED!");
            initLevel();
        }
    }
    endwin();
    printf("Final score: %d\n", score);
    return 0;
}
