#include <stdint.h>

#include "cmdproc.h"
#include "editline.h"
#include "leddriver.h"
#include "draw.h"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
#include <Arduino.h>
#include <NTPClient.h>

#define print Serial.printf

#define RAWRGB_TCP_PORT    1234

const char* NTP_SERVER = "nl.pool.ntp.org";
// enter your time zone (https://remotemonitoringsystems.ca/time-zone-abbreviations.php)
const char* TZ_INFO    = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";    // Europe/Amsterdam

static WiFiManager wifiManager;
static WiFiUDP ntpUDP;
static NTPClient ntpClient(ntpUDP, NTP_SERVER);
static WiFiServer tcpServer(RAWRGB_TCP_PORT);

static char line[120];
static pixel_t framebuffer[LED_HEIGHT][LED_WIDTH];
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
    for (int y = 0; y < LED_HEIGHT; y++) {
        draw_hline(y, c);
    }
}

static int do_pat(int argc, char *argv[])
{
    if (argc < 2) {
        return CMD_ARG;
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
                draw_pixel(x, y, c);
            }
        }
        break;
    case 5:
        print("rasta vertical\n");
        for (int x = 0; x < 80; x++) {
            for (int y = 0; y < 7; y++) {
                pixel_t c;
                c.r = map(y, 0, 6, 255, 0);
                c.g = map(y, 0, 6, 0, 255);
                draw_pixel(x, y, c);
            }
        }
        break;
    case 6:
        print("rasta horizontal\n");
        for (int x = 0; x < 80; x++) {
            for (int y = 0; y < 7; y++) {
                pixel_t c;
                c.r = map(x, 0, 79, 255, 0);
                c.g = map(x, 0, 79, 0, 255);
                draw_pixel(x, y, c);
            }
        }
        break;
    case 7:
        print("red shades\n");
        for (int x = 0; x < 80; x++) {
            for (int y = 0; y < 7; y++) {
                pixel_t c;
                c.r = map(x, 0, 79, 255, 0);
                c.g = 0;
                draw_pixel(x, y, c);
            }
        }
        break;
    case 8:
        print("green shades\n");
        for (int x = 0; x < 80; x++) {
            for (int y = 0; y < 7; y++) {
                pixel_t c;
                c.r = 0;
                c.g = map(x, 0, 79, 255, 0);
                draw_pixel(x, y, c);
            }
        }
        break;
    case 9:
        print("red/green shades\n");
        for (int x = 0; x < 80; x++) {
            for (int y = 0; y < 7; y++) {
                pixel_t c;
                c.r = map(x, 0, 79, 0, 255);
                c.g = map(y, 0, 6, 0, 255);
                draw_pixel(x, y, c);
            }
        }
        break;
    default:
        print("Unhandled pattern %d\n", pat);
        return CMD_ARG;
    }

    return CMD_OK;
}

static int do_line(int argc, char *argv[])
{
    if (argc < 2) {
        return CMD_ARG;
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
    pixel_t c = { r, g };
    draw_hline(line, c);

    return CMD_OK;
}

static int do_pix(int argc, char *argv[])
{
    if (argc < 4) {
        return CMD_ARG;
    }
    int x = atoi(argv[1]);
    int y = atoi(argv[2]);
    uint32_t c = strtoul(argv[3], NULL, 16);

    pixel_t p;
    p.r = (c >> 16) & 0xFF;
    p.g = (c >> 8) & 0xFF;
    bool result = draw_pixel(x, y, p);

    return result ? CMD_OK : CMD_ARG;
}

static int do_text(int argc, char *argv[])
{
    const char *text = "The quick brown fox jumped over the lazy dog";
    if (argc > 1) {
        text = argv[1];
    }
    draw_text(text, 0, {255, 0}, {0, 0});
    return CMD_OK;
}

static int do_datetime(int argc, char *argv[])
{
    time_t now = ntpClient.getEpochTime();
    tm *timeinfo = localtime(&now);

    char text[32];
    int x;
    pixel_t color;
    const char *cmd = argv[0];
    if (strcmp(cmd, "date") == 0) {
        strftime(text, sizeof(text), "%a %F", timeinfo);
        x = 0;
        color = {255, 0};
    } else if (strcmp(cmd, "time") == 0) {
        strftime(text, sizeof(text), "%T", timeinfo);
        x = 20;
        color = {0, 255};
    } else {
        return CMD_ARG;
    }

    memset(framebuffer, 0, sizeof(framebuffer));
    draw_text(text, x, color, {0, 0});
    return CMD_OK;
}

static int do_enable(int argc, char *argv[])
{
    bool enable = true;
    if (argc > 1) {
        enable = atoi(argv[1]) != 0;
    }
    if (enable) {
        led_enable();
    } else {
        led_disable();
    }
    return CMD_OK;
}

static int do_reboot(int argc, char *argv[])
{
    led_disable();
    ESP.restart();
    return CMD_OK;
}

static int do_help(int argc, char *argv[]);
const cmd_t commands[] = {
    { "fps", do_fps, "Show FPS" },
    { "pat", do_pat, "[pattern] display a specific pattern" },
    { "line", do_line, "<line> [r] [g] fill one row colour {r.g}" },
    { "pix", do_pix, "<col> <row> <hexcode> Set pixel with colour" },
    { "text", do_text, "<text> Write text on the display" },
    { "date", do_datetime, "show date" },
    { "time", do_datetime, "show time" },
    { "enable", do_enable, "[0|1] Enable/disable" },
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

// reads one raw RGB frame from TCP client into the frame buffer, returns true if successful
static bool read_tcp_frame(WiFiClient *client, uint8_t *buf, int size)
{
    uint8_t *ptr = buf;
    int remain = size;
    while (client->connected() && (remain > 0)) {
        int len = client->read(ptr, remain);
        remain -= len;
        ptr += len;
        yield();
    }
    return (remain == 0);
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

    EditInit(line, sizeof(line));
    draw_init((pixel_t *)framebuffer);

    led_init(vsync);
    memset(framebuffer, 0, sizeof(framebuffer));

    wifiManager.autoConnect("ESP-LEDSIGN");
    draw_text(WiFi.localIP().toString().c_str(), 6, {255, 255}, {0, 0});

    tcpServer.begin();
    MDNS.begin("esp-ledsign");
    MDNS.addService("raw_rgb", "tcp", RAWRGB_TCP_PORT);

    setenv("TZ", TZ_INFO, 1);
    ntpClient.begin();

    led_enable();
}

void loop(void)
{
    // handle incoming TCP frames
    WiFiClient client = tcpServer.available();
    if (client) {
        print("Accepted TCP connection from %s...", client.remoteIP().toString().c_str());
        int frames = 0;
        unsigned long int start = millis();
        while (read_tcp_frame(&client, (uint8_t *)framebuffer, sizeof(framebuffer))) {
            frames++;
        }
        unsigned long int duration_ms = millis() - start;
        int fps = 1000 * frames / duration_ms;
        client.stop();
        print("closed, %d frames/%lu ms = %d fps\n", frames, duration_ms, fps);
    }

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
        case CMD_ARG:
            print("Invalid argument(s)\n");
            break;
        default:
            print("%d\n", result);
            break;
        }
        print(">");
    }
    
    // NTP update
    ntpClient.update();

    // mDNS update
    MDNS.update();
}

