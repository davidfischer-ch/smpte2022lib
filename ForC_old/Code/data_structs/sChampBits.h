/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description : Bit-field : A big unisgned number
    Authors     : David Fischer
    Contact     : david.fischer.ch@gmail.com / david.fischer@hesge.ch
    Copyright   : 2008-2013 smpte2022lib Team. All rights reserved.
    Sponsoring  : Developed for a HES-SO CTI Ra&D project called GaVi
                  Haute école du paysage, d'ingénierie et d'architecture @ Genève
                  Telecommunications Laboratory
\**************************************************************************************************/
/*
  This file is part of smpte2022lib.

  This project is free software: you can redistribute it and/or modify it under the terms of the
  GNU General Public License as published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This project is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with this project.
  If not, see <http://www.gnu.org/licenses/>

  Retrieved from:
    git clone git://github.com/davidfischer-ch/smpte2022lib.git
*/

#ifndef __SCHAMPBITS__
#define __SCHAMPBITS__

// Types de données ============================================================

// CHAMP_NO_MAX est 255 pour un pNo uint8_t, etc ...

#define CHAMP_NO_MAX        (UINT8_MAX+1)
#define CHAMP_VALEUR_MAX    (UINT32_MAX)
#define CHAMP_TAILLE_UNITE  (UINT32_BITS)
#define CHAMP_NOMBRE_UNITE  (CHAMP_NO_MAX / CHAMP_TAILLE_UNITE)

// Type de paquet de FEC (colonne ou ligne) ------------------------------------
typedef enum { LSB_FIRST = 0, MSB_FIRST = 1 } eDir;

// Structure représentant un champ de bits (grand entier non signé) ------------
typedef struct
{
  uint32_t buffer[CHAMP_NOMBRE_UNITE]; //. Stockage des bits
}
  sChampBits;

// Déclaration des Fonctions ===================================================

sChampBits sChampBits_New();

void sChampBits_Print (const sChampBits*);

bool sChampBits_GetBit   (const sChampBits*, uint8_t pNo);
void sChampBits_SetBit   (      sChampBits*, uint8_t pNo, bool pValeur);
signed sChampBits_GetOne (const sChampBits*, uint8_t pNo, eDir pDirection);

void       sChampBits_Incremente (      sChampBits*);
signed     sChampBits_Compare    (const sChampBits*, const sChampBits*);
sChampBits sChampBits_Add        (const sChampBits*, const sChampBits*);

bool sChampBits_SelfTest
      (sChampBits (*pFonction)(const sChampBits*, const sChampBits*));

#endif
