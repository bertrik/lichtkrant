#include <stdint.h>

#include "cmdproc.h"
#include "editline.h"
#include "leddriver.h"

#include <Arduino.h>

#define PIN_LED D4

#define print Serial.printf

static char line[120];

static pixel_t framebuffer[LED_NUM_ROWS][LED_NUM_COLS];
static volatile uint32_t frames = 0;

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
    case 4:
        print("quattro stagioni\n");
        for (int x = 0; x < 80; x++) {
            for (int y = 0; y < 7; y++) {
                pixel_t c;
                c.r = (x < 40) || (y == 3) ? 0 : 255;
                c.g = (y < 4) ? 0 : 255;
                framebuffer[y][x] = c;
            }
        }
        break;
    case 5:
        print("rasta vertical\n");
        for (int x = 0; x < 80; x++) {
            for (int y = 0; y < 7; y++) {
                framebuffer[y][x].r = map(y, 0, 6, 255, 0);
                framebuffer[y][x].g = map(y, 0, 6, 0, 255);
            }
        }
        break;
    case 6:
        print("rasta horizontal\n");
        for (int x = 0; x < 80; x++) {
            for (int y = 0; y < 7; y++) {
                framebuffer[y][x].r = map(x, 0, 79, 255, 0);
                framebuffer[y][x].g = map(x, 0, 79, 0, 255);
            }
        }
        break;
    case 7:
        print("red shades\n");
        for (int x = 0; x < 80; x++) {
            for (int y = 0; y < 7; y++) {
                framebuffer[y][x].r = map(x, 0, 79, 255, 0);
                framebuffer[y][x].g = 0;
            }
        }
        break;
    default:
        print("Unhandled pattern %d\n", pat);
        return -1;
    }

    return CMD_OK;
}

static int do_line(int argc, char *argv[])
{
    if (argc < 2) {
        return -1;
    }
    int line = atoi(argv[1]);
    if ((line < 0) || (line >= 7)) {
        print("Invalid line %d\n", line);
        return -2;
    }
    uint8_t r = 255;
    if (argc > 2) {
        r = atoi(argv[2]);
    }
    uint8_t g = 255;
    if (argc > 3) {
        g = atoi(argv[3]);
    }
    pixel_t c = {r, g};
    for (int x = 0; x < 80; x++) {
        framebuffer[line][x] = c;
    }

    return CMD_OK;
}

static int do_pix(int argc, char *argv[])
{
    if (argc < 3) {
        return -1;
    }
    int x = atoi(argv[1]);
    int y = atoi(argv[2]);
    uint8_t r = 255;
    if (argc > 3) {
        r = atoi(argv[3]);
    }
    uint8_t g = 255;
    if (argc > 4) {
        g = atoi(argv[4]);
    }
    pixel_t c = {r, g};
    framebuffer[y][x] = c;

    return CMD_OK;
}

static int do_reboot(int argc, char *argv[])
{
    ESP.restart();
    return CMD_OK;
}

static int do_help(int argc, char *argv[]);
const cmd_t commands[] = {
    { "fps", do_fps, "Show FPS" },
    { "pat", do_pat, "[pattern] display a specific pattern" },
    { "line", do_line, "<line> [r] [g] fill one row colour {r.g}" },
    { "pix", do_pix, "<col> <row> [r] [g] Set pixel with colour {r.g}" },
    { "reboot", do_reboot, "Reboot" },
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

// vsync callback
static void ICACHE_RAM_ATTR vsync(int frame_nr)
{
    led_write_framebuffer(framebuffer);
    frames = frame_nr;
}

void setup(void)
{
    Serial.begin(115200);
    print("\nESP-lichtkrant\n");

    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, 1);

    EditInit(line, sizeof(line));

    for (int x = 0; x < 80; x++) {
        for (int y = 0; y < 7; y++) {
            framebuffer[y][x].r = map(x, 0, 79, 255, 0);
            framebuffer[y][x].g = map(x, 0, 79, 0, 255);
        }
    }

    led_init(vsync);
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
