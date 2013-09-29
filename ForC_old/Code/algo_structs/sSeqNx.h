/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description    : Null-able media/FEC packet sequence number
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
