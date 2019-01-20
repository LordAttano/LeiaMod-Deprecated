#include "g_local.h"
#include "bg_public.h"

#define DUEL_MAX_LEN	131072
#define DUEL_VALUE_LEN	16384
#define DUEL_GROUP_LEN	65536

typedef struct lmForce_s
{
	char *name;
	int	 index;
} lmForce_t;

lmForce_t forceList[] =
{
	ENUM2STRING(FP_HEAL),
	ENUM2STRING(FP_LEVITATION),
	ENUM2STRING(FP_SPEED),
	ENUM2STRING(FP_PUSH),
	ENUM2STRING(FP_PULL),
	ENUM2STRING(FP_TELEPATHY),
	ENUM2STRING(FP_GRIP),
	ENUM2STRING(FP_LIGHTNING),
	ENUM2STRING(FP_RAGE),
	ENUM2STRING(FP_PROTECT),
	ENUM2STRING(FP_ABSORB),
	ENUM2STRING(FP_TEAM_HEAL),
	ENUM2STRING(FP_TEAM_FORCE),
	ENUM2STRING(FP_DRAIN),
	ENUM2STRING(FP_SEE),
	ENUM2STRING(FP_SABERATTACK),
	ENUM2STRING(FP_SABERDEFEND),
	ENUM2STRING(FP_SABERTHROW),
	ENUM2STRING(NUM_FORCE_POWERS)
};

// Initiate duel settings.
void LM_DuelInit( gentity_t *ent )
{
	int i;
	mvclientSession_t *mvSess = &mv_clientSessions[ent - g_entities];

	// Dual blade
	if (mvSess->player.duel.dualblade)
		ent->client->ps.dualBlade = qtrue;

	if (!mvSess->player.duel.toggle)
	{
		G_Sound(ent, CHAN_AUTO, G_SoundIndex("sound/weapons/saber/saberoffquick.wav"));
		ent->client->ps.saberHolstered = qtrue;
	}

	// Store original health. For duels with multiple rounds update original health at respawn.
	mvSess->player.duel.origHealth = ent->client->ps.stats[STAT_HEALTH];
	mvSess->player.duel.origShield = ent->client->ps.stats[STAT_ARMOR];

	if (mvSess->player.duel.engaged)
	{
		// Respawn.
		ent->client->ps.duelInProgress = qtrue;
		ent->client->ps.duelIndex	   = mvSess->player.duel.index;

		// Bacta at spawn.
		if (mvSess->player.duel.bacta == 2)
			ent->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << HI_MEDPAC);
	}
	else
	{
		// Initiation.
		mvSess->player.duel.engaged		   = qtrue;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = mvSess->player.duel.health;
		ent->client->ps.stats[STAT_ARMOR]  = mvSess->player.duel.shield;

		// Bacta at spawn.
		if (mvSess->player.duel.bacta)
			ent->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << HI_MEDPAC);
		else
			ent->client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_MEDPAC);
	}

	// Force power presets.
	if (mvSess->player.duel.force && mvSess->player.duel.forcepreset)
	{
		for (i = 0; i < NUM_FORCE_POWERS; i++)
		{
			if (mvSess->player.duel.knownpowers[i])
			{
				// We have a level set for this power.
				ent->client->ps.fd.forcePowerBaseLevel[i] = mvSess->player.duel.knownpowers[i];
				ent->client->ps.fd.forcePowerLevel[i]	  = mvSess->player.duel.knownpowers[i];
				ent->client->ps.fd.forcePowersKnown		 |= (1 << i);
			}
			else
			{
				// The power has no level set for it.
				ent->client->ps.fd.forcePowerBaseLevel[i] = 0;
				ent->client->ps.fd.forcePowerLevel[i]	  = 0;
				ent->client->ps.fd.forcePowersKnown		 &= ~(1 << i);
			}
		}
	}
}

// Clean duel settings from player struct.
void LM_DuelClean( gentity_t *ent )
{
	mvclientSession_t *mvSess;
	int				  i;

	if (ent && ent->client && ent->inuse)
	{
		mvSess = &mv_clientSessions[ent - g_entities];

		// Send end event to the client.
		G_AddEvent(ent, EV_PRIVATE_DUEL, 0);

		// Clear settings.
		Q_strncpyz(mvSess->player.duel.mode, "", sizeof(mvSess->player.duel.mode));
		Q_strncpyz(mvSess->player.duel.name, "", sizeof(mvSess->player.duel.name));

		mvSess->player.duel.toggle		   = 0;
		mvSess->player.duel.force		   = 0;
		mvSess->player.duel.forcepreset    = 0;
		mvSess->player.duel.health		   = 0;
		mvSess->player.duel.shield		   = 0;
		mvSess->player.duel.bacta		   = 0;
		mvSess->player.duel.dualblade	   = 0;
		mvSess->player.duel.pickups		   = 0;
		mvSess->player.duel.distance	   = 0;
		mvSess->player.duel.fraglimit	   = 0;
		mvSess->player.duel.timelimit	   = 0;
		mvSess->player.duel.bestof		   = 0;
		mvSess->player.duel.infforce	   = 0;
		mvSess->player.duel.forceregentime = 0;
		mvSess->player.duel.knockback	   = 0;
		mvSess->player.duel.kickdmg		   = 0;
		mvSess->player.duel.falldmg		   = 0;
		mvSess->player.duel.ffrules		   = 0;
		mvSess->player.duel.saberlock	   = 0;
		mvSess->player.duel.gravity		   = 0;
		mvSess->player.duel.speed		   = 0;

		mvSess->player.duel.trainingwheels = 0;

		ent->client->ps.dualBlade		   = qfalse;
		ent->client->ps.duelInProgress	   = qfalse;
		ent->client->ps.duelIndex		   = ENTITYNUM_NONE;

		ent->client->ps.gravity			   = g_gravity.value;
		ent->client->ps.speed			   = g_speed.value;
		ent->client->ps.basespeed		   = g_speed.value;

		mvSess->player.duel.index		   = ENTITYNUM_NONE;
		mvSess->player.duel.engaged		   = qfalse;

		mvSess->player.duel.counter		   = 0;

		mvSess->player.duel.starttime	   = 0;

		if (ent->health > 0 && ent->client->ps.stats[STAT_HEALTH] > 0)
		{
			ent->client->ps.stats[STAT_HEALTH] = ent->health = mvSess->player.duel.origHealth;
			ent->client->ps.stats[STAT_ARMOR]  = mvSess->player.duel.origShield;

			mvSess->player.duel.origHealth	   = 0;
			mvSess->player.duel.origShield	   = 0;

			if (g_spawnInvulnerability.integer)
			{
				ent->client->ps.eFlags |= EF_INVULNERABLE;
				ent->client->invulnerableTimer = level.time + g_spawnInvulnerability.integer;
			}
		}

		// Duel collision.
		if (mvapi)
		{
			for (i = 0; i < MAX_CLIENTS; i++)
			{
				mv_entities[i].snapshotIgnore[ent - g_entities] = 0;
			}
		}

		WP_InitForcePowers(ent);
	}
}

// List duels.
void LM_DuelList( gentity_t *ent )
{
	int		numdirs, i, dirlen, len;
	char	dirlist[2048], outbuf[MAX_STRING_CHARS], deli[2];
	char	*dirptr;

	numdirs	  = trap_FS_GetFileList("config/duels", ".txt", dirlist, sizeof(dirlist));

	if (!numdirs)
	{
		trap_SendServerCommand(ent - g_entities, va("print \"%sNo duel modes available%s.\n\"", LM_TEXT_COLOR, LM_SYMBOL_COLOR));
		return;
	}

	dirptr	  = dirlist;
	outbuf[0] = '\0';

	strcpy(deli, ",");

	for ( i = 0; i < numdirs; i++ )
	{
		dirlen = strlen(dirptr) + 1;
		len	   = strlen(dirptr);
		
		if (!Q_stricmp(dirptr + len - 4, ".txt"))
			dirptr[len - 4] = '\0';

		if (i == numdirs - 1)
			strcpy(deli, ".");

		Q_strcat(outbuf, sizeof(outbuf), va("%sengage%s_%s%s%s%s ", LM_TEXT_COLOR, LM_SYMBOL_COLOR, LM_TEXT_COLOR, dirptr, LM_SYMBOL_COLOR, deli));
		dirptr += dirlen;
	}

	trap_SendServerCommand(ent - g_entities, va("print \"%s\n\"", outbuf));
}

// Check duel existence.
int LM_DuelExists( char *mode )
{
	fileHandle_t	f;
	int				len;

	len = trap_FS_FOpenFile(va("config/duels/%s.txt", mode), &f, FS_READ);

	if (len < 0 || !f)
	{
		trap_FS_FCloseFile(f);
		return qfalse;
	}

	trap_FS_FCloseFile(f);

	return qtrue;
}

// Extract the name from the 
char *LM_GetDuel( char *mode )
{
	mode = strchr(mode, '_');
	return mode + 1;
}

// Read duel file and assign variables.
void LM_DuelRead( gentity_t *ent, char *mode )
{
	fileHandle_t	  f;
	int				  len, rlen;
	int				  i;
	mvclientSession_t *mvSess;
	char			  *buf = (char *)BG_TempAlloc(DUEL_MAX_LEN);
	char			  *readbuf, *group;

	mvSess = &mv_clientSessions[ent - g_entities];
	len	   = trap_FS_FOpenFile(va("config/duels/%s.txt", mode), &f, FS_READ);

	if (!f)
	{
		G_Printf("%s[%sError%s] %sSpecified duel not found%s.\n", LM_SYMBOL_COLOR, LM_ERROR_COLOR, LM_SYMBOL_COLOR, LM_TEXT_COLOR, LM_SYMBOL_COLOR);
		BG_TempFree(DUEL_MAX_LEN);
		return;
	}

	if (len >= DUEL_MAX_LEN)
	{
		G_Printf("%s[%sError%s] %sDuel file exceeds maximum length%s.\n", LM_SYMBOL_COLOR, LM_ERROR_COLOR, LM_SYMBOL_COLOR, LM_TEXT_COLOR, LM_SYMBOL_COLOR);
		BG_TempFree(DUEL_MAX_LEN);
		return;
	}

	trap_FS_Read(buf, len, f);

	rlen = len;

	while (len < DUEL_MAX_LEN)
	{
		buf[len] = '\0';
		len++;
	}

	len	    = rlen;

	readbuf = (char *)BG_TempAlloc(DUEL_VALUE_LEN);
	group	= (char *)BG_TempAlloc(DUEL_GROUP_LEN);

	if (strlen(LM_GetValueGroup(buf, "Settings", group)))
	{
		if (strlen(LM_ParseInfo(group, readbuf)))
		{
			Q_strncpyz(mvSess->player.duel.mode, mode, sizeof(mvSess->player.duel.mode));

			if (strlen(Info_ValueForKey(readbuf, "name")))
				Q_strncpyz(mvSess->player.duel.name, Info_ValueForKey(readbuf, "name"), sizeof(mvSess->player.duel.name));
			else
				Q_strncpyz(mvSess->player.duel.name, "Unknown Duel", sizeof(mvSess->player.duel.name));

			mvSess->player.duel.toggle		   = atoi(Info_ValueForKey(readbuf, "toggle"))		   ? atoi(Info_ValueForKey(readbuf, "toggle"))			  : 0;
			mvSess->player.duel.force		   = atoi(Info_ValueForKey(readbuf, "force"))		   ? atoi(Info_ValueForKey(readbuf, "force"))			  : 0;
			mvSess->player.duel.forcepreset    = atoi(Info_ValueForKey(readbuf, "forcepreset"))    ? atoi(Info_ValueForKey(readbuf, "forcepreset"))		  : 0;
			mvSess->player.duel.health		   = atoi(Info_ValueForKey(readbuf, "health"))		   ? atoi(Info_ValueForKey(readbuf, "health"))			  : 1;
			mvSess->player.duel.shield		   = atoi(Info_ValueForKey(readbuf, "shield"))		   ? atoi(Info_ValueForKey(readbuf, "shield"))			  : 0;
			mvSess->player.duel.bacta		   = atoi(Info_ValueForKey(readbuf, "bacta"))		   ? atoi(Info_ValueForKey(readbuf, "bacta"))			  : 0;
			mvSess->player.duel.pickups		   = atoi(Info_ValueForKey(readbuf, "pickups"))		   ? atoi(Info_ValueForKey(readbuf, "pickups"))			  : 0;
			mvSess->player.duel.distance	   = atoi(Info_ValueForKey(readbuf, "distance"))	   ? atoi(Info_ValueForKey(readbuf, "distance"))		  : 0;
			mvSess->player.duel.fraglimit	   = atoi(Info_ValueForKey(readbuf, "fraglimit"))	   ? atoi(Info_ValueForKey(readbuf, "fraglimit"))		  : 0;
			mvSess->player.duel.timelimit	   = atoi(Info_ValueForKey(readbuf, "timelimit"))	   ? atoi(Info_ValueForKey(readbuf, "timelimit"))		  : 0;
			mvSess->player.duel.bestof		   = atoi(Info_ValueForKey(readbuf, "bestof"))		   ? atoi(Info_ValueForKey(readbuf, "bestof"))			  : 0;
			mvSess->player.duel.infforce	   = atoi(Info_ValueForKey(readbuf, "infiniteforce"))  ? atoi(Info_ValueForKey(readbuf, "infiniteforce"))	  : 0;
			mvSess->player.duel.forceregentime = atoi(Info_ValueForKey(readbuf, "forceregentime")) ? atoi(Info_ValueForKey(readbuf, "forceregentime"))	  : 0;
			mvSess->player.duel.dualblade	   = atoi(Info_ValueForKey(readbuf, "dualblade"))	   ? atoi(Info_ValueForKey(readbuf, "dualblade"))		  : 0;
			mvSess->player.duel.knockback	   = atoi(Info_ValueForKey(readbuf, "knockback"))	   ? atoi(Info_ValueForKey(readbuf, "knockback"))	      : 0;
			mvSess->player.duel.kickdmg		   = atoi(Info_ValueForKey(readbuf, "kickdmg"))		   ? atoi(Info_ValueForKey(readbuf, "kickdmg"))			  : 0;
			mvSess->player.duel.falldmg		   = atoi(Info_ValueForKey(readbuf, "falldmg"))		   ? atoi(Info_ValueForKey(readbuf, "falldmg"))			  : 0;
			mvSess->player.duel.ffrules		   = atoi(Info_ValueForKey(readbuf, "ffrules"))		   ? atoi(Info_ValueForKey(readbuf, "ffrules"))			  : 0;
			mvSess->player.duel.saberlock	   = atoi(Info_ValueForKey(readbuf, "saberlock"))	   ? atoi(Info_ValueForKey(readbuf, "saberlock"))		  : 0;
			mvSess->player.duel.gravity		   = atoi(Info_ValueForKey(readbuf, "gravity"))		   ? atoi(Info_ValueForKey(readbuf, "gravity"))			  : 800;
			mvSess->player.duel.speed		   = atoi(Info_ValueForKey(readbuf, "speed"))		   ? atoi(Info_ValueForKey(readbuf, "speed"))			  : 250;
			mvSess->player.duel.trainingwheels = atoi(Info_ValueForKey(readbuf, "trainingwheels")) ? atoi(Info_ValueForKey(readbuf, "trainingwheels"))	  : 0;
		}
	}
	else
	{
		G_Printf("%s[%sError%s] %sDuel file %s'%s%s' %scontains no Settings group%s.\n", LM_SYMBOL_COLOR, LM_ERROR_COLOR, LM_SYMBOL_COLOR, LM_TEXT_COLOR, LM_SYMBOL_COLOR, LM_TEXT_COLOR, mode, LM_SYMBOL_COLOR, LM_TEXT_COLOR, LM_SYMBOL_COLOR);
	}

	// Force presets.
	if (mvSess->player.duel.force && mvSess->player.duel.forcepreset)
	{
		if (strlen(LM_GetValueGroup(buf, "Force", group)))
		{
			if (strlen(LM_ParseInfo(group, readbuf)))
			{
				// Read through force group.
				for (i = 0; i < NUM_FORCE_POWERS; i++)
				{
					mvSess->player.duel.knownpowers[i] = atoi(Info_ValueForKey(readbuf, forceList[i].name));
				}
			}
		}
		else
		{
			G_Printf("%s[%sError%s] %sDuel file %s'%s%s' %scontains no Force group%s.\n", LM_SYMBOL_COLOR, LM_ERROR_COLOR, LM_SYMBOL_COLOR, LM_TEXT_COLOR, LM_SYMBOL_COLOR, LM_TEXT_COLOR, mode, LM_SYMBOL_COLOR, LM_TEXT_COLOR, LM_SYMBOL_COLOR);
		}
	}

	// Clear memory and close file handle.
	BG_TempFree(DUEL_MAX_LEN);
	BG_TempFree(DUEL_VALUE_LEN);
	BG_TempFree(DUEL_GROUP_LEN);
	trap_FS_FCloseFile(f);
}

// Handle duel engage/disengage, messages, tracing, etc.
void LM_DuelHandle( gentity_t *ent, int action )
{
	int				  i;
	char			  cmd[MAX_TOKEN_CHARS];
	char			  *mode;
	trace_t			  tr;
	vec3_t			  forward, fwdOrg;
	gentity_t		  *challenged;
	mvclientSession_t *mvSess;
	mvclientSession_t *mvSessChallenged;

	mvSess = &mv_clientSessions[ent - g_entities];

	// 2 is GENCMD_ENGAGE_DUEL, otherwise it is console input.
	if (action == 2)
		Q_strncpyz(cmd, "engage_duel", sizeof(cmd));
	else
		trap_Argv(0, cmd, sizeof(cmd));

	// Engage.
	if (action)
	{
		// Grab mode from command string and check if it exists.
		mode = LM_GetDuel(cmd);
		if (LM_DuelExists(mode))
		{
			// If private duels are disabled we obviously don't allow them.
			if (!g_privateDuel.integer)
			{
				return;
			}

			// Don't allow private duels in these modes ...
			if (g_gametype.integer == GT_TOURNAMENT || g_gametype.integer >= GT_TEAM)
			{
				trap_SendServerCommand(ent - g_entities, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "NODUEL_GAMETYPE")));
				return;
			}

			// Don't allow duels if ...
			if (ent->client->ps.duelTime >= level.time || ent->client->ps.weapon != WP_SABER || ent->client->ps.saberInFlight || ent->client->ps.duelInProgress
				|| ent->client->ps.forceHandExtend != HANDEXTEND_NONE)
			{
				return;
			}

			// Copy our view angle.
			AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);

			// Calculate distance.
			fwdOrg[0] = ent->client->ps.origin[0]  + forward[0] * 256;
			fwdOrg[1] = ent->client->ps.origin[1]  + forward[1] * 256;
			fwdOrg[2] = (ent->client->ps.origin[2] + ent->client->ps.viewheight) + forward[2] * 256;

			// Trace opponent.
			trap_Trace(&tr, ent->client->ps.origin, NULL, NULL, fwdOrg, ent->s.number, MASK_PLAYERSOLID);

			// Opponent found!
			if (tr.fraction != 1 && tr.entityNum < MAX_CLIENTS)
			{
				challenged		 = &g_entities[tr.entityNum];
				mvSessChallenged = &mv_clientSessions[challenged - g_entities];

				// Opponent no longer available if ...
				if (!challenged || !challenged->client || !challenged->inuse ||
					challenged->health < 1 || challenged->client->ps.stats[STAT_HEALTH] < 1 ||
					challenged->client->ps.weapon != WP_SABER || challenged->client->ps.duelInProgress ||
					challenged->client->ps.saberInFlight || challenged->client->ps.forceHandExtend != HANDEXTEND_NONE)
				{
					return;
				}

				// Duel index matches. Prepare for the duel.
				if (challenged->client->ps.duelIndex == ent - g_entities && challenged->client->ps.duelTime >= level.time)
				{
					// Mark us as in a duel.
					ent->client->ps.duelInProgress		  = qtrue;
					challenged->client->ps.duelInProgress = qtrue;

					// Freeze them until Mothma allow us to proceed.
					ent->client->ps.duelTime		= level.time + 2000;
					challenged->client->ps.duelTime = level.time + 2000;

					// Send duel event to the client.
					G_AddEvent(ent, EV_PRIVATE_DUEL, 1);
					G_AddEvent(challenged, EV_PRIVATE_DUEL, 1);

					// Copy duel settings from one player struct to the other. No need to read the duel file twice.
					Q_strncpyz(mvSess->player.duel.mode, mvSessChallenged->player.duel.mode, sizeof(mvSess->player.duel.mode));
					Q_strncpyz(mvSess->player.duel.name, mvSessChallenged->player.duel.name, sizeof(mvSess->player.duel.name));

					mvSess->player.duel.toggle			  = mvSessChallenged->player.duel.toggle;
					mvSess->player.duel.force			  = mvSessChallenged->player.duel.force;
					mvSess->player.duel.forcepreset	      = mvSessChallenged->player.duel.forcepreset;
					mvSess->player.duel.health			  = mvSessChallenged->player.duel.health;
					mvSess->player.duel.shield			  = mvSessChallenged->player.duel.shield;
					mvSess->player.duel.bacta			  = mvSessChallenged->player.duel.bacta;
					mvSess->player.duel.dualblade		  = mvSessChallenged->player.duel.dualblade;
					mvSess->player.duel.pickups			  = mvSessChallenged->player.duel.pickups;
					mvSess->player.duel.distance		  = mvSessChallenged->player.duel.distance;
					mvSess->player.duel.fraglimit		  = mvSessChallenged->player.duel.fraglimit;
					mvSess->player.duel.timelimit		  = mvSessChallenged->player.duel.timelimit;
					mvSess->player.duel.bestof			  = mvSessChallenged->player.duel.bestof;
					mvSess->player.duel.infforce		  = mvSessChallenged->player.duel.infforce;
					mvSess->player.duel.forceregentime	  = mvSessChallenged->player.duel.forceregentime;
					mvSess->player.duel.knockback	      = mvSessChallenged->player.duel.knockback;
					mvSess->player.duel.kickdmg			  = mvSessChallenged->player.duel.kickdmg;
					mvSess->player.duel.falldmg			  = mvSessChallenged->player.duel.falldmg;
					mvSess->player.duel.ffrules			  = mvSessChallenged->player.duel.ffrules;
					mvSess->player.duel.saberlock		  = mvSessChallenged->player.duel.saberlock;
					mvSess->player.duel.gravity			  = mvSessChallenged->player.duel.gravity;
					mvSess->player.duel.speed			  = mvSessChallenged->player.duel.speed;
					mvSess->player.duel.trainingwheels	  = mvSessChallenged->player.duel.trainingwheels;

					mvSess->player.duel.wantOut			  = 0;
					mvSessChallenged->player.duel.wantOut = 0;

					mvSess->player.duel.counter			  = 0;
					mvSessChallenged->player.duel.counter = 0;

					mvSess->player.duel.index			  = challenged - g_entities;
					mvSessChallenged->player.duel.index	  = ent - g_entities;

					if (mvSessChallenged->player.duel.force && mvSessChallenged->player.duel.forcepreset)
					{
						for (i = 0; i < NUM_FORCE_POWERS; i++)
						{
							mvSess->player.duel.knownpowers[i] = mvSessChallenged->player.duel.knownpowers[i];
						}
					}

					// Holster their sabers.
					if (!ent->client->ps.saberHolstered)
					{
						G_Sound(ent, CHAN_AUTO, G_SoundIndex("sound/weapons/saber/saberoffquick.wav"));
						ent->client->ps.weaponTime	   = 400;
						ent->client->ps.saberHolstered = qtrue;
					}
					if (!challenged->client->ps.saberHolstered)
					{
						G_Sound(challenged, CHAN_AUTO, G_SoundIndex("sound/weapons/saber/saberoffquick.wav"));
						challenged->client->ps.weaponTime	  = 400;
						challenged->client->ps.saberHolstered = qtrue;
					}

					// Broadcast the event.
					trap_SendServerCommand(-1, va("print \"%s[%sDuel%s] %s%s has become engaged in a %s%s with %s%s!\n\"", LM_SYMBOL_COLOR, LM_TEXT_COLOR, LM_SYMBOL_COLOR, 
						challenged->client->pers.netname, LM_TEXT_COLOR, mvSess->player.duel.name, LM_TEXT_COLOR, ent->client->pers.netname, LM_SYMBOL_COLOR));

					// Initiate the duel. Apply health and other settings.
					LM_DuelInit(ent);
					LM_DuelInit(challenged);

					// Make sure nobody else share duelIndex with us and our opponent.
					for (i = 0; i < MAX_CLIENTS; i++)
					{
						gentity_t *check = &g_entities[i];

						if (i != challenged - g_entities && i != ent - g_entities)
						{
							if (mvapi)
							{
								mv_entities[i].snapshotIgnore[ent - g_entities] = 1;
								mv_entities[i].snapshotIgnore[challenged - g_entities] = 1;
							}

							if (check->client->ps.duelIndex == (ent - g_entities || challenged - g_entities))
								check->client->ps.duelIndex = ENTITYNUM_NONE;
						}
					}
				}
				else
				{
					// If we have already challenged someone we check if the mode we used last time match. Could save us the need for an extra run through the parser.
					if (Q_stricmp(mode, mvSess->player.duel.mode))
						LM_DuelRead(ent, mode);

					// Print challenge message.
					trap_SendServerCommand(challenged - g_entities, va("cp \"%s\n%shas challenged you to a\n%s\n\"", ent->client->pers.netname, LM_TEXT_COLOR, mvSess->player.duel.name));
					trap_SendServerCommand(ent - g_entities, va("cp \"%sYou have challenged\n%s\n%sto a\n%s\n\"", LM_TEXT_COLOR, challenged->client->pers.netname, LM_TEXT_COLOR, mvSess->player.duel.name));
				}

				// Animate our hand.
				ent->client->ps.forceHandExtend		= HANDEXTEND_DUELCHALLENGE;
				ent->client->ps.forceHandExtendTime = level.time + 1000;

				// Assign duel index and cooldown before next challenge.
				ent->client->ps.duelIndex = challenged - g_entities;
				ent->client->ps.duelTime  = level.time + 5000;
			}
		}
		else
		{
			// Duel mode doesn't exist. Print available modes.
			trap_SendServerCommand(ent - g_entities, va("print \"%s%s %sDuels %s%s\n\"", LM_SYMBOL_COLOR, LM_START_SYMBOL, LM_TEXT_COLOR, LM_SYMBOL_COLOR, LM_END_SYMBOL));
			LM_DuelList(ent);
		}
	}
	else
	{
		challenged		 = &g_entities[ent->client->ps.duelIndex];
		mvSessChallenged = &mv_clientSessions[challenged - g_entities];

		// Broadcast the end results.
		if (mvSess->player.duel.fraglimit > 1 || !mvSess->player.duel.fraglimit)
		{
			if (mvSess->player.duel.counter > mvSessChallenged->player.duel.counter)
			{
				trap_SendServerCommand(-1, va("print \"%s[%sDuel%s] %s %sdefeated %s %sin a %s %sby ^2%i%s - ^1%i%s! \"", LM_SYMBOL_COLOR, LM_TEXT_COLOR, LM_SYMBOL_COLOR,
					ent->client->pers.netname, LM_TEXT_COLOR, challenged->client->pers.netname, LM_TEXT_COLOR, mvSess->player.duel.name, LM_TEXT_COLOR, mvSess->player.duel.counter,
					LM_SYMBOL_COLOR, mvSessChallenged->player.duel.counter, LM_SYMBOL_COLOR));
			}
			else if (mvSessChallenged->player.duel.counter > mvSess->player.duel.counter)
			{
				trap_SendServerCommand(-1, va("print \"%s[%sDuel%s] %s %sdefeated %s %sin a %s %sby ^2%i%s - ^1%i%s! \"", LM_SYMBOL_COLOR, LM_TEXT_COLOR, LM_SYMBOL_COLOR,
					challenged->client->pers.netname, LM_TEXT_COLOR, ent->client->pers.netname, LM_TEXT_COLOR, mvSess->player.duel.name, LM_TEXT_COLOR, mvSessChallenged->player.duel.counter,
					LM_SYMBOL_COLOR, mvSess->player.duel.counter, LM_SYMBOL_COLOR));
			}
			else
			{
				trap_SendServerCommand(-1, va("print \"%s[%sDuel%s] %s %stied with %s %sin a %s %sby ^2%i%s - ^1%i%s! \"", LM_SYMBOL_COLOR, LM_TEXT_COLOR, LM_SYMBOL_COLOR,
					ent->client->pers.netname, LM_TEXT_COLOR, challenged->client->pers.netname, LM_TEXT_COLOR, mvSess->player.duel.name, LM_TEXT_COLOR, mvSess->player.duel.counter,
					LM_SYMBOL_COLOR, mvSessChallenged->player.duel.counter, LM_SYMBOL_COLOR));
			}

			trap_SendServerCommand(-1, va("print \"%s[%s%i%s:%s%02i%s]\n\"",
				LM_SYMBOL_COLOR, LM_TEXT_COLOR, (level.time - mvSess->player.duel.starttime) / 60000,
				LM_SYMBOL_COLOR, LM_TEXT_COLOR, ((level.time - mvSess->player.duel.starttime) / 1000) % 60, LM_SYMBOL_COLOR));
		}
		else
		{
			if (ent->health > 0 && ent->client->ps.stats[STAT_HEALTH] > 0)
			{
				if (mvSess->player.duel.fraglimit == 1)
					trap_SendServerCommand(-1, va("print \"%s[%sDuel%s] %s %sdefeated %s %sin a %s %swith ^1%i%s/^2%i %sremaining%s! \"", LM_SYMBOL_COLOR, LM_TEXT_COLOR, LM_SYMBOL_COLOR,
						ent->client->pers.netname, LM_TEXT_COLOR, challenged->client->pers.netname, LM_TEXT_COLOR, mvSess->player.duel.name, LM_TEXT_COLOR, ent->client->ps.stats[STAT_HEALTH],
						LM_SYMBOL_COLOR, ent->client->ps.stats[STAT_ARMOR], LM_TEXT_COLOR, LM_SYMBOL_COLOR));

				trap_SendServerCommand(-1, va("print \"%s[%s%i%s:%s%02i%s]\n\"",
					LM_SYMBOL_COLOR, LM_TEXT_COLOR, (level.time - mvSess->player.duel.starttime) / 60000,
					LM_SYMBOL_COLOR, LM_TEXT_COLOR, ((level.time - mvSess->player.duel.starttime) / 1000) % 60, LM_SYMBOL_COLOR));
			}
			else
			{
				trap_SendServerCommand(-1, va("print \"%s\n\"", G_GetStripEdString("SVINGAME", "PLDUELTIE")));
			}
		}

		// Clean duel structs.
		LM_DuelClean(ent);
		LM_DuelClean(challenged);
	}
}

// Alternative check for BG files where the use of mvSess, g_entities, and cg_entities would require dirty programming.
int LM_DuelBG( int clientNum, int number )
{
	int retval				  = 0;
	mvclientSession_t *mvSess = &mv_clientSessions[clientNum];
	
	// What are we checking? Toggle? Force? Infinite force?
	switch (number)
	{
		case 1:
			if (mvSess->player.duel.toggle) 
				retval = 1;
			break;
		case 2:
			if (mvSess->player.duel.infforce)
				retval = 1;
			break;
		case 3:
			if (mvSess->player.duel.force)
				retval = 1;
			break;
		case 4:
			if (mvSess->player.duel.pickups)
				retval = 1;
			break;
		case 5:
			if (mvSess->player.duel.pickups || mvSess->player.duel.bacta)
				retval = 1;
			break;
		default:
			retval = 0;
			break;
	}

	return retval;
}

// End duel before limits are hit.
void LM_DuelEnd( gentity_t *ent )
{
	mvclientSession_t *mvSess;
	mvclientSession_t *mvSessChallenged;

	if (!ent->client->ps.duelInProgress)
		return;

	mvSess						= &mv_clientSessions[ent - g_entities];
	mvSessChallenged			= &mv_clientSessions[ent->client->ps.duelIndex];

	mvSess->player.duel.wantOut = qtrue;

	if (mvSess->player.duel.wantOut && mvSessChallenged->player.duel.wantOut)
	{
		LM_DuelHandle(ent, 0);
		return;
	}
	else
	{
		trap_SendServerCommand(ent->client->ps.duelIndex, va("print \"%s[%sDuel%s] %s %shas requested for the match to come to an end%s.\n\"", LM_SYMBOL_COLOR, LM_TEXT_COLOR, LM_SYMBOL_COLOR, ent->client->pers.netname, LM_TEXT_COLOR, LM_SYMBOL_COLOR));
	}
}

// Duel stats
void LM_DuelStats( gentity_t *ent )
{
	gentity_t		  *challenged;
	mvclientSession_t *mvSessChallenged;

	mvclientSession_t *mvSess = &mv_clientSessions[ent - g_entities];

	if (!ent->client->ps.duelInProgress || mvSess->player.duel.fraglimit == 1)
		return;

	challenged		 = &g_entities[ent->client->ps.duelIndex];
	mvSessChallenged = &mv_clientSessions[challenged - g_entities];

	if (mvSess->player.duel.counter > mvSessChallenged->player.duel.counter)
	{
		trap_SendServerCommand(-1, va("print \"%s[%sDuel%s] %s %sis ahead of %s %sin a %s %sby ^2%i%s - ^1%i%s! \"", LM_SYMBOL_COLOR, LM_TEXT_COLOR, LM_SYMBOL_COLOR,
			ent->client->pers.netname, LM_TEXT_COLOR, challenged->client->pers.netname, LM_TEXT_COLOR, mvSess->player.duel.name, LM_TEXT_COLOR, mvSess->player.duel.counter,
			LM_SYMBOL_COLOR, mvSessChallenged->player.duel.counter, LM_SYMBOL_COLOR));
	}
	else if (mvSessChallenged->player.duel.counter > mvSess->player.duel.counter)
	{
		trap_SendServerCommand(-1, va("print \"%s[%sDuel%s] %s %sis ahead of %s %sin a %s %sby ^2%i%s - ^1%i%s! \"", LM_SYMBOL_COLOR, LM_TEXT_COLOR, LM_SYMBOL_COLOR,
			challenged->client->pers.netname, LM_TEXT_COLOR, ent->client->pers.netname, LM_TEXT_COLOR, mvSess->player.duel.name, LM_TEXT_COLOR, mvSessChallenged->player.duel.counter,
			LM_SYMBOL_COLOR, mvSess->player.duel.counter, LM_SYMBOL_COLOR));
	}
	else
	{
		trap_SendServerCommand(-1, va("print \"%s[%sDuel%s] %s %sis currently tied with %s %sin a %s %sby ^2%i%s - ^1%i%s! \"", LM_SYMBOL_COLOR, LM_TEXT_COLOR, LM_SYMBOL_COLOR,
			ent->client->pers.netname, LM_TEXT_COLOR, challenged->client->pers.netname, LM_TEXT_COLOR, mvSess->player.duel.name, LM_TEXT_COLOR, mvSess->player.duel.counter,
			LM_SYMBOL_COLOR, mvSessChallenged->player.duel.counter, LM_SYMBOL_COLOR));
	}

	trap_SendServerCommand(-1, va("print \"%s[%s%i%s:%s%02i%s]\n\"",
		LM_SYMBOL_COLOR, LM_TEXT_COLOR, (level.time - mvSess->player.duel.starttime) / 60000,
		LM_SYMBOL_COLOR, LM_TEXT_COLOR, ((level.time - mvSess->player.duel.starttime) / 1000) % 60, LM_SYMBOL_COLOR));
}
