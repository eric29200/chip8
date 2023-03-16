#include <stdlib.h>
#include <string.h>

#include "chip8.h"

/*
 * Clear screen.
 */
void chip8_clear_screen(struct chip8_t *chip8)
{
	memset(chip8->gfx, 0, CHIP8_GFX_SIZE);
	chip8->draw_flag = 1;
	chip8->pc += 2;
}

/*
 * Return from a subroutine.
 */
void chip8_return_subroutine(struct chip8_t *chip8)
{
	chip8->sp--;
	chip8->pc = chip8->stack[chip8->sp];
	chip8->pc += 2;
}

/*
 * Jump to address.
 */
void chip8_jump(struct chip8_t *chip8, uint16_t addr)
{
	chip8->pc = addr;
}

/*
 * Call subroutine.
 */
void chip8_call_subroutine(struct chip8_t *chip8, uint16_t addr)
{
	chip8->stack[chip8->sp] = chip8->pc;
	chip8->sp++;
	chip8->pc = addr;
}

/*
 * Skip next instruction if Vx == val.
 */
void chip8_skip_if_reg_equal_val(struct chip8_t *chip8, uint8_t x, uint8_t val)
{
	if (chip8->V[x] == val)
		chip8->pc += 4;
	else
		chip8->pc += 2;
}

/*
 * Skip next instruction if Vx != val.
 */
void chip8_skip_if_reg_different_val(struct chip8_t *chip8, uint8_t x, uint8_t val)
{
	if (chip8->V[x] != val)
		chip8->pc += 4;
	else
		chip8->pc += 2;
}

/*
 * Skip next instruction if Vx == Vy.
 */
void chip8_skip_if_reg_equal_reg(struct chip8_t *chip8, uint8_t x, uint8_t y)
{
	if (chip8->V[x] == chip8->V[y])
		chip8->pc += 4;
	else
		chip8->pc += 2;
}

/*
 * Skip next instruction if Vx != Vy.
 */
void chip8_skip_if_reg_different_reg(struct chip8_t *chip8, uint8_t x, uint8_t y)
{
	if (chip8->V[x] != chip8->V[y])
		chip8->pc += 4;
	else
		chip8->pc += 2;
}

/*
 * Vx = val.
 */
void chip8_set_reg_val(struct chip8_t *chip8, uint8_t x, uint8_t val)
{
	chip8->V[x] = val;
	chip8->pc += 2;
}

/*
 * Vx += val (Carry Flag is not changed).
 */
void chip8_add_reg_val(struct chip8_t *chip8, uint8_t x, uint8_t val)
{
	chip8->V[x] += val;
	chip8->pc += 2;
}

/*
 * Vx = Vy.
 */
void chip8_set_reg_reg(struct chip8_t *chip8, uint8_t x, uint8_t y)
{
	chip8->V[x] = chip8->V[y];
	chip8->pc += 2;
}

/*
 * Vx |= Vy.
 */
void chip8_or_reg_reg(struct chip8_t *chip8, uint8_t x, uint8_t y)
{
	chip8->V[x] |= chip8->V[y];
	chip8->pc += 2;
}

/*
 * Vx &= Vy.
 */
void chip8_and_reg_reg(struct chip8_t *chip8, uint8_t x, uint8_t y)
{
	chip8->V[x] &= chip8->V[y];
	chip8->pc += 2;
}

/*
 * Vx ^= Vy.
 */
void chip8_xor_reg_reg(struct chip8_t *chip8, uint8_t x, uint8_t y)
{
	chip8->V[x] ^= chip8->V[y];
	chip8->pc += 2;
}

/*
 * Vx += Vy.
 */
void chip8_add_reg_reg(struct chip8_t *chip8, uint8_t x, uint8_t y)
{
	chip8->V[x] += chip8->V[y];

	/* Set Carry Flag */
	if (chip8->V[y] > (0xFF - chip8->V[x]))
		chip8->V[0xF] = 1;
	else
		chip8->V[0xF] = 0;

	chip8->pc += 2;
}

/*
 * Vx -= Vy.
 */
void chip8_sub_reg_reg(struct chip8_t *chip8, uint8_t x, uint8_t y)
{
	/* Set Borrow flag */
	if (chip8->V[y] > chip8->V[x])
		chip8->V[0xF] = 0;
	else
		chip8->V[0xF] = 1;

	chip8->V[x] -= chip8->V[y];
	chip8->pc += 2;
}

/*
 * Vx >>= 1.
 */
void chip8_rshift_reg(struct chip8_t *chip8, uint8_t x)
{
	/* store least significant bit in Vf */
	chip8->V[0xF] = chip8->V[x] & 0x1;

	/* right shift */
	chip8->V[x] >>= 1;
	chip8->pc += 2;
}

/*
 * Vx <<= 1.
 */
void chip8_lshift_reg(struct chip8_t *chip8, uint8_t x)
{
	/* store most significant bit in Vf */
	chip8->V[0xF] = chip8->V[x] >> 7;

	/* right shift */
	chip8->V[x] <<= 1;
	chip8->pc += 2;

}

/*
 * Vx = Vy - Vx.
 */
void chip8_sub_reg_reg_inv(struct chip8_t *chip8, uint8_t x, uint8_t y)
{
	/* Set Borrow flag */
	if (chip8->V[x] > chip8->V[y])
		chip8->V[0xF] = 0;
	else
		chip8->V[0xF] = 1;

	chip8->V[x] = chip8->V[y] - chip8->V[x];
	chip8->pc += 2;
}

/*
 * I = addr.
 */
void chip8_set_I(struct chip8_t *chip8, uint16_t addr)
{
	chip8->I = addr;
	chip8->pc += 2;
}

/*
 * PC = V0 + addr.
 */
void chip8_jump_plus_v0(struct chip8_t *chip8, uint16_t addr)
{
	chip8->pc = chip8->V[0] + addr;
}

/*
 * Vx = rand() & val.
 */
void chip8_rand(struct chip8_t *chip8, uint8_t x, uint8_t val)
{
	chip8->V[x] = (rand() % (0xFF + 1)) & val;
	chip8->pc += 2;
}

/*
 * Draw a sprite at coordinate (Vx ; Vy) of width = 8 and height.
 * Each row of 8 pixels is read from memory location I.
 * Vf is set to 1 if any pixels are flipped. 
 */
void chip8_draw(struct chip8_t *chip8, uint8_t x, uint8_t y, uint8_t height)
{
	int i, j, val, pos, collision = 0;

	/* reset Vf */
	chip8->V[0xF] = 0;

	/* draw sprite */
	for (i = 0; i < height; i++) {
		for (j = 0; j < 8; j++) {
			/* get value */
			val = chip8->memory[chip8->I + i] >> (7 - j) & 1;
			
			/* get pixel position */
			pos = CHIP8_GFX_WIDTH * ((y + i) % CHIP8_GFX_HEIGHT) + ((x + j) % CHIP8_GFX_WIDTH);

			/* detect collision */
			collision |= chip8->gfx[pos] && val;

			/* update gfx */
			chip8->gfx[pos] ^= val;
		}
	}

	/* set Vf if collisions occured */
	if (collision)
		chip8->V[0xF] = 1;
	
	/* set draw flag */
	chip8->draw_flag = 1;
	chip8->pc += 2;
}


/*
 * Skip next instruction if the key stored in Vx is pressed.
 */
void chip8_skip_if_key_pressed(struct chip8_t *chip8, uint8_t x)
{
	if (chip8->key[chip8->V[x]] != 0)
		chip8->pc += 4;
	else
		chip8->pc += 2;
}

/*
 * Skip next instruction if the key stored in Vx is not pressed.
 */
void chip8_skip_if_key_not_pressed(struct chip8_t *chip8, uint8_t x)
{
	if (chip8->key[chip8->V[x]] == 0)
		chip8->pc += 4;
	else
		chip8->pc += 2;
}

/*
 * Vx = delay_timer.
 */
void chip8_get_delay(struct chip8_t *chip8, uint8_t x)
{
	chip8->V[x] = chip8->delay_timer;
	chip8->pc += 2;
}

/*
 * Wait for a key press and store it in Vx (blocking operation).
 */
void chip8_get_key(struct chip8_t *chip8, uint8_t x)
{
	int i, key_pressed = 0;

	/* check if a key is pressed */
	for (i = 0; i < CHIP8_NR_KEYS; i++) {
		if (chip8->key[i] != 0) {
			chip8->V[x] = i;
			key_pressed = 1;
		}
	}

	/* no key pressed : exit and don't increment pc */
	if (!key_pressed)
		return;

	chip8->pc += 2;
}

/*
 * delay_timer = Vx.
 */
void chip8_set_delay_timer(struct chip8_t *chip8, uint8_t x)
{
	chip8->delay_timer = chip8->V[x];
	chip8->pc += 2;
}

/*
 * sound_timer = Vx.
 */
void chip8_set_sound_timer(struct chip8_t *chip8, uint8_t x)
{
	chip8->sound_timer = chip8->V[x];
	chip8->pc += 2;
}

/*
 * I += Vx.
 */
void chip8_add_vx_to_i(struct chip8_t *chip8, uint8_t x)
{
	if (chip8->I + chip8->V[x] > 0xFFFF)
		chip8->V[0xF] = 1;
	else
		chip8->V[0xF] = 0;

	chip8->I += chip8->V[x];
	chip8->pc += 2;
}

/*
 * Set I to the location of the sprite for the character in Vx.
 */
void chip8_sprite_addr(struct chip8_t *chip8, uint8_t x)
{
	chip8->I = chip8->V[x] * CHIP8_FONT_SIZE;
	chip8->pc += 2;
}

/*
 * Store the binary-coded decimal representation of Vx at I, I+1 and I+2.
 */
void chip8_bcd(struct chip8_t *chip8, uint8_t x)
{
	chip8->memory[chip8->I] = chip8->V[x] / 100;
	chip8->memory[chip8->I + 1] = (chip8->V[x] / 10) % 10;
	chip8->memory[chip8->I + 2] = chip8->V[x] % 10;
	chip8->pc += 2;
}

/*
 * Store registers from V0 to Vx (including Vx) at I.
 */
void chip8_reg_dump(struct chip8_t *chip8, uint8_t x)
{
	int i;

	for (i = 0; i <= x; i++)
		chip8->memory[chip8->I + i] = chip8->V[i];

 	chip8->I += x + 1;
	chip8->pc += 2;
}

/*
 * Fills registers from V0 to Vx (including Vx) with values stored at I.
 */
void chip8_reg_load(struct chip8_t *chip8, uint8_t x)
{
	int i;

	for (i = 0; i <= x; i++)
		chip8->V[i] = chip8->memory[chip8->I + i];

 	chip8->I += x + 1;
	chip8->pc += 2;
}
