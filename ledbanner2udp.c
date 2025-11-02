#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>

static uint16_t rgb888_to_rgb565(const char *rgb)
{
    uint8_t r = rgb[0];
    uint8_t g = rgb[1];
    uint8_t b = rgb[2];
    uint16_t rgb565 = ((r << 8) & 0xF800) | ((g << 3) & 0x07E0) | ((b >> 3) & 0x001F);
    return rgb565;
}

#define NUM_PIXELS (80 * 8)
#define PORT 1565
#define DEFAULT_IP "239.0.0.1"

static void convert_frame(char *buffer)
{
    for (int n = 0; n < NUM_PIXELS; ++n) {
        char *in_data = &buffer[n * 3];
        uint16_t pixel = rgb888_to_rgb565(in_data);
        buffer[(n * 2) + 0] = pixel >> 8;
        buffer[(n * 2) + 1] = pixel & 0xff;
    }
}

// Helper: detect if IPv4 address is multicast (224.0.0.0–239.255.255.255)
static bool is_multicast_address(uint32_t addr)
{
    uint8_t first_octet = (addr >> 24) & 0xFF;
    return (first_octet >= 224 && first_octet <= 239);
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in destaddr;
    const char *ip_addr = (argc > 1) ? argv[1] : DEFAULT_IP;

    memset(&destaddr, 0, sizeof(destaddr));
    destaddr.sin_family = AF_INET;
    destaddr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, ip_addr, &destaddr.sin_addr) <= 0) {
        perror("Invalid IP address");
        return 1;
    }

    uint32_t ip_be = ntohl(destaddr.sin_addr.s_addr);
    bool is_multicast = is_multicast_address(ip_be);

    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    if (is_multicast) {
        // Configure multicast TTL = 1 (stay in local network)
        unsigned char ttl = 1;
        if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
            perror("setsockopt(IP_MULTICAST_TTL)");
        }
        printf("→ Sending MULTICAST to %s:%d\n", ip_addr, PORT);
    } else {
        printf("→ Sending UNICAST to %s:%d\n", ip_addr, PORT);
    }

    // Main loop
    while (1) {
        char buffer[NUM_PIXELS * 3];
        ssize_t r = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (r <= 0)
            break;

        write(STDOUT_FILENO, buffer, r);

        convert_frame(buffer);
        sendto(sockfd, buffer, NUM_PIXELS * 2, 0, (struct sockaddr *)&destaddr, sizeof(destaddr));
    }

    close(sockfd);
    return 0;
}
