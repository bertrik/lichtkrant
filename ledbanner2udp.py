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

def convert_frame(data):
    """ Converts the supplied rgb888 frame and returns a rgb565 frame  """
    num = len(data) // 3
    out = bytearray(2 * num)
    pi = po = 0
    for i in range(0, num):
        rgb888 = data[pi:pi+3]
        pi += 3
        out[po:po+2] = rgb888_to_rgb565(rgb888)
        po += 2
    return out

def main():
    """ The main entry point """
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--ip", type=str, help="The UDP host ip", default="localhost")
    parser.add_argument("-p", "--port", type=int, help="The UDP host port", default="1565")
    args = parser.parse_args()

    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    while True:
        # read ledbanner frame from stdin
        data = sys.stdin.buffer.read(80*8*3)
        if len(data) != 80*8*3:
            break

        # copy to stdout
        sys.stdout.buffer.write(data)

        # convert
        frame = convert_frame(data)

        # send over UDP
        s.sendto(frame, (args.ip, args.port))

if __name__ == "__main__":
    main()

