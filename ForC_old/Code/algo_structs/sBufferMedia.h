/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description    : Media buffer struct
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

#ifndef __SBUFFERMEDIA__
#define __SBUFFERMEDIA__

// Types de données ============================================================

// Structure stockant les paquets média dans un arbre rouge-noire. Le buffer ---
// média de VLC est une liste chaînée, ce qui n'est pas optimal lors de      ---
// nombreuses manipulations du buffer. Une ancienne version avec liste       ---
// doublement chaînée est disponible au cas où (version précédente).         ---
typedef struct
{
  sRbTree rbtree; //. Key = paquetMedia.mediaNo, Value = paquetMedia

  sRbNode* reading; //. Position de la lecture   (dernier lu)
  sRbNode* arrival; //. Position de la réception (dernier réceptionné)

  sMediaNx readingNx; //. Position de la lecture   (dernier médiaNo lu)
  sMediaNx arrivalNx; //. Position de la réception (dernier médiaNo réceptionné)
}
  sBufferMedia;

// Déclaration des fonctions ===================================================

sBufferMedia  sBufferMedia_New     ();
void          sBufferMedia_Release (      sBufferMedia*);
void          sBufferMedia_Print   (const sBufferMedia*, bool pBuffers);

bool sBufferMedia_AddByReference (sBufferMedia*, sPaquetMedia*, bool pOver);

sPaquetMedia* sBufferMedia_Find (const sBufferMedia*, sMediaNo);

bool          sBufferMedia_InitForeach  (      sBufferMedia*, bool pReverse);
bool          sBufferMedia_NextForeach  (      sBufferMedia*);
sMediaNo      sBufferMedia_ForeachKey   (const sBufferMedia*);
sPaquetMedia* sBufferMedia_ForeachValue (const sBufferMedia*);

bool sBufferMedia_IsMediaNoInBuffer (const sBufferMedia*, sMediaNo);
bool sBufferMedia_ReadMedia         (sBufferMedia*, FILE*, sMediaNo* pReadedNo);

#endif
