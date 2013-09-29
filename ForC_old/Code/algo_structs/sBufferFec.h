/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description    : FEC buffer struct
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

#ifndef __SBUFFERFEC__
#define __SBUFFERFEC__

// Types de données ============================================================

// Structure liant paquet de FEC et paquet média manquant ----------------------
typedef struct
{
  sRbTree cross;   //. Key= paquetMedia.médiaNo, Val= Cross (FEC colNx ou rowNx)
  sRbTree wait[2]; //. Key= {fecNo}, Val= wait (contenu utile du FEC + etc)
}
  sBufferFec;

// Déclaration des fonctions ===================================================

sBufferFec sBufferFec_New();

void sBufferFec_Release (sBufferFec*);

void sBufferFec_PrintCross (const sBufferFec*,        bool pBuffers);
void sBufferFec_PrintWait  (const sBufferFec*, eFecD, bool pBuffers);

bool sBufferFec_AddCrossByReference (sBufferFec*, sMediaNo, sCrossFec*, bool);
sCrossFec* sBufferFec_FindCross   (const sBufferFec*, sMediaNo);
bool       sBufferFec_DeleteCross (      sBufferFec*, sMediaNo);

bool     sBufferFec_AddWaitByReference (sBufferFec*, sWaitFec*, bool pOver);
sWaitFec*  sBufferFec_FindWait   (const sBufferFec*, eFecD, sFecNo);
bool       sBufferFec_DeleteWait (      sBufferFec*, eFecD, sFecNo);

bool sBufferFec_DeleteCrossAndWait (sBufferFec*, eFecD, sFecNo);

bool       sBufferFec_InitForeachCross  (      sBufferFec*, bool pReverse);
bool       sBufferFec_NextForeachCross  (      sBufferFec*);
sMediaNo   sBufferFec_ForeachKeyCross   (const sBufferFec*);
sCrossFec* sBufferFec_ForeachValueCross (const sBufferFec*);

#endif
