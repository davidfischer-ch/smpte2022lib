/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description    : Link between a media packet <-> FEC packets able to recover it
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

#include "../smpte.h"

// Constantes privées ==========================================================

const sCrossFec INIT_CROSS_FEC= //. Valeur par défaut d'un cross (à FEC_NX_NULL)
  {{0, true}, {0, true}};

// Fonctions publiques =========================================================

// Création d'un nouveau cross                                           -------
// Remarque : ne pas oublier de faire le ménage avec sCrossFec_Release ! -------
//> Pointeur sur le nouveau cross ou 0 si problème
sCrossFec* sCrossFec_New()
{
  sCrossFec* _cross = malloc (sizeof (sCrossFec));
  IFNOT     (_cross, 0) // Allocation ratée ?

  *_cross = INIT_CROSS_FEC; // Initialise le contenu du cross

  return _cross;
}

// Libère la mémoire allouée par un cross --------------------------------------
void sCrossFec_Release
  (sCrossFec* pCross) //: Cross à vider
{
  ASSERTpc (pCross,, cExNullPtr)
  free     (pCross);
}

// Affiche le contenu d'un cross -----------------------------------------------
void sCrossFec_Print
  (const sCrossFec* pCross) //: Cross à afficher
{
  ASSERTpc (pCross,, cExNullPtr)

  PRINT1 ("cross {")
  PRINT1 ("colNx=") sFecNx_Print (pCross->colNx);
  PRINT1 ("rowNx=") sFecNx_Print (pCross->rowNx);
  PRINT1 ("} ")
}
