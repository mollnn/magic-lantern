/** \file
 * Magic Lantern debugging and reverse engineering code
 */
#include "dryos.h"
#include "bmp.h"
#include "tasks.h"
#include "debug.h"
#include "menu.h"
#include "property.h"
#include "config.h"
#include "gui.h"
#include "lens.h"
//#include "lua.h"

#if defined(CONFIG_50D)// || defined(CONFIG_60D)
#define CONFIG_KILL_FLICKER // this will block all Canon drawing routines when the camera is idle 
#endif                      // but it will display ML graphics


#ifdef CONFIG_1100D
#include "disable-this-module.h"
#endif
extern int config_autosave;
extern void config_autosave_toggle(void* unused);

void Beep();
void NormalDisplay();
void MirrorDisplay();
void HijackFormatDialogBox_main();
void config_menu_init();
void display_on();
void display_on_force();
void display_off();
void display_off_force();

void fake_halfshutter_step();


//~ CONFIG_INT("halfshutter.fake", fake_halfshutter, 0);

//////////////////////////////////////////////////////////
// debug manager enable/disable
//////////////////////////////////////////////////////////

CONFIG_INT("dm.enable", dm_enable, 0);

static void dm_update()
{
	if (dm_enable) dmstart();
	else dmstop();
}

static void
dm_display(
	void *			priv,
	int			x,
	int			y,
	int			selected
)
{
	bmp_printf(
		selected ? MENU_FONT_SEL : MENU_FONT,
		x, y,
		"Debug logging  : %s",
		dm_enable ? "ON, Q=dump" : "OFF,Q=dump"
	);
}

static void dm_toggle(void* priv)
{
	dm_enable = !dm_enable;
	dm_update();
}
//////////////////////////////////////////////////////////

//~ extern void bootdisk_disable();


void take_screenshot( void * priv )
{
	Beep();
	call( "dispcheck" );
	silent_pic_take_lv_dbg();
}

int draw_prop = 0;

static void
draw_prop_select( void * priv )
{
	draw_prop = !draw_prop;
}

static int dbg_propn = 0;
static void 
draw_prop_reset( void * priv )
{
	dbg_propn = 0;
}

int mem_spy = 0;

#if CONFIG_DEBUGMSG
int mem_spy_start = 0; // start from here
int mem_spy_bool = 0;           // only display booleans (0,1,-1)
int mem_spy_fixed_addresses = 0; // only look from a list of fixed addresses
const int mem_spy_addresses[] = {};//0xc0000044, 0xc0000048, 0xc0000057, 0xc00011cf, 0xc02000a8, 0xc02000ac, 0xc0201004, 0xc0201010, 0xc0201100, 0xc0201104, 0xc0201200, 0xc0203000, 0xc020301c, 0xc0203028, 0xc0203030, 0xc0203034, 0xc020303c, 0xc0203044, 0xc0203048, 0xc0210200, 0xc0210208, 0xc022001c, 0xc0220028, 0xc0220034, 0xc0220070, 0xc02200a4, 0xc02200d0, 0xc02200d4, 0xc02200d8, 0xc02200e8, 0xc02200ec, 0xc0220100, 0xc0220104, 0xc022010c, 0xc0220118, 0xc0220130, 0xc0220134, 0xc0220138, 0xc0222000, 0xc0222004, 0xc0222008, 0xc022200c, 0xc0223000, 0xc0223010, 0xc0223060, 0xc0223064, 0xc0223068, 0xc0224100, 0xc0224104, 0xc022d000, 0xc022d02c, 0xc022d074, 0xc022d1ec, 0xc022d1f0, 0xc022d1f4, 0xc022d1f8, 0xc022d1fc, 0xc022dd14, 0xc022f000, 0xc022f004, 0xc022f200, 0xc022f210, 0xc022f214, 0xc022f340, 0xc022f344, 0xc022f430, 0xc022f434, 0xc0238060, 0xc0238064, 0xc0238080, 0xc0238084, 0xc0238098, 0xc0242010, 0xc0300000, 0xc0300100, 0xc0300104, 0xc0300108, 0xc0300204, 0xc0400004, 0xc0400008, 0xc0400018, 0xc040002c, 0xc0400080, 0xc0400084, 0xc040008c, 0xc04000b4, 0xc04000c0, 0xc04000c4, 0xc04000cc, 0xc0410000, 0xc0410008, 0xc0500080, 0xc0500088, 0xc0500090, 0xc0500094, 0xc05000a0, 0xc05000a8, 0xc05000b0, 0xc05000b4, 0xc05000c0, 0xc05000c4, 0xc05000c8, 0xc05000cc, 0xc05000d0, 0xc05000d4, 0xc05000d8, 0xc0520000, 0xc0520004, 0xc0520008, 0xc052000c, 0xc0520014, 0xc0520018, 0xc0720000, 0xc0720004, 0xc0720008, 0xc072000c, 0xc0720014, 0xc0720024, 0xc07200ec, 0xc07200f0, 0xc0720100, 0xc0720104, 0xc0720108, 0xc072010c, 0xc0720110, 0xc0720114, 0xc0720118, 0xc072011c, 0xc07201c8, 0xc0720200, 0xc0720204, 0xc0720208, 0xc072020c, 0xc0720210, 0xc0800008, 0xc0800014, 0xc0800018, 0xc0820000, 0xc0820304, 0xc0820308, 0xc082030c, 0xc0820310, 0xc0820318, 0xc0920000, 0xc0920004, 0xc0920008, 0xc092000c, 0xc0920010, 0xc0920100, 0xc0920118, 0xc092011c, 0xc0920120, 0xc0920124, 0xc0920204, 0xc0920208, 0xc092020c, 0xc0920210, 0xc0920220, 0xc0920224, 0xc0920238, 0xc0920320, 0xc0920344, 0xc0920348, 0xc0920354, 0xc0920358, 0xc0a00000, 0xc0a00008, 0xc0a0000c, 0xc0a00014, 0xc0a00018, 0xc0a0001c, 0xc0a00020, 0xc0a00024, 0xc0a00044, 0xc0a10008 };
int mem_spy_len = 0x10000/4;    // look at ### int32's; use only when mem_spy_fixed_addresses = 0
//~ int mem_spy_len = COUNT(mem_spy_addresses); // use this when mem_spy_fixed_addresses = 1

int mem_spy_count_lo = 5; // how many times is a value allowed to change
int mem_spy_count_hi = 50; // (limits)
int mem_spy_freq_lo = 0; 
int mem_spy_freq_hi = 0;  // or check frequecy between 2 limits (0 = disable)
int mem_spy_value_lo = 0;
int mem_spy_value_hi = 50;  // or look for a specific range of values (0 = disable)

#endif

static void
mem_spy_select( void * priv )
{
	mem_spy = !mem_spy;
}

void card_led_on() { cli_save(); *(uint8_t*)CARD_LED_ADDRESS = 0x46; sei_restore(); }
void card_led_off() { cli_save(); *(uint8_t*)CARD_LED_ADDRESS = 0x44; sei_restore(); }
void card_led_blink(int times, int delay_on, int delay_off)
{
	int i;
	for (i = 0; i < times; i++)
	{
		card_led_on();
		msleep(delay_on);
		card_led_off();
		msleep(delay_off);
	}
}

int config_ok = 0;

void
save_config( void * priv )
{
	config_save_file( CARD_DRIVE "magic.cfg" );
}
static void
delete_config( void * priv )
{
	FIO_RemoveFile( CARD_DRIVE "magic.cfg" );
	if (config_autosave) config_autosave_toggle(0);
}

static void
config_autosave_display(
	void *			priv,
	int			x,
	int			y,
	int			selected
)
{
	bmp_printf(
		selected ? MENU_FONT_SEL : MENU_FONT,
		x, y,
		"Config AutoSave: %s", 
		config_autosave ? "ON" : "OFF"
	);
}

#if CONFIG_DEBUGMSG

static int vmax(int* x, int n)
{
	int i; 
	int m = -100000;
	for (i = 0; i < n; i++)
		if (x[i] > m)
			m = x[i];
	return m;
}

static void dump_rom_task(void* priv)
{
	msleep(200);
	FILE * f = FIO_CreateFile(CARD_DRIVE "ROM0.BIN");
	if (f != (void*) -1)
	{
		bmp_printf(FONT_LARGE, 0, 60, "Writing ROM");
		FIO_WriteFile(f, (void*) 0xFF010000, 0x900000);
		FIO_CloseFile(f);
	}

	msleep(200);

	f = FIO_CreateFile(CARD_DRIVE "BOOT0.BIN");
	if (f != (void*) -1)
	{
		bmp_printf(FONT_LARGE, 0, 60, "Writing BOOT");
		FIO_WriteFile(f, (void*) 0xFFFF0000, 0x10000);
		FIO_CloseFile(f);
	}
	
	msleep(200);

	dump_big_seg(0, CARD_DRIVE "RAM0.BIN");
}

static void dump_rom(void* priv)
{
	gui_stop_menu();
	task_create("dump_task", 0x1e, 0, dump_rom_task, 0);
}
#endif

void beep()
{
	// just to make sure it's thread safe
	static struct semaphore * beep_sem = 0;
	if (beep_sem == 0) beep_sem = create_named_semaphore("beep_sem",1);
	
	take_semaphore(beep_sem, 0);
	call("StartPlayWaveData");
	msleep(100);
	call("StopPlayWaveData");
	give_semaphore(beep_sem);
}

void Beep()
{
	task_create("beep", 0x1c, 0, beep, 0);
}

// http://www.iro.umontreal.ca/~simardr/rng/lfsr113.c
unsigned int rand (void)
{
   static unsigned int z1 = 12345, z2 = 12345, z3 = 12345, z4 = 12345;
   unsigned int b;
   b  = ((z1 << 6) ^ z1) >> 13;
   z1 = ((z1 & 4294967294U) << 18) ^ b;
   b  = ((z2 << 2) ^ z2) >> 27; 
   z2 = ((z2 & 4294967288U) << 2) ^ b;
   b  = ((z3 << 13) ^ z3) >> 21;
   z3 = ((z3 & 4294967280U) << 7) ^ b;
   b  = ((z4 << 3) ^ z4) >> 12;
   z4 = ((z4 & 4294967168U) << 13) ^ b;
   return (z1 ^ z2 ^ z3 ^ z4);
}

void fake_buttons()
{
	msleep(2000);
	int i;
	int delay = 1000;
	for (i = 0; i < 10000; i++)
	{
		switch(rand() % 5) {
			case 0: 
				fake_simple_button(BGMT_PLAY); msleep(rand() % delay);
				break;
			case 1:
				fake_simple_button(BGMT_MENU); msleep(rand() % delay);
				break;
			case 2:
#ifndef CONFIG_50D
				fake_simple_button(BGMT_Q); msleep(rand() % delay);
#endif
				break;
			case 3:
				SW1(1,rand() % 2000);
				SW1(0,rand() % 2000);
				break;
			//~ case 4:
				//~ fake_simple_button(BGMT_LV); msleep(rand() % delay);
				//~ break;
		}
	}
}

void change_colors_like_crazy()
{
	msleep(2000);
	int i;
	int delay = 200;
	for (i = 0; i < 10000; i++)
	{
		bmp_off();
		msleep(rand() % delay);
		bmp_on();
		msleep(rand() % delay);
		redraw();
		PauseLiveView();
		msleep(rand() % delay);
		ResumeLiveView();
		msleep(rand() % delay);
		display_on();
		msleep(rand() % delay);
		display_off();
		msleep(rand() % delay);
		//~ cli_save();
		//~ if (tft_status == 1 && lv) ChangeColorPalette(rand() % 5);
		//~ sei_restore();
		//~ msleep(rand() % delay);
	}
}

/*void dlg_test_task()
{
	gui_stop_menu();
	msleep(1000);
	test_dialog_create();
}

void dlg_test(void* priv)
{
	task_create("dlg_test", 0x1c, 0, dlg_test_task, 0);
}*/

volatile int aff[26];

int hdmi_code_array[8];

PROP_HANDLER(PROP_HDMI_CHANGE_CODE)
{
	memcpy(hdmi_code_array, buf, 32);
	return prop_cleanup(token, property);
}

void ChangeHDMIOutputSizeToVGA()
{
	hdmi_code_array[0] = 2;
	prop_request_change(PROP_HDMI_CHANGE_CODE, hdmi_code_array, 32);
}

void ChangeHDMIOutputSizeToFULLHD()
{
	hdmi_code_array[0] = 5;
	prop_request_change(PROP_HDMI_CHANGE_CODE, hdmi_code_array, 32);
}


#ifdef CONFIG_600D
#define SENSOR_TIMING_TABLE MEM(0xCB20)
#endif
#ifdef CONFIG_60D
#define SENSOR_TIMING_TABLE MEM(0x2a668)
#endif

#ifdef SENSOR_TIMING_TABLE

/** 
 * FPS control
 * http://magiclantern.wikia.com/wiki/VideoTimer
 * 
 * Found by g3gg0
 **/

struct lv_path_struct
{
	int SM; // ?! 1 in video mode, 0 in zoom and photo mode
	int fps_sensor_mode; // 24p:4, 25p:3, 30p:2, 50p:1, 60p:0
	int S; // 1920:0, 720:1, vgacrop:4, zoom:8
	int R; // movie size: 1920:0, 720:1, 480:2
	int Z; // (1 / 5 / A) << 16
	int recording;
	int DZ; // bool?
};

extern struct lv_path_struct lv_path_struct;

#define TG_FREQ_PAL  50000000
#define TG_FREQ_NTSC 52747252

#define FPS_x1000_TO_TIMER_PAL(fps_x1000) (TG_FREQ_PAL/(fps_x1000))
#define FPS_x1000_TO_TIMER_NTSC(fps_x1000) (TG_FREQ_NTSC/(fps_x1000))
#define TIMER_TO_FPS_x1000_PAL(t) (TG_FREQ_PAL/(t))
#define TIMER_TO_FPS_x1000_NTSC(t) (TG_FREQ_NTSC/(t))

uint16_t * sensor_timing_table_original = 0;
uint16_t sensor_timing_table_patched[128];

int fps_override = 0;

int video_mode[5];
PROP_HANDLER(PROP_VIDEO_MODE)
{
	memcpy(video_mode, buf, 20);
	return prop_cleanup(token, property);
}

void fps_init()
{
	// make a copy of the original sensor timing table (so we can patch it)
	sensor_timing_table_original = (void*)SENSOR_TIMING_TABLE;
	memcpy(sensor_timing_table_patched, sensor_timing_table_original,  sizeof(sensor_timing_table_patched));
}

INIT_FUNC("fps", fps_init);

const int mode_offset_map[] = { 3, 6, 1, 5, 4, 0, 2 };

int fps_get_current_x1000()
{
	int mode = 
		video_mode_fps == 60 ? 0 : 
		video_mode_fps == 50 ? 1 : 
		video_mode_fps == 30 ? 2 : 
		video_mode_fps == 25 ? 3 : 
		video_mode_fps == 24 ? 4 : 0;
    int fps_timer = ((uint16_t*)SENSOR_TIMING_TABLE)[mode_offset_map[mode]];
    int ntsc = (mode % 2 == 0);
    int fps_x1000 = ntsc ? TIMER_TO_FPS_x1000_NTSC(fps_timer) : TIMER_TO_FPS_x1000_PAL(fps_timer);
    return fps_x1000;
}

static void
fps_print(
	void *			priv,
	int			x,
	int			y,
	int			selected
)
{
	int current_fps = fps_get_current_x1000();
	
	char msg[30];
	snprintf(msg, sizeof(msg), "%d (%d.%03d)", 
		(current_fps+500)/1000, 
		current_fps/1000, current_fps%1000
		);
	
	bmp_printf(
		selected ? MENU_FONT_SEL : MENU_FONT,
		x, y,
		"FPS override : %s",
		fps_override ? msg : "OFF"
	);
	menu_draw_icon(x, y, MNI_BOOL(SENSOR_TIMING_TABLE != sensor_timing_table_original), 0);
	//~ bmp_hexdump(FONT_SMALL, 0, 400, SENSOR_TIMING_TABLE, 32);
}

void fps_change_mode(int mode, int fps)
{
    /** 
     * 60fps = mode 0 - NTSC
     * 50fps = mode 1
     * 30fps = mode 2 - NTSC
     * 25fps = mode 3
     * 24fps = mode 4 - NTSC?
     **/


    // NTSC is 29.97, not 30
    int ntsc = (mode % 2 == 0);
    int fps_x1000 = ntsc ? (fps * 1000 * 1000 / 1001) : (fps * 1000);
    
    // for PAL, 12.5 fps and 6.25 fps may be better rounding choices
    if (fps_x1000 == 13000) fps_x1000 = 12500;
    if (fps_x1000 == 6000) fps_x1000 = 6250; 
    if (fps_x1000 == 3000) fps_x1000 = 3125; 

    // convert fps into timer ticks (for sensor drive speed)
    int fps_timer = ntsc ? FPS_x1000_TO_TIMER_NTSC(fps_x1000) : FPS_x1000_TO_TIMER_PAL(fps_x1000);
    int fps_timer_default = sensor_timing_table_original[mode_offset_map[mode]];

    // make sure we set a valid value
    int fps_timer_absolute_minimum = sensor_timing_table_original[21 + mode_offset_map[mode]];
    fps_timer = MAX(fps_timer_absolute_minimum * 120/100, fps_timer);
    
    // fps = 0 means "don't override, use default"
    sensor_timing_table_patched[mode_offset_map[mode]] = fps ? fps_timer : fps_timer_default;

    // use the patched sensor table
    SENSOR_TIMING_TABLE = sensor_timing_table_patched;
}

void fps_change_all_modes(int fps)
{
	if (!fps)
	{
		// use the original sensor table (firmware default)
		SENSOR_TIMING_TABLE = sensor_timing_table_original;
	}
	else
	{
		// patch all video modes
		for (int i = 0; i < 2; i++)
			fps_change_mode(i, fps);
		for (int i = 2; i < 5; i++)
			fps_change_mode(i, MIN(fps, 35));
	}

	if (!lv) return;

	// flip video mode back and forth to apply settings instantly
	int f0 = video_mode[2];
	video_mode[2] = 
		f0 == 24 ? 30 : 
		f0 == 25 ? 50 : 
		f0 == 30 ? 24 : 
		f0 == 50 ? 25 :
	  /*f0 == 60*/ 30;
	prop_request_change(PROP_VIDEO_MODE, video_mode, 20);
	video_mode[2] = f0;
	prop_request_change(PROP_VIDEO_MODE, video_mode, 20);
}

void reset_fps(void* priv, int delta)
{
	fps_override = 0;
	fps_change_all_modes(0);
	menu_show_only_selected();
}

void set_fps(void* priv, int delta)
{
	// first click won't change value
	int fps = (fps_get_current_x1000() + 500) / 1000; // rounded value
	if (fps_override) fps = COERCE(fps + delta, 5, 60);
	fps_override = 1;
	
	fps_change_all_modes(fps);
	menu_show_only_selected();
}

#endif

void run_test()
{
	msleep(2000);
	//~ lv_path_struct.Z = 0x50000;
	//~ beep();
	//~ int ans = FIO_RenameFile("B:/README", "B:/FOO.BAR");
	//~ NotifyBox(1000, "%x ", ans);
	//~ GUI_SetMovieSize_a(2);
	//~ struct gui_task * current = gui_task_list.current;
	//~ struct dialog * dialog = current->priv;
	//~ dialog->handler = spy_handler;
	//~ reloc_liveviewapp_install();
	//~ beep();

	//~ ReverseDraftVram();
	//~ NotifyBox(2000, "%x", MEM(MEM(0x267C)+4)+0x10);
	//~ MEM(0x288D8) = 1;
	//~ beep();
	//~ RedrawDisplay();
	//~ bmp_idle_copy(1);
	//~ prop_dump();
	//~ lens_take_picture(64, 0);
	//~ bulb_take_pic(250);
	//~ trans_test();
}

void xx_test(void* priv)
{
	//~ #ifdef CONFIG_550D
	gui_stop_menu();
	//~ SetGUIRequestMode(29); // Jackie Chan :)
	//~ #endif
	//~ *(uint8_t*)0x14c08 = 0x3;
	//~ gui_stop_menu();
	//~ set_display_gain(512);
	task_create("run_test", 0x1c, 0, run_test, 0); // don't delete this!
	//~ guiNotifyDialogRefresh();
}

static void stress_test_long(void* priv)
{
	gui_stop_menu();
	task_create("fake_buttons", 0x1c, 0, fake_buttons, 0);
	task_create("change_colors", 0x1c, 0, change_colors_like_crazy, 0);
}

static void stress_test_picture(int n, int delay)
{
	if (shutter_count > 50000) { beep(); return; }
	msleep(delay);
	for (int i = 0; i < n; i++)
	{
		NotifyBox(10000, "Picture taking: %d/%d", i+1, n);
		msleep(200);
		lens_take_picture(64, 0);
	}
	lens_wait_readytotakepic(64);
	msleep(delay);
}

static void stress_test_task(void* unused)
{
	NotifyBox(10000, "Stability Test..."); msleep(2000);

	extern struct semaphore * gui_sem;

	NotifyBox(1000, "ML menu scroll...");
	give_semaphore(gui_sem);
	msleep(1000);
	for (int i = 0; i <= 1000; i++)
	{
		static int dir = 0;
		switch(dir)
		{
			case 0: fake_simple_button(BGMT_WHEEL_LEFT); break;
			case 1: fake_simple_button(BGMT_WHEEL_RIGHT); break;
			case 2: fake_simple_button(BGMT_WHEEL_UP); break;
			case 3: fake_simple_button(BGMT_WHEEL_DOWN); break;
		}
		dir = mod(dir + rand()%3 - 1, 4);
		msleep(10);
	}
	give_semaphore(gui_sem);

	msleep(2000);
	if (!lv) force_liveview();
	for (int i = 0; i <= 100; i++)
	{
		NotifyBox(1000, "ML menu toggle: %d", i);
		give_semaphore(gui_sem);
		msleep(50);
	}

	msleep(2000);

	#ifndef CONFIG_50D
	ensure_movie_mode();
	msleep(1000);
	for (int i = 0; i <= 5; i++)
	{
		NotifyBox(1000, "Pics while recording: %d", i);
		movie_start();
		msleep(1000);
		lens_take_picture(64, 0);
		msleep(1000);
		lens_take_picture(64, 0);
		msleep(1000);
		lens_take_picture(64, 0);
		while (lens_info.job_state) msleep(100);
		while (!lv) msleep(100);
		msleep(1000);
		movie_end();
		msleep(2000);
	}
	#endif

	msleep(2000);
	beep();
	fake_simple_button(BGMT_PLAY); msleep(1000);
	for (int i = 0; i < 100; i++)
	{
		NotifyBox(1000, "PLAY: image compare: %d", i);
		playback_compare_images_task(1);
	}
	get_out_of_play_mode();
	msleep(2000);

	fake_simple_button(BGMT_PLAY); msleep(1000);
	for (int i = 0; i < 10; i++)
	{
		NotifyBox(1000, "PLAY: exposure fusion: %d", i);
		expfuse_preview_update_task(1);
	}
	get_out_of_play_mode();
	msleep(2000);

	fake_simple_button(BGMT_PLAY); msleep(1000);
	for (int i = 0; i < 100; i++)
	{
		NotifyBox(1000, "PLAY: 422 scrolling: %d", i);
		play_next_422_task(1);
	}
	get_out_of_play_mode();
	msleep(2000);

	fake_simple_button(BGMT_PLAY); msleep(1000);
	for (int i = 0; i < 50; i++)
	{
		NotifyBox(1000, "PLAY scrolling: %d", i);
		next_image_in_play_mode(1);
	}
	extern int timelapse_playback;
	timelapse_playback = 1;
	for (int i = 0; i < 50; i++)
	{
		NotifyBox(1000, "PLAY scrolling: %d", i+50);
		msleep(200);
	}
	timelapse_playback = 0;
	get_out_of_play_mode();

	for (int i = 0; i < 100; i++)
	{
		NotifyBox(1000, "Disabling Canon GUI (%d)...", i);
		canon_gui_disable();
		msleep(rand()%300);
		canon_gui_enable();
		msleep(rand()%300);
	}
	
	msleep(2000);

	NotifyBox(10000, "LCD backlight...");
	int old_backlight_level = backlight_level;
	for (int i = 0; i < 5; i++)
	{
		for (int k = 1; k <= 7; k++)
		{
			set_backlight_level(k);
			msleep(50);
		}
		for (int k = 7; k >= 1; k--)
		{
			set_backlight_level(k);
			msleep(50);
		}
	}
	set_backlight_level(old_backlight_level);

	if (!lv) force_liveview();
	for (int k = 0; k < 10; k++)
	{
		NotifyBox(1000, "LiveView / Playback (%d)...", k*10);
		fake_simple_button(BGMT_PLAY);
		msleep(rand() % 1000);
		SW1(1, rand()%100);
		SW1(0, rand()%100);
		msleep(rand() % 1000);
	}
	if (!lv) force_liveview();
	msleep(2000);
	lens_set_rawiso(0);
	for (int k = 0; k < 5; k++)
	{
		NotifyBox(1000, "LiveView gain test: %d", k*20);
		for (int i = 0; i <= 16; i++)
		{
			set_display_gain(1<<i);
			msleep(100);
		}
		for (int i = 16; i >= 0; i--)
		{
			set_display_gain(1<<i);
			msleep(100);
		}
	}
	set_display_gain(0);

	msleep(1000);
	
	for (int i = 0; i <= 10; i++)
	{
		NotifyBox(1000, "LED blinking: %d", i*10);
		card_led_blink(10, i*3, (10-i)*3);
	}
	
	msleep(2000);
	
	for (int i = 0; i <= 100; i++)
	{
		NotifyBox(1000, "Redraw test: %d", i);
		msleep(50);
		redraw();
		msleep(50);
	}

	msleep(2000);

	NotifyBox(10000, "Menu scrolling");
	open_canon_menu();
	for (int i = 0; i < 5000; i++)
		fake_simple_button(BGMT_WHEEL_LEFT);
	for (int i = 0; i < 5000; i++)
		fake_simple_button(BGMT_WHEEL_RIGHT);
	SW1(1,0);
	SW1(0,0);

	stress_test_picture(2, 2000); // make sure we have at least 2 pictures for scrolling :)

	msleep(2000);

	for (int i = 0; i <= 10; i++)
	{
		NotifyBox(1000, "Mode switching: %d", i*10);
		set_shooting_mode(SHOOTMODE_AUTO);	msleep(100);
		set_shooting_mode(SHOOTMODE_MOVIE);	msleep(2000);
		set_shooting_mode(SHOOTMODE_SPORTS);	msleep(100);
		set_shooting_mode(SHOOTMODE_NIGHT);	msleep(100);
		set_shooting_mode(SHOOTMODE_CA);	msleep(100);
		set_shooting_mode(SHOOTMODE_M);	msleep(100);
		ensure_bulb_mode(); msleep(100);
		set_shooting_mode(SHOOTMODE_TV);	msleep(100);
		set_shooting_mode(SHOOTMODE_AV);	msleep(100);
		set_shooting_mode(SHOOTMODE_P);	msleep(100);
	}
	
	stress_test_picture(2, 2000);

	if (!lv) force_liveview();
	NotifyBox(10000, "Focus tests...");
	msleep(2000);
	for (int i = 1; i <= 3; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			lens_focus( 1, i, 1, 0);
			lens_focus(-1, i, 1, 0);
		}
	}

	msleep(2000);

	NotifyBox(10000, "Expo tests...");
	
	if (!lv) force_liveview();
	msleep(1000);
	for (int i = KELVIN_MIN; i <= KELVIN_MAX; i += KELVIN_STEP)
	{
		NotifyBox(1000, "Kelvin: %d", i);
		lens_set_kelvin(i); msleep(200);
	}
	lens_set_kelvin(6500);

	stress_test_picture(2, 2000);

	set_shooting_mode(SHOOTMODE_M);

	if (!lv) force_liveview();
	msleep(1000);

	for (int i = 72; i <= 136; i++)
	{
		NotifyBox(1000, "ISO: raw %d  ", i);
		lens_set_rawiso(i); msleep(200);
	}
	lens_set_iso(88);

	stress_test_picture(2, 2000);

	msleep(5000);
	if (!lv) force_liveview();
	msleep(1000);

	for (int i = 0; i <= 100; i++)
	{
		NotifyBox(1000, "Pause LiveView: %d", i);
		PauseLiveView(); msleep(rand()%200);
		ResumeLiveView(); msleep(rand()%200);
	}

	stress_test_picture(2, 2000);

	msleep(2000);
	if (!lv) force_liveview();
	msleep(1000);

	for (int i = 0; i <= 100; i++)
	{
		NotifyBox(1000, "BMP overlay: %d", i);
		bmp_off(); msleep(rand()%200);
		bmp_on(); msleep(rand()%200);
	}

	stress_test_picture(2, 2000);

	msleep(2000);
	if (!lv) force_liveview();
	msleep(1000);

	for (int i = 0; i <= 100; i++)
	{
		NotifyBox(1000, "Display on/off: %d", i);
		display_off_force(); msleep(rand()%200);
		display_on_force(); msleep(rand()%200);
	}

	stress_test_picture(2, 2000);

	NotifyBox(10000, "LiveView switch...");
	set_shooting_mode(SHOOTMODE_M);
	for (int i = 0; i < 21; i++)
	{
		fake_simple_button(BGMT_LV); msleep(rand()%200);
	}

	stress_test_picture(2, 2000);

	set_shooting_mode(SHOOTMODE_BULB);
	
	msleep(1000);
	NotifyBox(10000, "Bulb picture taking");
	bulb_take_pic(2000);
	bulb_take_pic(100);
	bulb_take_pic(1500);
	bulb_take_pic(10);
	bulb_take_pic(1000);
	bulb_take_pic(1);

	NotifyBox(10000, "Movie recording");
	ensure_movie_mode();
	msleep(1000);
	for (int i = 0; i <= 5; i++)
	{
		NotifyBox(10000, "Movie recording: %d", i);
		movie_start();
		msleep(5000);
		movie_end();
		msleep(5000);
	}

	stress_test_picture(2, 2000);

	NotifyBox(2000, "Test complete."); msleep(2000);
	NotifyBox(2000, "Is the camera still working?"); msleep(2000);
	NotifyBox(10000, ":)");
	//~ NotifyBox(10000, "Burn-in test (will take hours!)");
	//~ set_shooting_mode(SHOOTMODE_M);
	//~ xx_test2(0);
}

void stress_test()
{
	gui_stop_menu();
	task_create("stress_test", 0x1c, 0, stress_test_task, 0);
}

void ui_lock(int x)
{
	int unlocked = UILOCK_NONE;
	prop_request_change(PROP_ICU_UILOCK, &unlocked, 4);
	msleep(50);
	prop_request_change(PROP_ICU_UILOCK, &x, 4);
	msleep(50);
}

void toggle_mirror_display()
{
	#if !defined(CONFIG_50D) && !defined(CONFIG_500D) && !defined(CONFIG_5D2)
	//~ zebra_pause();
	if (lv) msleep(200); // redrawing screen while zebra is active seems to cause trouble
	static int i = 0;
	if (i) MirrorDisplay();
	else NormalDisplay();
	i = !i;
	msleep(200);
	//~ zebra_resume();
	#endif
}

/*void fake_simple_button(int bgmt_code)
{
	struct event e = {
		.type = 0,
		.param = bgmt_code, 
		.obj = 0,
		.arg = 0,
	};
	GUI_CONTROL(&e);
}*/

#if CONFIG_DEBUGMSG

static int* dbg_memmirror = 0;
static int* dbg_memchanges = 0;

static int dbg_memspy_get_addr(int i)
{
	if (mem_spy_fixed_addresses)
		return mem_spy_addresses[i];
	else
		return mem_spy_start + i*4;
}

// for debugging purpises only
int _t = 0;
static int _get_timestamp(struct tm * t)
{
	return t->tm_sec + t->tm_min * 60 + t->tm_hour * 3600 + t->tm_mday * 3600 * 24;
}
static void _tic()
{
	struct tm now;
	LoadCalendarFromRTC(&now);
	_t = _get_timestamp(&now);
}
static int _toc()
{
	struct tm now;
	LoadCalendarFromRTC(&now);
	return _get_timestamp(&now) - _t;
}

static void dbg_memspy_init() // initial state of the analyzed memory
{
	bmp_printf(FONT_MED, 10,10, "memspy init @ %x ... (+%x) ... %x", mem_spy_start, mem_spy_len, mem_spy_start + mem_spy_len * 4);
	//~ msleep(2000);
	//mem_spy_len is number of int32's
	if (!dbg_memmirror) dbg_memmirror = AllocateMemory(mem_spy_len*4 + 100); // local copy of mem area analyzed
	if (!dbg_memmirror) return;
	if (!dbg_memchanges) dbg_memchanges = AllocateMemory(mem_spy_len*4 + 100); // local copy of mem area analyzed
	if (!dbg_memchanges) return;
	int i;
	//~ bmp_printf(FONT_MED, 10,10, "memspy alloc");
	int crc = 0;
	for (i = 0; i < mem_spy_len; i++)
	{
		uint32_t addr = dbg_memspy_get_addr(i);
		dbg_memmirror[i] = *(int*)(addr);
		dbg_memchanges[i] = 0;
		crc += dbg_memmirror[i];
		//~ bmp_printf(FONT_MED, 10,10, "memspy: %8x => %8x ", addr, dbg_memmirror[i]);
		//~ msleep(1000);
	}
	bmp_printf(FONT_MED, 10,10, "memspy OK: %x", crc);
	_tic();
}
static void dbg_memspy_update()
{
	static int init_done = 0;
	if (!init_done) dbg_memspy_init();
	init_done = 1;

	if (!dbg_memmirror) return;
	if (!dbg_memchanges) return;

	int elapsed_time = _toc();
	bmp_printf(FONT_MED, 50, 400, "%d ", elapsed_time);

	int i;
	int k=0;
	for (i = 0; i < mem_spy_len; i++)
	{
		uint32_t fnt = FONT_SMALL;
		uint32_t addr = dbg_memspy_get_addr(i);
		int oldval = dbg_memmirror[i];
		int newval = *(int*)(addr);
		if (oldval != newval)
		{
			//~ bmp_printf(FONT_MED, 10,460, "memspy: %8x: %8x => %8x", addr, oldval, newval);
			dbg_memmirror[i] = newval;
			if (dbg_memchanges[i] < 1000000) dbg_memchanges[i]++;
			fnt = FONT(FONT_SMALL, 5, COLOR_BG);
		}
		//~ else continue;

		if (mem_spy_bool && newval != 0 && newval != 1 && newval != -1) continue;

		if (mem_spy_value_lo && newval < mem_spy_value_lo) continue;
		if (mem_spy_value_hi && newval > mem_spy_value_hi) continue;
		
		if (mem_spy_count_lo && dbg_memchanges[i] < mem_spy_count_lo) continue;
		if (mem_spy_count_hi && dbg_memchanges[i] > mem_spy_count_hi) continue;
		
		int freq = dbg_memchanges[i] / elapsed_time;
		if (mem_spy_freq_lo && freq < mem_spy_freq_lo) continue;
		if (mem_spy_freq_hi && freq > mem_spy_freq_hi) continue;

		int x = 10 + 8 * 22 * (k % 4);
		int y = 10 + 12 * (k / 4);
		bmp_printf(fnt, x, y, "%8x:%2d:%8x", addr, dbg_memchanges[i], newval);
		k = (k + 1) % 120;
	}

	for (i = 0; i < 10; i++)
	{
		int x = 10 + 8 * 22 * (k % 4);
		int y = 10 + 12 * (k / 4);
		bmp_printf(FONT_SMALL, x, y, "                    ");
		k = (k + 1) % 120;
	}
}
#endif

static void display_shortcut_key_hints_lv()
{
	static int old_mode = 0;
	int mode = 0;
	if (!zebra_should_run()) return;
	if (is_movie_mode() && FLASH_BTN_MOVIE_MODE) mode = 1;
	else if (get_lcd_sensor_shortcuts() && !gui_menu_shown() && display_sensor && DISPLAY_SENSOR_POWERED) mode = 2;
	else if (is_follow_focus_active()==1 && !is_manual_focus() && (!display_sensor || !get_lcd_sensor_shortcuts())) mode = 3;
	if (mode == 0 && old_mode == 0) return;

	int mz = (mode == 2 && get_zoom_overlay_trigger_mode() <= 2 && lv_dispsize == 1);
	
	int x0 = os.x0 + os.x_ex/2;
	int y0 = os.y0 + os.y_ex/2;
	
	if (mode == 1)
	{
		bmp_printf(FONT_MED, x0 - 150 - font_med.width*2, y0 - font_med.height/2, "-ISO");
		bmp_printf(FONT_MED, x0 + 150 - font_med.width*2, y0 - font_med.height/2, "ISO+");
		bmp_printf(FONT_MED, x0 - font_med.width*2, y0 - 100 - font_med.height/2, "Kel+");
		bmp_printf(FONT_MED, x0 - font_med.width*2, y0 + 100 - font_med.height/2, "-Kel");
	}
	else if (mode == 2)
	{
		bmp_printf(FONT_MED, x0 - 150 - font_med.width*2, y0 - font_med.height/2, "-Vol");
		bmp_printf(FONT_MED, x0 + 150 - font_med.width*2, y0 - font_med.height/2, "Vol+");
		bmp_printf(FONT_MED, x0 - font_med.width*2, y0 - 100 - font_med.height/2, "LCD+");
		bmp_printf(FONT_MED, x0 - font_med.width*2, y0 + 100 - font_med.height/2, "-LCD");
	}
	else if (mode == 3)
	{
		//~ if (is_follow_focus_active() == 1)
		//~ {
			//~ int xf = is_follow_focus_active() == 1 ? x0 : 650;
			//~ int yf = is_follow_focus_active() == 1 ? y0 : 50;
			//~ int xs = is_follow_focus_active() == 1 ? 100 : 30;
			const int xf = x0;
			const int yf = y0;
			const int xs = 150;
			bmp_printf(FONT(FONT_MED, COLOR_WHITE, 0), xf - xs - font_med.width*2, yf - font_med.height/2, get_follow_focus_dir_h() > 0 ? " +FF" : " -FF");
			bmp_printf(FONT(FONT_MED, COLOR_WHITE, 0), xf + xs - font_med.width*2, yf - font_med.height/2, get_follow_focus_dir_h() > 0 ? "FF- " : "FF+ ");
			//~ if (is_follow_focus_active() == 1) // arrows
			//~ {
			bmp_printf(FONT(FONT_MED, COLOR_WHITE, 0), xf - font_med.width*2, yf - 100 - font_med.height/2, get_follow_focus_dir_v() > 0 ? "FF++" : "FF--");
			bmp_printf(FONT(FONT_MED, COLOR_WHITE, 0), xf - font_med.width*2, yf + 100 - font_med.height/2, get_follow_focus_dir_v() > 0 ? "FF--" : "FF++");
			//~ }
		//~ }
	}
	else
	{
		bmp_printf(FONT(FONT_MED, COLOR_WHITE, 0), x0 - 150 - font_med.width*2, y0 - font_med.height/2, "    ");
		bmp_printf(FONT(FONT_MED, COLOR_WHITE, 0), x0 + 150 - font_med.width*2, y0 - font_med.height/2, "    ");
		bmp_printf(FONT(FONT_MED, COLOR_WHITE, 0), x0 - font_med.width*2, y0 - 100 - font_med.height/2, "    ");
		bmp_printf(FONT(FONT_MED, COLOR_WHITE, 0), x0 - font_med.width*2, y0 + 100 - font_med.height/2, "    ");

		if (!should_draw_zoom_overlay())
			crop_set_dirty(20);

	}

	static int prev_mz = 0;
	if (mz) bmp_printf(FONT_MED, 360 + 100, 240 - 150, "Magic Zoom");
	else if (prev_mz) redraw();
	prev_mz = mz;

	old_mode = mode;
}

void display_clock()
{
	int bg = bmp_getpixel(15, 430);

	struct tm now;
	LoadCalendarFromRTC( &now );
	if (!lv)
	{
		uint32_t fnt = FONT(FONT_LARGE, COLOR_FG_NONLV, bg);
		bmp_printf(fnt, DISPLAY_CLOCK_POS_X, DISPLAY_CLOCK_POS_Y, "%02d:%02d", now.tm_hour, now.tm_min);
	}
}

PROP_INT(PROP_APERTURE, aper1);
PROP_INT(PROP_APERTURE2, aper2);
PROP_INT(PROP_APERTURE3, aper3);

struct rolling_pitching 
{
	uint8_t status;
	uint8_t cameraposture;
	uint8_t roll_sensor1;
	uint8_t roll_sensor2;
	uint8_t pitch_sensor1;
	uint8_t pitch_sensor2;
};
struct rolling_pitching level_data;

PROP_HANDLER(PROP_ROLLING_PITCHING_LEVEL)
{
	memcpy(&level_data, buf, 6);
	return prop_cleanup(token, property);
}

#if CONFIG_DEBUGMSG
static void dbg_draw_props(int changed);
static unsigned dbg_last_changed_propindex = 0;
#endif
int screenshot_sec = 0;

PROP_INT(PROP_ICU_UILOCK, uilock);

#ifdef CONFIG_60D
void
memfilt(void* m, void* M, int value)
{
	int k = 0;
	bmp_printf(FONT_SMALL, 0, 0, "%8x", value);
	for (void* i = m; i < M; i ++)
	{
		if ((*(uint8_t*)i) == value)
		{
			int x = 10 + 4 * 22 * (k % 8);
			int y = 10 + 12 * (k / 8);
			bmp_printf(FONT_SMALL, x, y, "%8x", i);
			k = (k + 1) % 240;
		}
	}
	int x = 10 + 4 * 22 * (k % 8);
	int y = 10 + 12 * (k / 8);
	bmp_printf(FONT_SMALL, x, y, "        ");
}

void draw_electronic_level(int angle, int prev_angle, int force_redraw)
{
	if (!force_redraw && angle == prev_angle) return;
	
	int x0 = os.x0 + os.x_ex/2;
	int y0 = os.y0 + os.y_ex/2;
	int r = 200;
	draw_angled_line(x0, y0, r, prev_angle, 0);
	draw_angled_line(x0+1, y0+1, r, prev_angle, 0);
	draw_angled_line(x0, y0, r, angle, angle % 900 ? COLOR_BLACK : COLOR_GREEN1);
	draw_angled_line(x0+1, y0+1, r, angle, angle % 900 ? COLOR_WHITE : COLOR_GREEN2);
}

void disable_electronic_level()
{
	if (level_data.status == 2)
	{
		GUI_SetRollingPitchingLevelStatus(1);
		msleep(100);
	}
}

void show_electronic_level()
{
	static int prev_angle10 = 0;
	int force_redraw = 0;
	if (level_data.status != 2)
	{
		GUI_SetRollingPitchingLevelStatus(0);
		msleep(100);
		force_redraw = 1;
	}
	
	static int k = 0;
	k++;
	if (k % 10 == 0) force_redraw = 1;
	
	int angle100 = level_data.roll_sensor1 * 256 + level_data.roll_sensor2;
	int angle10 = angle100/10;
	draw_electronic_level(angle10, prev_angle10, force_redraw);
	draw_electronic_level(angle10 + 1800, prev_angle10 + 1800, force_redraw);
	//~ draw_line(x0, y0, x0 + r * cos(angle), y0 + r * sin(angle), COLOR_BLUE);
	prev_angle10 = angle10;
	
	if (angle10 > 1800) angle10 -= 3600;
	bmp_printf(FONT_MED, 0, 35, "%s%3d", angle10 < 0 ? "-" : angle10 > 0 ? "+" : " ", ABS(angle10/10));
}

void roll_spy()
{
	show_electronic_level();
	NotifyBox(1000, "%x", level_data.roll_sensor1); msleep(1000);
	memfilt((void*)0xC0220000, (void*)0xC0230000, level_data.roll_sensor1);
	beep();
}
#endif

PROP_INT(0x8005002E, dzoom);
static void
debug_loop_task( void* unused ) // screenshot, draw_prop
{
	extern int ml_started;
	while (!ml_started) msleep(100);
	
	config_menu_init();
		
	/*dump_seg(&(font_large.bitmap), ('~' + (31 << 7)) * 4, CARD_DRIVE "large.fnt");
	dump_seg(&(font_med.bitmap), ('~' + (19 << 7)) * 4, CARD_DRIVE "medium.fnt");
	dump_seg(&(FONT_SMALL.bitmap), ('~' + (11 << 7)) * 4, CARD_DRIVE "small.fnt");*/
	
	int k;
	for (k = 0; ; k++)
	{
		msleep(10);
		
		//ui_lock(UILOCK_NONE); msleep(1000);		for debugging purposes (A1ex)
		
		//~ struct tm now;
		//~ LoadCalendarFromRTC(&now);
		//~ bmp_hexdump(FONT_SMALL, 0, 20, &mvr_config, 32*30);
		//~ bmp_hexdump(FONT_SMALL, 0, 200, &lv_path_struct, 32*5);
		
		//~ if (recording == 2)
			//~ void* x = get_lvae_info();
			//~ bmp_hexdump(FONT_SMALL, 0, 20, 0x529c, 32*20);
		//~ extern int disp_pressed;
		//~ DEBUG("MovRecState: %d", MOV_REC_CURRENT_STATE);
		
		//~ bmp_printf(FONT_LARGE, 50, 50, "%x ", lv_path_struct.Z);
		//~ maru(50, 50, liveview_display_idle() ? COLOR_RED : COLOR_GREEN1);
		//~ maru(100, 50, LV_BOTTOM_BAR_DISPLAYED ? COLOR_RED : COLOR_GREEN1);

		if (get_global_draw())
		{
			#if !defined(CONFIG_50D) && !defined(CONFIG_5D2)
			if (!lv && gui_state == GUISTATE_IDLE && !gui_menu_shown() && CURRENT_DIALOG_MAYBE == 0 && !ISO_ADJUSTMENT_ACTIVE && !EXT_MONITOR_CONNECTED) BMP_LOCK
			(
				display_clock();
				display_shooting_info();
				free_space_show_photomode();
			)
			#endif
		
			if (lv && !gui_menu_shown())
			{
				BMP_LOCK (
					display_shooting_info_lv();
					display_shortcut_key_hints_lv();
				)
				#if !defined(CONFIG_50D) && !defined(CONFIG_500D)
				static int ae_warned = 0;
				if (is_movie_mode() && !ae_mode_movie) 
				{
					if (!ae_warned && !gui_menu_shown())
					{
						bmp_printf(FONT(FONT_MED, COLOR_WHITE, 0), 50, 50, 
							"!!! Auto exposure !!!\n"
							"Set 'Movie Exposure -> Manual'");
						msleep(2000);
						redraw();
						ae_warned = 1;
					}
				}
				#endif

				/*static int rca_warned = 0;
				if (ext_monitor_rca) 
				{
					beep();
					if (!rca_warned && !gui_menu_shown())
					{
						msleep(2000);
						NotifyBox(2000, "SD monitors are NOT supported!"); msleep(2000);
						NotifyBox(2000, "RGB tools and magic zoom\nwill not work well.'"); msleep(2000);
						rca_warned = 1;
					}
				}*/
			}
		}
		
		if (screenshot_sec)
		{
			card_led_blink(1, 20, 1000-20-200);
			screenshot_sec--;
			if (!screenshot_sec)
				take_screenshot(0);
		}

		if (MENU_MODE) 
		{
			HijackFormatDialogBox_main();
		}
		//~ if (BTN_METERING_PRESSED_IN_LV)
		//~ {
			//~ while (BTN_METERING_PRESSED_IN_LV) msleep(100);
			//~ toggle_disp_mode();
		//~ }
		
		#if CONFIG_DEBUGMSG
		if (draw_prop)
		{
			dbg_draw_props(dbg_last_changed_propindex);
			continue;
		}
		else if (mem_spy)
		{
			dbg_memspy_update();
			continue;
		}
		#endif
		
		msleep(200);
	}
}

void screenshot_start(void* priv)
{
	screenshot_sec = 10;
}

void toggle_draw_event( void * priv );

static void
spy_print(
	void *			priv,
	int			x,
	int			y,
	int			selected
)
{
	bmp_printf(
		selected ? MENU_FONT_SEL : MENU_FONT,
		x, y,
		"Spy %s/%s/%s (s/p/q)",
		draw_prop ? "PROP" : "prop",
		get_draw_event() ? "EVT" : "evt", 
		mem_spy ? "MEM" : "mem"
	);
	menu_draw_icon(x, y, MNI_BOOL(draw_prop || get_draw_event() || mem_spy), 0);
}



PROP_INT(PROP_STROBO_REDEYE, red_eye);
void flashlight_frontled_task()
{
	msleep(100);
	display_off_force();
	int r = red_eye;
	int x = 1;
	int l = lv;
	int m = shooting_mode;
	int d = drive_mode;
	set_shooting_mode(SHOOTMODE_AUTO);
	msleep(100);
	if (lv) { fake_simple_button(BGMT_LV); while (lv) msleep(100); }
	msleep(100);
	prop_request_change(PROP_POPUP_BUILTIN_FLASH, &x, 4);
	assign_af_button_to_star_button();
	prop_request_change(PROP_STROBO_REDEYE, &x, 4);
	lens_set_drivemode(DRIVE_SINGLE);
	msleep(100);
	SW1(1,0);
	msleep(100);
	while (get_halfshutter_pressed()) { msleep(100); display_off_force(); }
	lens_set_drivemode(d);
	prop_request_change(PROP_STROBO_REDEYE, &r, 4);
	restore_af_button_assignment();
	set_shooting_mode(m);
	display_on();
	if (l) force_liveview();
}

void flashlight_lcd_task()
{
	msleep(500);
	while (get_halfshutter_pressed()) msleep(100);
	idle_globaldraw_dis();
	msleep(100);
	#if defined(CONFIG_600D) || defined(CONFIG_1100D)
	#define BGMT_DISP BGMT_INFO
	#endif
	if (tft_status) { fake_simple_button(BGMT_DISP); msleep(500); }

	canon_gui_disable_front_buffer();
	int b = backlight_level;
	set_backlight_level(7);
	
	while (!get_halfshutter_pressed() && tft_status == 0)
	{
		bmp_fill(COLOR_WHITE, 0, 0, 960, 540);
		msleep(50);
	}
	set_backlight_level(b);
	canon_gui_enable_front_buffer(1);
	idle_globaldraw_en();
}

static void flashlight_frontled(void* priv)
{
	gui_stop_menu();
	if (is_movie_mode()) task_create("flashlight_task", 0x1e, 0, flashlight_lcd_task, 0);
	else task_create("flashlight_task", 0x1e, 0, flashlight_frontled_task, 0);
}

static void flashlight_lcd(void* priv)
{
	gui_stop_menu();
	task_create("flashlight_task", 0x1e, 0, flashlight_lcd_task, 0);
}


static void meminfo_display(
	void *			priv,
	int			x,
	int			y,
	int			selected
)
{
	int a,b;
	GetMemoryInformation(&a,&b);
	bmp_printf(
		selected ? MENU_FONT_SEL : MENU_FONT,
		x, y,
		"Free Memory  : %dK/%dK",
		b/1024, a/1024
	);
	menu_draw_icon(x, y, MNI_BOOL(b > 1024*1024), 0);
}

static void shuttercount_display(
	void *			priv,
	int			x,
	int			y,
	int			selected
)
{
	bmp_printf(
		selected ? MENU_FONT_SEL : MENU_FONT,
		x, y,
		"Shutter Count:%3dK = %d+%d",
		(shutter_count_plus_lv_actuations + 500) / 1000,
		shutter_count, shutter_count_plus_lv_actuations - shutter_count
	);
	menu_draw_icon(x, y, shutter_count_plus_lv_actuations > 50000 ? MNI_WARNING : MNI_ON, 0);
}

static void efictemp_display(
	void *			priv,
	int			x,
	int			y,
	int			selected
)
{
	bmp_printf(
		selected ? MENU_FONT_SEL : MENU_FONT,
		x, y,
		"CMOS Temperat: %d",
		efic_temp
	);
	menu_draw_icon(x, y, MNI_ON, 0);
}

static void batt_display(
	void *			priv,
	int			x,
	int			y,
	int			selected
)
{
	int l = GetBatteryLevel();
	int r = GetBatteryTimeRemaining();
	int d = GetBatteryDrainRate();
	bmp_printf(
		selected ? MENU_FONT_SEL : MENU_FONT,
		x, y,
		"Battery level: %d%%, %dh%02dm, %d%%/h",
		l, 0, 
		r / 3600, (r % 3600) / 60,
		d, 0
	);
	menu_draw_icon(x, y, MNI_ON, 0);
}


#ifdef CONFIG_KILL_FLICKER
CONFIG_INT("kill.canon.gui", kill_canon_gui_mode, 1);

static void kill_canon_gui_print(
	void *			priv,
	int			x,
	int			y,
	int			selected
)
{
	bmp_printf(
		selected ? MENU_FONT_SEL : MENU_FONT,
		x, y,
		"Kill Canon GUI   : %s",
		kill_canon_gui_mode == 0 ? "OFF" :
		//~ kill_canon_gui_mode == 1 ? "BottomBar" :
		kill_canon_gui_mode == 1 ? "Idle/Menus" :
		kill_canon_gui_mode == 2 ? "Idle/Menus+Keys" :
		 "err"
	);
	menu_draw_icon(x, y, MNI_BOOL_GDR(kill_canon_gui_mode));
}
#endif


#if 1
CONFIG_INT("prop.i", prop_i, 0);
CONFIG_INT("prop.j", prop_j, 0);
CONFIG_INT("prop.k", prop_k, 0);

static void prop_display(
	void *			priv,
	int			x,
	int			y,
	int			selected
)
{
	unsigned prop = (prop_i << 24) | (prop_j << 16) | (prop_k);
	int* data = 0;
	int len = 0;
	int err = prop_get_value(prop, &data, &len);
	bmp_printf(
		FONT_MED,
		x, y,
		"PROP %8x: %d: %8x %8x %8x %8x\n"
		"'%s' ",
		prop,
		len,
		len > 0x00 ? data[0] : 0,
		len > 0x04 ? data[1] : 0,
		len > 0x08 ? data[2] : 0,
		len > 0x0c ? data[3] : 0,
		strlen(data) < 100 ? data : ""
	);
	menu_draw_icon(x, y, MNI_BOOL(!err), 0);
}

void prop_dump()
{
	FIO_RemoveFile(CARD_DRIVE "PROP.LOG");
	FILE* f = FIO_CreateFile(CARD_DRIVE "PROP.LOG");

	FIO_RemoveFile(CARD_DRIVE "PROP-STR.LOG");
	FILE* g = FIO_CreateFile(CARD_DRIVE "PROP-STR.LOG");
	
	unsigned i, j, k;
	unsigned actual_num_properties = 0;
	
	for( i=0 ; i<256 ; i++ )
	{
		if (i > 0x10 && i != 0x80) continue;
		for( j=0 ; j<=0xA ; j++ )
		{
			for( k=0 ; k<0x50 ; k++ )
			{
				unsigned prop = 0
					| (i << 24) 
					| (j << 16)
					| (k <<  0);
		
				bmp_printf(FONT_LARGE, 0, 0, "PROP %x...", prop);
				int* data = 0;
				int len = 0;
				int err = prop_get_value(prop, &data, &len);
				if (!err)
				{
					my_fprintf(f, "\nPROP %8x: %5d:", prop, len );
					my_fprintf(g, "\nPROP %8x: %5d:", prop, len );
					for (int i = 0; i < (MIN(len,40)+3)/4; i++)
					{
						my_fprintf(f, "%8x ", data[i]);
					}
					if (strlen(data) < 100) my_fprintf(g, "'%s'", data);
				}
			}
		}
	}
	FIO_CloseFile(f);
	FIO_CloseFile(g);
	beep();
	redraw();
}

static void prop_toggle_i(void* priv) {prop_i = prop_i < 5 ? prop_i + 1 : prop_i == 5 ? 0xE : prop_i == 0xE ? 0x80 : 0; }
static void prop_toggle_j(void* priv) {prop_j = mod(prop_j + 1, 0x10); }
static void prop_toggle_k(void* priv) {prop_k = mod(prop_k + 1, 0x51); }
#endif

void menu_kill_flicker()
{
	gui_stop_menu();
	canon_gui_disable_front_buffer();
}

static void CR2toAVI(void* priv)
{
	EyeFi_RenameCR2toAVI("B:/DCIM/100CANON");
}

struct menu_entry debug_menus[] = {
	#ifdef SENSOR_TIMING_TABLE
	{
		.name = "FPS override", 
		.priv = &fps_override,
		.select = set_fps,
		.select_auto = reset_fps,
		.display = fps_print,
		.help = "Makes French Fries with the camera sensor. Turn off sound!"
	},
	#endif
#if defined(CONFIG_60D) || defined(CONFIG_600D)
	{
		.priv		= "Rename CR2 to AVI",
		.select		= CR2toAVI,
		.display	= menu_print,
		.help = "Rename CR2 files to AVI (trick for EyeFi cards)."
	},
#endif
	{
		.priv		= "Flashlight [SET/Q]",
		.select		= flashlight_frontled,
		.select_auto = flashlight_lcd,
		.display	= menu_print,
		.help = "Turn on the front LED [SET] or make display bright [Q]."
	},
#if CONFIG_DEBUGMSG
	{
		.name = "Draw palette",
		.priv		= "Draw palette",
		.select		= (void(*)(void*))bmp_draw_palette,
		.display	= menu_print,
		.help = "Display a test pattern to see the color palette."
	},
	{
		.name = "Spy prop/evt/mem",
		.select		= draw_prop_select,
		.select_reverse = toggle_draw_event,
		.select_auto = mem_spy_select,
		.display	= spy_print,
		.help = "Spy properties / events / memory addresses which change."
	},
/*	{
		.priv		= "Dialog test",
		.select		= dlg_test,
		.display	= menu_print,
		.help = "Dialog templates (up/dn) and color palettes (left/right)"
	},*/
	{
		.name = "Debug logging",
		.priv = &dm_enable,
		.select = dm_toggle, 
		.select_auto		= (void*) dumpf,
		.display	= dm_display,
		.help = "While ON, debug messages are saved. [Q] => LOGnnn.LOG."
	},
	{
		.priv		= "Dump ROM and RAM",
		.select		= dump_rom,
		.display	= menu_print,
		.help = "0.BIN:0-0FFFFFFF, ROM0.BIN:FF010000, BOOT0.BIN:FFFF0000."
	},
#endif
	{
		.priv		= "Don't click me!",
		.select		= xx_test,
		.display	= menu_print,
		.help = "The camera may turn into a 1DX or it may explode."
	},
	{
		.name		= "Stability tests",
		.priv		= "Stability tests",
		.select		= stress_test,
		.display	= menu_print,
		.select_reverse = stress_test_long,
		.help = "SET: quick test; PLAY: burn-in test (around 2 hours)",
		.essential = FOR_MOVIE | FOR_PHOTO,
	},
/*	{
		.select = focus_test,
		.display = focus_print,
		.select_reverse = focus_en_bump,
		.select_auto = focus_mod_bump
	},
	{
		.priv = "CBR test", 
		.select = cbr_set,
		.display = menu_print,
	}*/

#if 0
	{
		.priv		= "Enable full HD",
		.select		= enable_full_hd,
		.display	= display_full_hd,
	},
	{
		.display	= mvr_time_const_display,
		.select		= mvr_time_const_select,
	},
#endif
/*	{
		.priv		= "Clear config",
		.select		= clear_config,
		.display	= menu_print,
	}, */
	#if !defined(CONFIG_50D) && !defined(CONFIG_500D) && !defined(CONFIG_5D2)
	{
		.name = "Free Memory",
		.display = meminfo_display,
		.help = "Memory information (from AllocateMemory)",
		.essential = 0,
	},
	#endif
	{
		.name = "Shutter Count",
		.display = shuttercount_display,
		.help = "Number of pics taken + number of LiveView actuations",
		.essential = FOR_MOVIE | FOR_PHOTO,
	},
	{
		.name = "EFIC temperature",
		.display = efictemp_display,
		.help = "EFIC temperature, in raw units (don't rely on it).",
		.essential = FOR_MOVIE | FOR_PHOTO,
	},
	#ifdef CONFIG_60D
	{
		.name = "Battery remaining",
		.display = batt_display,
		.help = "Battery remaining. Wait for 2%% discharge before reading.",
		.essential = FOR_MOVIE | FOR_PHOTO,
	},
	#endif
	#if CONFIG_DEBUGMSG
	{
		.name = "PROP display",
		.display = prop_display,
		.select = prop_toggle_k, 
		.select_reverse = prop_toggle_j,
		.select_auto = prop_toggle_i,
		.help = "Raw property display (read-only)",
	},
	#endif
};

static struct menu_entry cfg_menus[] = {
	{
		.name = "Config AutoSave",
		.priv = &config_autosave,
		.display	= config_autosave_display,
		.select		= config_autosave_toggle,
		.help = "If enabled, ML settings are saved automatically at shutdown."
	},
	{
		.name = "Save config now",
		.priv = "Save config now",
		.display	= menu_print,
		.select		= save_config,
		.help = "Save ML settings to MAGIC.CFG"
	},
	{
		.name = "Delete config file",
		.priv = "Delete config file",
		.display	= menu_print,
		.select		= delete_config,
		.help = "Use this to restore ML default settings. Restart needed."
	},
	
};




static void * debug_token;

static void
debug_token_handler(
	void *			token,
	void *			arg1,
	void *			arg2,
	void *			arg3
)
{
	debug_token = token;
	DebugMsg( DM_MAGIC, 3, "token %08x arg=%08x %08x %08x",
		(unsigned) token,
		(unsigned) arg1,
		(unsigned) arg2,
		(unsigned) arg3
	);
}

#if CONFIG_DEBUGMSG
//~ static int dbg_propn = 0;
#define MAXPROP 30
static unsigned dbg_props[MAXPROP] = {0};
static unsigned dbg_props_len[MAXPROP] = {0};
static unsigned dbg_props_a[MAXPROP] = {0};
static unsigned dbg_props_b[MAXPROP] = {0};
static unsigned dbg_props_c[MAXPROP] = {0};
static unsigned dbg_props_d[MAXPROP] = {0};
static unsigned dbg_props_e[MAXPROP] = {0};
static unsigned dbg_props_f[MAXPROP] = {0};
static void dbg_draw_props(int changed)
{
	dbg_last_changed_propindex = changed;
	int i; 
	for (i = 0; i < dbg_propn; i++)
	{
		unsigned x = 80;
		unsigned y = 15 + i * font_small.height;
		unsigned property = dbg_props[i];
		unsigned len = dbg_props_len[i];
		unsigned fnt = FONT_SMALL;
		if (i == changed) fnt = FONT(FONT_SMALL, 5, COLOR_BG);
		bmp_printf(fnt, x, y,
			"%08x %04x: %8lx %8lx %8lx %8lx %8lx %8lx",
			property,
			len,
			len > 0x00 ? dbg_props_a[i] : 0,
			len > 0x04 ? dbg_props_b[i] : 0,
			len > 0x08 ? dbg_props_c[i] : 0,
			len > 0x0c ? dbg_props_d[i] : 0,
			len > 0x10 ? dbg_props_e[i] : 0,
			len > 0x14 ? dbg_props_f[i] : 0
		);
	}
}


static void *
debug_property_handler(
	unsigned		property,
	void *			UNUSED( priv ),
	void *			buf,
	unsigned		len
)
{
	const uint32_t * const addr = buf;

	/*console_printf("Prop %08x: %2x: %08x %08x %08x %08x\n",
		property,
		len,
		len > 0x00 ? addr[0] : 0,
		len > 0x04 ? addr[1] : 0,
		len > 0x08 ? addr[2] : 0,
		len > 0x0c ? addr[3] : 0
	);*/
	
	if( !draw_prop )
		goto ack;
	
	// maybe the property is already in the array
	int i;
	for (i = 0; i < dbg_propn; i++)
	{
		if (dbg_props[i] == property)
		{
			dbg_props_len[i] = len;
			dbg_props_a[i] = addr[0];
			dbg_props_b[i] = addr[1];
			dbg_props_c[i] = addr[2];
			dbg_props_d[i] = addr[3];
			dbg_props_e[i] = addr[4];
			dbg_props_f[i] = addr[5];
			dbg_draw_props(i);
			goto ack; // return with cleanup
		}
	}
	// new property
	if (dbg_propn >= MAXPROP) dbg_propn = MAXPROP-1; // too much is bad :)
	dbg_props[dbg_propn] = property;
	dbg_props_len[dbg_propn] = len;
	dbg_props_a[dbg_propn] = addr[0];
	dbg_props_b[dbg_propn] = addr[1];
	dbg_props_c[dbg_propn] = addr[2];
	dbg_props_d[dbg_propn] = addr[3];
	dbg_props_e[dbg_propn] = addr[4];
	dbg_props_f[dbg_propn] = addr[5];
	dbg_propn++;
	dbg_draw_props(dbg_propn);

ack:
	return prop_cleanup( debug_token, property );
}

#endif

#ifndef CONFIG_500D
#define num_properties 8192
#else
#define num_properties 2048
#endif
unsigned* property_list = 0;


void
debug_init( void )
{
	draw_prop = 0;

#if CONFIG_DEBUGMSG
	if (!property_list) property_list = AllocateMemory(num_properties * sizeof(unsigned));
	if (!property_list) return;
	unsigned i, j, k;
	unsigned actual_num_properties = 0;
	
	unsigned is[] = {0x80, 0xe, 0x5, 0x4, 0x2, 0x1, 0x0};
	for( i=0 ; i<COUNT(is) ; i++ )
	{
		for( j=0 ; j<=0xA ; j++ )
		{
			for( k=0 ; k<0x50 ; k++ )
			{
				unsigned prop = 0
					| (is[i] << 24) 
					| (j << 16)
					| (k <<  0);

				property_list[ actual_num_properties++ ] = prop;

				if( actual_num_properties >= num_properties )
					goto thats_all;
			}
		}
	}

thats_all:

	prop_register_slave(
		property_list,
		actual_num_properties,
		debug_property_handler,
		(void*) 0xdeadbeef,
		debug_token_handler
	);
#endif

}

CONFIG_INT( "debug.timed-dump",		timed_dump, 0 );

//~ CONFIG_INT( "debug.dump_prop", dump_prop, 0 );
//~ CONFIG_INT( "debug.dumpaddr", dump_addr, 0 );
//~ CONFIG_INT( "debug.dumplen", dump_len, 0 );

/*
struct bmp_file_t * logo = (void*) -1;
void load_logo()
{
	if (logo == (void*) -1) 
		logo = bmp_load(CARD_DRIVE "logo.bmp",0);
}
void show_logo()
{
	load_logo();
	if ((int)logo > 0)
	{
		kill_flicker(); msleep(100);
		bmp_draw_scaled_ex(logo, 360 - logo->width/2, 240 - logo->height/2, logo->width, logo->height, 0, 0);
	}
}*/

void
debug_init_stuff( void )
{
	//~ set_pic_quality(PICQ_RAW);
	config_ok = 1;
	if (is_movie_mode()) restore_kelvin_wb();

	//~ dm_set_store_level( 255, 0);
	//~ dm_set_print_level( 255, 0);

	dm_update();
	
	/*
	DEBUG();
	dm_set_store_level( DM_DISP, 7 );
	dm_set_store_level( DM_LVFD, 7 );
	dm_set_store_level( DM_LVCFG, 7 );
	dm_set_store_level( DM_LVCDEV, 7 );
	dm_set_store_level( DM_LV, 7 );
	dm_set_store_level( DM_RSC, 7 );
	dm_set_store_level( DM_MAC, 7 );
	dm_set_store_level( DM_CRP, 7 );
	dm_set_store_level( DM_SETPROP, 7 );
	dm_set_store_level( DM_PRP, 7 );
	dm_set_store_level( DM_PROPAD, 7 );
	dm_set_store_level( DM_INTCOM, 7 );
	dm_set_store_level( DM_WINSYS, 7 );
	dm_set_store_level( DM_CTRLSRV, 7 );
	dm_set_store_level( DM_GUI, 7);
	dm_set_store_level( DM_GUI_M, 7);
	dm_set_store_level( DM_GUI_E, 7);
	dm_set_store_level( DM_BIND, 7);
	dm_set_store_level( DM_DISP, 7);
	DEBUG();*/

	//~ int i;
	//~ for (i = 0; i < 256; i++)
		//~ dm_set_store_level( i, 15);
	
	//msleep(1000);
	//bmp_draw_palette();
	//dispcheck();

}


//~ TASK_CREATE( "dump_task", dump_task, 0, 0x1e, 0x1000 );
TASK_CREATE( "debug_loop_task", debug_loop_task, 0, 0x1e, 0x1000 );

//~ CONFIG_INT( "debug.timed-start",	timed_start, 0 );
/*
static void
movie_start( void )
{
	int sec = timed_start;
	if( sec == 0 )
		return;

	const int x = 320;
	const int y = 150;

	while( --sec > 0 )
	{
		msleep( 1000 );
		bmp_printf(
			FONT(
				FONT_LARGE,
				sec > 4 ? COLOR_WHITE : COLOR_RED,
				0
			),
			x, y,
			"T-%d",
			sec
		);
	}

	bmp_printf( FONT(FONT_LARGE,COLOR_WHITE,0), x, y, "GO!" );

	call( "MovieStart" );

	msleep( 1000 );

	bmp_printf( FONT(FONT_LARGE,COLOR_WHITE,0), x, y, "   " );
}*/

//~ TASK_CREATE( "movie_start", movie_start, 0, 0x1f, 0x1000 );

void ml_shutdown()
{
	static int config_saved = 0;
	if (config_autosave && !config_saved)
	{
		config_saved = 1;
		save_config(0);
	}
	card_led_on();
	#if defined(CONFIG_50D) || defined(CONFIG_500D) || defined(CONFIG_5D2)
	call("EdLedOn");
	#endif
	msleep(50); 
}

PROP_HANDLER(PROP_TERMINATE_SHUT_REQ)
{
	//bmp_printf(FONT_MED, 0, 0, "SHUT REQ %d ", buf[0]);
	if (buf[0] == 0)  ml_shutdown();
	return prop_cleanup(token, property);
}

PROP_HANDLER(PROP_CARD_COVER)
{
	if (buf[0] == 1) ml_shutdown();
	return prop_cleanup(token, property);
}


/*
PROP_HANDLER(PROP_APERTURE)
{
	static int old = 0;
	
	if (old && lv)
	{
		if (display_sensor)
		{
			if (buf[0] != old)
			{
				int newiso = COERCE(lens_info.raw_iso + buf[0] - old, codes_iso[1], codes_iso[COUNT(codes_iso)-1]);
				lens_set_rawiso(newiso);
			}
		}
	}

	old = buf[0];

	return prop_cleanup(token, property);
}*/

/*
PROP_HANDLER(PROP_SHUTTER)
{
	if (lv && is_movie_mode())
	{
		static volatile int old = 0;
		
		if (old)
		{
			if (buf[0] != old)
			{
				//~ int newiso = COERCE(lens_info.raw_iso + buf[0] - old, codes_iso[1], codes_iso[COUNT(codes_iso)-1]);
				//~ lens_set_rawiso(newiso);
				buf[0] = old;
			}
		}
		old = buf[0];
	}
	return prop_cleanup(token, property);
}*/

#ifdef CONFIG_550D

int iso_intercept = 1;

void iso_adj(int prev_iso, int sign)
{
	if (sign)
	{
		lens_info.raw_iso = prev_iso;
		iso_intercept = 0;
		iso_toggle(sign);
		iso_intercept = 1;
	}
}

int iso_adj_flag = 0;
int iso_adj_old = 0;
int iso_adj_sign = 0;

void iso_adj_task(void* unused)
{
	while(1)
	{
		msleep(20);
		if (iso_adj_flag)
		{
			iso_adj_flag = 0;
			iso_adj(iso_adj_old, iso_adj_sign);
			lens_display_set_dirty();
		}
	}
}

TASK_CREATE("iso_adj_task", iso_adj_task, 0, 0x1a, 0);

PROP_HANDLER(PROP_ISO)
{
	static int prev_iso = 0;
	if (!prev_iso) prev_iso = lens_info.raw_iso;
	static int k = 0;
	if (iso_intercept && ISO_ADJUSTMENT_ACTIVE && lv && lv_disp_mode == 0 && is_movie_mode())
	{
		if ((prev_iso && buf[0] && prev_iso < buf[0]) || // 100 -> 200 => +
			(prev_iso >= 112 && buf[0] == 0)) // 3200+ -> auto => +
		{
			//~ bmp_printf(FONT_LARGE, 50, 50, "[%d] ISO+", k++);
			iso_adj_old = prev_iso;
			iso_adj_sign = 1;
			iso_adj_flag = 1;
		}
		else if ((prev_iso && buf[0] && prev_iso > buf[0]) || // 200 -> 100 => -
			(prev_iso <= 88 && buf[0] == 0)) // 400- -> auto => -
		{
			//~ bmp_printf(FONT_LARGE, 50, 50, "[%d] ISO-", k++);
			iso_adj_old = prev_iso;
			iso_adj_sign = -1;
			iso_adj_flag = 1;
		}
	}
	prev_iso = buf[0];
	return prop_cleanup(token, property);
}

#endif

int keep_ml_after_format = 1;

void HijackFormatDialogBox()
{
	if (MEM(DIALOG_MnCardFormatBegin) == 0) return;
	struct gui_task * current = gui_task_list.current;
	struct dialog * dialog = current->priv;
	if (dialog && MEM(dialog->type) != DLG_SIGNATURE) return;

#ifdef CONFIG_50D
#define FORMAT_BTN "[FUNC]"
#define STR_LOC 6
#else
 #ifdef CONFIG_500D
  #define FORMAT_BTN "[LV]"
  #define STR_LOC 12
 #else
  #ifdef CONFIG_5D2
   #define FORMAT_BTN "[PicSty]"
   #define STR_LOC 6
  #else
   #define FORMAT_BTN "[Q]"
   #define STR_LOC 11
  #endif
 #endif
#endif

	if (keep_ml_after_format)
		dialog_set_property_str(dialog, 4, "Format card, keep Magic Lantern " FORMAT_BTN);
	else
		dialog_set_property_str(dialog, 4, "Format card, remove Magic Lantern " FORMAT_BTN);
	dialog_redraw(dialog);
}

void HijackCurrentDialogBox(int string_id, char* msg)
{
	struct gui_task * current = gui_task_list.current;
	struct dialog * dialog = current->priv;
	if (dialog && MEM(dialog->type) != DLG_SIGNATURE) return;
	dialog_set_property_str(dialog, string_id, msg);
	dialog_redraw(dialog);
}

int handle_keep_ml_after_format_toggle()
{
	if (!MENU_MODE) return 1;
	if (MEM(DIALOG_MnCardFormatBegin) == 0) return 1;
	keep_ml_after_format = !keep_ml_after_format;
	fake_simple_button(MLEV_HIJACK_FORMAT_DIALOG_BOX);
	return 0;
}

void HijackDialogBox()
{
	struct gui_task * current = gui_task_list.current;
	struct dialog * dialog = current->priv;
	if (dialog && MEM(dialog->type) != DLG_SIGNATURE) return;
	int i;
	for (i = 0; i<255; i++) {
			char s[30];
			snprintf(s, sizeof(s), "%d", i);
			dialog_set_property_str(dialog, i, s);
	}
	dialog_redraw(dialog);
}

unsigned GetFileSize(char* filename)
{
	unsigned size;
	if( FIO_GetFileSize( filename, &size ) != 0 )
		return 0xFFFFFFFF;
	return size;
}


int ReadFileToBuffer(char* filename, void* buf, int maxsize)
{
	int size = GetFileSize(filename);
	if (!size) return 0;

	FILE* f = FIO_Open(filename, O_RDONLY | O_SYNC);
	if (f == INVALID_PTR) return 0;
	int r = FIO_ReadFile(f, UNCACHEABLE(buf), MIN(size, maxsize));
	FIO_CloseFile(f);
	return r;
}

struct tmp_file {
	char name[30];
	void* buf;
	int size;
	int sig;
};

struct tmp_file * tmp_files = 0;
void* tmp_buffers[5] = {(void*)YUV422_HD_BUFFER_1, (void*)YUV422_HD_BUFFER_2, (void*)YUV422_LV_BUFFER_1, (void*)YUV422_LV_BUFFER_2, (void*)YUV422_LV_BUFFER_3};
int tmp_file_index = 0;
int tmp_buffer_index = 0;
void* tmp_buffer_ptr = 0;
#define TMP_MAX_BUF_SIZE 4000000

void TmpMem_Init()
{
	tmp_file_index = 0;
	tmp_buffer_index = 0;
	tmp_buffer_ptr = tmp_buffers[0];
	if (!tmp_files) tmp_files = AllocateMemory(200 * sizeof(struct tmp_file));
}

void TmpMem_AddFile(char* filename)
{
	if (!tmp_files) return;

	int filesize = GetFileSize(filename);
	if (filesize == -1) return;
	if (filesize > TMP_MAX_BUF_SIZE) return;
	if (tmp_file_index >= 200) return;
	if (tmp_buffer_ptr + filesize > tmp_buffers[tmp_buffer_index] + TMP_MAX_BUF_SIZE) tmp_buffer_index++;
	if (tmp_buffer_index >= COUNT(tmp_buffers)) return;
	
	//~ NotifyBoxHide();
	//~ NotifyBox(300, "Reading %s...", filename);
	char msg[100];
	snprintf(msg, sizeof(msg), "Reading %s...", filename);
	HijackCurrentDialogBox(4, msg);
	ReadFileToBuffer(filename, tmp_buffer_ptr, filesize);
	my_memcpy(tmp_files[tmp_file_index].name, filename, 30);
	tmp_files[tmp_file_index].buf = tmp_buffer_ptr;
	tmp_files[tmp_file_index].size = filesize;
	tmp_files[tmp_file_index].sig = compute_signature(tmp_buffer_ptr, filesize/4);
	tmp_file_index++;
	tmp_buffer_ptr += (filesize + 10) & ~3;
}


void CopyMLDirectoryToRAM_BeforeFormat(char* dir, int cropmarks_flag)
{
	struct fio_file file;
	struct fio_dirent * dirent = FIO_FindFirstEx( dir, &file );
	if( IS_ERROR(dirent) )
		return;

	do {
		if (file.mode & 0x10) continue; // is a directory
		if (file.name[0] == '.' || file.name[0] == '_') continue;
		if (cropmarks_flag && !is_valid_cropmark_filename(file.name)) continue;

		char fn[30];
		snprintf(fn, sizeof(fn), "%s%s", dir, file.name);
		TmpMem_AddFile(fn);

	} while( FIO_FindNextEx( dirent, &file ) == 0);
	FIO_CleanupAfterFindNext_maybe(dirent);
}

void CopyMLFilesToRAM_BeforeFormat()
{
	TmpMem_Init();
	TmpMem_AddFile(CARD_DRIVE "AUTOEXEC.BIN");
	TmpMem_AddFile(CARD_DRIVE "FONTS.DAT");
	TmpMem_AddFile(CARD_DRIVE "RECTILIN.LUT");
	TmpMem_AddFile(CARD_DRIVE "MAGIC.CFG");
	CopyMLDirectoryToRAM_BeforeFormat(CARD_DRIVE "CROPMKS/", 1);
	CopyMLDirectoryToRAM_BeforeFormat(CARD_DRIVE "DOC/", 0);
	CopyMLDirectoryToRAM_BeforeFormat(CARD_DRIVE, 0);
}

void CopyMLFilesBack_AfterFormat()
{
	FIO_CreateDirectory(CARD_DRIVE "CROPMKS");
	FIO_CreateDirectory(CARD_DRIVE "DOC");
	int i;
	for (i = 0; i < tmp_file_index; i++)
	{
		//~ NotifyBoxHide();
		//~ NotifyBox(1000, "Restoring %s...   ", tmp_files[i].name);
		char msg[100];
		snprintf(msg, sizeof(msg), "Restoring %s...", tmp_files[i].name);
		HijackCurrentDialogBox(STR_LOC, msg);
		dump_seg(tmp_files[i].buf, tmp_files[i].size, tmp_files[i].name);
		int sig = compute_signature(tmp_files[i].buf, tmp_files[i].size/4); 
		if (sig != tmp_files[i].sig)
		{
			NotifyBox(2000, "Could not restore %s :(", tmp_files[i].name);
			FIO_RemoveFile(tmp_files[i].name);
			if (i > 1) return; // if it copies AUTOEXEC.BIN and fonts, ignore the error, it's safe to run
		}
	}
	
	HijackCurrentDialogBox(STR_LOC, "Writing bootflags...");
	bootflag_write_bootblock();

	HijackCurrentDialogBox(STR_LOC, "Magic Lantern restored :)");
	msleep(1000);
	HijackCurrentDialogBox(STR_LOC, "Format");
	//~ NotifyBox(2000, "Magic Lantern restored :)   ");
}

// check if autoexec.bin is present on the card
int check_autoexec()
{
	FILE * f = FIO_Open(CARD_DRIVE "AUTOEXEC.BIN", 0);
	if (f != (void*) -1)
	{
		FIO_CloseFile(f);
		return 1;
	}
	return 0;
}

void HijackFormatDialogBox_main()
{
	if (!MENU_MODE) return;
	if (MEM(DIALOG_MnCardFormatBegin) == 0) return;
	// at this point, Format dialog box is active

	// make sure we have something to restore :)
	if (!check_autoexec()) return;
	
	// before user attempts to do something, copy ML files to RAM
	ui_lock(UILOCK_EVERYTHING);
	CopyMLFilesToRAM_BeforeFormat();
	ui_lock(UILOCK_NONE);

	// all files copied, we can change the message in the format box and let the user know what's going on
	fake_simple_button(MLEV_HIJACK_FORMAT_DIALOG_BOX);
	
	// waiting to exit the format dialog somehow
	while (MEM(DIALOG_MnCardFormatBegin))
		msleep(200);
	
	// and maybe to finish formatting the card
	while (MEM(DIALOG_MnCardFormatExecute))
		msleep(50);

	// card was formatted (autoexec no longer there) => restore ML
	if (keep_ml_after_format && !check_autoexec())
	{
		ui_lock(UILOCK_EVERYTHING);
		CopyMLFilesBack_AfterFormat();
		ui_lock(UILOCK_NONE);
	}
}

void config_menu_init()
{
	extern struct menu_entry livev_cfg_menus[];
	menu_add( "Config", cfg_menus, COUNT(cfg_menus) );
	menu_add( "Config", livev_cfg_menus,  1);
	menu_add( "Debug", debug_menus, COUNT(debug_menus) );
}

void spy_event(struct event * event)
{
	if (get_draw_event())
	{
		static int kev = 0;
		kev++;
		bmp_printf(FONT_SMALL, 0, 400, "Ev%d[%d]: p=%8x *o=%8x/%8x/%8x a=%8x", 
			kev,
			event->type, 
			event->param, 
			event->obj ? ((int)event->obj & 0xf0000000 ? (int)event->obj : *(int*)(event->obj)) : 0,
			event->obj ? ((int)event->obj & 0xf0000000 ? (int)event->obj : *(int*)(event->obj + 4)) : 0,
			event->obj ? ((int)event->obj & 0xf0000000 ? (int)event->obj : *(int*)(event->obj + 8)) : 0,
			event->arg);
		console_printf("Ev%d[%d]: p=%8x *o=%8x/%8x/%8x a=%8x\n", 
			kev,
			event->type, 
			event->param, 
			event->obj ? ((int)event->obj & 0xf0000000 ? event->obj : *(uint32_t*)(event->obj)) : 0,
			event->obj ? ((int)event->obj & 0xf0000000 ? event->obj : *(uint32_t*)(event->obj + 4)) : 0,
			event->obj ? ((int)event->obj & 0xf0000000 ? event->obj : *(uint32_t*)(event->obj + 8)) : 0,
			event->arg);
	}
}

int flash_movie_pressed = 0;
int get_flash_movie_pressed() { return flash_movie_pressed; }

int halfshutter_pressed = 0;
#ifdef CONFIG_500D
bool get_halfshutter_pressed() { return HALFSHUTTER_PRESSED && !dofpreview; }
#else
bool get_halfshutter_pressed() { return halfshutter_pressed && !dofpreview; }
#endif

int zoom_in_pressed = 0;
int zoom_out_pressed = 0;
int set_pressed = 0;
int get_zoom_in_pressed() { return zoom_in_pressed; }
int get_zoom_out_pressed() { return zoom_out_pressed; }
int get_set_pressed() { return set_pressed; }

int handle_buttons_being_held(struct event * event)
{
	// keep track of buttons being pressed
	if (event->param == BGMT_PRESS_SET) set_pressed = 1;
	if (event->param == BGMT_UNPRESS_SET) set_pressed = 0;
	if (event->param == BGMT_PLAY) set_pressed = 0;
	if (event->param == BGMT_MENU) set_pressed = 0;
	if (event->param == BGMT_PRESS_HALFSHUTTER) halfshutter_pressed = 1;
	if (event->param == BGMT_UNPRESS_HALFSHUTTER) halfshutter_pressed = 0;
	if (event->param == BGMT_PRESS_ZOOMIN_MAYBE) {zoom_in_pressed = 1; zoom_out_pressed = 0; }
	if (event->param == BGMT_UNPRESS_ZOOMIN_MAYBE) {zoom_in_pressed = 0; zoom_out_pressed = 0; }
	if (event->param == BGMT_PRESS_ZOOMOUT_MAYBE) { zoom_out_pressed = 1; zoom_in_pressed = 0; }
	if (event->param == BGMT_UNPRESS_ZOOMOUT_MAYBE) { zoom_out_pressed = 0; zoom_in_pressed = 0; }

	if (BGMT_FLASH_MOVIE)
	{
		flash_movie_pressed = BGMT_PRESS_FLASH_MOVIE;
		return !flash_movie_pressed;
	}
	if (recording && MVR_FRAME_NUMBER < 50) flash_movie_pressed = 0; // workaround for issue 688
	return 1;
}

int handle_flash_button_shortcuts(struct event * event)
{
	if (is_movie_mode() && gui_state == GUISTATE_IDLE && FLASH_BTN_MOVIE_MODE)
	{
		if (event->param == BGMT_PRESS_UP)
		{
			kelvin_toggle(0, 1);
			return 0;
		}
		if (event->param == BGMT_PRESS_DOWN)
		{
			kelvin_toggle(0, -1);
			return 0;
		}
		if (event->param == BGMT_PRESS_LEFT)
		{
			iso_toggle(0, -1);
			return 0;
		}
		if (event->param == BGMT_PRESS_RIGHT)
		{
			iso_toggle(0, 1);
			return 0;
		}
	}
	return 1;
}

int handle_lcd_sensor_shortcuts(struct event * event)
{
	if (get_lcd_sensor_shortcuts() && !gui_menu_shown() && display_sensor && DISPLAY_SENSOR_POWERED) // button presses while display sensor is covered
	{ // those are shortcut keys
		if (!gui_menu_shown())
		{
			if (event->param == BGMT_PRESS_UP)
			{
				adjust_backlight_level(1);
				return 0;
			}
			else if (event->param == BGMT_PRESS_DOWN)
			{
				adjust_backlight_level(-1);
				return 0;
			}
		}
		if (lv)
		{
			if (event->param == BGMT_PRESS_LEFT)
			{
				volume_down();
				return 0;
			}
			else if (event->param == BGMT_PRESS_RIGHT)
			{
				volume_up();
				return 0;
			}
		}
	}
	return 1;
}

void fake_simple_button(int bgmt_code)
{
	GUI_Control(bgmt_code, 0, FAKE_BTN, 0);
}

int _display_is_off = 0;

// those functions seem not to be thread safe
// calling them from gui_main_task seems to sync them with other Canon calls properly
int handle_tricky_canon_calls(struct event * event)
{
	switch (event->param)
	{
		case MLEV_HIJACK_FORMAT_DIALOG_BOX:
			HijackFormatDialogBox();
			break;
		case MLEV_TURN_ON_DISPLAY:
			if (_display_is_off && is_safe_to_mess_with_the_display(0)) call("TurnOnDisplay");
			_display_is_off = 0;
			break;
		case MLEV_TURN_OFF_DISPLAY:
			if (!_display_is_off && is_safe_to_mess_with_the_display(0)) call("TurnOffDisplay");
			_display_is_off = 1;
			break;
		case MLEV_ChangeHDMIOutputSizeToVGA:
			ChangeHDMIOutputSizeToVGA();
			break;
		case MLEV_LCD_SENSOR_START:
			#if defined(CONFIG_550D) || defined(CONFIG_500D)
			DispSensorStart();
			#endif
			break;
		case MLEV_REDRAW:
			redraw_do();
			break;
		case MLEV_KILL_FLICKER:
			canon_gui_disable_gmt();
			break;
		case MLEV_STOP_KILLING_FLICKER:
			canon_gui_enable_gmt();
			break;
		case MLEV_HIDE_CANON_BOTTOM_BAR:
			#ifdef CONFIG_500D
			if (lv && LV_BOTTOM_BAR_DISPLAYED)
			{
				prop_change__DispType(3);
				SetOutputTypeByPressInfoToStorage(GetDisplayType(), 0);
				set_lv_stuff_to_win_system__maybe(2, 2);
			}
			#else
				#if !defined(CONFIG_50D) && !defined(CONFIG_5D2)
				if (lv && LV_BOTTOM_BAR_DISPLAYED)
				{
					HideBottomInfoDisp_maybe();
				}
				#endif
			break;
			#endif
		case MLEV_BV_ENABLE:
			bv_enable_do();
			break;
		case MLEV_BV_DISABLE:
			bv_disable_do();
			break;
		case MLEV_BV_AUTO_UPDATE:
			bv_auto_update_do();
			break;
	}
	if (event->param < 0) return 0;
	return 1;
}

void display_on()
{
	fake_simple_button(MLEV_TURN_ON_DISPLAY);
}
void display_on_force()
{
	_display_is_off = 1;
	display_on();
}
void display_off()
{
	fake_simple_button(MLEV_TURN_OFF_DISPLAY);
}
void display_off_force()
{
	_display_is_off = 0;
	display_off();
}
int display_is_on() { return !_display_is_off; }


#if defined(CONFIG_60D) || defined(CONFIG_600D)

void EyeFi_RenameCR2toAVI(char* dir)
{
	struct fio_file file;
	struct fio_dirent * dirent = FIO_FindFirstEx( dir, &file );
	if( IS_ERROR(dirent) )
		return;

	do {
		if (file.mode & 0x10) continue; // is a directory
		if (file.name[0] == '.') continue;
		if (!streq(file.name + 8, ".CR2")) continue;

		static char oldname[50];
		static char newname[50];
		snprintf(oldname, sizeof(oldname), "%s/%s", dir, file.name);
		strcpy(newname, oldname);
		newname[strlen(newname) - 4] = 0;
		STR_APPEND(newname, ".AVI");
		bmp_printf(FONT_LARGE, 0, 0, "%s...", newname);
		FIO_RenameFile(oldname, newname);

	} while( FIO_FindNextEx( dirent, &file ) == 0);
	FIO_CleanupAfterFindNext_maybe(dirent);
	beep();
	redraw();
}

#endif
