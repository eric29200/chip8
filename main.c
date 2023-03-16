#include <unistd.h>
#include <gtk/gtk.h>

#include "chip8.h"

#define WINDOW_WIDTH		800
#define WINDOW_HEIGHT		600
#define UNUSED(x)		((void) (x))

/*
 * Chip8 emulator.
 */
struct chip8_emulator_t {
	struct chip8_t		chip8;			/* chip8 device */
	GtkWidget *		window;			/* main window */
	GtkWidget *		frame;			/* main frame */
	GdkPixbuf *		pixbuf;			/* pix buf */
	GtkWidget *		drawing_area;		/* drawing area */
	gint64			prev_tick_time;		/* previous tick time */
};

/*
 * Exit handler.
 */
static void exit_cb(GtkWidget *widget)
{
	UNUSED(widget);
	gtk_main_quit();
}

/*
 * Draw callback.
 */
static gboolean draw_cb(GtkWidget *widget, cairo_t *cr, gpointer data)
{
	struct chip8_emulator_t *emu = (struct chip8_emulator_t *) data;
	GdkPixbuf *scaled;

	UNUSED(widget);

	/* paint surface */
	scaled = gdk_pixbuf_scale_simple(emu->pixbuf, WINDOW_WIDTH, WINDOW_HEIGHT, GDK_INTERP_NEAREST);
	gdk_cairo_set_source_pixbuf(cr, scaled, 0, 0);
	cairo_paint(cr);
	g_object_unref(scaled);

	return TRUE;
}

/*
 * Key pressed/released callback.
 */
static void key_cb(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	struct chip8_emulator_t *emu = (struct chip8_emulator_t *) data;
	int i;

	UNUSED(widget);

	if (event->keyval > 0xFF)
		return;
	
	/* find matching chip8 key */
	for (i = 0; i < CHIP8_NR_KEYS; i++)
		if (event->keyval == chip8_keymap[i])
			break;

	/* no matching key */
	if (i >= CHIP8_NR_KEYS)
		return;

	/* set chip8 key */
	if (event->type == GDK_KEY_PRESS)
		emu->chip8.key[i] = 1;
	else
		emu->chip8.key[i] = 0;
}

/*
 * Tick callback.
 */
static gboolean tick_cb(GtkWidget *widget, GdkFrameClock *frame_clock, gpointer data)
{
	struct chip8_emulator_t *emu = (struct chip8_emulator_t *) data;
	int ret, x, y, i, nb_chip8_ticks;
	gint64 current_time;
	guchar *pixels;
	uint8_t val;
	guint ptr;

	/* unused variables */
	UNUSED(widget);

	/* get number of chip8 ticks to emulate */
	current_time = gdk_frame_clock_get_frame_time(frame_clock);
	if (!emu->prev_tick_time)
		nb_chip8_ticks = 1;
	else
		nb_chip8_ticks = (current_time - emu->prev_tick_time) / CHIP8_TICK_FREQ_US;

	/* update previous tick time */
	emu->prev_tick_time = current_time;

	/* emulate chip8 */
	for (i = 0; i < nb_chip8_ticks; i++) {
		/* next tick */
		ret = chip8_tick(&emu->chip8);
		if (ret)
			exit(EXIT_FAILURE);

		/* redraw if needed */
		if (emu->chip8.draw_flag) {
			/* get gtk pixels */
			pixels = gdk_pixbuf_get_pixels(emu->pixbuf);

			/* draw gfx */
			for (y = 0; y < CHIP8_GFX_HEIGHT; y++) {
				for (x = 0; x < CHIP8_GFX_WIDTH; x++) {
					/* get pixel value */
					ptr = y * CHIP8_GFX_WIDTH + x;
					val = emu->chip8.gfx[ptr] ? 0xFF : 0x00;

					/* set gtk pixel */
					ptr *= 3;
					pixels[ptr] = val;
					pixels[ptr + 1] = val;
					pixels[ptr + 1] = val;
				}
			}

			/* queue drawing area */
			gtk_widget_queue_draw(emu->drawing_area);
			
			/* mark gfx clean */
			emu->chip8.draw_flag = 0;
		}
	}

	return G_SOURCE_CONTINUE;
}

/*
 * Create chip8 emulator.
 */
struct chip8_emulator_t *chip8_emulator_create()
{
	struct chip8_emulator_t *emu;

	/* allocate emulator */
	emu = (struct chip8_emulator_t *) malloc(sizeof(struct chip8_emulator_t));
	if (!emu)
		return NULL;

	/* init emulator */
	emu->prev_tick_time = 0;

	/* create main window */
	emu->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(emu->window), "Chip-8 emulator");
	gtk_window_set_default_size(GTK_WINDOW(emu->window), WINDOW_WIDTH, WINDOW_HEIGHT);
	gtk_container_set_border_width(GTK_CONTAINER(emu->window), 8);

	/* create frame */
	emu->frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(emu->frame), GTK_SHADOW_IN);

	/* create drawing area */
	emu->pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, CHIP8_GFX_WIDTH, CHIP8_GFX_HEIGHT);
	emu->drawing_area = gtk_drawing_area_new();

	/* pack widgets */
	gtk_container_add(GTK_CONTAINER(emu->window), emu->frame);
	gtk_container_add(GTK_CONTAINER(emu->frame), emu->drawing_area);

	/* signal handlers */
	g_signal_connect(G_OBJECT(emu->window), "destroy", G_CALLBACK(exit_cb), emu);
	g_signal_connect(G_OBJECT(emu->window), "key_press_event", G_CALLBACK(key_cb), emu);
	g_signal_connect(G_OBJECT(emu->window), "key_release_event", G_CALLBACK(key_cb), emu);
	g_signal_connect(G_OBJECT(emu->drawing_area), "draw", G_CALLBACK(draw_cb), emu);
	gtk_widget_add_tick_callback(emu->drawing_area, tick_cb, emu, NULL);

	return emu;
}

/*
 * Main.
 */
int main(int argc, char **argv)
{
	struct chip8_emulator_t *emu;
	int ret;
	
	/* init gtk */
	gtk_init(&argc, &argv);

	/* check arguments */
	if (argc != 2) {
		printf("Usage: %s <rom>\n", argv[0]);
		return EXIT_FAILURE;
	}

	/* create chip8 emulator */
	emu = chip8_emulator_create();
	if (!emu) {
		fprintf(stderr, "Can't create chip8 emulator\n");
		return EXIT_FAILURE;
	}

	/* load rom */
	ret = chip8_load_rom(&emu->chip8, argv[1]);
	if (ret) {
		fprintf(stderr, "Can't load ROM \"%s\"\n", argv[1]);
		return EXIT_FAILURE;
	}

	/* show main window */
	gtk_widget_show_all(emu->window);
	gtk_main();

	return EXIT_SUCCESS;
}
