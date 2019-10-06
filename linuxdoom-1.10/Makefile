################################################################
#
# $Id:$
#
# $Log:$
#
CC=  clang

CPPFLAGS=-nostdlibinc -I$(GATEDIR)/include -Ilibc -DNORMALUNIX -DLINUX
CFLAGS=--target=wasm32-unknown-unknown -fomit-frame-pointer -fno-builtin-malloc -fno-builtin-realloc -O2 -Wall -Wno-unused-variable
LDFLAGS=-nostdlib -Wl,--entry=D_DoomMain -Wl,--allow-undefined-file=gate_abi.txt
LIBS=

# subdirectory for objects
O=linux

# not too sophisticated dependency
OBJS=				\
		$(O)/libc.o \
		$(O)/libc/printf/printf.o \
		$(O)/doomdef.o		\
		$(O)/doomstat.o		\
		$(O)/dstrings.o		\
		$(O)/i_system.o		\
		$(O)/gate_sound.o \
		$(O)/gate_video.o \
		$(O)/gate_net.o \
		$(O)/gate_support.o \
		$(O)/tables.o			\
		$(O)/f_finale.o		\
		$(O)/f_wipe.o 		\
		$(O)/d_main.o			\
		$(O)/d_net.o			\
		$(O)/d_items.o		\
		$(O)/g_game.o			\
		$(O)/m_menu.o			\
		$(O)/m_misc.o			\
		$(O)/m_argv.o  		\
		$(O)/m_bbox.o			\
		$(O)/m_fixed.o		\
		$(O)/m_swap.o			\
		$(O)/m_cheat.o		\
		$(O)/m_random.o		\
		$(O)/am_map.o			\
		$(O)/p_ceilng.o		\
		$(O)/p_doors.o		\
		$(O)/p_enemy.o		\
		$(O)/p_floor.o		\
		$(O)/p_inter.o		\
		$(O)/p_lights.o		\
		$(O)/p_map.o			\
		$(O)/p_maputl.o		\
		$(O)/p_plats.o		\
		$(O)/p_pspr.o			\
		$(O)/p_setup.o		\
		$(O)/p_sight.o		\
		$(O)/p_spec.o			\
		$(O)/p_switch.o		\
		$(O)/p_mobj.o			\
		$(O)/p_telept.o		\
		$(O)/p_tick.o			\
		$(O)/p_saveg.o		\
		$(O)/p_user.o			\
		$(O)/r_bsp.o			\
		$(O)/r_data.o			\
		$(O)/r_draw.o			\
		$(O)/r_main.o			\
		$(O)/r_plane.o		\
		$(O)/r_segs.o			\
		$(O)/r_sky.o			\
		$(O)/r_things.o		\
		$(O)/w_wad.o			\
		$(O)/wi_stuff.o		\
		$(O)/v_video.o		\
		$(O)/st_lib.o			\
		$(O)/st_stuff.o		\
		$(O)/hu_stuff.o		\
		$(O)/hu_lib.o			\
		$(O)/s_sound.o		\
		$(O)/z_zone.o			\
		$(O)/info.o				\
		$(O)/sounds.o

all:	 $(O)/doom.wasm

clean:
	rm -f *.o *~ *.flc
	rm -rf linux/*

$(O)/doom.wasm:	$(OBJS) $(O)/i_main.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(O)/i_main.o \
	-o $(O)/doom0.wasm $(LIBS)
	wasm-start -s D_DoomMain -x -o $(O)/doom.wasm $(O)/doom0.wasm

$(O)/%.o:	%.c $(wildcard *.h libc/*.h) Makefile
	@ mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

#############################################################
#
#############################################################
