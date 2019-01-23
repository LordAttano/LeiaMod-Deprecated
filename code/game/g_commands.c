#include "g_local.h"

// Functions called by say commands.
void LM_BuildTime( gentity_t *ent )
{
	trap_SendServerCommand(ent - g_entities, va("print \"%s\n%sCompiled%s:%s %s %s\n\"", GAMEVERSION, LM_TEXT_COLOR, LM_SYMBOL_COLOR, LM_TEXT_COLOR, __DATE__, __TIME__));
	LM_CPHandler(ent, va("%s\n%sCompiled%s:%s %s %s\n", GAMEVERSION, LM_TEXT_COLOR, LM_SYMBOL_COLOR, LM_TEXT_COLOR, __DATE__, __TIME__));
}

void LM_Motd( gentity_t *ent )
{
	LM_CPHandler(ent, lm_motd.string);
}
