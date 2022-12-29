
#ifndef __HWGAMMACOM__H__
#define __HWGAMMACOM__H__

typedef struct
{
	qbool			gammaworks;			// It is functional
	qbool			hwgamma_enabled;  
	qbool			customgamma;
	unsigned short	*currentgammaramp; // = NULL;
	unsigned short	systemgammaramp[3][256];
} hwgamma_com_t;

#endif // __HWGAMMACOM__H__

