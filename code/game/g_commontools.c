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

void LM_CPHandler( gentity_t *ent, char *message )
{
	mvclientSession_t *mvSess = &mv_clientSessions[ent - g_entities];
	memset(mvSess->player.common.centerPrintMessage, 0, sizeof(mvSess->player.common.centerPrintMessage));

	// Set timers.
	mvSess->player.common.centerPrintTimer[0] = level.time + 1000;
	mvSess->player.common.centerPrintTimer[1] = (lm_centerPrintTime.integer - 2);

	// Make sure the the timer is not less than 0.
	if (mvSess->player.common.centerPrintTimer[1] < 0)
		mvSess->player.common.centerPrintTimer[1] = 0;

	// Perform string escape and copy the message onto the player struct.
	LM_StringEscape(mvSess->player.common.centerPrintMessage, message, sizeof(mvSess->player.common.centerPrintMessage));
	trap_SendServerCommand(ent - g_entities, va("cp \"%s\"", mvSess->player.common.centerPrintMessage));
}

// Get group.
char *LM_GetValueGroup( char *buf, char *group, char *outbuf )
{
	char *place, *placesecond;
	int failed;
	int i;
	int startpoint, startletter;
	int subg = 0;

	i = 0;

	place = strstr(buf, group);

	if (!place)
	{
		return 0;
	}

	startpoint = place - buf + strlen(group) + 1;
	startletter = (place - buf) - 1;

	failed = 0;

	while (buf[startpoint + 1] != '{' || buf[startletter] != '\n')
	{
		placesecond = strstr(place + 1, group);

		if (placesecond)
		{
			startpoint += (placesecond - place);
			startletter += (placesecond - place);
			place = placesecond;
		}
		else
		{
			failed = 1;
			break;
		}
	}

	if (failed)
	{
		return 0;
	}

	//we have found the proper group name if we made it here, so find the opening brace and read into the outbuf
	//until hitting the end brace

	while (buf[startpoint] != '{')
	{
		startpoint++;
	}

	startpoint++;

	while (buf[startpoint] != '}' || subg)
	{
		if (buf[startpoint] == '{')
		{
			subg++;
		}
		else if (buf[startpoint] == '}')
		{
			subg--;
		}
		outbuf[i] = buf[startpoint];
		i++;
		startpoint++;
	}

	// FIXME: Prevent removal of brackets instead of re-adding them.
	outbuf[0] = '{';
	outbuf[i] = '}';
	i++;

	outbuf[i] = '\0';

	return outbuf;
}

// Parse group to get values.
char *LM_ParseInfo( char *buf, char *outbuf )
{
	char	*token;
	int		count;
	char	key[MAX_TOKEN_CHARS];
	char	info[MAX_INFO_STRING];

	while (1) {
		token = COM_Parse((const char **)(&buf));
		if (!token[0]) {
			break;
		}
		if (strcmp(token, "{")) {
			Com_Printf("Missing { in info file\n");
			break;
		}

		info[0] = '\0';
		while (1) {
			token = COM_ParseExt((const char **)(&buf), qtrue);
			if (!token[0]) {
				Com_Printf("Unexpected end of info file\n");
				break;
			}

			if (!strcmp(token, "}")) {
				break;
			}

			Q_strncpyz(key, token, sizeof(key));

			token = COM_ParseExt((const char **)(&buf), qfalse);
			if (!token[0])
				strcpy(token, "<NULL>");

			Info_SetValueForKey(info, key, token);
		}
	}

	return strcpy(outbuf, info);
}

qboolean LM_PlayerCollision( gentity_t *ent, gentity_t *other )
{
	if (ent - g_entities == other - g_entities)
		return qtrue;

	if (ent->client->ps.duelInProgress)
	{
		if (other - g_entities == ent->client->ps.duelIndex) 
			return qtrue;
		else
			return qfalse;
	}
	else if (other->client->ps.duelInProgress)
	{
		return qfalse;
	}
	else
	{
		return qtrue;
	}
}
