
/******************************************************************************\
                          ALGORITHME DE FEC SMPTE 2022-1

  Auteur    : David Fischer
  Intégration VLC : Rossier Jérémie (2011)
  Contact   : david.fischer.ch@gmail.com / david.fischer@hesge.ch
              jeremie.rossier@gmail.com

  Projet     : Implémentation SMPTE 2022 de VLC
  Date début : 02.05.2008
  Employeur  : Ecole d'Ingénieurs de Genève
               Laboratoire de Télécommunications
\******************************************************************************/

/* Copyright (c) 2009 David Fischer (david.fischer.ch@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Retrieved from:
  https://sourceforge.net/projects/smpte2022lib/
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
