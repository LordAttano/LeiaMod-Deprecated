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
