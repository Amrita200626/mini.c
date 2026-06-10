#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#define CANVAS_WIDTH 80
#define CANVAS_HEIGHT 25
#define MAX_SHAPES 100
// Coordinate structures
typedef struct {
    int x;
    int y;
} Point;
// Shape types
typedef enum {
    SHAPE_LINE,
    SHAPE_CIRCLE,
    SHAPE_RECTANGLE,
    SHAPE_TRIANGLE
} ShapeType;
// Individual shape data
typedef struct {
    int id;
    ShapeType type;
    bool active;
    union {
        struct {
            Point p1;
            Point p2;
        } line;
        struct {
            Point center;
            int radius;
        } circle;
        struct {
            Point top_left;
            int width;
            int height;
        } rect;
        struct {
            Point p1;
            Point p2;
            Point p3;
        } triangle;
    } data;
} Shape;
// Canvas representation
char canvas[CANVAS_HEIGHT][CANVAS_WIDTH];
// State of the graphics editor
Shape shapes[MAX_SHAPES];
int next_shape_id = 1;
// Function declarations
void clear_canvas();
void display_canvas();
void set_pixel(int x, int y);
void draw_line(Point p1, Point p2);
void draw_circle(Point center, int radius);
void draw_rect(Point top_left, int w, int h);
void draw_triangle(Point p1, Point p2, Point p3);
void render_all_shapes();
bool add_shape(Shape shape);
bool delete_shape(int id);
void list_shapes();
void clear_all_shapes();
// Input helper functions
int read_int(const char *prompt, int min_val, int max_val);
void read_point(const char *prompt, Point *p);
// -------------------------------------------------------------
// Canvas Manipulation Functions
// -------------------------------------------------------------
// Clears the canvas, filling it with the background character '_'
void clear_canvas() {
    for (int y = 0; y < CANVAS_HEIGHT; y++) {
        for (int x = 0; x < CANVAS_WIDTH; x++) {
            canvas[y][x] = '_';
        }
    }
}
// Sets a single pixel to '*' if it is within the bounds of the canvas
void set_pixel(int x, int y) {
    if (x >= 0 && x < CANVAS_WIDTH && y >= 0 && y < CANVAS_HEIGHT) {
        canvas[y][x] = '*';
    }
}
// Displays the 2D canvas to the standard output
void display_canvas() {
    printf("\n");
    // Draw top border ruler for reference
    printf("   ");
    for (int x = 0; x < CANVAS_WIDTH; x++) {
        if (x % 10 == 0) printf("%d", x / 10);
        else printf(" ");
    }
    printf("\n   ");
    for (int x = 0; x < CANVAS_WIDTH; x++) {
        printf("%d", x % 10);
    }
    printf("\n");
    // Print rows
    for (int y = 0; y < CANVAS_HEIGHT; y++) {
        printf("%2d ", y); // Row number indicator
        for (int x = 0; x < CANVAS_WIDTH; x++) {
            putchar(canvas[y][x]);
        }
        printf("\n");
    }
    printf("\n");
}
// -------------------------------------------------------------
// Geometric Drawing Algorithms
// -------------------------------------------------------------
// Bresenham's Line Algorithm
void draw_line(Point p1, Point p2) {
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    while (1) {
        set_pixel(x1, y1);
        if (x1 == x2 && y1 == y2) {
            break;
        }
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}
// Midpoint Circle Algorithm Helper
void draw_circle_points(int cx, int cy, int x, int y) {
    set_pixel(cx + x, cy + y);
    set_pixel(cx - x, cy + y);
    set_pixel(cx + x, cy - y);
    set_pixel(cx - x, cy - y);
    set_pixel(cx + y, cy + x);
    set_pixel(cx - y, cy + x);
    set_pixel(cx + y, cy - x);
    set_pixel(cx - y, cy - x);
}
// Midpoint Circle Algorithm
void draw_circle(Point center, int radius) {
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;
    draw_circle_points(center.x, center.y, x, y);
    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        draw_circle_points(center.x, center.y, x, y);
    }
}
// Rectangle drawing (outline borders)
void draw_rect(Point top_left, int w, int h) {
    int x = top_left.x;
    int y = top_left.y;
    // Draw horizontal lines (top and bottom edges)
    for (int i = 0; i < w; i++) {
        set_pixel(x + i, y);
        set_pixel(x + i, y + h - 1);
    }
    // Draw vertical lines (left and right edges)
    for (int i = 0; i < h; i++) {
        set_pixel(x, y + i);
        set_pixel(x + w - 1, y + i);
    }
}
// Triangle drawing (connects vertices with 3 lines)
void draw_triangle(Point p1, Point p2, Point p3) {
    draw_line(p1, p2);
    draw_line(p2, p3);
    draw_line(p3, p1);
}
// -------------------------------------------------------------
// Shape State & Renderer
// -------------------------------------------------------------
// Clear the canvas and render all active shapes onto it
void render_all_shapes() {
    clear_canvas();
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active) {
            switch (shapes[i].type) {
                case SHAPE_LINE:
                    draw_line(shapes[i].data.line.p1, shapes[i].data.line.p2);
                    break;
                case SHAPE_CIRCLE:
                    draw_circle(shapes[i].data.circle.center, shapes[i].data.circle.radius);
                    break;
                case SHAPE_RECTANGLE:
                    draw_rect(shapes[i].data.rect.top_left, shapes[i].data.rect.width, shapes[i].data.rect.height);
                    break;
                case SHAPE_TRIANGLE:
                    draw_triangle(shapes[i].data.triangle.p1, shapes[i].data.triangle.p2, shapes[i].data.triangle.p3);
                    break;
            }
        }
    }
}
// Adds a shape to the active list
bool add_shape(Shape shape) {
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (!shapes[i].active) {
            shapes[i] = shape;
            shapes[i].id = next_shape_id++;
            shapes[i].active = true;
            return true;
        }
    }
    return false; // List full
}
// Deletes a shape from the active list by its ID
bool delete_shape(int id) {
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active && shapes[i].id == id) {
            shapes[i].active = false;
            return true;
        }
    }
    return false; // Shape not found
}
// Lists details of all active shapes
void list_shapes() {
    bool found = false;
    printf("\n=== Active Shapes ===\n");
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active) {
            found = true;
            printf("ID: %d | ", shapes[i].id);
            switch (shapes[i].type) {
                case SHAPE_LINE:
                    printf("Line from (%d, %d) to (%d, %d)\n", 
                           shapes[i].data.line.p1.x, shapes[i].data.line.p1.y,
                           shapes[i].data.line.p2.x, shapes[i].data.line.p2.y);
                    break;
                case SHAPE_CIRCLE:
                    printf("Circle center: (%d, %d), radius: %d\n",
                           shapes[i].data.circle.center.x, shapes[i].data.circle.center.y,
                           shapes[i].data.circle.radius);
                    break;
                case SHAPE_RECTANGLE:
                    printf("Rectangle top-left: (%d, %d), width: %d, height: %d\n",
                           shapes[i].data.rect.top_left.x, shapes[i].data.rect.top_left.y,
                           shapes[i].data.rect.width, shapes[i].data.rect.height);
                    break;
                case SHAPE_TRIANGLE:
                    printf("Triangle points: (%d, %d), (%d, %d), (%d, %d)\n",
                           shapes[i].data.triangle.p1.x, shapes[i].data.triangle.p1.y,
                           shapes[i].data.triangle.p2.x, shapes[i].data.triangle.p2.y,
                           shapes[i].data.triangle.p3.x, shapes[i].data.triangle.p3.y);
                    break;
            }
        }
    }
    if (!found) {
        printf("(No shapes currently on canvas)\n");
    }
    printf("=====================\n");
}
// Clears all shape data from the list
void clear_all_shapes() {
    for (int i = 0; i < MAX_SHAPES; i++) {
        shapes[i].active = false;
    }
    next_shape_id = 1;
}
// -------------------------------------------------------------
// Interactive User Input Helpers
// -------------------------------------------------------------
// Reads an integer from stdin within a min/max range, with buffer clearing
int read_int(const char *prompt, int min_val, int max_val) {
    int val;
    char buffer[128];
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            continue;
        }
        // Remove newline if present
        buffer[strcspn(buffer, "\n")] = '\0';
        // Check if parsing matches exactly
        char extra;
        if (sscanf(buffer, "%d%c", &val, &extra) == 1 || 
            (sscanf(buffer, "%d%c", &val, &extra) == 2 && extra == '\r')) {
            if (val >= min_val && val <= max_val) {
                return val;
            }
        }
        printf("Invalid input. Please enter an integer between %d and %d.\n", min_val, max_val);
    }
}
// Reads a point coordinate pair (X, Y)
void read_point(const char *prompt, Point *p) {
    printf("%s\n", prompt);
    p->x = read_int("  Enter X (col, 0 to 79): ", 0, CANVAS_WIDTH - 1);
    p->y = read_int("  Enter Y (row, 0 to 24): ", 0, CANVAS_HEIGHT - 1);
}
// -------------------------------------------------------------
// Main Loop
// -------------------------------------------------------------
int main() {
    // Initialize editor state
    clear_all_shapes();
    render_all_shapes();
    printf("==================================================\n");
    printf("        WELCOME TO 2D GRAPHICS EDITOR IN C        \n");
    printf("==================================================\n");
    printf("Canvas Resolution: %d x %d\n", CANVAS_WIDTH, CANVAS_HEIGHT);
    printf("Coordinate System: Top-Left is (0,0)\n");
    printf("X increases rightward, Y increases downward.\n");
    while (1) {
        printf("\n--- MAIN MENU ---\n");
        printf("1. Add a Shape\n");
        printf("2. Delete a Shape\n");
        printf("3. List Active Shapes\n");
        printf("4. Display Canvas\n");
        printf("5. Clear All Shapes\n");
        printf("6. Exit\n");
        int choice = read_int("Choose an option (1-6): ", 1, 6);
        if (choice == 6) {
            printf("\nExiting 2D Graphics Editor. Goodbye!\n");
            break;
        }
        switch (choice) {
            case 1: { // Add a Shape
                printf("\n--- ADD A SHAPE ---\n");
                printf("1. Line\n");
                printf("2. Circle\n");
                printf("3. Rectangle\n");
                printf("4. Triangle\n");
                printf("5. Go Back\n");
                int type_choice = read_int("Select shape type (1-5): ", 1, 5);
                if (type_choice == 5) {
                    break;
                }
                Shape new_shape;
                memset(&new_shape, 0, sizeof(Shape));
                if (type_choice == 1) {
                    new_shape.type = SHAPE_LINE;
                    read_point("Start Point:", &new_shape.data.line.p1);
                    read_point("End Point:", &new_shape.data.line.p2);
                } else if (type_choice == 2) {
                    new_shape.type = SHAPE_CIRCLE;
                    read_point("Circle Center:", &new_shape.data.circle.center);
                    new_shape.data.circle.radius = read_int("  Enter Radius (1 to 40): ", 1, 40);
                } else if (type_choice == 3) {
                    new_shape.type = SHAPE_RECTANGLE;
                    read_point("Rectangle Top-Left Corner:", &new_shape.data.rect.top_left);
                    // Restrict dimensions so rectangle does not start off totally outside the grid
                    new_shape.data.rect.width = read_int("  Enter Width (1 to 80): ", 1, CANVAS_WIDTH);
                    new_shape.data.rect.height = read_int("  Enter Height (1 to 25): ", 1, CANVAS_HEIGHT);
                } else if (type_choice == 4) {
                    new_shape.type = SHAPE_TRIANGLE;
                    read_point("Triangle Vertex 1:", &new_shape.data.triangle.p1);
                    read_point("Triangle Vertex 2:", &new_shape.data.triangle.p2);
                    read_point("Triangle Vertex 3:", &new_shape.data.triangle.p3);
                }
                if (add_shape(new_shape)) {
                    printf("\nShape added successfully!\n");
                    render_all_shapes();
                    display_canvas();
                } else {
                    printf("\nError: Cannot add shape. The editor has reached the maximum shape capacity (%d).\n", MAX_SHAPES);
                }
                break;
            }
            case 2: { // Delete a Shape
                list_shapes();
                int delete_id = read_int("Enter the ID of the shape to delete (or 0 to cancel): ", 0, next_shape_id - 1);
                if (delete_id == 0) {
                    break;
                }
                if (delete_id > 0 && delete_shape(delete_id)) {
                    printf("\nShape ID %d deleted successfully.\n", delete_id);
                    render_all_shapes();
                    display_canvas();
                } else {
                    printf("\nError: Shape with ID %d not found.\n", delete_id);
                }
                break;
            }
            case 3: // List Shapes
                list_shapes();
                break;
            case 4: // Display Canvas
                render_all_shapes();
                display_canvas();
                break;
            case 5: { // Clear All
                int confirm = read_int("Are you sure you want to clear all shapes? (1 = Yes, 0 = No): ", 0, 1);
                if (confirm == 1) {
                    clear_all_shapes();
                    render_all_shapes();
                    printf("\nAll shapes cleared.\n");
                    display_canvas();
                } else {
                    printf("\nOperation cancelled.\n");
                }
                break;
            }
        }
    }
    return 0;
}
