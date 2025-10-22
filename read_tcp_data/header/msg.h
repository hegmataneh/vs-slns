#ifdef M_MSG
#undef M_MSG
#endif

#define M_MSG if(d_error!=errOK) { M_showMsg(__custom_message); }

void M_showMsg( const char * msg );

