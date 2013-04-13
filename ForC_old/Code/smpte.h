/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description : Library header file
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

//     <-----L (cols)----->
//
//  |  p 01 p 02 p 03 p 04  l 01      p nb = paquet nb média data
//  D  p 05 p 06 p 07 p 08  l 02      c nb = paquet nb FEC colonne
//  |  p 09 p 10 p 11 p 12  l 03      l nb = paquet nb FEC ligne
//
//     c 01 c 02 c 03 c 04

// GLOSSAIRE : paquet média = de données utile (le flux primaire)
//             paquet FEC   = paquet RTP transportant le payload de FEC

// Convention d'écriture :
//
// XYZ      = constante   XYZ en majuscule
// XYZ_t    = type        XYZ
// eXYZ     = enumeration XYZ
// sXYZ     = structure   XYZ
// pXYZ     = paramètre   XYZ de fonction
// _XYZ     = variable    XYZ temporaire (si < 4 lettre = pas de _)
// sXYZ_ABC = fonction du type XYZ dont l'utilié est ABC

// REMARQUE : New est l'équivalent de l'init. de session RTP     avec SMPTE 2022-1
// REMARQUE : Release est l'équivalent de la fin de session RTP  avec SMPTE 2022-1
// REMARQUE : ArriveePaquetMedia est l'équivalent de rtp_receive avec SMPTE 2022-1
// REMARQUE : ArriveePaquetFec est l'équivalent de smpte_receive avec SMPTE-2002
// REMARQUE : LecturePaquetMedia est l'équivalent de "lecture"   avec SMPTE 2022-1

#ifndef __SMPTE__
#define __SMPTE__

// Constantes de test ==========================================================

/* Ma notation des assertions et autres trucs
 *
 * Type        Description
 *
 * IFNOT       condition et si elle échoue retourne de la fonction ...
 * IFNOTc      condition et si elle échoue retourne ... avec l'embrouille du %s
 *
 * ASSERTp     assertion d'un paramètre de fonction
 * ASSERTpc    assertion d'un paramètre de fonction avec l'embrouille du %s
 * ASSERT      assertion d'une opération pouvant échouer
 * ASSERTc     assertion d'une opération pouvant échouer avec ... %s
 *
 * Types d'options
 *
 * Option                     Description
 *
 * OPTION_OS_IS_WINDOW        Indique que le système est Windows (pas Linux)
 * OPTION_OVERWRITE_FEC_NO    Indique d'auto générer fecNo lors du traitement de
 *                            FEC pour ne pas dépendre de celui donné par le pk.
 *                            de FEC car celui-ci est une source d'erreurs
 *                            potentiel pour l'algorithme si plusieurs paquets
 *                            seraient reçus avec un fecNo identique
 *                            (=bug de l'émetteur)
 *
 * OPTION_PRINT1_IS_NULL      PRINT1(c) sera <NULL>
 * OPTION_PRINT2_IS_NULL      PRINT2(c) sera <NULL> (et DETAILS2 pareil)
 *
 * OPTION_ASSERT_IS_ASSERT    ASSERT(c) sera ASSERTp(c) *1
 * OPTION_ASSERT_IS_IFNOT     ASSERT(c) sera IFNOTp(c)  *2
 * OPTION_ASSERT_IS_NULL      ASSERT(c) sera <NULL>
 *
 * OPTION_ASSERTp_IS_ASSERT   ASSERTp(c) sera ASSERTp(c) *1
 * OPTION_ASSERTp_IS_IFNOT    ASSERTp(c) sera IFNOTp(c)  *2
 * OPTION_ASSERTp_IS_NULL     ASSERTp(c) sera <NULL>
 *
 *
 * *1 : en cas d'erreur une assertion sera levée
 * *2 : en cas d'erreur la fonction quittera comme un IFNOT
 */

// TODO Payload type des pseudo paquets média (au hasard)
#define PAYLOAD_TYPE 33

//#define OPTION_OS_IS_WINDOWS
//#define OPTION_OVERWRITE_FEC_NO

//#define OPTION_PRINT1_IS_NULL
//#define OPTION_PRINT2_IS_NULL

#define OPTION_ASSERT_IS_ASSERT
//#define OPTION_ASSERT_IS_IFNOT
//#define OPTION_ASSERT_IS_NULL

//#define OPTION_ASSERTp_IS_ASSERT
//#define OPTION_ASSERTp_IS_IFNOT
#define OPTION_ASSERTp_IS_NULL

#include "common/types.h"

// Inclus de base ==============================================================

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <memory.h>
#include <math.h>
#include <assert.h>

#ifdef OPTION_OS_IS_WINDOWS
  #include <windows.h>
#endif

#include "common/macros.h"
#include "common/console.h"
#include "common/messages.h"
#include "common/project_types.h"

#endif
