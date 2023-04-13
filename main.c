#include <gb/gb.h>
#include <stdio.h>
#include <rand.h>
#include <gbdk/platform.h>
#include <gbdk/incbin.h>

int game_started = 0;
void reset_game();

//PLAYER SPRITES
int spr_reset = 0;

#include "sprites/UP.c"
#include "sprites/DOWN.c"
#include "sprites/RIGHT.c"
#include "sprites/LEFT.c"
#include "sprites/IDLE.c"

void draw_player(int status) {
    switch(status) {
        case 0:
            set_sprite_data(18,12,IDLE_tiles);
        break;
        case 1:
            set_sprite_data(18,12,LEFT_tiles);
        break;
        case 2:
            set_sprite_data(18,12,DOWN_tiles);
        break;
        case 3:
            set_sprite_data(18,12,UP_tiles);
        break;
        case 4:
            set_sprite_data(18,12,RIGHT_tiles);
        break;
    }
}

//BACKGROUND
INCBIN(bg_tiles, "background/BG_tiles.bin")
INCBIN_EXTERN(bg_tiles)
INCBIN(bg_map, "background/BG_map.bin")
INCBIN_EXTERN(bg_map)

INCBIN(bgs_tiles, "background/LOGO_tiles.bin")
INCBIN_EXTERN(bgs_tiles)
INCBIN(bgs_map, "background/LOGO_map.bin")
INCBIN_EXTERN(bgs_map)

//MUSIC
#include "gbt_player.h"
extern const unsigned char * song_Data[];

//NOTES
int reset_timer = 0;
#define NOTE_SPEED 2
int track1[] = {24, 84, 132, 276, 336, 408, 468, 516, 660, 720, 768, 864, 1056, 1188, 1248, 1440};
int track2[] = {36, 60, 228, 324, 420, 444, 612, 708, 840, 960, 996, 1020, 1224, 1344, 1380, 1404};
int track3[] = {48, 108, 180, 240, 300, 432, 492, 564, 624, 684, 744, 852, 1032, 1236, 1416};
int track4[] = {72, 168, 252, 312, 360, 456, 552, 636, 696, 804, 900, 936, 1152, 1284, 1320};
int sizes[] = {16,16,15,15};
int *tracks[4] = { track1, track2, track3, track4 };
int step = -180;
int track_steps[] = {0,0,0,0};

//ARROWS
#include "sprites/arrows.c"
#define MAX_ARROWS 6
typedef struct { int x,y; UINT8 active; UINT8 sprite_index; UINT8 type; } Arrow;
Arrow arrows[MAX_ARROWS];
Arrow* get_arrow() { for (UINT8 i = 0; i < MAX_ARROWS; i++) { if (!arrows[i].active) { arrows[i].active = 1; return &arrows[i]; } } return NULL; }
void set_arrow_type(Arrow* _a, UINT8 type) { _a->type = type; set_sprite_tile(_a->sprite_index*4,type*4); set_sprite_tile(_a->sprite_index*4+1,type*4+1); set_sprite_tile(_a->sprite_index*4+2,type*4+2); set_sprite_tile(_a->sprite_index*4+3,type*4+3); }
void update_arrow(Arrow* _a) { move_sprite(_a->sprite_index*4,_a->x,_a->y); move_sprite(_a->sprite_index*4+1,_a->x+8,_a->y); move_sprite(_a->sprite_index*4+2,_a->x,_a->y+8); move_sprite(_a->sprite_index*4+3,_a->x+8,_a->y+8); }

//SETUP
void setup() {
    draw_player(0);
    set_sprite_data(0,17,ArrowTile);

    set_sprite_tile(24,16);
    set_sprite_tile(25,16);
    set_sprite_tile(26,16);
    set_sprite_tile(27,16);
    move_sprite(24,24,144);
    move_sprite(25,44,144);
    move_sprite(26,64,144);
    move_sprite(27,84,144);

    for (UINT8 i = 0; i < MAX_ARROWS; i++) {
        arrows[i].x = -16;
        arrows[i].y = 0;
        arrows[i].active = 0;
        arrows[i].sprite_index = i;
        set_sprite_tile(i*4,0);
        set_sprite_tile(i*4+1,0);
        set_sprite_tile(i*4+2,0);
        set_sprite_tile(i*4+3,0);
    }

    set_bkg_data(0u,INCBIN_SIZE(bg_tiles)/16,bg_tiles);
    set_bkg_tiles(0u,0u,20u,18u,bg_map);
    SHOW_BKG;
    SHOW_SPRITES;

    for (int i = 0; i < 4; i++) {
        for(int x = 0; x < 3; x++) {
            set_sprite_tile(28+i*3+x,18+i*3+x);
            move_sprite(28+i*3+x,x*8+120,i*8+110);
        }
    }

}

UINT8 inputs[] = {J_LEFT,J_DOWN,J_UP,J_RIGHT};

//LOOP
void loop() {

    step ++;

    if (track_steps[0] >= 16) {
        reset_timer++;
        if (reset_timer >= 300) {
            game_started = 0;
            reset_game();
        }
    }

    if (step == 0) {
        disable_interrupts();
        gbt_play(song_Data, 2, 1);
        gbt_loop(0);
        set_interrupts(VBL_IFLAG);
        enable_interrupts();
    }

    if (spr_reset != 0) {
        spr_reset--;
        if (spr_reset == 0) {
            draw_player(0);
        }
    }

    for (int i = 0; i < 4; i++) {
        int *track = tracks[i];
        if (track_steps[i] > sizes[i]) continue;
        if (step > (track[track_steps[i]] - (142/NOTE_SPEED))) {
            track_steps[i]++;
            Arrow* a = get_arrow();
            if (a != NULL) {
                a->x = 20 + 20*i;
                a->y = 0;
                set_arrow_type(a,i);
            }

        }
    }

    for (UINT8 i = 0; i < MAX_ARROWS; i++) {
        if (arrows[i].active == 0) continue;
        arrows[i].y += NOTE_SPEED;
        if (arrows[i].y < 150 && arrows[i].y > 134) {

            if (joypad()&inputs[arrows[i].type]) {
                arrows[i].active = 0;
                arrows[i].x = -16;
                arrows[i].y = 0;
                spr_reset = 20;
                draw_player(arrows[i].type+1);
            }
        } else if (arrows[i].y >= 160) {
            arrows[i].active = 0;
            arrows[i].x = -16;
            arrows[i].y = 0;
        }
        update_arrow(&arrows[i]);
        //if (joypad() & J_DOWN) delay(1000);
    }
}

//INIT
void reset_game() {
    reset_timer = 0;
    set_bkg_data(0u,INCBIN_SIZE(bgs_tiles)/16,bgs_tiles);
    set_bkg_tiles(0u,0u,20u,18u,bgs_map);
    SHOW_BKG;

    spr_reset = 0;
    step = -180;
    
    for(int i = 0; i < 4; i++) {
        track_steps[i] = 0;
    }

    for (int i = 0; i < 40; i++) {
        move_sprite(i,0,0);
    }
    HIDE_SPRITES;

}

void main()
{
    set_bkg_data(0u,INCBIN_SIZE(bgs_tiles)/16,bgs_tiles);
    set_bkg_tiles(0u,0u,20u,18u,bgs_map);
    SHOW_BKG;
    while (1)
    {
        wait_vbl_done();

        if (game_started == 1) {
            loop();
        } else if (joypad() & J_START){
            setup(); loop(); game_started = 1;
        }

        gbt_update(); // This will change to ROM bank 1.
    }
}
