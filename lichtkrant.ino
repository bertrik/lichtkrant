#include <stdint.h>

#include "cmdproc.h"
#include "editline.h"
#include "leddriver.h"

#include <Arduino.h>
#include <Ticker.h>

#define PIN_LED D4

#define print Serial.printf

static Ticker ticker;
static char line[120];

static pixel_t framebuffer[LED_NUM_ROWS][LED_NUM_COLS];
static volatile uint32_t ticks = 0;
static volatile uint32_t frames = 0;

static int do_tick(int argc, char *argv[])
{
    print("Ticks = %d\n", ticks);
    return CMD_OK;
}

static int do_fps(int argc, char *argv[])
{
    print("Measuring ...");

    uint32_t count = frames;
    delay(1000);
    int fps = frames - count;

    print("FPS = %d\n", fps);

    return CMD_OK;
}

static void fill(pixel_t c)
{
    for (int y = 0; y < LED_NUM_ROWS; y++) {
        for (int x = 0; x < LED_NUM_COLS; x++) {
            framebuffer[y][x] = c;
        }
    }
}

static int do_pat(int argc, char *argv[])
{
    if (argc < 2) {
        return -1;
    }
    int pat = atoi(argv[1]);

    pixel_t c;
    switch (pat) {
    case 0:
        print("All clear\n");
        c = { 0, 0 };
        fill(c);
        break;
    case 1:
        print("All red\n");
        c = { 255, 0 };
        fill(c);
        break;
    case 2:
        print("All green\n");
        c = { 0, 255 };
        fill(c);
        break;
    case 3:
        print("All yellow\n");
        c = { 255, 255 };
        fill(c);
        break;
    default:
        print("Unhandled pattern %d\n", pat);
        return -1;
    }

    return CMD_OK;
}

static int do_help(int argc, char *argv[]);
const cmd_t commands[] = {
    { "tick", do_tick, "Show ticks" },
    { "fps", do_fps, "Show FPS" },
    { "pat", do_pat, "[pattern] display a specific pattern" },
    { "help", do_help, "Show help" },
    { NULL, NULL, NULL }
};

static void show_help(const cmd_t * cmds)
{
    for (const cmd_t * cmd = cmds; cmd->cmd != NULL; cmd++) {
        print("%10s: %s\n", cmd->name, cmd->help);
    }
}

static int do_help(int argc, char *argv[])
{
    show_help(commands);
    return CMD_OK;
}

// 1 ms interrupt
static void tick(void)
{
    digitalWrite(PIN_LED, 0);

    led_tick();
    ticks++;

    digitalWrite(PIN_LED, 1);
}

// vsync callback
static void vsync(void)
{
    led_write_framebuffer(framebuffer);
    frames++;
}

void setup(void)
{
    Serial.begin(115200);
    print("\nESP-lichtkrant\n");

    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, 1);

    EditInit(line, sizeof(line));

    memset(framebuffer, 0, sizeof(framebuffer));
    led_init(vsync);

    ticker.attach_ms(1, tick);
}

void loop(void)
{
    // parse command line
    bool haveLine = false;
    if (Serial.available()) {
        char c;
        haveLine = EditLine(Serial.read(), &c);
        Serial.write(c);
    }
    if (haveLine) {
        int result = cmd_process(commands, line);
        switch (result) {
        case CMD_OK:
            print("OK\n");
            break;
        case CMD_NO_CMD:
            break;
        case CMD_UNKNOWN:
            print("Unknown command, available commands:\n");
            show_help(commands);
            break;
        default:
            print("%d\n", result);
            break;
        }
        print(">");
    }
}