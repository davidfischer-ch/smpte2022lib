/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description : FEC packet struct
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

#ifndef __SPAQUETFEC__
#define __SPAQUETFEC__

// Types de données ============================================================

// Type de paquet de FEC (colonne ou ligne) ------------------------------------
typedef enum { COL = 0, ROW = 1 } eFecD;

// Type d'algorithme correcteur d'erreur du paquet de FEC ----------------------
typedef enum { XOR = 0, Hamming = 1, Reed_Solomon = 2 } eFecType;

// Structure représentant un paquet de FEC -------------------------------------
typedef struct
{
  // TODO enlever fecNo du header FEC car fecNo = numéro de séquence RTP du
  // paquet de FEC lors de l'encapsulation RTP du payload de FEC ci-dessous

  sFecNo fecNo; //. Numéro de séquence du paquet de FEC (à enlever du headerFEC)

  /* FEC header format (RFC 2733)
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |       SNBase low bits         |        Length recovery        |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |E| PT recovery |                    Mask                       |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                          TS recovery                          |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |X|D|type |index|    Offset     |      NA       |SNBase ext bits|
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */

  struct DWORD0 { uint32_t Length_recovery : 16;
                  uint32_t SNBase_low_bits : 16; } DWORD0;

  struct DWORD1 { uint32_t Mask            : 24;
                  uint32_t PT_recovery     : 7;
                  uint32_t E               : 1;  } DWORD1;

  struct DWORD2 { uint32_t TS_recovery     : 32; } DWORD2;

  struct DWORD3 { uint32_t SNBase_ext_bits : 8;
                  uint32_t NA              : 8;
                  uint32_t Offset          : 8;
                  uint32_t index           : 3;
                  uint32_t type            : 3;
                  uint32_t D               : 1;
                  uint32_t X               : 1;  } DWORD3;

  uint8_t* resXor; //. Résultat de l'op. de xor entre paquets média protégés
}
  sPaquetFec;

// Déclaration des Constantes ==================================================

#define FEC_SNBASE_MASK 0x00FFFFFF
#define FEC_MASK_0      0
#define FEC_X_0         0
#define FEC_E_1         1
#define FEC_INDEX_XOR   0

// Déclaration des Fonctions ===================================================

sPaquetFec* sPaquetFec_Copy  (const sPaquetFec*);
sPaquetFec* sPaquetFec_Forge (      sFecNo    pFecNo,
                                    uint16_t  pLength_recovery,
                                    sMediaNo  pSNBase,
                                    uint8_t   pPT_recovery,
                                    uint32_t  pTS_recovery,
                                    uint8_t   pL_cols,
                                    uint8_t   pD_rows,
                                    eFecD     pD,
                              const uint8_t*  pResXor);

void sPaquetFec_Release (      sPaquetFec*);
void sPaquetFec_Print   (const sPaquetFec*);

bool        sPaquetFec_ToFile   (const sPaquetFec*, FILE*);
sPaquetFec* sPaquetFec_FromFile (FILE*);
#endif
