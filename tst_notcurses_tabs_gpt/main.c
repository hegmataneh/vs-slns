// notcurses_tabs.c
// Simple multi-tab stats UI with mouse tabs + auto-resizing columns.
//
// Keys: q or ESC to quit. Click a tab to switch.
// Requires: libnotcurses-dev

#define _XOPEN_SOURCE 700

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
