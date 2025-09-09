// notcurses_tabs.c
// Simple multi-tab stats UI with mouse tabs + auto-resizing columns.
//
// Keys: q or ESC to quit. Click a tab to switch.
// Requires: libnotcurses-dev

#define _XOPEN_SOURCE 700

#define v1
#define v2 // not usefull
#define v3 // not usefull
	#define v4 // good tab header pair row
#define v5 // good header not usefull
#define v6 // not usefull
#define v7 // not usefull
	//#define v8 // good tab style but bad color . pair row color good

#ifndef v8

// notcurses_nasa_panel.c
#include <notcurses/notcurses.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>

#define TAB_H 3
#define CMD_H 3
#define MAX_COLS 8
#define MAX_ROWS 32

typedef struct
{
	const char * name; int minw;
} Column;
typedef struct
{
	const char * tabname;
	int ncols;
	Column cols[ MAX_COLS ];
	int nrows;
	const char * subhdrs[ MAX_COLS ];
	const char * cells[ MAX_ROWS ][ MAX_COLS ];
} Table;

// Demo tables
static Table TAB_CPU = {
  .tabname = "CPU", .ncols = 4,
  .cols = { {"Core",4}, {"Usage%",7}, {"Freq",6}, {"Gov",3} },
  .subhdrs = {"ID","Load","MHz","Policy"},
  .nrows = 4,
  .cells = { {"0","12.3","3600","schedutil"}, {"1","5.1","3580","ondemand"}, {"2","22.9","4025","performance"}, {"avg","13.4","3735","mixed"} }
};

static Table TAB_MEM = {
  .tabname = "Memory", .ncols = 3,
  .cols = { {"Type",4}, {"Used",5}, {"Total",5} },
  .subhdrs = {"Kind","Now","Capacity"},
  .nrows = 3,
  .cells = { {"RAM","3.2G","7.7G"}, {"Swap","0.0G","2.0G"}, {"Cache","1.1G","—"} }
};

static Table * ALLTABS[] = { &TAB_CPU, &TAB_MEM };
static const int NTABS = sizeof( ALLTABS ) / sizeof( ALLTABS[ 0 ] );

// Helpers
static int slen( const char * s )
{
	return s ? ( int )strlen( s ) : 0;
}
static void compute_widths( const Table * T , int availw , int * out )
{
	int desired[ MAX_COLS ] = { 0 }; int i , r;
	for ( i = 0; i < T->ncols; i++ )
	{
		int w = slen( T->cols[ i ].name ); if ( w < T->cols[ i ].minw ) w = T->cols[ i ].minw;
		int sw = slen( T->subhdrs[ i ] ); if ( sw > w ) w = sw;
		for ( r = 0; r < T->nrows; r++ )
		{
			int cw = slen( T->cells[ r ][ i ] ); if ( cw > w ) w = cw;
		}
		desired[ i ] = w;
	}
	int need = 0; for ( i = 0; i < T->ncols; i++ ) need += desired[ i ]; need += T->ncols + 1;
	for ( i = 0; i < T->ncols; i++ ) out[ i ] = desired[ i ];
	if ( need > availw )
	{
		int over = need - availw; while ( over > 0 )
		{
			int maxidx = -1 , maxw = -1; for ( i = 0; i < T->ncols; i++ ) if ( out[ i ] > maxw && out[ i ] > T->cols[ i ].minw )
			{
				maxw = out[ i ]; maxidx = i;
			}
			if ( maxidx < 0 ) break; out[ maxidx ]--; over--;
		}
	}
}

static void put_clipped( struct ncplane * n , int y , int x , const char * s , int w )
{
	if ( !s ) s = ""; int L = slen( s );
	if ( L <= w )
	{
		ncplane_putstr_yx( n , y , x , s ); for ( int i = L; i < w; i++ ) ncplane_putstr_yx( n , y , x + i , " " );
	}
	else
	{
		if ( w <= 3 )
		{
			for ( int i = 0; i < w; i++ ) ncplane_putstr_yx( n , y , x + i , "." );
		}
		else
		{
			for ( int i = 0; i < w - 3; i++ ) ncplane_putnstr_yx( n , y , x + i , 1 , s + i ); ncplane_putstr_yx( n , y , x + w - 3 , "..." );
		}
	}
}

// Draw tabs
typedef struct
{
	int x0 , x1;
} TabHit;
static void draw_tabs( struct ncplane * tabs , int termw , int active , TabHit hits[] , int * nhits )
{
	*nhits = 0; ncplane_erase( tabs );
	int x = 1;
	for ( int i = 0; i < NTABS; i++ )
	{
		const char * label = ALLTABS[ i ]->tabname;
		int w = slen( label ) + 2; if ( x + w + 1 >= termw ) break;
		if ( i == active ) ncplane_set_fg_rgb8( tabs , 255 , 255 , 255 ) , ncplane_set_bg_rgb8( tabs , 0 , 0 , 200 ) , ncplane_set_styles( tabs , NCSTYLE_BOLD );
		else ncplane_set_fg_rgb8( tabs , 0 , 0 , 0 ) , ncplane_set_bg_rgb8( tabs , 0 , 255 , 255 );
		ncplane_putstr_yx( tabs , 1 , x , " " ); ncplane_putstr( tabs , label ); ncplane_putstr( tabs , " " ); ncplane_set_styles( tabs , NCSTYLE_NONE );
		hits[ *nhits ].x0 = x; hits[ *nhits ].x1 = x + w - 1; ( *nhits )++; x += w + 1;
	}
	for ( int i = 0; i < termw; i++ )
	{
		ncplane_set_fg_rgb8( tabs , 150 , 150 , 150 ); ncplane_putstr_yx( tabs , 2 , i , "─" );
	}
	ncplane_set_fg_default( tabs ); ncplane_set_bg_default( tabs );
}

// Draw table with cell borders
static void draw_table( struct ncplane * body , const Table * T , int termw , int termh )
{
	ncplane_erase( body );
	int colw[ MAX_COLS ]; compute_widths( T , termw - 2 , colw );
	int y = 0;
	// Top border
	ncplane_putstr_yx( body , y++ , 0 , "┌" ); for ( int c = 0; c < T->ncols; c++ )
	{
		for ( int k = 0; k < colw[ c ]; k++ ) ncplane_putstr( body , "─" ); if ( c == T->ncols - 1 ) ncplane_putstr( body , "┐" ); else ncplane_putstr( body , "┬" );
	}
	// Header
	ncplane_set_styles( body , NCSTYLE_BOLD );
	ncplane_set_fg_rgb8( body , 0 , 255 , 255 ); int x = 0; ncplane_putstr_yx( body , y , 0 , "│" );
	for ( int c = 0; c < T->ncols; c++ )
	{
		put_clipped( body , y , 1 + x , T->cols[ c ].name , colw[ c ] ); x += colw[ c ]; ncplane_putstr_yx( body , y , 1 + x , "│" ); x += 1;
	}
	ncplane_set_styles( body , NCSTYLE_NONE ); y++;
	// Subheader
	x = 0; ncplane_putstr_yx( body , y , 0 , "│" ); ncplane_set_fg_rgb8( body , 0 , 255 , 255 ); ncplane_set_styles( body , NCSTYLE_UNDERLINE );
	for ( int c = 0; c < T->ncols; c++ )
	{
		put_clipped( body , y , 1 + x , T->subhdrs[ c ] , colw[ c ] ); x += colw[ c ]; ncplane_putstr_yx( body , y , 1 + x , "│" ); x += 1;
	}
	ncplane_set_styles( body , NCSTYLE_NONE ); ncplane_set_fg_default( body ); y++;
	// Separator
	ncplane_putstr_yx( body , y++ , 0 , "├" ); for ( int c = 0; c < T->ncols; c++ )
	{
		for ( int k = 0; k < colw[ c ]; k++ ) ncplane_putstr( body , "─" ); if ( c == T->ncols - 1 ) ncplane_putstr( body , "┤" ); else ncplane_putstr( body , "┼" );
	}
	// Rows
	for ( int r = 0; r < T->nrows; r++ )
	{
		x = 0;
		if ( r % 2 == 0 ) ncplane_set_bg_rgb8( body , 10 , 10 , 30 ); else ncplane_set_bg_rgb8( body , 20 , 20 , 50 );
		ncplane_putstr_yx( body , y , 0 , "│" );
		for ( int c = 0; c < T->ncols; c++ )
		{
			put_clipped( body , y , 1 + x , T->cells[ r ][ c ] , colw[ c ] ); x += colw[ c ]; ncplane_putstr_yx( body , y , 1 + x , "│" ); x += 1;
		}
		ncplane_set_bg_default( body ); y++;
	}
	// Bottom
	ncplane_putstr_yx( body , y++ , 0 , "└" ); for ( int c = 0; c < T->ncols; c++ )
	{
		for ( int k = 0; k < colw[ c ]; k++ ) ncplane_putstr( body , "─" ); if ( c == T->ncols - 1 ) ncplane_putstr( body , "┘" ); else ncplane_putstr( body , "┴" );
	}
}

// Draw command box
static void draw_cmdbox( struct ncplane * cmd , const char * buf )
{
	ncplane_erase( cmd );
	int w , h; ncplane_dim_yx( cmd , &h , &w );
	ncplane_set_fg_rgb8( cmd , 255 , 165 , 0 ); ncplane_set_bg_rgb8( cmd , 10 , 10 , 40 ); ncplane_set_styles( cmd , NCSTYLE_BOLD );
	ncplane_putstr_yx( cmd , 0 , 0 , "┌" ); for ( int i = 1; i < w - 1; i++ ) ncplane_putstr( cmd , "─" ); ncplane_putstr( cmd , "┐" );
	ncplane_putstr_yx( cmd , 1 , 0 , "│ Command: " ); ncplane_putstr( cmd , buf ? buf : "" ); ncplane_putstr_yx( cmd , 1 , w - 1 , "│" );
	ncplane_putstr_yx( cmd , 2 , 0 , "└" ); for ( int i = 1; i < w - 1; i++ ) ncplane_putstr( cmd , "─" ); ncplane_putstr( cmd , "┘" );
	ncplane_set_styles( cmd , NCSTYLE_NONE ); ncplane_set_fg_default( cmd ); ncplane_set_bg_default( cmd );
}

// Main
int main( void )
{
	setlocale( LC_ALL , "" );
	struct notcurses_options opts = { 0 }; opts.flags = NCOPTION_SUPPRESS_BANNERS;
	struct notcurses * nc = notcurses_core_init( &opts , NULL ); if ( !nc ) return 1;
	notcurses_mice_enable( nc , NCMICE_BUTTON_EVENT );
	struct ncplane * std = notcurses_stdplane( nc );
	int termh , termw; ncplane_dim_yx( std , &termh , &termw );

	struct ncplane_options topts = { .y = 0,.x = 0,.rows = TAB_H,.cols = termw };
	struct ncplane * tabs = ncplane_create( std , &topts );
	struct ncplane_options bopts = { .y = TAB_H,.x = 0,.rows = termh - TAB_H - CMD_H,.cols = termw };
	struct ncplane * body = ncplane_create( std , &bopts );
	struct ncplane_options copts = { .y = TAB_H + ( termh - TAB_H - CMD_H ),.x = 0,.rows = CMD_H,.cols = termw };
	struct ncplane * cmd = ncplane_create( std , &copts );

	int active = 0; TabHit hits[ 16 ]; int nhits = 0;
	draw_tabs( tabs , termw , active , hits , &nhits ); draw_table( body , ALLTABS[ active ] , termw , termh );
	char cmdbuf[ 128 ] = { 0 }; int cmdpos = 0; draw_cmdbox( cmd , cmdbuf ); notcurses_render( nc );

	while ( 1 )
	{
		struct ncinput ni; uint32_t id = notcurses_get_nblock( nc , &ni );
		if ( id == 'q' || id == NCKEY_ESC ) break;

		if ( id == NCKEY_RESIZE )
		{
			ncplane_dim_yx( std , &termh , &termw );
			ncplane_resize_simple( tabs , TAB_H , termw );
			ncplane_move_yx( body , TAB_H , 0 );
			ncplane_resize_simple( body , termh - TAB_H - CMD_H , termw );
			ncplane_move_yx( cmd , TAB_H + ( termh - TAB_H - CMD_H ) , 0 );
			ncplane_resize_simple( cmd , CMD_H , termw );
			draw_tabs( tabs , termw , active , hits , &nhits );
			draw_table( body , ALLTABS[ active ] , termw , termh );
			draw_cmdbox( cmd , cmdbuf ); notcurses_render( nc ); continue;
		}

		if ( ni.evtype == NCTYPE_PRESS && id == NCKEY_BUTTON1 )
		{
			if ( ni.y == 1 )
			{
				int x = ni.x; for ( int i = 0; i < nhits; i++ )
				{
					if ( x >= hits[ i ].x0 && x <= hits[ i ].x1 )
					{
						active = i; draw_tabs( tabs , termw , active , hits , &nhits ); draw_table( body , ALLTABS[ active ] , termw , termh ); notcurses_render( nc );
					}
				}
			}
		}

		if ( id == NCKEY_BACKSPACE || id == 127 )
		{
			if ( cmdpos > 0 )
			{
				cmdpos--; cmdbuf[ cmdpos ] = 0; draw_cmdbox( cmd , cmdbuf ); notcurses_render( nc );
			}
		}
		else if ( /*ni.evtype == NCTYPE_UNICODE &&*/ id >= 32 )
		{
			if ( cmdpos < 127 )
			{
				cmdbuf[ cmdpos++ ] = id; cmdbuf[ cmdpos ] = 0; draw_cmdbox( cmd , cmdbuf ); notcurses_render( nc );
			}
		}
		else if ( id == '\n' )
		{
			cmdpos = 0; cmdbuf[ 0 ] = 0; draw_cmdbox( cmd , cmdbuf ); notcurses_render( nc );
		}
	}

	notcurses_stop( nc );
	return 0;
}


#endif

#ifndef v7

// notcurses_tabs_cmd.c
#include <notcurses/notcurses.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>

#define TAB_H 3
#define CMD_H 3
#define MAX_COLS 8
#define MAX_ROWS 32

typedef struct
{
	const char * name;
	int minw;
} Column;

typedef struct
{
	const char * tabname;
	int ncols;
	Column cols[ MAX_COLS ];
	int nrows;
	const char * subhdrs[ MAX_COLS ];
	const char * cells[ MAX_ROWS ][ MAX_COLS ];
} Table;

// --- Demo tables ---
static Table TAB_CPU = {
  .tabname = "CPU",
  .ncols = 4,
  .cols = { {"Core",4}, {"Usage%",7}, {"Freq",6}, {"Gov",3} },
  .subhdrs = {"ID","Load","MHz","Policy"},
  .nrows = 4,
  .cells = {
	{"0","12.3","3600","schedutil"},
	{"1","5.1","3580","ondemand"},
	{"2","22.9","4025","performance"},
	{"avg","13.4","3735","mixed"},
  }
};

static Table TAB_MEM = {
  .tabname = "Memory",
  .ncols = 3,
  .cols = { {"Type",4}, {"Used",5}, {"Total",5} },
  .subhdrs = {"Kind","Now","Capacity"},
  .nrows = 3,
  .cells = {
	{"RAM","3.2G","7.7G"},
	{"Swap","0.0G","2.0G"},
	{"Cache","1.1G","—"},
  }
};

static Table * ALLTABS[] = { &TAB_CPU, &TAB_MEM };
static const int NTABS = sizeof( ALLTABS ) / sizeof( ALLTABS[ 0 ] );

// --- helpers ---
static int slen( const char * s )
{
	return s ? ( int )strlen( s ) : 0;
}

static void compute_widths( const Table * T , int availw , int * out )
{
	int desired[ MAX_COLS ] = { 0 };
	int i , r;
	for ( i = 0; i < T->ncols; i++ )
	{
		int w = slen( T->cols[ i ].name );
		if ( w < T->cols[ i ].minw ) w = T->cols[ i ].minw;
		int sw = slen( T->subhdrs[ i ] );
		if ( sw > w ) w = sw;
		for ( r = 0; r < T->nrows; r++ )
		{
			int cw = slen( T->cells[ r ][ i ] );
			if ( cw > w ) w = cw;
		}
		desired[ i ] = w;
	}
	int need = 0;
	for ( i = 0; i < T->ncols; i++ ) need += desired[ i ];
	need += T->ncols + 1;

	for ( i = 0; i < T->ncols; i++ ) out[ i ] = desired[ i ];
	if ( need > availw )
	{
		int over = need - availw;
		while ( over > 0 )
		{
			int maxidx = -1 , maxw = -1;
			for ( i = 0; i < T->ncols; i++ )
			{
				if ( out[ i ] > maxw && out[ i ] > T->cols[ i ].minw )
				{
					maxw = out[ i ]; maxidx = i;
				}
			}
			if ( maxidx < 0 ) break;
			out[ maxidx ]--; over--;
		}
	}
}

static void put_clipped( struct ncplane * n , int y , int x , const char * s , int w )
{
	if ( !s ) s = "";
	int L = slen( s );
	if ( L <= w )
	{
		ncplane_putstr_yx( n , y , x , s );
		for ( int i = L; i < w; i++ ) ncplane_putstr_yx( n , y , x + i , " " );
	}
	else
	{
		if ( w <= 3 )
		{
			for ( int i = 0; i < w; i++ ) ncplane_putstr_yx( n , y , x + i , "." );
		}
		else
		{
			for ( int i = 0; i < w - 3; i++ ) ncplane_putnstr_yx( n , y , x + i , 1 , s + i );
			ncplane_putstr_yx( n , y , x + w - 3 , "..." );
		}
	}
}

// --- draw tabs ---
typedef struct
{
	int x0 , x1;
} TabHit;
static void draw_tabs( struct ncplane * tabs , int termw , int active , TabHit hits[] , int * nhits )
{
	*nhits = 0; ncplane_erase( tabs );
	int x = 1;
	for ( int i = 0; i < NTABS; i++ )
	{
		const char * label = ALLTABS[ i ]->tabname;
		int w = slen( label ) + 2;
		if ( x + w + 1 >= termw ) break;
		if ( i == active ) ncplane_set_styles( tabs ,  NCSTYLE_BOLD );
		ncplane_putstr_yx( tabs , 1 , x , " " );
		ncplane_putstr( tabs , label );
		ncplane_putstr( tabs , " " );
		ncplane_set_styles( tabs , NCSTYLE_NONE );
		hits[ *nhits ].x0 = x; hits[ *nhits ].x1 = x + w - 1; ( *nhits )++;
		x += w + 1;
	}
	for ( int i = 0; i < termw; i++ ) ncplane_putstr_yx( tabs , 2 , i , "─" );
}

// --- draw table ---
static void draw_table( struct ncplane * body , const Table * T , int termw , int termh )
{
	ncplane_erase( body );
	int colw[ MAX_COLS ]; compute_widths( T , termw - 2 , colw );
	int x = 0 , y = 0;

	// top border
	ncplane_putstr_yx( body , y++ , 0 , "┌" );
	for ( int c = 0; c < T->ncols; c++ )
	{
		for ( int k = 0; k < colw[ c ]; k++ ) ncplane_putstr( body , "─" );
		if ( c == T->ncols - 1 ) ncplane_putstr( body , "┐" );
		else ncplane_putstr( body , "┬" );
	}

	// header
	ncplane_putstr_yx( body , y , 0 , "│" );
	ncplane_set_styles( body , NCSTYLE_BOLD );
	x = 0;
	for ( int c = 0; c < T->ncols; c++ )
	{
		put_clipped( body , y , 1 + x , T->cols[ c ].name , colw[ c ] );
		x += colw[ c ];
		ncplane_putstr_yx( body , y , 1 + x , "│" );
		x += 1;
	}
	ncplane_set_styles( body , NCSTYLE_NONE );
	y++;

	// subheader
	x = 0;
	ncplane_putstr_yx( body , y , 0 , "│" );
	ncplane_set_styles( body , NCSTYLE_UNDERLINE );
	for ( int c = 0; c < T->ncols; c++ )
	{
		put_clipped( body , y , 1 + x , T->subhdrs[ c ] , colw[ c ] );
		x += colw[ c ];
		ncplane_putstr_yx( body , y , 1 + x , "│" );
		x += 1;
	}
	ncplane_set_styles( body , NCSTYLE_NONE );
	y++;

	// separator
	ncplane_putstr_yx( body , y++ , 0 , "├" );
	for ( int c = 0; c < T->ncols; c++ )
	{
		for ( int k = 0; k < colw[ c ]; k++ ) ncplane_putstr( body , "─" );
		if ( c == T->ncols - 1 ) ncplane_putstr( body , "┤" );
		else ncplane_putstr( body , "┼" );
	}

	// rows
	for ( int r = 0; r < T->nrows; r++ )
	{
		x = 0;
		if ( r % 2 == 0 ) ncplane_set_bg_rgb8( body , 30 , 30 , 30 );
		else ncplane_set_bg_rgb8( body , 50 , 50 , 70 );
		ncplane_putstr_yx( body , y , 0 , "│" );
		for ( int c = 0; c < T->ncols; c++ )
		{
			put_clipped( body , y , 1 + x , T->cells[ r ][ c ] , colw[ c ] );
			x += colw[ c ];
			ncplane_putstr_yx( body , y , 1 + x , "│" );
			x += 1;
		}
		ncplane_set_bg_default( body );
		y++;
	}

	// bottom
	ncplane_putstr_yx( body , y++ , 0 , "└" );
	for ( int c = 0; c < T->ncols; c++ )
	{
		for ( int k = 0; k < colw[ c ]; k++ ) ncplane_putstr( body , "─" );
		if ( c == T->ncols - 1 ) ncplane_putstr( body , "┘" );
		else ncplane_putstr( body , "┴" );
	}
}

// --- draw command box ---
static void draw_cmdbox( struct ncplane * cmd , const char * buf )
{
	ncplane_erase( cmd );
	int w , h; ncplane_dim_yx( cmd , &h , &w );
	ncplane_set_fg_rgb8( cmd , 255 , 255 , 0 );
	ncplane_set_bg_rgb8( cmd , 80 , 0 , 0 );
	ncplane_putstr_yx( cmd , 0 , 0 , "┌" ); for ( int i = 1; i < w - 1; i++ ) ncplane_putstr( cmd , "─" ); ncplane_putstr( cmd , "┐" );
	ncplane_putstr_yx( cmd , 1 , 0 , "│ Command: " ); ncplane_putstr( cmd , buf ? buf : "" );
	ncplane_putstr_yx( cmd , 1 , w - 1 , "│" );
	ncplane_putstr_yx( cmd , 2 , 0 , "└" ); for ( int i = 1; i < w - 1; i++ ) ncplane_putstr( cmd , "─" ); ncplane_putstr( cmd , "┘" );
	ncplane_set_fg_default( cmd );
	ncplane_set_bg_default( cmd );
}

// --- main ---
int main( void )
{
	setlocale( LC_ALL , "" );
	struct notcurses_options opts = { 0 };
	opts.flags = NCOPTION_SUPPRESS_BANNERS;
	struct notcurses * nc = notcurses_core_init( &opts , NULL );
	if ( !nc ) return 1;

	notcurses_mice_enable( nc , NCMICE_BUTTON_EVENT );
	struct ncplane * std = notcurses_stdplane( nc );
	int termh , termw; ncplane_dim_yx( std , &termh , &termw );

	struct ncplane_options topts = { .y = 0,.x = 0,.rows = TAB_H,.cols = termw };
	struct ncplane * tabs = ncplane_create( std , &topts );
	struct ncplane_options bopts = { .y = TAB_H,.x = 0,.rows = termh - TAB_H - CMD_H,.cols = termw };
	struct ncplane * body = ncplane_create( std , &bopts );
	struct ncplane_options copts = { .y = TAB_H + ( termh - TAB_H - CMD_H ),.x = 0,.rows = CMD_H,.cols = termw };
	struct ncplane * cmd = ncplane_create( std , &copts );

	int active = 0; TabHit hits[ 16 ]; int nhits = 0;
	draw_tabs( tabs , termw , active , hits , &nhits );
	draw_table( body , ALLTABS[ active ] , termw , termh );
	char cmdbuf[ 128 ] = { 0 }; int cmdpos = 0;
	draw_cmdbox( cmd , cmdbuf );
	notcurses_render( nc );

	while ( 1 )
	{
		struct ncinput ni; uint32_t id = notcurses_get_nblock( nc , &ni );

		if ( id == 'q' || id == NCKEY_ESC ) break;
		if ( id == NCKEY_RESIZE )
		{
			ncplane_dim_yx( std , &termh , &termw );
			ncplane_resize_simple( tabs , TAB_H , termw );
			ncplane_move_yx( body , TAB_H , 0 );
			ncplane_resize_simple( body , termh - TAB_H - CMD_H , termw );
			ncplane_move_yx( cmd , TAB_H + ( termh - TAB_H - CMD_H ) , 0 );
			ncplane_resize_simple( cmd , CMD_H , termw );
			draw_tabs( tabs , termw , active , hits , &nhits );
			draw_table( body , ALLTABS[ active ] , termw , termh );
			draw_cmdbox( cmd , cmdbuf );
			notcurses_render( nc );
			continue;
		}

		// mouse tabs
		if ( ni.evtype == NCTYPE_PRESS && id == NCKEY_BUTTON1 )
		{
			if ( ni.y == 1 )
			{
				int x = ni.x;
				for ( int i = 0; i < nhits; i++ )
				{
					if ( x >= hits[ i ].x0 && x <= hits[ i ].x1 )
					{
						active = i;
						draw_tabs( tabs , termw , active , hits , &nhits );
						draw_table( body , ALLTABS[ active ] , termw , termh );
						notcurses_render( nc );
					}
				}
			}
		}

		// command input
		if ( id == NCKEY_BACKSPACE || id == 127 )
		{
			if ( cmdpos > 0 )
			{
				cmdpos--; cmdbuf[ cmdpos ] = 0; draw_cmdbox( cmd , cmdbuf ); notcurses_render( nc );
			}
		}
		else if ( /*ni.evtype == NCTYPE_UNICODE &&*/ id >= 32 )
		{
			if ( cmdpos < 127 )
			{
				cmdbuf[ cmdpos++ ] = id; cmdbuf[ cmdpos ] = 0; draw_cmdbox( cmd , cmdbuf ); notcurses_render( nc );
			}
		}
		else if ( id == '\n' )
		{
			// For demo: clear buffer after Enter
			cmdpos = 0; cmdbuf[ 0 ] = 0; draw_cmdbox( cmd , cmdbuf ); notcurses_render( nc );
		}
	}

	notcurses_stop( nc );
	return 0;
}


#endif

#ifndef v6

#include <notcurses/notcurses.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>

#define TAB_H 3
#define MAX_COLS 8
#define MAX_ROWS 32

typedef struct
{
	const char * name;
	int minw;
} Column;

typedef struct
{
	const char * tabname;
	int ncols;
	Column cols[ MAX_COLS ];
	int nrows;
	const char * subhdrs[ MAX_COLS ];           // subheader row
	const char * cells[ MAX_ROWS ][ MAX_COLS ];   // table data
} Table;

// --- Demo tables -------------------------------------------------------------
static Table TAB_CPU = {
  .tabname = "CPU",
  .ncols = 4,
  .cols = { {"Core",4}, {"Usage%",7}, {"Freq",6}, {"Gov",3} },
  .subhdrs = {"ID","Load","MHz","Policy"},
  .nrows = 4,
  .cells = {
	{"0","12.3","3600","schedutil"},
	{"1","5.1","3580","ondemand"},
	{"2","22.9","4025","performance"},
	{"avg","13.4","3735","mixed"},
  }
};

static Table TAB_MEM = {
  .tabname = "Memory",
  .ncols = 3,
  .cols = { {"Type",4}, {"Used",5}, {"Total",5} },
  .subhdrs = {"Kind","Now","Capacity"},
  .nrows = 3,
  .cells = {
	{"RAM","3.2G","7.7G"},
	{"Swap","0.0G","2.0G"},
	{"Cache","1.1G","—"},
  }
};

static Table * ALLTABS[] = { &TAB_CPU, &TAB_MEM };
static const int NTABS = sizeof( ALLTABS ) / sizeof( ALLTABS[ 0 ] );

// --- helpers -----------------------------------------------------------------
static int slen( const char * s )
{
	return s ? ( int )strlen( s ) : 0;
}

static void compute_widths( const Table * T , int availw , int * out )
{
	int desired[ MAX_COLS ] = { 0 };
	int i , r;
	for ( i = 0; i < T->ncols; i++ )
	{
		int w = slen( T->cols[ i ].name );
		if ( w < T->cols[ i ].minw ) w = T->cols[ i ].minw;
		int sw = slen( T->subhdrs[ i ] );
		if ( sw > w ) w = sw;
		for ( r = 0; r < T->nrows; r++ )
		{
			int cw = slen( T->cells[ r ][ i ] );
			if ( cw > w ) w = cw;
		}
		desired[ i ] = w;
	}
	int need = 0;
	for ( i = 0; i < T->ncols; i++ ) need += desired[ i ];
	need += T->ncols + 1; // vertical separators

	for ( i = 0; i < T->ncols; i++ ) out[ i ] = desired[ i ];
	if ( need > availw )
	{
		int over = need - availw;
		while ( over > 0 )
		{
			int maxidx = -1 , maxw = -1;
			for ( i = 0; i < T->ncols; i++ )
			{
				if ( out[ i ] > maxw && out[ i ] > T->cols[ i ].minw )
				{
					maxw = out[ i ]; maxidx = i;
				}
			}
			if ( maxidx < 0 ) break;
			out[ maxidx ]--; over--;
		}
	}
}

static void put_clipped( struct ncplane * n , int y , int x , const char * s , int w )
{
	if ( !s ) s = "";
	int L = slen( s );
	if ( L <= w )
	{
		ncplane_putstr_yx( n , y , x , s );
		for ( int i = L; i < w; i++ ) ncplane_putstr_yx( n , y , x + i , " " );
	}
	else
	{
		if ( w <= 3 )
		{
			for ( int i = 0; i < w; i++ ) ncplane_putstr_yx( n , y , x + i , "." );
		}
		else
		{
			for ( int i = 0; i < w - 3; i++ ) ncplane_putnstr_yx( n , y , x + i , 1 , s + i );
			ncplane_putstr_yx( n , y , x + w - 3 , "..." );
		}
	}
}

// --- draw tabs ---------------------------------------------------------------
typedef struct
{
	int x0 , x1;
} TabHit;
static void draw_tabs( struct ncplane * tabs , int termw , int active , TabHit hits[] , int * nhits )
{
	*nhits = 0; ncplane_erase( tabs );
	int x = 1;
	for ( int i = 0; i < NTABS; i++ )
	{
		const char * label = ALLTABS[ i ]->tabname;
		int w = slen( label ) + 2;
		if ( x + w + 1 >= termw ) break;
		if ( i == active ) ncplane_set_styles( tabs , NCSTYLE_BOLD );
		ncplane_putstr_yx( tabs , 1 , x , " " );
		ncplane_putstr( tabs , label );
		ncplane_putstr( tabs , " " );
		ncplane_set_styles( tabs , NCSTYLE_NONE );
		hits[ *nhits ].x0 = x; hits[ *nhits ].x1 = x + w - 1; ( *nhits )++;
		x += w + 1;
	}
	for ( int i = 0; i < termw; i++ ) ncplane_putstr_yx( tabs , 2 , i , "─" );
}

// --- draw table --------------------------------------------------------------
static void draw_table( struct ncplane * body , const Table * T , int termw , int termh )
{
	ncplane_erase( body );
	int colw[ MAX_COLS ]; compute_widths( T , termw - 2 , colw );
	int x = 0 , y = 0;

	// top border
	ncplane_putstr_yx( body , y++ , 0 , "┌" );
	for ( int c = 0; c < T->ncols; c++ )
	{
		for ( int k = 0; k < colw[ c ]; k++ ) ncplane_putstr( body , "─" );
		if ( c == T->ncols - 1 ) ncplane_putstr( body , "┐" );
		else ncplane_putstr( body , "┬" );
	}

	// header
	ncplane_putstr_yx( body , y , 0 , "│" );
	ncplane_set_styles( body , NCSTYLE_BOLD );
	for ( int c = 0; c < T->ncols; c++ )
	{
		put_clipped( body , y , 1 + x , T->cols[ c ].name , colw[ c ] );
		x += colw[ c ];
		if ( c == T->ncols - 1 ) ncplane_putstr_yx( body , y , 1 + x , "│" );
		else ncplane_putstr_yx( body , y , 1 + x , "│" );
		x += 1;
	}
	ncplane_set_styles( body , NCSTYLE_NONE );
	y++;

	// subheader
	x = 0;
	ncplane_putstr_yx( body , y , 0 , "│" );
	ncplane_set_styles( body , NCSTYLE_UNDERLINE );
	for ( int c = 0; c < T->ncols; c++ )
	{
		put_clipped( body , y , 1 + x , T->subhdrs[ c ] , colw[ c ] );
		x += colw[ c ];
		if ( c == T->ncols - 1 ) ncplane_putstr_yx( body , y , 1 + x , "│" );
		else ncplane_putstr_yx( body , y , 1 + x , "│" );
		x += 1;
	}
	ncplane_set_styles( body , NCSTYLE_NONE );
	y++;

	// separator line under subheader
	ncplane_putstr_yx( body , y++ , 0 , "├" );
	for ( int c = 0; c < T->ncols; c++ )
	{
		for ( int k = 0; k < colw[ c ]; k++ ) ncplane_putstr( body , "─" );
		if ( c == T->ncols - 1 ) ncplane_putstr( body , "┤" );
		else ncplane_putstr( body , "┼" );
	}

	// rows with lightly contrasting paired row colors
	for ( int r = 0; r < T->nrows; r++ )
	{
		x = 0;
		if ( r % 2 == 0 ) ncplane_set_bg_rgb8( body , 40 , 40 , 40 ); // light gray for even rows
		else ncplane_set_bg_rgb8( body , 60 , 60 , 60 );       // slightly darker gray for odd rows
		ncplane_putstr_yx( body , y , 0 , "│" );
		for ( int c = 0; c < T->ncols; c++ )
		{
			put_clipped( body , y , 1 + x , T->cells[ r ][ c ] , colw[ c ] );
			x += colw[ c ];
			if ( c == T->ncols - 1 ) ncplane_putstr_yx( body , y , 1 + x , "│" );
			else ncplane_putstr_yx( body , y , 1 + x , "│" );
			x += 1;
		}
		ncplane_set_bg_default( body );
		y++;
	}

	// bottom border
	ncplane_putstr_yx( body , y++ , 0 , "└" );
	for ( int c = 0; c < T->ncols; c++ )
	{
		for ( int k = 0; k < colw[ c ]; k++ ) ncplane_putstr( body , "─" );
		if ( c == T->ncols - 1 ) ncplane_putstr( body , "┘" );
		else ncplane_putstr( body , "┴" );
	}
}

// --- main loop ---------------------------------------------------------------
int main( void )
{
	setlocale( LC_ALL , "" );
	struct notcurses_options opts = { 0 };
	opts.flags = NCOPTION_SUPPRESS_BANNERS;
	struct notcurses * nc = notcurses_core_init( &opts , NULL );
	if ( !nc ) return 1;

	notcurses_mice_enable( nc , NCMICE_BUTTON_EVENT );
	struct ncplane * std = notcurses_stdplane( nc );
	int termh , termw; ncplane_dim_yx( std , &termh , &termw );

	struct ncplane_options topts = { .y = 0,.x = 0,.rows = TAB_H,.cols = termw };
	struct ncplane * tabs = ncplane_create( std , &topts );
	struct ncplane_options bopts = { .y = TAB_H,.x = 0,.rows = termh - TAB_H,.cols = termw };
	struct ncplane * body = ncplane_create( std , &bopts );

	int active = 0; TabHit hits[ 16 ]; int nhits = 0;
	draw_tabs( tabs , termw , active , hits , &nhits );
	draw_table( body , ALLTABS[ active ] , termw , termh );
	notcurses_render( nc );

	while ( 1 )
	{
		struct ncinput ni; uint32_t id = notcurses_get_nblock( nc , &ni );
		if ( id == 'q' || id == NCKEY_ESC ) break;
		if ( id == NCKEY_RESIZE )
		{
			ncplane_dim_yx( std , &termh , &termw );
			ncplane_resize_simple( tabs , TAB_H , termw );
			ncplane_move_yx( body , TAB_H , 0 );
			ncplane_resize_simple( body , termh - TAB_H , termw );
			draw_tabs( tabs , termw , active , hits , &nhits );
			draw_table( body , ALLTABS[ active ] , termw , termh );
			notcurses_render( nc );
			continue;
		}
		if ( ni.evtype == NCTYPE_PRESS && id == NCKEY_BUTTON1 )
		{
			if ( ni.y == 1 )
			{ // click row with tabs
				int x = ni.x;
				for ( int i = 0; i < nhits; i++ )
				{
					if ( x >= hits[ i ].x0 && x <= hits[ i ].x1 )
					{
						active = i;
						draw_tabs( tabs , termw , active , hits , &nhits );
						draw_table( body , ALLTABS[ active ] , termw , termh );
						notcurses_render( nc );
					}
				}
			}
		}
	}

	notcurses_stop( nc );
	return 0;
}

#endif

#ifndef v5

// services_dashboard.c
// Tabs with mouse-click focus + table with alternating row colors + vertical column lines
#include <notcurses/notcurses.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>

#define TAB_H 3
#define MAX_COLS 6
#define MAX_ROWS 23

typedef struct
{
	const char * name;
	int minw;
} Column;

typedef struct
{
	const char * tabname;
	int ncols;
	Column cols[ MAX_COLS ];
	int nrows;
	const char * cells[ MAX_ROWS ][ MAX_COLS ];   // table data
} Table;

// --- Demo tables -------------------------------------------------------------
static Table TAB_ACTIVE = {
	.tabname = "Active (20)",
	.ncols = 6,
	.cols = {
		{"Service name", 15},
		{"Status", 10},
		{"Type", 15},
		{"Runtime", 10},
		{"Region", 10},
		{"Last deployed", 12}
	},
	.nrows = 8,
	.cells = {
		{"simple-wor...", "Deployed", "Background Worker", "Node", "Oregon", "3 months ago"},
		{"my-cron-job", "Successful run", "Cron Job", "Node", "Oregon", "7 months ago"},
		{"mystite", "Available", "PostgreSQL", "PostgreSQL I5", "Oregon", "5 months ago"},
		{"example-ra...", "Available", "Redis", "Redis", "Oregon", "a year ago"},
		{"llama-front...", "Deployed", "Static Site", "Static", "Global", "8 months ago"},
		{"docusauru...", "Deployed", "Static Site", "Static", "Global", "a year ago"},
		{"mystite", "Deploying", "Web Service", "Python 3", "Oregon", "6 days ago"},
		{"prometheu...", "Deployed", "Web Service", "Docker", "Oregon", "2 months ago"}
	}
};

static Table TAB_SUSPENDED = {
	.tabname = "Suspended (3)",
	.ncols = 6,
	.cols = {
		{"Service name", 15},
		{"Status", 10},
		{"Type", 15},
		{"Runtime", 10},
		{"Region", 10},
		{"Last deployed", 12}
	},
	.nrows = 3,
	.cells = {
		{"old-service", "Suspended", "Web Service", "Python 2", "Oregon", "2 years ago"},
		{"test-app", "Suspended", "Web Service", "Node", "Oregon", "18 months ago"},
		{"legacy-db", "Suspended", "PostgreSQL", "PostgreSQL I3", "Oregon", "3 years ago"}
	}
};

static Table TAB_ALL = {
	.tabname = "All (23)",
	.ncols = 6,
	.cols = {
		{"Service name", 15},
		{"Status", 10},
		{"Type", 15},
		{"Runtime", 10},
		{"Region", 10},
		{"Last deployed", 12}
	},
	.nrows = 11,
	.cells = {
		{"simple-wor...", "Deployed", "Background Worker", "Node", "Oregon", "3 months ago"},
		{"my-cron-job", "Successful run", "Cron Job", "Node", "Oregon", "7 months ago"},
		{"mystite", "Available", "PostgreSQL", "PostgreSQL I5", "Oregon", "5 months ago"},
		{"example-ra...", "Available", "Redis", "Redis", "Oregon", "a year ago"},
		{"llama-front...", "Deployed", "Static Site", "Static", "Global", "8 months ago"},
		{"docusauru...", "Deployed", "Static Site", "Static", "Global", "a year ago"},
		{"mystite", "Deploying", "Web Service", "Python 3", "Oregon", "6 days ago"},
		{"prometheu...", "Deployed", "Web Service", "Docker", "Oregon", "2 months ago"},
		{"old-service", "Suspended", "Web Service", "Python 2", "Oregon", "2 years ago"},
		{"test-app", "Suspended", "Web Service", "Node", "Oregon", "18 months ago"},
		{"legacy-db", "Suspended", "PostgreSQL", "PostgreSQL I3", "Oregon", "3 years ago"}
	}
};

static Table * ALLTABS[] = { &TAB_ACTIVE, &TAB_SUSPENDED, &TAB_ALL };
static const int NTABS = sizeof( ALLTABS ) / sizeof( ALLTABS[ 0 ] );

// --- helpers -----------------------------------------------------------------
static int slen( const char * s )
{
	return s ? ( int )strlen( s ) : 0;
}

static void compute_widths( const Table * T , int availw , int * out )
{
	int desired[ MAX_COLS ] = { 0 };
	int i , r;
	for ( i = 0; i < T->ncols; i++ )
	{
		int w = slen( T->cols[ i ].name );
		if ( w < T->cols[ i ].minw ) w = T->cols[ i ].minw;
		for ( r = 0; r < T->nrows; r++ )
		{
			int cw = slen( T->cells[ r ][ i ] );
			if ( cw > w ) w = cw;
		}
		desired[ i ] = w;
	}
	int need = 0;
	for ( i = 0; i < T->ncols; i++ ) need += desired[ i ];
	need += T->ncols + 1; // vertical separators

	for ( i = 0; i < T->ncols; i++ ) out[ i ] = desired[ i ];
	if ( need > availw )
	{
		int over = need - availw;
		while ( over > 0 )
		{
			int maxidx = -1 , maxw = -1;
			for ( i = 0; i < T->ncols; i++ )
			{
				if ( out[ i ] > maxw && out[ i ] > T->cols[ i ].minw )
				{
					maxw = out[ i ]; maxidx = i;
				}
			}
			if ( maxidx < 0 ) break;
			out[ maxidx ]--; over--;
		}
	}
}

static void put_clipped( struct ncplane * n , int y , int x , const char * s , int w )
{
	if ( !s ) s = "";
	int L = slen( s );
	if ( L <= w )
	{
		ncplane_putstr_yx( n , y , x , s );
		for ( int i = L; i < w; i++ ) ncplane_putstr_yx( n , y , x + i , " " );
	}
	else
	{
		if ( w <= 3 )
		{
			for ( int i = 0; i < w; i++ ) ncplane_putstr_yx( n , y , x + i , "." );
		}
		else
		{
			for ( int i = 0; i < w - 3; i++ ) ncplane_putnstr_yx( n , y , x + i , 1 , s + i );
			ncplane_putstr_yx( n , y , x + w - 3 , "..." );
		}
	}
}

// Status color mapping
static uint64_t get_status_color( const char * status )
{
	if ( strcmp( status , "Deployed" ) == 0 ) return NCCHANNELS_INITIALIZER( 0x00 , 0xFF , 0x00 , 0x00 , 0x00 , 0x00 ); // Green
	if ( strcmp( status , "Available" ) == 0 ) return NCCHANNELS_INITIALIZER( 0x00 , 0xFF , 0x00 , 0x00 , 0x00 , 0x00 ); // Green
	if ( strcmp( status , "Successful run" ) == 0 ) return NCCHANNELS_INITIALIZER( 0x00 , 0xFF , 0x00 , 0x00 , 0x00 , 0x00 ); // Green
	if ( strcmp( status , "Deploying" ) == 0 ) return NCCHANNELS_INITIALIZER( 0xFF , 0xFF , 0x00 , 0x00 , 0x00 , 0x00 ); // Yellow
	if ( strcmp( status , "Suspended" ) == 0 ) return NCCHANNELS_INITIALIZER( 0xFF , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 ); // Red
	return NCCHANNELS_INITIALIZER( 0xFF , 0xFF , 0xFF , 0x00 , 0x00 , 0x00 ); // White (default)
}

// --- draw tabs ---------------------------------------------------------------
typedef struct
{
	int x0 , x1;
} TabHit;
static void draw_tabs( struct ncplane * tabs , int termw , int active , TabHit hits[] , int * nhits )
{
	*nhits = 0; ncplane_erase( tabs );

	// Draw top border
	for ( int i = 0; i < termw; i++ ) ncplane_putstr_yx( tabs , 0 , i , "─" );

	int x = 1;
	for ( int i = 0; i < NTABS; i++ )
	{
		const char * label = ALLTABS[ i ]->tabname;
		int w = slen( label ) + 4; // Add padding

		if ( i == active )
		{
			ncplane_set_styles( tabs , NCSTYLE_BOLD );
			ncplane_set_bg_rgb8( tabs , 60 , 60 , 80 ); // Dark blue background for active tab
			ncplane_set_fg_rgb8( tabs , 255 , 255 , 255 ); // White text
		}
		else
		{
			ncplane_set_bg_default( tabs );
			ncplane_set_fg_rgb8( tabs , 200 , 200 , 200 ); // Light gray text
		}

		ncplane_putstr_yx( tabs , 1 , x , " " );
		ncplane_putstr( tabs , label );
		ncplane_putstr( tabs , " " );

		// Reset styles
		ncplane_set_styles( tabs , NCSTYLE_NONE );
		ncplane_set_bg_default( tabs );
		ncplane_set_fg_default( tabs );

		hits[ *nhits ].x0 = x; hits[ *nhits ].x1 = x + w - 1; ( *nhits )++;
		x += w;
	}

	// Draw bottom border of tab area
	for ( int i = 0; i < termw; i++ ) ncplane_putstr_yx( tabs , 2 , i , "─" );
}

static void
draw_table( struct ncplane * body , const Table * T , int termw , int termh )
{
	ncplane_erase( body );

	int colw[ MAX_COLS ];
	compute_widths( T , termw - 2 , colw );

	int y = 0 , x = 0;

	/* ---------- top border ---------- */
	ncplane_putstr_yx( body , y++ , 0 , "┌" );
	for ( int c = 0; c < T->ncols; ++c )
	{
		for ( int k = 0; k < colw[ c ]; ++k ) ncplane_putstr( body , "─" );
		ncplane_putstr( body , ( c == T->ncols - 1 ) ? "┐" : "┬" );
	}

	/* ---------- header (bold white) ---------- */
	ncplane_putstr_yx( body , y , 0 , "│" );
	ncplane_set_styles( body , NCSTYLE_BOLD );
	for ( int c = 0 , xoff = 1; c < T->ncols; ++c )
	{
		put_clipped( body , y , xoff , T->cols[ c ].name , colw[ c ] );
		xoff += colw[ c ];
		ncplane_putstr_yx( body , y , xoff++ , "│" );
	}
	ncplane_set_styles( body , NCSTYLE_NONE );
	++y;

	/* ---------- sub-header (underlined white) ---------- */
	ncplane_putstr_yx( body , y , 0 , "│" );
	ncplane_set_styles( body , NCSTYLE_UNDERLINE );
	for ( int c = 0 , xoff = 1; c < T->ncols; ++c )
	{



		put_clipped( body , y , xoff , T->cols[ c ].name /*T->subhdrs[c]*/ , colw[ c ] );
		xoff += colw[ c ];
		ncplane_putstr_yx( body , y , xoff++ , "│" );
	}
	ncplane_set_styles( body , NCSTYLE_NONE );
	++y;

	/* ---------- separator under sub-header ---------- */
	ncplane_putstr_yx( body , y++ , 0 , "├" );
	for ( int c = 0; c < T->ncols; ++c )
	{
		for ( int k = 0; k < colw[ c ]; ++k ) ncplane_putstr( body , "─" );
		ncplane_putstr( body , ( c == T->ncols - 1 ) ? "┤" : "┼" );
	}

	/* ---------- data rows – alternating “Render” colours ---------- */
	for ( int r = 0; r < T->nrows; ++r )
	{
		/* choose background: odd rows very-dark-blue, even slightly lighter */
		uint32_t bg = ( r & 1 ) ? 0x0c0c1c : 0x141428;
		ncplane_set_bg_rgb( body , bg );

		ncplane_putstr_yx( body , y , 0 , "│" );
		for ( int c = 0 , xoff = 1; c < T->ncols; ++c )
		{
			put_clipped( body , y , xoff , T->cells[ r ][ c ] , colw[ c ] );
			xoff += colw[ c ];
			ncplane_putstr_yx( body , y , xoff++ , "│" );
		}
		ncplane_set_bg_default( body );
		++y;
	}

	/* ---------- bottom border ---------- */
	ncplane_putstr_yx( body , y++ , 0 , "└" );
	for ( int c = 0; c < T->ncols; ++c )
	{
		for ( int k = 0; k < colw[ c ]; ++k ) ncplane_putstr( body , "─" );
		ncplane_putstr( body , ( c == T->ncols - 1 ) ? "┘" : "┴" );
	}
}

// --- main loop ---------------------------------------------------------------
int main( void )
{
	setlocale( LC_ALL , "" );
	struct notcurses_options opts = { 0 };
	opts.flags = NCOPTION_SUPPRESS_BANNERS;
	struct notcurses * nc = notcurses_core_init( &opts , NULL );
	if ( !nc ) return 1;

	notcurses_mice_enable( nc , NCMICE_BUTTON_EVENT );
	struct ncplane * std = notcurses_stdplane( nc );
	int termh , termw; ncplane_dim_yx( std , &termh , &termw );

	struct ncplane_options topts = { .y = 0,.x = 0,.rows = TAB_H,.cols = termw };
	struct ncplane * tabs = ncplane_create( std , &topts );
	struct ncplane_options bopts = { .y = TAB_H,.x = 0,.rows = termh - TAB_H,.cols = termw };
	struct ncplane * body = ncplane_create( std , &bopts );

	int active = 0; TabHit hits[ 16 ]; int nhits = 0;
	draw_tabs( tabs , termw , active , hits , &nhits );
	draw_table( body , ALLTABS[ active ] , termw , termh - TAB_H );
	notcurses_render( nc );

	while ( 1 )
	{
		struct ncinput ni; uint32_t id = notcurses_get_nblock( nc , &ni );
		if ( id == 'q' || id == NCKEY_ESC ) break;
		if ( id == NCKEY_RESIZE )
		{
			ncplane_dim_yx( std , &termh , &termw );
			ncplane_resize_simple( tabs , TAB_H , termw );
			ncplane_move_yx( body , TAB_H , 0 );
			ncplane_resize_simple( body , termh - TAB_H , termw );
			draw_tabs( tabs , termw , active , hits , &nhits );
			draw_table( body , ALLTABS[ active ] , termw , termh - TAB_H );
			notcurses_render( nc );
			continue;
		}
		if ( ni.evtype == NCTYPE_PRESS && id == NCKEY_BUTTON1 )
		{
			if ( ni.y == 1 )
			{ // click row with tabs
				int x = ni.x;
				for ( int i = 0; i < nhits; i++ )
				{
					if ( x >= hits[ i ].x0 && x <= hits[ i ].x1 )
					{
						active = i;
						draw_tabs( tabs , termw , active , hits , &nhits );
						draw_table( body , ALLTABS[ active ] , termw , termh - TAB_H );
						notcurses_render( nc );
					}
				}
			}
		}
	}

	notcurses_stop( nc );
	return 0;
}

#endif

#ifndef v4

// services_dashboard.c
// Tabs with mouse-click focus + table with alternating row colors + vertical column lines
#include <notcurses/notcurses.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>

#define TAB_H 3
#define MAX_COLS 6
#define MAX_ROWS 23

typedef struct
{
	const char * name;
	int minw;
} Column;

typedef struct
{
	const char * tabname;
	int ncols;
	Column cols[ MAX_COLS ];
	int nrows;
	const char * cells[ MAX_ROWS ][ MAX_COLS ];   // table data
} Table;

// --- Demo tables -------------------------------------------------------------
static Table TAB_ACTIVE = {
	.tabname = "Active (20)",
	.ncols = 6,
	.cols = {
		{"Service name", 15},
		{"Status", 10},
		{"Type", 15},
		{"Runtime", 10},
		{"Region", 10},
		{"Last deployed", 12}
	},
	.nrows = 8,
	.cells = {
		{"simple-wor...", "Deployed", "Background Worker", "Node", "Oregon", "3 months ago"},
		{"my-cron-job", "Successful run", "Cron Job", "Node", "Oregon", "7 months ago"},
		{"mystite", "Available", "PostgreSQL", "PostgreSQL I5", "Oregon", "5 months ago"},
		{"example-ra...", "Available", "Redis", "Redis", "Oregon", "a year ago"},
		{"llama-front...", "Deployed", "Static Site", "Static", "Global", "8 months ago"},
		{"docusauru...", "Deployed", "Static Site", "Static", "Global", "a year ago"},
		{"mystite", "Deploying", "Web Service", "Python 3", "Oregon", "6 days ago"},
		{"prometheu...", "Deployed", "Web Service", "Docker", "Oregon", "2 months ago"}
	}
};

static Table TAB_SUSPENDED = {
	.tabname = "Suspended (3)",
	.ncols = 6,
	.cols = {
		{"Service name", 15},
		{"Status", 10},
		{"Type", 15},
		{"Runtime", 10},
		{"Region", 10},
		{"Last deployed", 12}
	},
	.nrows = 3,
	.cells = {
		{"old-service", "Suspended", "Web Service", "Python 2", "Oregon", "2 years ago"},
		{"test-app", "Suspended", "Web Service", "Node", "Oregon", "18 months ago"},
		{"legacy-db", "Suspended", "PostgreSQL", "PostgreSQL I3", "Oregon", "3 years ago"}
	}
};

static Table TAB_ALL = {
	.tabname = "All (23)",
	.ncols = 6,
	.cols = {
		{"Service name", 15},
		{"Status", 10},
		{"Type", 15},
		{"Runtime", 10},
		{"Region", 10},
		{"Last deployed", 12}
	},
	.nrows = 11,
	.cells = {
		{"simple-wor...", "Deployed", "Background Worker", "Node", "Oregon", "3 months ago"},
		{"my-cron-job", "Successful run", "Cron Job", "Node", "Oregon", "7 months ago"},
		{"mystite", "Available", "PostgreSQL", "PostgreSQL I5", "Oregon", "5 months ago"},
		{"example-ra...", "Available", "Redis", "Redis", "Oregon", "a year ago"},
		{"llama-front...", "Deployed", "Static Site", "Static", "Global", "8 months ago"},
		{"docusauru...", "Deployed", "Static Site", "Static", "Global", "a year ago"},
		{"mystite", "Deploying", "Web Service", "Python 3", "Oregon", "6 days ago"},
		{"prometheu...", "Deployed", "Web Service", "Docker", "Oregon", "2 months ago"},
		{"old-service", "Suspended", "Web Service", "Python 2", "Oregon", "2 years ago"},
		{"test-app", "Suspended", "Web Service", "Node", "Oregon", "18 months ago"},
		{"legacy-db", "Suspended", "PostgreSQL", "PostgreSQL I3", "Oregon", "3 years ago"}
	}
};

static Table * ALLTABS[] = { &TAB_ACTIVE, &TAB_SUSPENDED, &TAB_ALL };
static const int NTABS = sizeof( ALLTABS ) / sizeof( ALLTABS[ 0 ] );

// --- helpers -----------------------------------------------------------------
static int slen( const char * s )
{
	return s ? ( int )strlen( s ) : 0;
}

static void compute_widths( const Table * T , int availw , int * out )
{
	int desired[ MAX_COLS ] = { 0 };
	int i , r;
	for ( i = 0; i < T->ncols; i++ )
	{
		int w = slen( T->cols[ i ].name );
		if ( w < T->cols[ i ].minw ) w = T->cols[ i ].minw;
		for ( r = 0; r < T->nrows; r++ )
		{
			int cw = slen( T->cells[ r ][ i ] );
			if ( cw > w ) w = cw;
		}
		desired[ i ] = w;
	}
	int need = 0;
	for ( i = 0; i < T->ncols; i++ ) need += desired[ i ];
	need += T->ncols + 1; // vertical separators

	for ( i = 0; i < T->ncols; i++ ) out[ i ] = desired[ i ];
	if ( need > availw )
	{
		int over = need - availw;
		while ( over > 0 )
		{
			int maxidx = -1 , maxw = -1;
			for ( i = 0; i < T->ncols; i++ )
			{
				if ( out[ i ] > maxw && out[ i ] > T->cols[ i ].minw )
				{
					maxw = out[ i ]; maxidx = i;
				}
			}
			if ( maxidx < 0 ) break;
			out[ maxidx ]--; over--;
		}
	}
}

static void put_clipped( struct ncplane * n , int y , int x , const char * s , int w )
{
	if ( !s ) s = "";
	int L = slen( s );
	if ( L <= w )
	{
		ncplane_putstr_yx( n , y , x , s );
		for ( int i = L; i < w; i++ ) ncplane_putstr_yx( n , y , x + i , " " );
	}
	else
	{
		if ( w <= 3 )
		{
			for ( int i = 0; i < w; i++ ) ncplane_putstr_yx( n , y , x + i , "." );
		}
		else
		{
			for ( int i = 0; i < w - 3; i++ ) ncplane_putnstr_yx( n , y , x + i , 1 , s + i );
			ncplane_putstr_yx( n , y , x + w - 3 , "..." );
		}
	}
}

// Status color mapping
static uint64_t get_status_color( const char * status )
{
	if ( strcmp( status , "Deployed" ) == 0 ) return NCCHANNELS_INITIALIZER( 0x00 , 0xFF , 0x00 , 0x00 , 0x00 , 0x00 ); // Green
	if ( strcmp( status , "Available" ) == 0 ) return NCCHANNELS_INITIALIZER( 0x00 , 0xFF , 0x00 , 0x00 , 0x00 , 0x00 ); // Green
	if ( strcmp( status , "Successful run" ) == 0 ) return NCCHANNELS_INITIALIZER( 0x00 , 0xFF , 0x00 , 0x00 , 0x00 , 0x00 ); // Green
	if ( strcmp( status , "Deploying" ) == 0 ) return NCCHANNELS_INITIALIZER( 0xFF , 0xFF , 0x00 , 0x00 , 0x00 , 0x00 ); // Yellow
	if ( strcmp( status , "Suspended" ) == 0 ) return NCCHANNELS_INITIALIZER( 0xFF , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 ); // Red
	return NCCHANNELS_INITIALIZER( 0xFF , 0xFF , 0xFF , 0x00 , 0x00 , 0x00 ); // White (default)
}

// --- draw tabs ---------------------------------------------------------------
typedef struct
{
	int x0 , x1;
} TabHit;
static void draw_tabs( struct ncplane * tabs , int termw , int active , TabHit hits[] , int * nhits )
{
	*nhits = 0; ncplane_erase( tabs );

	// Draw top border
	for ( int i = 0; i < termw; i++ ) ncplane_putstr_yx( tabs , 0 , i , "─" );

	int x = 1;
	for ( int i = 0; i < NTABS; i++ )
	{
		const char * label = ALLTABS[ i ]->tabname;
		int w = slen( label ) + 4; // Add padding

		if ( i == active )
		{
			ncplane_set_styles( tabs , NCSTYLE_BOLD );
			ncplane_set_bg_rgb8( tabs , 60 , 60 , 80 ); // Dark blue background for active tab
			ncplane_set_fg_rgb8( tabs , 255 , 255 , 255 ); // White text
		}
		else
		{
			ncplane_set_bg_default( tabs );
			ncplane_set_fg_rgb8( tabs , 200 , 200 , 200 ); // Light gray text
		}

		ncplane_putstr_yx( tabs , 1 , x , " " );
		ncplane_putstr( tabs , label );
		ncplane_putstr( tabs , " " );

		// Reset styles
		ncplane_set_styles( tabs , NCSTYLE_NONE );
		ncplane_set_bg_default( tabs );
		ncplane_set_fg_default( tabs );

		hits[ *nhits ].x0 = x; hits[ *nhits ].x1 = x + w - 1; ( *nhits )++;
		x += w;
	}

	// Draw bottom border of tab area
	for ( int i = 0; i < termw; i++ ) ncplane_putstr_yx( tabs , 2 , i , "─" );
}

// --- draw table --------------------------------------------------------------
static void draw_table( struct ncplane * body , const Table * T , int termw , int termh )
{
	ncplane_erase( body );
	int colw[ MAX_COLS ]; compute_widths( T , termw - 2 , colw );
	int x = 0 , y = 0;

	// Header row with dark background
	ncplane_set_bg_rgb8( body , 40 , 40 , 60 ); // Dark blue background
	ncplane_set_fg_rgb8( body , 255 , 255 , 255 ); // White text
	ncplane_set_styles( body , NCSTYLE_BOLD );

	ncplane_putstr_yx( body , y , 0 , " " );
	x = 1;
	for ( int c = 0; c < T->ncols; c++ )
	{
		put_clipped( body , y , x , T->cols[ c ].name , colw[ c ] );
		x += colw[ c ];
		ncplane_putstr( body , " " );
		x += 1;
	}

	// Reset styles
	ncplane_set_styles( body , NCSTYLE_NONE );
	ncplane_set_bg_default( body );
	ncplane_set_fg_default( body );
	y++;

	// Rows with alternating background
	for ( int r = 0; r < T->nrows; r++ )
	{
		x = 0;

		// Alternate row colors
		if ( r % 2 == 0 )
		{
			ncplane_set_bg_rgb8( body , 30 , 30 , 40 ); // Dark blue-gray
		}
		else
		{
			ncplane_set_bg_rgb8( body , 50 , 50 , 70 ); // Slightly lighter blue-gray
		}

		ncplane_putstr_yx( body , y , 0 , " " );
		x = 1;

		for ( int c = 0; c < T->ncols; c++ )
		{
			// Special coloring for status column
			if ( c == 1 )
			{ // Status column
				uint64_t channels = get_status_color( T->cells[ r ][ c ] );
				ncplane_set_channels( body , channels );
			}
			else
			{
				ncplane_set_fg_rgb8( body , 220 , 220 , 220 ); // Light gray text
			}

			put_clipped( body , y , x , T->cells[ r ][ c ] , colw[ c ] );
			ncplane_set_fg_default( body );

			x += colw[ c ];
			ncplane_putstr( body , " " );
			x += 1;
		}

		ncplane_set_bg_default( body );
		y++;

		// Don't overflow the screen
		if ( y >= termh - TAB_H - 1 ) break;
	}
}

// --- main loop ---------------------------------------------------------------
int main( void )
{
	setlocale( LC_ALL , "" );
	struct notcurses_options opts = { 0 };
	opts.flags = NCOPTION_SUPPRESS_BANNERS;
	struct notcurses * nc = notcurses_core_init( &opts , NULL );
	if ( !nc ) return 1;

	notcurses_mice_enable( nc , NCMICE_BUTTON_EVENT );
	struct ncplane * std = notcurses_stdplane( nc );
	int termh , termw; ncplane_dim_yx( std , &termh , &termw );

	struct ncplane_options topts = { .y = 0,.x = 0,.rows = TAB_H,.cols = termw };
	struct ncplane * tabs = ncplane_create( std , &topts );
	struct ncplane_options bopts = { .y = TAB_H,.x = 0,.rows = termh - TAB_H,.cols = termw };
	struct ncplane * body = ncplane_create( std , &bopts );

	int active = 0; TabHit hits[ 16 ]; int nhits = 0;
	draw_tabs( tabs , termw , active , hits , &nhits );
	draw_table( body , ALLTABS[ active ] , termw , termh - TAB_H );
	notcurses_render( nc );

	while ( 1 )
	{
		struct ncinput ni; uint32_t id = notcurses_get_nblock( nc , &ni );
		if ( id == 'q' || id == NCKEY_ESC ) break;
		if ( id == NCKEY_RESIZE )
		{
			ncplane_dim_yx( std , &termh , &termw );
			ncplane_resize_simple( tabs , TAB_H , termw );
			ncplane_move_yx( body , TAB_H , 0 );
			ncplane_resize_simple( body , termh - TAB_H , termw );
			draw_tabs( tabs , termw , active , hits , &nhits );
			draw_table( body , ALLTABS[ active ] , termw , termh - TAB_H );
			notcurses_render( nc );
			continue;
		}
		if ( ni.evtype == NCTYPE_PRESS && id == NCKEY_BUTTON1 )
		{
			if ( ni.y == 1 )
			{ // click row with tabs
				int x = ni.x;
				for ( int i = 0; i < nhits; i++ )
				{
					if ( x >= hits[ i ].x0 && x <= hits[ i ].x1 )
					{
						active = i;
						draw_tabs( tabs , termw , active , hits , &nhits );
						draw_table( body , ALLTABS[ active ] , termw , termh - TAB_H );
						notcurses_render( nc );
					}
				}
			}
		}
	}

	notcurses_stop( nc );
	return 0;
}

#endif

#ifndef v3

// notcurses_ui.c

#include <locale.h>
#include <string.h>
#include <stdlib.h>
#include <notcurses/notcurses.h>
#include <unistd.h>

typedef struct
{
	const char * name;
	const char * value1;
	const char * value2;
} Row;

static Row demo_rows[] = {
  {"Boror lloochmanta","Senana","Bqude"},
  {"Seepreehiteriencka","Senarie","Squde"},
  {"Eolian rtotabgaktoir","Senaria","Squde"},
  {"Evam inruiliuna","Senania","Bqude"},
  {"Burnoqentmyanune","Senania","Bqude"},
  {"Penteón ileserotanta","Senania","Squde"},
};
static int NROWS = sizeof( demo_rows ) / sizeof( demo_rows[ 0 ] );

// draw header row (orange)
static void draw_header( struct ncplane * n , int y , int termw )
{
	uint64_t chan;
	ncchannels_set_bg_rgb8( &chan , 255 , 140 , 0 ); // orange
	ncchannels_set_fg_rgb8( &chan , 255 , 255 , 255 ); // white
	ncplane_set_base( n , " " , 0 , chan );

	ncplane_set_styles( n , NCSTYLE_BOLD );
	ncplane_putstr_yx( n , y , 2 , "Aveup" );
	ncplane_putstr_yx( n , y , 25 , "Row Traaky" );
	ncplane_putstr_yx( n , y , 45 , "Somha" );
	ncplane_set_styles( n , NCSTYLE_NONE );
	ncplane_set_base( n , " " , 0 , 0 );
}

// draw table rows with alternating colors
static void draw_rows( struct ncplane * n , int y , int termw )
{
	for ( int i = 0; i < NROWS; i++ )
	{
		uint64_t chan;
		if ( i % 2 == 0 )
		{
			ncchannels_set_bg_rgb8( &chan , 25 , 35 , 80 ); // dark blue
		}
		else
		{
			ncchannels_set_bg_rgb8( &chan , 40 , 55 , 110 ); // lighter blue
		}
		ncchannels_set_fg_rgb8( &chan , 255 , 255 , 255 ); // white
		ncplane_set_base( n , " " , 0 , chan );

		ncplane_putstr_yx( n , y + i , 2 , demo_rows[ i ].name );
		ncplane_putstr_yx( n , y + i , 25 , demo_rows[ i ].value1 );
		ncplane_putstr_yx( n , y + i , 45 , demo_rows[ i ].value2 );
	}
	ncplane_set_base( n , " " , 0 , 0 );
}

// draw text input + button
static void draw_input( struct ncplane * n , int y )
{
	ncplane_putstr_yx( n , y , 2 , "Search:" );
	uint64_t boxchan;
	ncchannels_set_fg_rgb8( &boxchan , 0 , 0 , 0 );
	ncchannels_set_bg_rgb8( &boxchan , 255 , 255 , 255 );
	ncplane_set_base( n , " " , 0 , boxchan );
	ncplane_putstr_yx( n , y , 12 , "                " ); // text box placeholder
	ncplane_set_base( n , " " , 0 , 0 );

	uint64_t btnchan;
	ncchannels_set_fg_rgb8( &btnchan , 255 , 255 , 255 );
	ncchannels_set_bg_rgb8( &btnchan , 0 , 122 , 255 );
	ncplane_set_base( n , " " , 0 , btnchan );
	ncplane_putstr_yx( n , y , 30 , " [Search] " );
	ncplane_set_base( n , " " , 0 , 0 );
}

int main()
{
	setlocale( LC_ALL , "" );
	struct notcurses_options opts = { 0 };
	opts.flags = NCOPTION_SUPPRESS_BANNERS;
	struct notcurses * nc = notcurses_core_init( &opts , NULL );
	if ( !nc ) return 1;

	struct ncplane * std = notcurses_stdplane( nc );
	int termh , termw; ncplane_dim_yx( std , &termh , &termw );

	// draw background
	uint64_t bg;
	ncchannels_set_bg_rgb8( &bg , 10 , 20 , 50 );
	ncchannels_set_fg_rgb8( &bg , 255 , 255 , 255 );
	ncplane_set_base( std , " " , 0 , bg );

	// draw input bar
	draw_input( std , 2 );

	// draw header
	draw_header( std , 4 , termw );

	// draw rows
	draw_rows( std , 5 , termw );

	notcurses_render( nc );

	//notcurses_getc_blocking( nc , NULL ); // wait key

	sleep(1000);
	notcurses_stop( nc );
	return 0;
}


#endif

#ifndef v2

// notcurses_tabs.c
// Tabs with mouse-click focus + table with alternating row colors + subheader + vertical column lines
#include <stdio.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <notcurses/notcurses.h>

#define TAB_H 3
#define MAX_COLS 8
#define MAX_ROWS 32

typedef struct
{
	const char * name;
	int minw;
} Column;

typedef struct
{
	const char * tabname;
	int ncols;
	Column cols[ MAX_COLS ];
	int nrows;
	const char * subhdrs[ MAX_COLS ];           // subheader row
	const char * cells[ MAX_ROWS ][ MAX_COLS ];   // table data
} Table;

// --- Demo tables -------------------------------------------------------------
static Table TAB_CPU = {
  .tabname = "CPU",
  .ncols = 4,
  .cols = { {"Core",4}, {"Usage%",7}, {"Freq",6}, {"Gov",3} },
  .subhdrs = {"ID","Load","MHz","Policy"},
  .nrows = 4,
  .cells = {
	{"0","12.3","3600","schedutil"},
	{"1","5.1","3580","ondemand"},
	{"2","22.9","4025","performance"},
	{"avg","13.4","3735","mixed"},
  }
};

static Table TAB_MEM = {
  .tabname = "Memory",
  .ncols = 3,
  .cols = { {"Type",4}, {"Used",5}, {"Total",5} },
  .subhdrs = {"Kind","Now","Capacity"},
  .nrows = 3,
  .cells = {
	{"RAM","3.2G","7.7G"},
	{"Swap","0.0G","2.0G"},
	{"Cache","1.1G","—"},
  }
};

static Table * ALLTABS[] = { &TAB_CPU, &TAB_MEM };
static const int NTABS = sizeof( ALLTABS ) / sizeof( ALLTABS[ 0 ] );

// --- helpers -----------------------------------------------------------------
static int slen( const char * s )
{
	return s ? ( int )strlen( s ) : 0;
}

static void compute_widths( const Table * T , int availw , int * out )
{
	int desired[ MAX_COLS ] = { 0 };
	int i , r;
	for ( i = 0; i < T->ncols; i++ )
	{
		int w = slen( T->cols[ i ].name );
		if ( w < T->cols[ i ].minw ) w = T->cols[ i ].minw;
		int sw = slen( T->subhdrs[ i ] );
		if ( sw > w ) w = sw;
		for ( r = 0; r < T->nrows; r++ )
		{
			int cw = slen( T->cells[ r ][ i ] );
			if ( cw > w ) w = cw;
		}
		desired[ i ] = w;
	}
	int need = 0;
	for ( i = 0; i < T->ncols; i++ ) need += desired[ i ];
	need += T->ncols + 1; // vertical separators

	for ( i = 0; i < T->ncols; i++ ) out[ i ] = desired[ i ];
	if ( need > availw )
	{
		int over = need - availw;
		while ( over > 0 )
		{
			int maxidx = -1 , maxw = -1;
			for ( i = 0; i < T->ncols; i++ )
			{
				if ( out[ i ] > maxw && out[ i ] > T->cols[ i ].minw )
				{
					maxw = out[ i ]; maxidx = i;
				}
			}
			if ( maxidx < 0 ) break;
			out[ maxidx ]--; over--;
		}
	}
}

static void put_clipped( struct ncplane * n , int y , int x , const char * s , int w )
{
	if ( !s ) s = "";
	int L = slen( s );
	if ( L <= w )
	{
		ncplane_putstr_yx( n , y , x , s );
		for ( int i = L; i < w; i++ ) ncplane_putstr_yx( n , y , x + i , " " );
	}
	else
	{
		if ( w <= 3 )
		{
			for ( int i = 0; i < w; i++ ) ncplane_putstr_yx( n , y , x + i , "." );
		}
		else
		{
			for ( int i = 0; i < w - 3; i++ ) ncplane_putnstr_yx( n , y , x + i , 1 , s + i );
			ncplane_putstr_yx( n , y , x + w - 3 , "..." );
		}
	}
}

// --- draw tabs ---------------------------------------------------------------
typedef struct
{
	int x0 , x1;
} TabHit;
static void draw_tabs( struct ncplane * tabs , int termw , int active , TabHit hits[] , int * nhits )
{
	*nhits = 0; ncplane_erase( tabs );
	int x = 1;
	for ( int i = 0; i < NTABS; i++ )
	{
		const char * label = ALLTABS[ i ]->tabname;
		int w = slen( label ) + 2;
		if ( x + w + 1 >= termw ) break;
		if ( i == active ) ncplane_set_styles( tabs , NCSTYLE_BOLD | NCSTYLE_UNDERLINE );
		ncplane_putstr_yx( tabs , 1 , x , " " );
		ncplane_putstr( tabs , label );
		ncplane_putstr( tabs , " " );
		ncplane_set_styles( tabs , NCSTYLE_NONE );
		hits[ *nhits ].x0 = x; hits[ *nhits ].x1 = x + w - 1; ( *nhits )++;
		x += w + 1;
	}
	for ( int i = 0; i < termw; i++ ) ncplane_putstr_yx( tabs , 2 , i , "─" );
}

// --- draw table --------------------------------------------------------------
static void draw_table( struct ncplane * body , const Table * T , int termw , int termh )
{
	ncplane_erase( body );
	int colw[ MAX_COLS ]; compute_widths( T , termw - 2 , colw );
	int x = 0 , y = 0;

	// top border
	ncplane_putstr_yx( body , y++ , 0 , "┌" );
	for ( int c = 0; c < T->ncols; c++ )
	{
		for ( int k = 0; k < colw[ c ]; k++ ) ncplane_putstr( body , "─" );
		if ( c == T->ncols - 1 ) ncplane_putstr( body , "┐" );
		else ncplane_putstr( body , "┬" );
	}

	// header
	ncplane_putstr_yx( body , y , 0 , "│" );
	ncplane_set_styles( body , NCSTYLE_BOLD );
	for ( int c = 0; c < T->ncols; c++ )
	{
		put_clipped( body , y , 1 + x , T->cols[ c ].name , colw[ c ] );
		x += colw[ c ];
		if ( c == T->ncols - 1 ) ncplane_putstr_yx( body , y , 1 + x , "│" );
		else ncplane_putstr_yx( body , y , 1 + x , "│" );
		x += 1;
	}
	ncplane_set_styles( body , NCSTYLE_NONE );
	y++;

	// subheader
	x = 0;
	ncplane_putstr_yx( body , y , 0 , "│" );
	ncplane_set_styles( body , NCSTYLE_UNDERLINE );
	for ( int c = 0; c < T->ncols; c++ )
	{
		put_clipped( body , y , 1 + x , T->subhdrs[ c ] , colw[ c ] );
		x += colw[ c ];
		if ( c == T->ncols - 1 ) ncplane_putstr_yx( body , y , 1 + x , "│" );
		else ncplane_putstr_yx( body , y , 1 + x , "│" );
		x += 1;
	}
	ncplane_set_styles( body , NCSTYLE_NONE );
	y++;

	// separator line under subheader
	ncplane_putstr_yx( body , y++ , 0 , "├" );
	for ( int c = 0; c < T->ncols; c++ )
	{
		for ( int k = 0; k < colw[ c ]; k++ ) ncplane_putstr( body , "─" );
		if ( c == T->ncols - 1 ) ncplane_putstr( body , "┤" );
		else ncplane_putstr( body , "┼" );
	}

	// rows with alternating bg
	for ( int r = 0; r < T->nrows; r++ )
	{
		x = 0;
		if ( r % 2 == 0 ) ncplane_set_bg_rgb8( body , 30 , 30 , 30 ); // dark gray
		else ncplane_set_bg_rgb8( body , 50 , 50 , 70 );       // bluish
		ncplane_putstr_yx( body , y , 0 , "│" );
		for ( int c = 0; c < T->ncols; c++ )
		{
			put_clipped( body , y , 1 + x , T->cells[ r ][ c ] , colw[ c ] );
			x += colw[ c ];
			if ( c == T->ncols - 1 ) ncplane_putstr_yx( body , y , 1 + x , "│" );
			else ncplane_putstr_yx( body , y , 1 + x , "│" );
			x += 1;
		}
		ncplane_set_bg_default( body );
		y++;
	}

	// bottom border
	ncplane_putstr_yx( body , y++ , 0 , "└" );
	for ( int c = 0; c < T->ncols; c++ )
	{
		for ( int k = 0; k < colw[ c ]; k++ ) ncplane_putstr( body , "─" );
		if ( c == T->ncols - 1 ) ncplane_putstr( body , "┘" );
		else ncplane_putstr( body , "┴" );
	}
}

// --- main loop ---------------------------------------------------------------
int main( void )
{
	setlocale( LC_ALL , "" );
	struct notcurses_options opts = { 0 };
	opts.flags = NCOPTION_SUPPRESS_BANNERS;
	struct notcurses * nc = notcurses_core_init( &opts , NULL );
	if ( !nc ) return 1;

	notcurses_mice_enable( nc , NCMICE_BUTTON_EVENT );
	struct ncplane * std = notcurses_stdplane( nc );
	int termh , termw; ncplane_dim_yx( std , &termh , &termw );

	struct ncplane_options topts = { .y = 0,.x = 0,.rows = TAB_H,.cols = termw };
	struct ncplane * tabs = ncplane_create( std , &topts );
	struct ncplane_options bopts = { .y = TAB_H,.x = 0,.rows = termh - TAB_H,.cols = termw };
	struct ncplane * body = ncplane_create( std , &bopts );

	int active = 0; TabHit hits[ 16 ]; int nhits = 0;
	draw_tabs( tabs , termw , active , hits , &nhits );
	draw_table( body , ALLTABS[ active ] , termw , termh );
	notcurses_render( nc );

	while ( 1 )
	{
		struct ncinput ni; uint32_t id = notcurses_get_nblock( nc , &ni );
		if ( id == 'q' || id == NCKEY_ESC ) break;
		if ( id == NCKEY_RESIZE )
		{
			ncplane_dim_yx( std , &termh , &termw );
			ncplane_resize_simple( tabs , TAB_H , termw );
			ncplane_move_yx( body , TAB_H , 0 );
			ncplane_resize_simple( body , termh - TAB_H , termw );
			draw_tabs( tabs , termw , active , hits , &nhits );
			draw_table( body , ALLTABS[ active ] , termw , termh );
			notcurses_render( nc );
			continue;
		}
		if ( ni.evtype == NCTYPE_PRESS && id == NCKEY_BUTTON1 )
		{
			if ( ni.y == 1 )
			{ // click row with tabs
				int x = ni.x;
				for ( int i = 0; i < nhits; i++ )
				{
					if ( x >= hits[ i ].x0 && x <= hits[ i ].x1 )
					{
						active = i;
						draw_tabs( tabs , termw , active , hits , &nhits );
						draw_table( body , ALLTABS[ active ] , termw , termh );
						notcurses_render( nc );
					}
				}
			}
		}
	}

	notcurses_stop( nc );
	return 0;
}


#endif

#ifndef v1

#include <stdio.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <notcurses/notcurses.h>

#define MAX_COLS 8
#define MAX_ROWS 32
#define TAB_H 3

typedef struct
{
	const char * name;
	int minw; // minimum width for this column
} Column;

typedef struct
{
	const char * tabname;
	int ncols;
	Column cols[ MAX_COLS ];
	int nrows;
	const char * cells[ MAX_ROWS ][ MAX_COLS ]; // row-major cells[r][c]
} Table;

// --- Demo data for three tabs ------------------------------------------------
static Table TAB_CPU = {
  .tabname = "CPU",
  .ncols = 4,
  .cols = { {"Core",4}, {"Usage%",7}, {"Freq(MHz)",9}, {"Gov",3} },
  .nrows = 6,
  .cells = {
	{"0","11.2","3600","schedutil"},
	{"1","3.1","3600","schedutil"},
	{"2","22.9","4025","performance"},
	{"3","7.0","4000","ondemand"},
	{"avg","11.0","3906","mixed"},
	{"temp","59.5","—","—"},
  }
};

static Table TAB_MEM = {
  .tabname = "Memory",
  .ncols = 4,
  .cols = { {"Type",4}, {"Used",4}, {"Total",5}, {"Notes",5} },
  .nrows = 5,
  .cells = {
	{"RAM","3.2G","7.7G","ok"},
	{"Swap","0.0G","2.0G","disabled rarely used"},
	{"Page Cache","1.1G","—","active"},
	{"Buffers","128M","—","stable"},
	{"HugePages","0","—","not configured"},
  }
};

static Table TAB_NET = {
  .tabname = "Net",
  .ncols = 5,
  .cols = { {"IF",2}, {"RX pkts",7}, {"TX pkts",7}, {"RX/s",4}, {"TX/s",4} },
  .nrows = 6,
  .cells = {
	{"lo","1023","1023","0","0"},
	{"enp0s3","19023","11231","12","8"},
	{"wlan0","0","0","0","0"},
	{"tun0","230","229","1","1"},
	{"avg","—","—","4","3"},
	{"drops","0","2","—","—"},
  }
};

static Table * ALLTABS[] = { &TAB_CPU, &TAB_MEM, &TAB_NET };
static const int NTABS = sizeof( ALLTABS ) / sizeof( ALLTABS[ 0 ] );

// --- Utility: max string visible length (UTF-8 naive: byte len) -------------
static int slen( const char * s )
{
	return ( int )( s ? ( int )strlen( s ) : 0 );
}

// --- Compute column widths given window width --------------------------------
static void compute_widths( const Table * T , int availw , int * out , int padx )
{
	// padx: spaces between columns
	int desired[ MAX_COLS ] = { 0 };
	int i , r;
	for ( i = 0; i < T->ncols; i++ )
	{
		int w = slen( T->cols[ i ].name );
		if ( w < T->cols[ i ].minw ) w = T->cols[ i ].minw;
		for ( r = 0; r < T->nrows; r++ )
		{
			int cw = slen( T->cells[ r ][ i ] );
			if ( cw > w ) w = cw;
		}
		desired[ i ] = w;
	}
	// space needed including separators
	int need = 0;
	for ( i = 0; i < T->ncols; i++ ) need += desired[ i ];
	need += ( T->ncols - 1 ) * padx;

	// cap to available: proportional shrink but never below minw
	for ( i = 0; i < T->ncols; i++ ) out[ i ] = desired[ i ];

	if ( need > availw )
	{
		// compute shrink budget
		int over = need - availw;
		// simple pass: repeatedly reduce widest columns until fits
		while ( over > 0 )
		{
			int maxidx = -1 , maxw = -1;
			for ( i = 0; i < T->ncols; i++ )
			{
				int mn = T->cols[ i ].minw;
				if ( out[ i ] > maxw && out[ i ] > mn )
				{
					maxw = out[ i ]; maxidx = i;
				}
			}
			if ( maxidx < 0 ) break; // cannot shrink more
			out[ maxidx ]--;
			over--;
		}
	}
}

// --- Draw a string clipped to width, with "..." if truncated -----------------
static void put_clipped( struct ncplane * n , int y , int x , const char * s , int w )
{
	if ( w <= 0 ) return;
	int L = slen( s );
	if ( L <= w )
	{
		ncplane_putstr_yx( n , y , x , s ? s : "" );
		// pad to width with spaces
		for ( int i = L; i < w; i++ ) ncplane_putstr_yx( n , y , x + i , " " );
		return;
	}
	if ( w <= 3 )
	{
		// fill with dots
		for ( int i = 0; i < w; i++ ) ncplane_putstr_yx( n , y , x + i , "." );
		return;
	}
	// print first w-3 chars then "..."
	for ( int i = 0; i < w - 3; i++ )
	{
		char c = s[ i ];
		ncplane_putstr_yx( n , y , x + i , ( char[] )
		{
			c , 0
		} );
	}
	ncplane_putstr_yx( n , y , x + w - 3 , "..." );
}

// --- Render the tab strip ----------------------------------------------------
typedef struct
{
	int x0 , x1; // inclusive range on X for click hit-test
} TabHit;

static void draw_tabs( struct ncplane * tabs , int termw , int active , TabHit hits[] , int * nhits )
{
	*nhits = 0;
	ncplane_erase( tabs );

	int x = 1;
	for ( int i = 0; i < NTABS; i++ )
	{
		const char * label = ALLTABS[ i ]->tabname;
		int w = slen( label ) + 2; // padding
		if ( x + w + 1 >= termw ) break; // stop if no room
		// outline
		if ( i == active ) ncplane_set_styles( tabs , NCSTYLE_BOLD | NCSTYLE_UNDERLINE );
		ncplane_putstr_yx( tabs , 1 , x , " " );
		ncplane_putstr_yx( tabs , 1 , x + 1 , label );
		ncplane_putstr_yx( tabs , 1 , x + w - 1 , " " );
		ncplane_set_styles( tabs , NCSTYLE_NONE );

		// save hit range
		hits[ *nhits ].x0 = x;
		hits[ *nhits ].x1 = x + w - 1;
		( *nhits )++;

		x += w + 1;
	}

	// underline
	for ( int i = 0; i < termw; i++ ) ncplane_putstr_yx( tabs , 2 , i , "─" );
}

// --- Draw the table for the active tab --------------------------------------
static void draw_table( struct ncplane * body , const Table * T , int termw , int termh )
{
	ncplane_erase( body );
	int pady = 1 , padx = 2;
	int innerw = termw - 2;
	int innerh = termh - ( TAB_H + 1 );
	if ( innerw < 10 || innerh < 2 )
	{
		notcurses_render( ncplane_notcurses( body ) ); return;
	}

	int colw[ MAX_COLS ] = { 0 };
	compute_widths( T , innerw , colw , padx );

	// Header
	int y = 0;
	int x = 1;
	ncplane_set_styles( body , NCSTYLE_BOLD | NCSTYLE_UNDERLINE );
	for ( int c = 0; c < T->ncols; c++ )
	{
		put_clipped( body , y , x , T->cols[ c ].name , colw[ c ] );
		x += colw[ c ];
		if ( c != T->ncols - 1 )
		{
			for ( int k = 0; k < padx; k++ ) ncplane_putstr_yx( body , y , x + k , " " ); x += padx;
		}
	}
	ncplane_set_styles( body , NCSTYLE_NONE );
	y += 1 + pady;

	// Rows
	for ( int r = 0; r < T->nrows && y < innerh; r++ )
	{
		x = 1;
		for ( int c = 0; c < T->ncols; c++ )
		{
			put_clipped( body , y , x , T->cells[ r ][ c ] , colw[ c ] );
			x += colw[ c ];
			if ( c != T->ncols - 1 )
			{
				for ( int k = 0; k < padx; k++ ) ncplane_putstr_yx( body , y , x + k , " " ); x += padx;
			}
		}
		y++;
	}

	// simple border
	// (Let’s just leave margins. Notcurses boxes look nice but are optional here.)
	( void )pady;
}

// --- Main --------------------------------------------------------------------
int main( void )
{
	setlocale( LC_ALL , "" );

	struct notcurses_options opts = { 0 };
	opts.flags = NCOPTION_SUPPRESS_BANNERS; // cleaner
	struct notcurses * nc = notcurses_core_init( &opts , NULL );
	if ( !nc ) return 1;

	notcurses_mice_enable( nc , NCMICE_BUTTON_EVENT ); // listen for clicks

	struct ncplane * std = notcurses_stdplane( nc );
	int termh , termw;
	ncplane_dim_yx( std , &termh , &termw );

	// Tabs plane
	struct ncplane_options topts = {
	  .y = 0, .x = 0, .rows = TAB_H, .cols = termw,
	  .userptr = NULL, .name = "tabs"
	};
	struct ncplane * tabs = ncplane_create( std , &topts );

	// Body plane
	struct ncplane_options bopts = {
	  .y = TAB_H, .x = 0, .rows = termh - TAB_H, .cols = termw,
	  .userptr = NULL, .name = "body"
	};
	struct ncplane * body = ncplane_create( std , &bopts );

	int active = 0;
	TabHit hits[ 16 ]; int nhits = 0;

	// Initial draw
	draw_tabs( tabs , termw , active , hits , &nhits );
	draw_table( body , ALLTABS[ active ] , termw , termh );
	notcurses_render( nc );

	// Event loop
	while ( 1 )
	{
		struct ncinput ni;
		uint32_t id = notcurses_get_nblock( nc , &ni );

		// Window resize?
		if ( id == NCKEY_RESIZE )
		{
			ncplane_dim_yx( std , &termh , &termw );
			// resize planes
			ncplane_resize_simple( tabs , TAB_H , termw );
			ncplane_move_yx( body , TAB_H , 0 );
			ncplane_resize_simple( body , termh - TAB_H , termw );
			draw_tabs( tabs , termw , active , hits , &nhits );
			draw_table( body , ALLTABS[ active ] , termw , termh );
			notcurses_render( nc );
			continue;
		}

		if ( id == NCKEY_EOF ) break;

		// Quit?
		if ( id == 'q' || id == NCKEY_ESC ) break;

		// Mouse click on tabs
		if ( ni.evtype == NCTYPE_PRESS && ( id == NCKEY_BUTTON1 /* || id == NCKEY_BUTTON1_PRESSED*/ ) )
		{
			// If click within tabs plane row 1
			if ( ni.y >= 1 && ni.y <= 1 )
			{
				int x = ni.x;
				for ( int i = 0; i < nhits; i++ )
				{
					if ( x >= hits[ i ].x0 && x <= hits[ i ].x1 )
					{
						if ( active != i )
						{
							active = i;
							draw_tabs( tabs , termw , active , hits , &nhits );
							draw_table( body , ALLTABS[ active ] , termw , termh );
							notcurses_render( nc );
						}
						break;
					}
				}
			}
			continue;
		}

		// Any other key: ignore (keeps UI responsive for mouse/resizes)
	}

	notcurses_stop( nc );
	return 0;
}

#endif
