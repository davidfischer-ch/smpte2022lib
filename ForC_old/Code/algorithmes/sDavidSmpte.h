/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description    : Optimized implementation of SMPTE 2022-1 FEC
    Main Developer : David Fischer (david.fischer.ch@gmail.com)
    Copyright      : Copyright (c) 2008-2013 smpte2022lib Team. All rights reserved.
    Sponsoring     : Developed for a HES-SO CTI Ra&D project called GaVi
                     Haute école du paysage, d'ingénierie et d'architecture @ Genève
                     Telecommunications Laboratory
\**************************************************************************************************/
/*
  This file is part of smpte2022lib Project.

  This project is free software: you can redistribute it and/or modify it under the terms of the
  EUPL v. 1.1 as provided by the European Commission. This project is distributed in the hope that
  it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.

  See the European Union Public License for more details.

  You should have received a copy of the EUPL General Public License along with this project.
  If not, see he EUPL licence v1.1 is available in 22 languages:
      22-07-2013, <https://joinup.ec.europa.eu/software/page/eupl/licence-eupl>

  Retrieved from https://github.com/davidfischer-ch/smpte2022lib.git
*/

#ifndef __SDAVIDSMPTE__
#define __SDAVIDSMPTE__

// Types de données ============================================================

// Structure stockant ce qu'il faut pour l'algorithme de SMPTE 2022-1 optimisé ---
typedef struct
{
  sBufferMedia media; //. Stockage des paquets de média
  sBufferFec   fec;   //. Stockage des paquets de FEC et FEC <-> média

  bool overwriteMedia; //. Ecrasage des doublons dans buffer média autorisé ?

  unsigned recovered;            //. Nombre de paquets média récupérés
  unsigned unrecoveredOnReading; //. Nb paq. média manquants lors de la lecture !

  unsigned nbArPaMedia; //. Nombre d'appels à ArriveePaquetMedia
  unsigned nbArPaFec;   //. Nombre d'appels à ArriveePaquetFec
  unsigned nbLePaMedia; //. Nombre d'appels à LecturePaquetMedia
  unsigned nbPePaMedia; //. Nombre d'appels à PerduPaquetMedia
  unsigned nbRePaMedia; //. Nombre d'appels à RecupPaquetMedia

  unsigned maxC; //. Nombre max d'éléments stockés dans Cross
  unsigned maxW; //. Nombre max d'éléments stockés dans Wait

  clock_t chronoTotal; //. Temps d'exécution total en TICKS
  clock_t chronoMedia; //. Temps d'exécution total pour média en TICKS
  clock_t chronoFec;   //. Temps d'exécution total pour FEC   en TICKS
}
  sDavidSmpte;

// Déclaration des fonctions ===================================================

sDavidSmpte sDavidSmpte_New (bool pOverwriteMedia);

void sDavidSmpte_Release (      sDavidSmpte*);
void sDavidSmpte_Print   (const sDavidSmpte*, bool pBuffers);

void sDavidSmpte_ArriveePaquetMedia (sDavidSmpte*, sPaquetMedia*);
void sDavidSmpte_ArriveePaquetFec   (sDavidSmpte*, sPaquetFec*);
bool sDavidSmpte_LecturePaquetMedia (sDavidSmpte*, sMediaNo pBufferSize, FILE*);

/*http://yarchive.net/comp/ansic_broken_unsigned.html

	unsigned short s = USHRT_MAX;
	int i = -1;

	if (i < s)	// eg, -1 < 65535
		printf("ANSI C, and sizeof(int) > sizeof(short)\n");
	else		// eg, 0xffffU == 0xffffU or 0xffffffffU > 0xffffU
		printf("pre-ANSI C, or sizeof(int) == sizeof(short)\n");

   // Either output can occur under a conforming ANSI C system.  This
   // is why the choice that was made for ANSI C is wrong.  The correct
   // choice, `unsigned preserving' semantics, does not depend on the
   // relative sizes of short and int, and will always do an unsigned
   // comparison, so that we always test UINT_MAX >= USHRT_MAX, and the
   // second printf fires.  But Plauger insisted on the broken `value
   // preserving' semantics, arguing that it more often did what most
   // programmers expected.  The problem with this argument is that it
   // only does what these programmers expect when sizeof(int) >
   // sizeof(short) -- when the sizes are the same, it acts like pre-ANSI
   // C, so programmers cannot assume a signed comparison will occur.
   // It would be better always to do an unsigned comparison, giving a
   // fixed answer, rather than an implementation-specific answer.

*/

//#define dprintf(...) realdprintf(__FILE__, __LINE__, __VA_ARGS__)

#endif
