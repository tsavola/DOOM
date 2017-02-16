#!/usr/bin/env python

from __future__ import print_function

import os
import subprocess
import sys
from select import select
from struct import pack, unpack

import numpy

import pygame
from pygame.locals import *

WIDTH = 320
HEIGHT = 200

SCALE = 4

READ_TIMEOUT = 3

GRAB = False

grabbed = False


def main():
    global grabbed

    pygame.init()
    screen = pygame.display.set_mode((WIDTH * SCALE, HEIGHT * SCALE), 0, 8)
    image = pygame.Surface((WIDTH, HEIGHT), SWSURFACE, 8)

    def ungrab():
        global grabbed

        if grabbed:
            pygame.event.set_grab(False)
            pygame.mouse.set_visible(True)
            grabbed = False

    proc = subprocess.Popen(["npipe"] + sys.argv[1:], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    stdout_fd = proc.stdout.fileno()

    try:
        proc.stdin.write(5 * pack("<II", 8, 0x65766e74))

        clock = pygame.time.Clock()
        font = pygame.font.SysFont(pygame.font.get_default_font(), 24)
        color = pygame.Color(255, 255, 255, 255)

        while True:
            evinfos = ""

            while True:
                event = pygame.event.poll()

                if event.type == NOEVENT:
                    break

                if event.type == KEYDOWN:
                    if event.key == 319:  # menu button
                        ungrab()
                    else:
                        evinfos += pack("<BBI", 0, 0, event.key)
                elif event.type == KEYUP:
                    evinfos += pack("<BBI", 1, 0, event.key)
                elif event.type == MOUSEBUTTONDOWN:
                    if grabbed or not GRAB:
                        evinfos += pack("<BBI", 2, event.button, 0)
                elif event.type == MOUSEBUTTONUP:
                    if grabbed or not GRAB:
                        evinfos += pack("<BBI", 3, event.button, 0)
                    else:
                        pygame.event.set_grab(True)
                        pygame.mouse.set_visible(False)
                        grabbed = True
                elif event.type == MOUSEMOTION:
                    evinfos += pack("<BBhh", 4, 0, event.rel[0], event.rel[1])
                elif event.type == QUIT:
                    sys.exit()

            if evinfos:
                proc.stdin.write(pack("<II", 8 + len(evinfos), 0x65766e74) + evinfos)

            header = read_full(stdout_fd, 8, ungrab)
            size, typ = unpack("<II", header)
            data = read_full(stdout_fd, size - 8, ungrab)

            if typ == 0x66726d65:  # frme
                array = numpy.ndarray((WIDTH, HEIGHT), numpy.int8, data, order="F")
                pygame.surfarray.blit_array(image, array)
                pygame.transform.scale(image, (WIDTH * SCALE, HEIGHT * SCALE), screen)

                clock.tick()
                screen.blit(font.render("FPS: %2.1f" % clock.get_fps(), False, color), (0, 0))

                pygame.display.flip()
            elif typ == 0x706c7474:  # pltt
                palette = [(ord(data[i + 0]), ord(data[i + 1]), ord(data[i + 2])) for i in xrange(0, 256 * 3, 3)]
                screen.set_palette(palette)
            elif typ == 0x65766e74:  # evnt
                pass
            else:
                assert False, hex(typ)
    finally:
        proc.terminate()


def read_full(fd, size, handle_timeout):
    buf = ""

    while len(buf) < size:
        readable, _, _ = select([fd], [], [], READ_TIMEOUT)
        if fd in readable:
            data = os.read(fd, size - len(buf))
            if not data:
                sys.exit(0)

            buf += data
        else:
            handle_timeout()

    return buf


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        pass
