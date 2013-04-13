/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description : Null-able media/FEC packet sequence number
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

#ifndef __SSEQNX__
#define __SSEQNX__

// Types de données ============================================================

// Type numéro de séquence RTP attribuable à un paquet média -------------------
typedef uint16_t sMediaNo;

// Type numéro de séquence RTP attribuable à un paquet de FEC ------------------
typedef uint16_t sFecNo;

// Structure représentant un numéro de séquence média qui peut-être null -------
typedef struct
{
  sMediaNo v;    //. Valeur du numéro de séquence
  bool     null; //. True = numéro de séquence null
}
  sMediaNx;

// Structure représentant un numéro de séquence FEC qui peut-être null ---------
typedef struct
{
  sFecNo v;    //. Valeur du numéro de séquence
  bool   null; //. True = numéro de séquence null
}
  sFecNx;

// Déclaration des Constantes ==================================================

extern const sMediaNx MEDIA_NX_NULL; //. médiaNx à NULL
extern const sFecNx   FEC_NX_NULL;   //. fecNx   à NULL

// Déclaration des Fonctions ===================================================

inline sMediaNx sMediaNo_to_sMediaNx (sMediaNo);
inline sFecNx   sFecNo_to_sFecNx     (sFecNo);

inline void sMediaNx_Print (sMediaNx);
inline void sFecNx_Print   (sFecNx);

#endif
