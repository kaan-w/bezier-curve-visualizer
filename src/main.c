#include <raylib.h>
#include <raymath.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

static const int WINDOW_WIDTH = 720;
static const int WINDOW_HEIGHT = 720;
static const char WINDOW_TITLE[] = "Bézier Curve Visualizer";

static const int GAP = 16;
static const int BUTTON_WIDTH = 110;
static const int BUTTON_HEIGHT = 50;

static const int TARGET_FPS = 60;

static const Color CONTROL_COLOR = GRAY;
static const Color EXTRA_CONTROL_COLORS[] = {
    VIOLET,
    MAROON,
    SKYBLUE,
    LIME,
    GOLD
};
static const int CONTROL_POINT_RADIUS = 4;
static const int CONTROL_LINE_THICKNESS = 3;

static const Color TRACE_COLOR = BLACK;
static const int TRACE_POINT_RADIUS = 5;
static const int TRACE_THICKNESS = 4;

enum { MAX_POINTS = 1024 };
typedef struct {
    int length;
    Vector2 points[MAX_POINTS];
} Vector2Array;

static Vector2Array control_chain = { 0 };
static Vector2Array trace = { 0 };
static bool is_paused = true;
static float animation_speed = 0.2f;
static float t = 0.0f;

void Vector2Array_Clear(Vector2Array* array) {
    array->length = 0;
}

void Vector2Array_Append(Vector2Array* array, const Vector2 v) {
    if (array->length < MAX_POINTS) {
        array->points[array->length++] = v;
    } else {
        Vector2Array_Clear(array);
    }
}

void Vector2Array_Pop(Vector2Array* array) {
    if (array->length > 0) {
        array->length--;
    }
}

void DrawUI(void) {
    const Rectangle play_pause_btn_box = {
        .x = GAP,
        .y = GetScreenHeight() - BUTTON_HEIGHT - GAP,
        .width = BUTTON_WIDTH,
        .height = BUTTON_HEIGHT
    };
    const char* play_pause_btn_text = is_paused ? "#131#Play" : "#132#Pause";
    if (GuiButton(play_pause_btn_box, play_pause_btn_text) || IsKeyPressed(KEY_SPACE)) {
        is_paused = !is_paused;
    }

    const Rectangle undo_btn_box = {
      .x = play_pause_btn_box.x + BUTTON_WIDTH + GAP,
      .y = play_pause_btn_box.y,
      .width = BUTTON_WIDTH,
      .height = BUTTON_HEIGHT
    };
    const char* undo_btn_text = "#72#Undo";
    if (GuiButton(undo_btn_box, undo_btn_text) || IsKeyPressed(KEY_U)) {
        Vector2Array_Pop(&control_chain);
    }

    const Rectangle reset_btn_box = {
      .x = undo_btn_box.x + BUTTON_WIDTH + GAP,
      .y = undo_btn_box.y,
      .width = BUTTON_WIDTH,
      .height = BUTTON_HEIGHT
    };
    const char* reset_btn_text = "#211#Reset";
    if (GuiButton(reset_btn_box, reset_btn_text) || IsKeyPressed(KEY_R)) {
        t = 0.0f;
        is_paused = true;
        Vector2Array_Clear(&control_chain);
        Vector2Array_Clear(&trace);
    }

    const Rectangle plus_speed_btn_box = {
        .x = GetScreenWidth() - BUTTON_WIDTH - GAP,
        .y = reset_btn_box.y,
        .width = BUTTON_WIDTH,
        .height = BUTTON_HEIGHT,
    };
    const char* plus_speed_btn_text = "#220#Speed";
    if (GuiButton(plus_speed_btn_box, plus_speed_btn_text) || IsKeyPressed(KEY_EQUAL)) {
        animation_speed += 0.1f;
    }

    const Rectangle minus_speed_btn_box = {
        .x = plus_speed_btn_box.x - BUTTON_WIDTH - GAP,
        .y = reset_btn_box.y,
        .width = BUTTON_WIDTH,
        .height = BUTTON_HEIGHT,
    };
    const char* minus_speed_btn_text = "#221#Speed";
    if (GuiButton(minus_speed_btn_box, minus_speed_btn_text) || IsKeyPressed(KEY_MINUS)) {
        animation_speed -= 0.1f;
    }
}

void DrawTrace(const Vector2Array* trace) {
    for (int i = 0; i < trace->length - 1; i++) {
        DrawLineEx(trace->points[i], trace->points[i + 1], TRACE_THICKNESS, TRACE_COLOR);
    }
}

void DrawPolygonalChain(const Vector2Array* chain, const Color color) {
    for (int i = 0; i < chain->length; i++) {
        DrawCircleV(chain->points[i], CONTROL_POINT_RADIUS, color);
        if (i < chain->length - 1) {
            DrawLineEx(chain->points[i], chain->points[i + 1], CONTROL_LINE_THICKNESS, color);
        }
    }
}

Vector2 DrawDeCasteljau(const Vector2Array* chain, const float t, const int depth) {
    const Color color = EXTRA_CONTROL_COLORS[depth % (sizeof(EXTRA_CONTROL_COLORS) / sizeof(Color))];

    if (chain->length == 1) {
        DrawCircleV(chain->points[0], TRACE_POINT_RADIUS, TRACE_COLOR);
        return chain->points[0];
    };

    Vector2Array next_chain = { 0 };
    next_chain.length = chain->length - 1;
    for (int i = 0; i < chain->length - 1; i++) {
        next_chain.points[i] = Vector2Lerp(chain->points[i], chain->points[i + 1], t);
    }

    DrawPolygonalChain(&next_chain, color);
    return DrawDeCasteljau(&next_chain, t, depth + 1);
}

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    SetTargetFPS(TARGET_FPS);

    GuiLoadIcons("assets/iconset.rgi", false);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    GuiSetIconScale(2);

    while(!WindowShouldClose()) {
        if (!is_paused) {
            t += GetFrameTime() * animation_speed;
            if (t > 1.0f) {
                t = 0.0f;
                Vector2Array_Clear(&trace);
            } else if (t < 0.0f) {
                t = 1.0f;
                Vector2Array_Clear(&trace);
            };
        }

        BeginDrawing();
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                const Vector2 mouse_position = GetMousePosition();
                if (mouse_position.y < GetScreenHeight() - BUTTON_HEIGHT - GAP) {
                    Vector2Array_Append(&control_chain, mouse_position);
                }
            }

            DrawPolygonalChain(&control_chain, CONTROL_COLOR);
            if (control_chain.length > 1 && t > 0.0f) {
                const Vector2 point = DrawDeCasteljau(&control_chain, t, 0);
                if (!is_paused) {
                    Vector2Array_Append(&trace, point);
                }
                DrawTrace(&trace);
            }
            DrawUI();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
