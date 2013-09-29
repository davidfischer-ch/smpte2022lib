
/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Main Developer : David Fischer (david.fischer.ch@gmail.com)
    VLC Integrator : Jérémie Rossier (jeremie.rossier@gmail.com)
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

// Inclus du module SMPTE2022 ==================================================

#include "smpte2022tools.h"
#include "sRbTree.h"
#include "smpte2022media.h"
#include "smpte2022fec.h"
#include "smpte2022algorithm.h"

#endif
