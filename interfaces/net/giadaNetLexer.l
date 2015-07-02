%{

#include "../../engine/control.h"
#include "lexbuffer.h"

#define YY_INPUT(buf,result,max_size) result=lexbuffer.getline(buf,max_size)
#define YY_DECL int giadaNetLex (void)

extern int giadaNetLex(void)
extern FileBuffer lexbuffer;

%}
[ \t\n]
"stop"		return control::STOP_SEQ;
"start"		return control::START_SEQ;
"rewind"	return control::REWIND_SEQ;
"play"		return control::PLAY_ALL;
%%
