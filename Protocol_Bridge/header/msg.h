#ifdef M_MSG
	#undef M_MSG
#endif

#ifdef ENABLE_LOGGING
	#define M_MSG if(d_error!=errOK) { M_showMsg(__custom_message); \
	log_write( LOG_ERROR , "%s\n" , __custom_message ); \
	}
#else
	#define M_MSG if(d_error!=errOK) { M_showMsg(__custom_message); }
#endif

_STRONG_ATTR void M_showMsg( const char * msg );

void _Breaked();