#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ---------------- CONFIG ----------------
#define WIDTH 60
#define HEIGHT 20
#define MAX_SHAPES 100
#define HISTORY_SIZE 20

// ---------------- SHAPE TYPES ----------------
typedef enum {
    LINE,
    RECTANGLE,
    CIRCLE,
    TRIANGLE
} ShapeType;

// ---------------- SHAPE STRUCT ----------------
typedef struct {
    int id;
    int active;
    ShapeType type;

    int x1, y1;   // start / center / point1
    int x2, y2;   // end / width-height helper / point2
    int x3, y3;   // triangle point3
    int radius;   // circle radius
} Shape;

// ---------------- GLOBAL DATA ----------------
Shape shapes[MAX_SHAPES];
Shape history[HISTORY_SIZE][MAX_SHAPES];

int history_top = 0;
int history_size = 1;
int current_id = 1;

char canvas[HEIGHT][WIDTH];

// ---------------- COLORS ----------------
#define GREEN "\x1b[32m"
#define RESET "\x1b[0m"

// ---------------- CANVAS ----------------
void clear_canvas() {
    for (int i = 0; i < HEIGHT; i++)
        for (int j = 0; j < WIDTH; j++)
            canvas[i][j] = ' ';
}

void draw_point(int x, int y) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
        canvas[y][x] = '*';
}

// ---------------- DRAWING ALGORITHMS ----------------
void draw_line(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1), sx = (x1 < x2) ? 1 : -1;
    int dy = -abs(y2 - y1), sy = (y1 < y2) ? 1 : -1;
    int err = dx + dy;

    while (1) {
        draw_point(x1, y1);
        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

void draw_rectangle_center(int cx, int cy, int w, int h) {
    int x1 = cx - w / 2, y1 = cy - h / 2;
    int x2 = cx + w / 2, y2 = cy + h / 2;

    draw_line(x1, y1, x2, y1);
    draw_line(x1, y2, x2, y2);
    draw_line(x1, y1, x1, y2);
    draw_line(x2, y1, x2, y2);
}

void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3) {
    draw_line(x1, y1, x2, y2);
    draw_line(x2, y2, x3, y3);
    draw_line(x3, y3, x1, y1);
}

void draw_circle(int xc, int yc, int r) {
    int x = 0, y = r, d = 3 - 2 * r;
    while (y >= x) {
        draw_point(xc + x, yc + y);
        draw_point(xc - x, yc + y);
        draw_point(xc + x, yc - y);
        draw_point(xc - x, yc - y);
        draw_point(xc + y, yc + x);
        draw_point(xc - y, yc + x);
        draw_point(xc + y, yc - x);
        draw_point(xc - y, yc - x);

        x++;
        if (d > 0) { y--; d += 4 * (x - y) + 10; }
        else d += 4 * x + 6;
    }
}

// ---------------- RENDER ENGINE ----------------
void render_all_shapes() {
    clear_canvas();
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (!shapes[i].active) continue;
        switch (shapes[i].type) {
            case LINE:      draw_line(shapes[i].x1, shapes[i].y1, shapes[i].x2, shapes[i].y2); break;
            case RECTANGLE: draw_rectangle_center(shapes[i].x1, shapes[i].y1, shapes[i].x2, shapes[i].y2); break;
            case CIRCLE:    draw_circle(shapes[i].x1, shapes[i].y1, shapes[i].radius); break;
            case TRIANGLE:  draw_triangle(shapes[i].x1, shapes[i].y1, shapes[i].x2, shapes[i].y2, shapes[i].x3, shapes[i].y3); break;
        }
    }
}

// ---------------- DISPLAY ----------------
void display_canvas() {
    render_all_shapes();
    printf("\n+------------------------------------------------------------+\n");
    for (int i = 0; i < HEIGHT; i++) {
        printf("|");
        for (int j = 0; j < WIDTH; j++) {
            if (canvas[i][j] == '*') printf(GREEN "*" RESET);
            else printf(" ");
        }
        printf("|\n");
    }
    printf("+------------------------------------------------------------+\n");
}

// ---------------- HISTORY (UNDO) ----------------
void push_state() {
    if (history_size < HISTORY_SIZE) {
        memcpy(history[history_size], shapes, sizeof(shapes));
        history_size++;
        history_top = history_size - 1;
    }
}

void undo() {
    if (history_top <= 0) {
        printf("Nothing to undo!\n");
        return;
    }
    history_top--;
    memcpy(shapes, history[history_top], sizeof(shapes));
    display_canvas();
    printf("Undo successful!\n");
}

// ---------------- RESET ----------------
void reset_engine() {
    for (int i = 0; i < MAX_SHAPES; i++) shapes[i].active = 0;
    clear_canvas();
    history_size = 1;
    history_top = 0;
    current_id = 1;
    memcpy(history[0], shapes, sizeof(shapes));
    display_canvas();
    printf("All drawings cleared! Engine reset.\n");
}

// ---------------- ADD SHAPE ----------------
void add_shape() {
    int choice, idx = -1;
    printf("\n1.Line\n2.Rectangle\n3.Circle\n4.Triangle\nChoice: ");
    if (scanf("%d", &choice) != 1) { printf("Invalid input!\n"); return; }

    for (int i = 0; i < MAX_SHAPES; i++) if (!shapes[i].active) { idx = i; break; }
    if (idx == -1) { printf("Shape limit reached!\n"); return; }

    shapes[idx].id = current_id++;
    shapes[idx].active = 1;

    switch (choice) {
        case 1: shapes[idx].type = LINE;
                printf("Enter Start(x1 y1) and End(x2 y2): ");
                scanf("%d %d %d %d", &shapes[idx].x1, &shapes[idx].y1, &shapes[idx].x2, &shapes[idx].y2);
                break;
        case 2: shapes[idx].type = RECTANGLE;
                printf("Enter Center(x y) Width Height: ");
                scanf("%d %d %d %d", &shapes[idx].x1, &shapes[idx].y1, &shapes[idx].x2, &shapes[idx].y2);
                break;
        case 3: shapes[idx].type = CIRCLE;
                printf("Enter Center(x y) Radius: ");
                scanf("%d %d %d", &shapes[idx].x1, &shapes[idx].y1, &shapes[idx].radius);
                break;
        case 4: shapes[idx].type = TRIANGLE;
                printf("Enter 3 Points (x1 y1 x2 y2 x3 y3): ");
                scanf("%d %d %d %d %d %d", &shapes[idx].x1, &shapes[idx].y1, &shapes[idx].x2, &shapes[idx].y2, &shapes[idx].x3, &shapes[idx].y3);
                break;
        default: printf("Invalid choice!\n"); shapes[idx].active = 0; return;
    }

    push_state();
    display_canvas();
    printf("Shape added successfully (ID %d)\n", shapes[idx].id);
}

// ---------------- INIT ----------------
void init() {
    for (int i = 0; i < MAX_SHAPES; i++) shapes[i].active = 0;
    memcpy(history[0], shapes, sizeof(shapes));
}

// ---------------- MAIN ----------------

int main() {
    init();
    int choice;
    do {
        printf("\n===== MINI GRAPHICS ENGINE =====\n");
        printf("1.Add Shape\n2.Undo\n3.Display\n4.Reset\n5.Exit\n");
        printf("Choice: ");
        if (scanf("%d", &choice) != 1) { 
            printf("Invalid input!\n"); 
            break; 
        }

        switch (choice) {
            case 1: add_shape(); break;
            case 2: undo(); break;
            case 3: display_canvas(); break;
            case 4: reset_engine(); break;
            case 5: printf("Bye!\n"); break;
            default: printf("Invalid!\n");
        }
    } while (choice != 5);

    return 0;
}
