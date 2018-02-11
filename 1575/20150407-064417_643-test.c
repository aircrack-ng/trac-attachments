#include <stdlib.h>
#include <stdio.h>
#define _GNU_SOURCE
#include <string.h>

/* Extract a name:value pair from a null-terminated line of JSON.
 * 
 * Returns 1 if the name was found, or 0 otherwise. 
 * 
 * The string in "value" is null-terminated if the name was found.  If
 * the name was not found, the contents of "value" are undefined. 
 */
static int json_get_value_for_name( const char *buffer, const char *name, char *value )
{
	char * to_find;
	char *cursor;
	size_t to_find_len;
	char *vcursor = value;
	int ret = 0;
	
	if (buffer == NULL || strlen(buffer) == 0 || name == NULL || strlen(name) == 0 || value == NULL)
	{
		return 0;
	}

	to_find_len = strlen(name) + 3;
	to_find = (char*) malloc(to_find_len);
	snprintf(to_find, sizeof(to_find), "\"%s\"", name);
	cursor = strcasestr(buffer, to_find);
	free(to_find);
	if(cursor != NULL)
	{
		printf("Line: %x (line length: %d) - Cursor: %x\n", buffer, strlen(buffer), cursor); 
		cursor += to_find_len -1;
		printf("Moved cursor (by %u): %x\n", to_find_len -1, cursor);
		while(*cursor != ':' && *cursor != '\0')
		{
			cursor++;
		}
		if(*cursor != '\0')
		{
			cursor++;
			while(isspace(*cursor) && *cursor != '\0')
			{
				cursor++;
			}
		}
		if('\0' == *cursor)
		{
			return 0;
		}

		if('"' == *cursor)
		{
			/* Quoted string */
			cursor++;
			while(*cursor != '"' && *cursor != '\0')
			{
				if('\\' == *cursor && '"' == *(cursor+1))
				{
					/* Escaped quote */
					*vcursor = '"';
					cursor++;
				}
				else
				{
					*vcursor = *cursor;
				}
				vcursor++;
				cursor++;
			}
			*vcursor = '\0';
			ret = 1;
		}
		else if(strncmp(cursor, "true", 4) == 0)
		{
			/* Boolean */
			strcpy(value, "true");
			ret = 1;
		}
		else if(strncmp(cursor, "false", 5) == 0)
		{
			/* Boolean */
			strcpy(value, "false");
			ret = 1;
		}
		else if('{' == *cursor || '[' == *cursor)
		{
			/* Object or array.  Too hard to handle and not needed for
			 * getting coords from GPSD, so pretend we didn't see anything.
			 */
			ret = 0;
		}
		else
		{
			/* Number, supposedly.  Copy as-is. */
			while(*cursor != ',' && *cursor != '}' && !isspace(*cursor))
			{
				*vcursor = *cursor;
				cursor++; vcursor++;
			}
			*vcursor = '\0';
			ret = 1;
		}
	}

	return ret;
}

int main(int nbarg, char * argv[])
{
	char line[1537], data[1537];
	memset(line, 0, 1537);
	memset(data, 0, 1537);
	strcat(line, "{\"class\":\"VERSION\",\"release\":\"3.15~dev\",\"rev\":\"release-3.14-290-g0a4510d\",\"proto_major\":3,\"proto_minor\":10}");
	//printf("Line: %s\n", line);
	//printf("Searching for: %s\n", argv[1]);
	int ret = json_get_value_for_name(line, argv[1], data);
	//printf("ret: %d - is Version: %d - data: %s\n", ret, strncmp(data, "VERSION", 7), data);
	return 0;
}