#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <strings.h>
#include <arpa/inet.h>

uint16_t rgb888_to_rgb565(char *rgb) {
	uint8_t r = rgb[0];
	uint8_t g = rgb[1];
	uint8_t b = rgb[2];
        uint16_t rgb565 = ((r << 8) & 0xF800) | ((g << 3) & 0x07E0) | ((b >> 3) & 0x001F);
	return rgb565;
}

#define NUM_PIXELS (80*8)
#define PORT 1565
#define IP_ADDR "10.42.43.53"

// overwrite input buffer
void convert_frame(char *buffer) {
	for (int n = 0; n < NUM_PIXELS; ++n) {
		char *in_data = &buffer[n * 3];
		uint16_t pixel = rgb888_to_rgb565(in_data);
		buffer[(n * 2) + 0] = pixel >> 8;
		buffer[(n * 2) + 1] = pixel & 0xff;
	}
}

int main() {
	int sockfd, n; 
	struct sockaddr_in servaddr; 
      
	bzero(&servaddr, sizeof(servaddr)); 
	servaddr.sin_addr.s_addr = inet_addr(IP_ADDR); 
	servaddr.sin_port = htons(PORT); 
	servaddr.sin_family = AF_INET; 
	sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
	connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	while (1) {
		char buffer[NUM_PIXELS*3];
		read(STDIN_FILENO, buffer, sizeof(buffer));
		write(STDOUT_FILENO, buffer, sizeof(buffer));

		convert_frame(buffer);
		sendto(sockfd, buffer, NUM_PIXELS*2, 0, NULL, NULL);
	}
}

