//
// location.c
//
// JPG
//
// This entire file is new in proquake.  It is used to translate map areas
// to names for the %l formatting specifier
//

#include "quakedef.h"

#define MAX_LOCATIONS 64

static location_t	locations[MAX_LOCATIONS];
static int			numlocations = 0;

/*
===============
LOC_LoadLocations

Load the locations for the current level from the location file
===============
*/
void LOC_LoadLocations (void)
{
	FILE *f;
	char		*ch;
	char		locs_filename[64];
	char		buff[256];
	location_t *thisloc;
	int i;
	float temp;

	numlocations = 0;
//	mapname = cl.worldmodethisloc->name;

//	if (!COM_StringMatchNCaseless(mapname, "maps/")) // What situation is this trying to stop?  Nomap on client?  Well, it won't hurt 'em.
//		return;
//	strcpy(filename + 5, mapname + 5);

	snprintf (locs_filename, sizeof(locs_filename), "locs/%s.loc", cl.worldname);
//	ch = strrchr(filename, '.');
//	if (ch)
//		*ch = 0;
//	strlcat (filename, ".loc", sizeof(filename));

	if (!QFS_FOpenFile(locs_filename, &f, cl.worldmodel->loadinfo.searchpath /*PATH LIMIT ME*/))
		return;

	thisloc = locations;
	while (!feof(f) && numlocations < MAX_LOCATIONS)
	{
		if (fscanf(f, "%f, %f, %f, %f, %f, %f, ", &thisloc->mins_corner[0], &thisloc->mins_corner[1], &thisloc->mins_corner[2], &thisloc->maxs_corner[0], &thisloc->maxs_corner[1], &thisloc->maxs_corner[2]) == 6)
		{
			thisloc->sd = 0;	// JPG 1.05
			for (i = 0 ; i < 3 ; i++)
			{
				if (thisloc->mins_corner[i] > thisloc->maxs_corner[i])
				{
					temp = thisloc->mins_corner[i];
					thisloc->mins_corner[i] = thisloc->maxs_corner[i];
					thisloc->maxs_corner[i] = temp;
				}
				thisloc->sd += thisloc->maxs_corner[i] - thisloc->mins_corner[i];  // JPG 1.05
			}
			thisloc->mins_corner[2] -= 32.0;
			thisloc->maxs_corner[2] += 32.0;
			fgets(buff, 256, f);

			ch = strrchr(buff, '\n');	if (ch)		*ch = 0;	// Eliminate trailing newline characters
			ch = strrchr(buff, '\"');	if (ch)		*ch = 0;	// Eliminate trailing quotes

			for (ch = buff ; *ch == ' ' || *ch == '\t' || *ch == '\"' ; ch++);	// Go through the string and forward past any spaces, tabs or double quotes to find start of the name

			StringLCopy (thisloc->name, ch); // Baker: Non-critical
			thisloc = &locations[++numlocations];
		}
		else
			fgets(buff, 256, f);
	}

	fclose(f);
}

/*
===============
LOC_GetLocation

Get the name of the location of a point
===============
*/
// JPG 1.05 - rewrote this to return the nearest rectangle if you aren't in any (manhattan distance)
char *LOC_GetLocation (vec3_t worldposition)
{
	location_t *thisloc;
	location_t *bestloc;
	float dist, bestdist;

	if (numlocations == 0)
		return "(Not loaded)";


	bestloc = NULL;
	bestdist = 999999;
	for (thisloc = locations ; thisloc < locations + numlocations ; thisloc++)
	{
		dist =	fabsf(thisloc->mins_corner[0] - worldposition[0]) + fabsf(thisloc->maxs_corner[0] - worldposition[0]) +
				fabsf(thisloc->mins_corner[1] - worldposition[1]) + fabsf(thisloc->maxs_corner[1] - worldposition[1]) +
				fabsf(thisloc->mins_corner[2] - worldposition[2]) + fabsf(thisloc->maxs_corner[2] - worldposition[2]) - thisloc->sd;

		if (dist < .01)
			return thisloc->name;

		if (dist < bestdist)
		{
			bestdist = dist;
			bestloc = thisloc;
		}
	}
	if (bestloc)
		return bestloc->name;
	return "somewhere";
}

