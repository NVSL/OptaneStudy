#!/usr/bin/env python3
import repeat
import os

stride_array=[0, 64, 128, 192, 256, 320, 384, 448, 512, 768, 1024, 1280, 1536, 2048]
tag_array=['ldram', 'lpmem', 'ldram_emon', 'lpmem_emon', 'lpmem_18', 'ldram_18']


for sizebit in range(24, 30):
    size = 2**sizebit;
    for tag in tag_array:
        tag = 'AEP2-5-256-{0}-{1}'.format(size, tag)
        repeat.download(tag)
        repeat.parse(tag)

