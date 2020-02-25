#!/usr/bin/env python3

"""
 Bridge between the revspace ledbanner and a 80x7 scrolling led sign (red-green)

 - accepts revspace ledbanner frames (80x8 pixels raw RGB) from stdin
 - copies each frame to stdout
 - converts each frame to 80x8 pixels rgb565 which is sent using UDP to a host on the network
"""

import sys
import argparse
import socket

def rgb888_to_rgb565(rgb):
    r, g, b = rgb
    rgb565 = ((r << 8) & 0xF800) | ((g << 3) & 0x07E0) | ((b >> 3) & 0x001F)
    return rgb565.to_bytes(2, byteorder='big')

def convert(host, port):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    while True:
        # read ledbanner frame from stdin
        data = sys.stdin.buffer.read(80*8*3)
        if len(data) != 80*8*3:
            break

        # copy to stdout
        sys.stdout.buffer.write(data)

        # convert to RGB565
        out = bytearray()
        for y in range(8):
            for x in range(80):
                pos = (y * 80 + x) * 3
                rgb888 = data[pos:pos+3]
                rgb565 = rgb888_to_rgb565(rgb888)
                out += rgb565

        # send over UDP
        s.sendto(out, (host, port))

def main():
    """ The main entry point """
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--ip", type=str, help="The UDP host ip", default="localhost")
    parser.add_argument("-p", "--port", type=int, help="The UDP host port", default="1565")
    args = parser.parse_args()

    convert(args.ip, args.port)

if __name__ == "__main__":
    main()

