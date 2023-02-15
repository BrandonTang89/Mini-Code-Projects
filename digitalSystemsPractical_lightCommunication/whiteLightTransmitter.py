import pygame
import time

pygame.init()

pygame.display.set_caption('Quick Start')
window_surface = pygame.display.set_mode((800, 600))

background = pygame.Surface((800, 600))
background.fill(pygame.Color('#000000'))

backgroundWhite = pygame.Surface((800, 600))
backgroundWhite.fill(pygame.Color('#FFFFFF'))

baud = 10 # 10 bits per second
message = "brandon"
bits_to_send = [[int(x) for x in bin(int.from_bytes(c.encode(), 'big'))[2:]] for c in message]
bits_to_send = [[0] + bits_for_char + [1, 1, 1] for bits_for_char in bits_to_send] # add 1 start bit and 3 stop bits
bits_to_send = [bit for bits_for_char in bits_to_send for bit in bits_for_char]
print(bits_to_send)

bits_to_send = [1] * 50 + bits_to_send # add preamble
print("length of bits_to_send: ", len(bits_to_send))
bit_index = 0
is_running = True
start_time = time.time_ns()
while bit_index < len(bits_to_send) and is_running:
    if bits_to_send[bit_index] == 0:
        window_surface.blit(background, (0, 0))
    else:
        window_surface.blit(backgroundWhite, (0, 0))
        
    while (time.time_ns() - start_time) < (bit_index * 1e9 / baud):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                is_running = False
                break
            
    print(bits_to_send[bit_index], bit_index)
    bit_index += 1
    if bit_index == len(bits_to_send):
        bit_index = 0
        start_time = time.time_ns()
        
        
    pygame.display.update()