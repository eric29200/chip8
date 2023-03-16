#ifndef _CHIP8_H_
#define _CHIP8_H_

#include <stdint.h>

#define CHIP8_MEMORY_SIZE		4096
#define CHIP8_MEMORY_ROM_START		0X200
#define CHIP8_GFX_WIDTH			64
#define CHIP8_GFX_HEIGHT		32
#define CHIP8_STACK_SIZE		16
#define CHIP8_NR_REGISTERS		16
#define CHIP8_NR_KEYS			16
#define CHIP8_FONT_SIZE			5
#define CHIP8_TICK_FREQ_US		1800

#define CHIP8_GFX_SIZE			(CHIP8_GFX_WIDTH * CHIP8_GFX_HEIGHT)

/*
 * Chip8 structure.
 */
struct chip8_t {
	uint16_t	stack[CHIP8_STACK_SIZE];	/* stack */
	uint16_t	sp;				/* stack pointer */
	uint8_t		V[CHIP8_NR_REGISTERS];		/* registers */
	uint8_t		memory[CHIP8_MEMORY_SIZE];	/* memory */
	uint16_t	pc;				/* program counter */
	uint16_t	I;				/* index register */
	uint8_t		delay_timer;			/* delay timer */
	uint8_t		sound_timer;			/* sound timer */
	uint8_t		gfx[CHIP8_GFX_SIZE];		/* graphics buffer */
	uint8_t		key[CHIP8_NR_KEYS];		/* keypad */
	char		draw_flag;			/* draw flag : 1 if screen is dirty */
};

extern uint8_t chip8_keymap[];

/* prototypes */
void chip8_init(struct chip8_t *chip8);
int chip8_load_rom(struct chip8_t *chip8, const char *path);
int chip8_tick(struct chip8_t *chip8);

/* instructions */
void chip8_clear_screen(struct chip8_t *chip8);
void chip8_return_subroutine(struct chip8_t *chip8);
void chip8_jump(struct chip8_t *chip8, uint16_t addr);
void chip8_call_subroutine(struct chip8_t *chip8, uint16_t addr);
void chip8_skip_if_reg_equal_val(struct chip8_t *chip8, uint8_t x, uint8_t val);
void chip8_skip_if_reg_different_val(struct chip8_t *chip8, uint8_t x, uint8_t val);
void chip8_skip_if_reg_equal_reg(struct chip8_t *chip8, uint8_t x, uint8_t y);
void chip8_skip_if_reg_different_reg(struct chip8_t *chip8, uint8_t x, uint8_t y);
void chip8_set_reg_val(struct chip8_t *chip8, uint8_t x, uint8_t val);
void chip8_add_reg_val(struct chip8_t *chip8, uint8_t x, uint8_t val);
void chip8_set_reg_reg(struct chip8_t *chip8, uint8_t x, uint8_t y);
void chip8_or_reg_reg(struct chip8_t *chip8, uint8_t x, uint8_t y);
void chip8_and_reg_reg(struct chip8_t *chip8, uint8_t x, uint8_t y);
void chip8_xor_reg_reg(struct chip8_t *chip8, uint8_t x, uint8_t y);
void chip8_add_reg_reg(struct chip8_t *chip8, uint8_t x, uint8_t y);
void chip8_sub_reg_reg(struct chip8_t *chip8, uint8_t x, uint8_t y);
void chip8_rshift_reg(struct chip8_t *chip8, uint8_t x);
void chip8_lshift_reg(struct chip8_t *chip8, uint8_t x);
void chip8_sub_reg_reg_inv(struct chip8_t *chip8, uint8_t x, uint8_t y);
void chip8_set_I(struct chip8_t *chip8, uint16_t addr);
void chip8_jump_plus_v0(struct chip8_t *chip8, uint16_t addr);
void chip8_rand(struct chip8_t *chip8, uint8_t x, uint8_t val);
void chip8_draw(struct chip8_t *chip8, uint8_t x, uint8_t y, uint8_t height);
void chip8_skip_if_key_pressed(struct chip8_t *chip8, uint8_t x);
void chip8_skip_if_key_not_pressed(struct chip8_t *chip8, uint8_t x);
void chip8_get_delay(struct chip8_t *chip8, uint8_t x);
void chip8_get_key(struct chip8_t *chip8, uint8_t x);
void chip8_set_delay_timer(struct chip8_t *chip8, uint8_t x);
void chip8_set_sound_timer(struct chip8_t *chip8, uint8_t x);
void chip8_add_vx_to_i(struct chip8_t *chip8, uint8_t x);
void chip8_sprite_addr(struct chip8_t *chip8, uint8_t x);
void chip8_bcd(struct chip8_t *chip8, uint8_t x);
void chip8_reg_dump(struct chip8_t *chip8, uint8_t x);
void chip8_reg_load(struct chip8_t *chip8, uint8_t x);

#endif
