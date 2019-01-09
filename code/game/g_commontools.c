#include "g_local.h"

char *LM_SanitizeString(char *destination, char *source, int destinationSize)
{
	char	string[MAX_TOKEN_CHARS];
	char	clean[MAX_TOKEN_CHARS];
	int		i, j, length;

	memset(string, 0, sizeof(string));
	memset(clean, 0, sizeof(clean));

	Q_strncpyz(string, source, sizeof(string));

	length = strlen(string);

	j = 0;

	for (i = 0; i < length; i++)
	{
		if (string[i] != '^')
		{
			clean[j] = tolower(string[i]);
			j++;
		}
		else if (string[i] == '^')
		{
			i++;
		}
	}

	Q_strncpyz(destination, clean, destinationSize);
	return destination;
}

void LM_StringEscape(char *out, char *in, int outSize)
{
	char	ch, ch1;
	int		len = 0;
	
	outSize--;

	while (1)
	{
		ch  = *in++;
		ch1 = *in;

		if (ch == '\\' && ch1 == 'n')
		{
			in++;
			*out++ = '\n';
		}
		else
		{
			*out++ = ch;
		}

		if (len > outSize - 1)
			break;

		len++;
	}
}

void LM_CPHandler(gentity_t *ent, char *message)
{
	char outbuf[MAX_STRING_CHARS];
	mvclientSession_t *mvSess = &mv_clientSessions[ent - g_entities];

	// Only allow one centerprint at a time.
	if (level.time > mvSess->player.common.centerPrintTimer[0] && !mvSess->player.common.centerPrintTimer[1])
	{
		memset(mvSess->player.common.centerPrintMessage, 0, sizeof(mvSess->player.common.centerPrintMessage));
		mvSess->player.common.centerPrintTimer[0] = level.time + 1000;
		mvSess->player.common.centerPrintTimer[1] = (lm_centerPrintTime.integer - 2);

		if (mvSess->player.common.centerPrintTimer[1] < 0)
		{
			mvSess->player.common.centerPrintTimer[1] = 0;
		}

		Q_strncpyz(mvSess->player.common.centerPrintMessage, message, sizeof(mvSess->player.common.centerPrintMessage));
	}
	else if (mvSess->player.common.centerPrintTimer[0] < level.time && mvSess->player.common.centerPrintTimer[1])
	{
		mvSess->player.common.centerPrintTimer[0]  = level.time + 1000;
		mvSess->player.common.centerPrintTimer[1] -= 1;
	}

	if (strlen(mvSess->player.common.centerPrintMessage))
	{
		LM_StringEscape(outbuf, mvSess->player.common.centerPrintMessage, sizeof(outbuf));
		trap_SendServerCommand(ent - g_entities, va("cp \"%s\"", outbuf));
	}
}
