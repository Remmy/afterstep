/*
 * Copyright (C) 1999 Sasha Vasko <sashav@sprintmail.com>
 * merged with envvar.c originally created by :
 * Copyright (C) 1999 Ethan Fischer <allanon@crystaltokyo.com>
 * Copyright (C) 1998 Pierre Clerissi <clerissi@pratique.fr>
 */
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "config.h"

/****************************************************************************
 *
 * Find the specified icon file somewhere along the given path.
 *
 * There is a possible race condition here:  We check the file and later
 * do something with it.  By then, the file might not be accessible.
 * Oh well.
 *
 ****************************************************************************/
/* supposedly pathlist should not include any environment variables
   including things like ~/
 */

char         *
findIconFile (const char *icon, const char *pathlist, int type)
{
	char         *path;
	int           icon_len;
	int           max_path = 0;
	char         *ptr;
	register int  i;

	if (icon == NULL)
		return NULL;
	if (*icon == '/' || *icon == '~' || ((pathlist == NULL) || (*pathlist == '\0')))
	{
		path = PutHome (icon);
		if (access (path, type) == 0)
			return path;
		free (path);
		return NULL;
	}

	for (icon_len = 0; *(icon + icon_len); icon_len++);
	for (ptr = (char *)pathlist; *ptr; ptr += i)
	{
		if (*ptr == ':')
			ptr++;
		for (i = 0; *(ptr + i) && *(ptr + i) != ':'; i++);
		if (i > max_path)
			max_path = i;
	}

	path = safemalloc (max_path + 1 + icon_len + 1);

	/* Search each element of the pathlist for the icon file */
	for (ptr = (char *)pathlist; *ptr; ptr += i)
	{
		if (*ptr == ':')
			ptr++;
		for (i = 0; *(ptr + i) && *(ptr + i) != ':'; i++)
			*(path + i) = *(ptr + i);
		if (i == 0)
			continue;
		*(path + i) = '/';
		*(path + i + 1) = '\0';
		strcat (path, icon);
		if (access (path, type) == 0)
			return path;
	}
	/* Hmm, couldn't find the file.  Return NULL */
	free (path);
	return NULL;
}

char         *
find_envvar (char *var_start, int *end_pos)
{
	char          backup, *name_start = var_start;
	register int  i;
	char         *var = NULL;

	if (*var_start == '{')
	{
		name_start++;
		for (i = 1; *(var_start + i) && *(var_start + i) != '}'; i++);
	} else
		for (i = 0; isalnum (*(var_start + i)) || *(var_start + i) == '_'; i++);

	backup = *(var_start + i);
	*(var_start + i) = '\0';
	var = getenv (name_start);
	*(var_start + i) = backup;

	*end_pos = i;
	if (backup == '}')
		(*end_pos)++;
	return var;
}

void
replaceEnvVar (char **path)
{
	char         *data = *path, *tmp;
	char         *home = getenv ("HOME");
	int           pos = 0, len, home_len = 0;

	if (*path == NULL)
		return;
	if (**path == '\0')
		return;
	len = strlen (*path);
	if (home)
		home_len = strlen (home);

	while (*(data + pos))
	{
		char         *var;
		int           var_len, end_pos;

		while (*(data + pos) != '$' && *(data + pos))
		{
			if (*(data + pos) == '~' && *(data + pos + 1) == '/')
			{
				if (pos > 0)
					if (*(data + pos - 1) != ':')
					{
						pos += 2;
						continue;
					}
				if (home == NULL)
					*(data + (pos++)) = '.';
				else
				{
					len += home_len;
					tmp = safemalloc (len);
					strncpy (tmp, data, pos);
					strcpy (tmp + pos, home);
					strcpy (tmp + pos + home_len, data + pos + 1);
					free (data);
					data = tmp;
					pos += home_len;
				}
			}
			pos++;
		}
		if (*(data + pos) == '\0')
			break;
		/* found $ sign - trying to replace var */
		if ((var = find_envvar (data + pos + 1, &end_pos)) == NULL)
		{
			pos++;
			continue;
		}
		var_len = strlen (var);
		len += var_len;
		tmp = safemalloc (len);
		strncpy (tmp, data, pos);
		strcpy (tmp + pos, var);
		strcpy (tmp + pos + var_len, data + pos + end_pos + 1);
		free (data);
		data = tmp;

	}
	*path = data;
}

/*
 * only checks first word in name, to allow full command lines with
 * options to be passed
 */
int
is_executable_in_path (const char *name)
{
	static char  *cache = NULL;
	static int    cache_result = 0, cache_len = 0, cache_size = 0;
	static char  *env_path = NULL;
	static int    max_path = 0;
	register int  i;

	if (name == NULL)
	{
		if (cache)
		{
			free (cache);
			cache = NULL;
		}
		cache_size = 0;
		cache_len = 0;
		if (env_path)
		{
			free (env_path);
			env_path = NULL;
		}
		max_path = 0;
		return 0;
	}

	/* cut leading "exec" enclosed in spaces */
	for (; isspace (*name); name++);
	if (!strncmp (name, "exec", 4) && isspace (name[4]))
		name += 4;
	for (; isspace (*name); name++);
	if (*name == '\0')
		return 0;

	for (i = 0; *(name + i) && !isspace (*(name + i)); i++);
	if (i == 0)
		return 0;

	if (cache)
		if (i == cache_len && strncmp (cache, name, i) == 0)
			return cache_result;

	if (i > cache_size)
	{
		if (cache)
			free (cache);
		/* allocating slightly more space then needed to avoid
		   too many reallocations */
		cache = (char *)safemalloc (i + (i >> 1) + 1);
		cache_size = i + (i >> 1);
	}
	strncpy (cache, name, i);
	cache[i] = '\0';
	cache_len = i;

	if (*cache == '/')
		cache_result = (CheckFile (cache) == 0) ? 1 : 0;
	else
	{
		char         *ptr, *path;
		struct stat   st;

		if (env_path == NULL)
		{
			env_path = mystrdup (getenv ("PATH"));
			replaceEnvVar (&env_path);
			for (ptr = env_path; *ptr; ptr += i)
			{
				if (*ptr == ':')
					ptr++;
				for (i = 0; *(ptr + i) && *(ptr + i) != ':'; i++);
				if (i > max_path)
					max_path = i;
			}
		}
		path = safemalloc (max_path + cache_len + 2);
		cache_result = 0;
		for (ptr = env_path; *ptr && cache_result == 0; ptr += i)
		{
			if (*ptr == ':')
				ptr++;
			for (i = 0; *(ptr + i) && *(ptr + i) != ':'; i++)
				path[i] = *(ptr + i);
			path[i] = '/';
			path[i + 1] = '\0';
			strcat (path, cache);
			if ((stat (path, &st) != -1) && (st.st_mode & S_IXUSR))
				cache_result = 1;
		}
		free (path);
	}
	return cache_result;
}


