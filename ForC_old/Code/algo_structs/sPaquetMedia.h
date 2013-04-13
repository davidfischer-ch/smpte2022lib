/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description : Media packet struct
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

#ifndef __SPAQUETMEDIA__
#define __SPAQUETMEDIA__

#define PLDS 188*7

// Types de données ============================================================

// Structure représentant un paquet média (simplifié) --------------------------
typedef struct
{
  sMediaNo mediaNo;     //. Numéro de séquence du paquet média
  uint32_t timeStamp;   //. TimeStamp lié au flux
  uint8_t  payloadType; //. Type de payload
  unsigned payloadSize; //. Taille du payload
  uint8_t* payload;     //. Payload (je dirais même playload :-p)
}
  sPaquetMedia;

// Déclaration des Fonctions ===================================================

sPaquetMedia* sPaquetMedia_Copy  (const sPaquetMedia*);
sPaquetMedia* sPaquetMedia_Forge (sMediaNo, uint32_t, uint8_t,
                                  size_t, const uint8_t*);

void sPaquetMedia_Release (      sPaquetMedia*);
void sPaquetMedia_Print   (const sPaquetMedia*);

bool          sPaquetMedia_ToFile   (const sPaquetMedia*, FILE*, bool pHeader);
sPaquetMedia* sPaquetMedia_FromFile (FILE*);

#endif
