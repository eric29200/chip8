#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "chip8.h"

/*
 * Chip8 fontset.
 */
static uint8_t chip8_fontset[] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, 	/* 0 */
	0x20, 0x60, 0x20, 0x20, 0x70, 	/* 1 */
	0xF0, 0x10, 0xF0, 0x80, 0xF0, 	/* 2 */
	0xF0, 0x10, 0xF0, 0x10, 0xF0, 	/* 3 */
	0x90, 0x90, 0xF0, 0x10, 0x10, 	/* 4 */
	0xF0, 0x80, 0xF0, 0x10, 0xF0, 	/* 5 */
	0xF0, 0x80, 0xF0, 0x90, 0xF0, 	/* 6 */
	0xF0, 0x10, 0x20, 0x40, 0x40, 	/* 7 */
	0xF0, 0x90, 0xF0, 0x90, 0xF0, 	/* 8 */
	0xF0, 0x90, 0xF0, 0x10, 0xF0, 	/* 9 */
	0xF0, 0x90, 0xF0, 0x90, 0x90, 	/* A */
	0xE0, 0x90, 0xE0, 0x90, 0xE0, 	/* B */
	0xF0, 0x80, 0x80, 0x80, 0xF0, 	/* C */
	0xE0, 0x90, 0x90, 0x90, 0xE0, 	/* D */
	0xF0, 0x80, 0xF0, 0x80, 0xF0, 	/* E */
	0xF0, 0x80, 0xF0, 0x80, 0x80  	/* F */
};

/*
 * Chip8 key map.
 */
uint8_t chip8_keymap[] = {
	'x', '1', '2', '3', 'q', 'w', 'e', 'a', 's', 'd', 'z', 'c', '4', 'r', 'f', 'v'
};

/*
 * Init chip8.
 */
void chip8_init(struct chip8_t *chip8)
{
	int i;

	/* clear chip8 */
	memset(chip8, 0, sizeof(struct chip8_t));

	/* set program counter to 0x200 */
	chip8->pc = CHIP8_MEMORY_ROM_START;

	/* load fontset in memory */
	for (i = 0; i < CHIP8_NR_KEYS * CHIP8_FONT_SIZE; i++)
		chip8->memory[i] = chip8_fontset[i];

	/* seed */
	srand(time(NULL));
}

/*
 * Load a ROM.
 */
int chip8_load_rom(struct chip8_t *chip8, const char *path)
{
	int ret = EXIT_FAILURE;
	FILE *rom_fp = NULL;
	size_t rom_size;

	/* reset chip8 */
	chip8_init(chip8);

	/* open rom */
	rom_fp = fopen(path, "rb");
	if (!rom_fp)
		goto out;

	/* get rom size */
	fseek(rom_fp, 0, SEEK_END);
	rom_size = ftell(rom_fp);
	fseek(rom_fp, 0, SEEK_SET);

	/* check rom size */
	if (rom_size > CHIP8_MEMORY_SIZE - CHIP8_MEMORY_ROM_START)
		goto out;

	/* load rom into memory */
	if (fread(chip8->memory + CHIP8_MEMORY_ROM_START, sizeof(char), rom_size, rom_fp) != rom_size)
		goto out;

	ret = EXIT_SUCCESS;
out:
	if (rom_fp)
		fclose(rom_fp);

	return ret;
}

/*
 * Handle next tick.
 */
int chip8_tick(struct chip8_t *chip8)
{
	uint16_t opcode;

	/* fetch next opcode */
	opcode = (chip8->memory[chip8->pc] << 8) | chip8->memory[chip8->pc + 1];

	/* process opcode */
	switch (opcode & 0xF000) {
		case 0x0000:
			switch (opcode & 0x00FF) {
				case 0x00E0:								/* 00E0 -> clear screen */
					chip8_clear_screen(chip8);
					break;
				case 0x00EE:								/* 00EE -> return from a subroutine */
					chip8_return_subroutine(chip8);
					break;
				default:
					goto err_opcode;
			}

			break;
		case 0x1000:										/* 1NNN -> jump to address NNN */
			chip8_jump(chip8, opcode & 0x0FFF);
			break;
		case 0x2000:										/* 2NNN -> call subroutine at NNN */
			chip8_call_subroutine(chip8, opcode & 0x0FFF);
			break;
		case 0x3000:										/* 3XNN -> skip next instruction if V[X] == NN */
			chip8_skip_if_reg_equal_val(chip8, (opcode & 0x0F00) >> 8, opcode & 0x00FF);
			break;
		case 0x4000:										/* 3XNN -> skip next instruction if V[X] != NN */
			chip8_skip_if_reg_different_val(chip8, (opcode & 0x0F00) >> 8, opcode & 0x00FF);
			break;
		case 0x5000:										/* 5XY0 -> skip next instruction if V[X] == V[Y] */
			chip8_skip_if_reg_equal_reg(chip8, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4);
			break;
		case 0x6000:										/* 6XNN -> V[X] = NN */
			chip8_set_reg_val(chip8, (opcode & 0x0F00) >> 8, opcode & 0x00FF);
			break;
		case 0x7000:										/* 7XNN -> V[X] += NN */
			chip8_add_reg_val(chip8, (opcode & 0x0F00) >> 8, opcode & 0x00FF);
			break;
		case 0x8000:
			switch (opcode & 0x000F) {
				case 0x0000:								/* 8XY0 -> V[X] = V[Y] */
					chip8_set_reg_reg(chip8, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4);
					break;
				case 0x0001:								/* 8XY1 -> V[X] |= V[Y] */
					chip8_or_reg_reg(chip8, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4);
					break;
				case 0x0002:								/* 8XY2 -> V[X] &= V[Y] */
					chip8_and_reg_reg(chip8, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4);
					break;
				case 0x0003:								/* 8XY3 -> V[X] ^= V[Y] */
					chip8_xor_reg_reg(chip8, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4);
					break;
				case 0x0004:								/* 8XY4 -> V[X] += V[Y] */
					chip8_add_reg_reg(chip8, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4);
					break;
				case 0x0005:								/* 8XY5 -> V[X] -= V[Y] */
					chip8_sub_reg_reg(chip8, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4);
					break;
				case 0x0006:								/* 8XY6 -> V[X] >>= 1 */
					chip8_rshift_reg(chip8, (opcode & 0x0F00) >> 8);
					break;
				case 0x0007:								/* 8XY7 -> V[X] = V[Y] - V[X] */
					chip8_sub_reg_reg_inv(chip8, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4);
					break;
				case 0x000E:								/* 8XYE -> V[X] <<= 1 */
					chip8_lshift_reg(chip8, (opcode & 0x0F00) >> 8);
					break;
				default:
					goto err_opcode;
			}

			break;
		case 0x9000:										/* 9XY0 -> skip next instruction if V[X] != V[Y] */
			chip8_skip_if_reg_different_reg(chip8, (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4);
			break;
		case 0xA000:										/* ANNN -> I = NNN */
			chip8_set_I(chip8, opcode & 0x0FFF);
			break;
		case 0xB000:										/* BNNN -> jump to the address NNN + V[0] */
			chip8_jump_plus_v0(chip8, opcode & 0x0FFF);
			break;
		case 0xC000:										/* CXNN -> V[X] = rand() & NN */
			chip8_rand(chip8, (opcode & 0x0F00) >> 8, opcode & 0x00FF);
			break;
		case 0xD000:										/* DXYN -> draw at (Vx ; Vy) of height N */
			chip8_draw(chip8, chip8->V[(opcode & 0x0F00) >> 8], chip8->V[(opcode & 0x00F0) >> 4], opcode & 0x000F);
			break;
		case 0xE000:
			switch (opcode & 0x00FF) {
				case 0x009E:								/* EX9E -> skip next instruction if key in V[X] is pressed */
					chip8_skip_if_key_pressed(chip8, (opcode & 0x0F00) >> 8);
					break;
				case 0x00A1:								/* EXA1 -> skip next instruction if key in V[X] is not pressed */
					chip8_skip_if_key_not_pressed(chip8, (opcode & 0x0F00) >> 8);
					break;
				default:
					goto err_opcode;
			}

			break;
		case 0xF000:
			switch (opcode & 0x00FF) {
				case 0x0007:								/* FX07 -> V[X] = get_delay() : blocking instruction */
				 	chip8_get_delay(chip8, (opcode & 0x0F00) >> 8);
					break;
				case 0x000A:								/* FX0A -> V[X] = get_key() */
					chip8_get_key(chip8, (opcode & 0x0F00) >> 8);
					break;
				case 0x0015:								/* FX15 -> set_delay(V[X]) */
				 	chip8_set_delay_timer(chip8, (opcode & 0x0F00) >> 8);
					break;
				case 0x0018:								/* FX18 -> set_sound(V[X]) */
				 	chip8_set_sound_timer(chip8, (opcode & 0x0F00) >> 8);
					break;
				case 0x001E:								/* FX1E -> I += V[X] */
					chip8_add_vx_to_i(chip8, (opcode & 0x0F00) >> 8);
					break;
				case 0x0029:								/* FX29 -> I = sprite_addr(X) */
					chip8_sprite_addr(chip8, (opcode & 0x0F00) >> 8);
					break;
				case 0x0033:								/* FX33 -> store Binary Coded Decimal at I, I+1 and I+2*/
				 	chip8_bcd(chip8, (opcode & 0x0F00) >> 8);
					break;
				case 0x0055:								/* FX55 -> reg_dump(V[X], &I) */
					chip8_reg_dump(chip8, (opcode & 0x0F00) >> 8);
					break;
				case 0x0065:								/* FX65 -> reg_load(V[X], &I) */
					chip8_reg_load(chip8, (opcode & 0x0F00) >> 8);
					break;
				default:
					goto err_opcode;
			}

			break;
		default:
			goto err_opcode;
	}

	/* update delay timer */
	if (chip8->delay_timer > 0)
		chip8->delay_timer--;

	/* update sound timer */
	if (chip8->sound_timer > 0)
		chip8->sound_timer--;

	return EXIT_SUCCESS;
err_opcode:
	fprintf(stderr, "Unknown opcode %x\n", opcode);
	return EXIT_FAILURE;
}
