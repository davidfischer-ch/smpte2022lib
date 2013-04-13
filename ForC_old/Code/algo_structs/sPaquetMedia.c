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

#include "../smpte.h"

// Constantes privées ==========================================================

const char*  cBegMediaString = "THIS_IS_A_MED_PK"; //. Entête parsing -> fichier
const size_t cBegMediaLength = 16;    //. Longueur de l'entête parsing
static char  cBegMediaBuffer  [16+1]; //. Buffer lecture de l'entête parsing

// Fonctions publiques =========================================================

// Copie un paquet média                                                    ----                                                  ------
// Remarque : ne pas oublier de faire le ménage avec sPaquetMedia_Release ! ----
//> Pointeur sur le nouveau paquet média ou 0 si problème
sPaquetMedia* sPaquetMedia_Copy
  (const sPaquetMedia* pMedia) //: Paquet à copier
{
  ASSERTpc (pMedia, 0, cExNullPtr)

  sPaquetMedia* _media = malloc (sizeof (sPaquetMedia));
  IFNOT        (_media, 0) // Allocation ratée ?

  void*     ok = memcpy (_media, pMedia, sizeof (sPaquetMedia));
  IFNOT_OP (ok, free (_media), 0) // Copie ratée ?

  if (pMedia->payload == 0) return _media;

  _media->payload = malloc (pMedia->payloadSize);
  IFNOT_OP (_media->payload, free (_media), 0) // Allocation ratée ?

  // TODO Attention : trop grande confiance en le payload donné en paramètre
  ok = memcpy (_media->payload, pMedia->payload, pMedia->payloadSize);
  IFNOT_OP (ok, sPaquetMedia_Release (_media), 0) // Copie ratée ?

  return _media;
}

// Forge un nouveau paquet média                                            ----
// Remarque : ne pas oublier de faire le ménage avec sPaquetMedia_Release ! ----
//> Pointeur sur le nouveau paquet média ou 0 si problème
sPaquetMedia* sPaquetMedia_Forge
  (sMediaNo       pMediaNo,     //: MédiaNo à affecteur au paquet
   uint32_t       pTimeStamp,   //: TimeStamp lié au flux
   uint8_t        pPayloadType, //: Type de payload
   size_t         pPayloadSize, //: Longueur du payload
   const uint8_t* pPayload)     //: Contenu à copier dans payload
{
  sPaquetMedia* _media = malloc (sizeof (sPaquetMedia));
  IFNOT        (_media, 0) // Allocation ratée ?

  _media->mediaNo     = pMediaNo;
  _media->timeStamp   = pTimeStamp;
  _media->payloadType = pPayloadType;
  _media->payloadSize = pPayloadSize;

  if (pPayloadSize == 0) return _media;

  _media->payload = malloc (pPayloadSize);
  IFNOT_OP (_media->payload, free (_media), 0) // Allocation ratée ?

  void*     ok = memcpy (_media->payload, pPayload, pPayloadSize);
  IFNOT_OP (ok, sPaquetMedia_Release (_media), 0) // Copie ratée ?

  return _media;
}

// Libère la mémoire allouée par un paquet média -------------------------------
void sPaquetMedia_Release
  (sPaquetMedia* pMedia) //: Paquet à vider
{
  ASSERTpc (pMedia,, cExNullPtr)

  if (pMedia->payload) free (pMedia->payload);
  free (pMedia);
}

// Affiche le contenu d'un paquet média ----------------------------------------
void sPaquetMedia_Print
  (const sPaquetMedia* pMedia) //: Paquet à afficher
{
  ASSERTpc (pMedia,, cExNullPtr)

  PRINT1 ("{mediaNo=%u, timeStamp=%u, "
          "payloadType=%u, payloadSize=%u payload=...} ",
          pMedia->mediaNo,
          pMedia->timeStamp,
          pMedia->payloadType,
          pMedia->payloadSize)
}

// Enregistre le contenu d'un paquet média vers un fichier. pHeader indique ----
// s'il faut copier le paquet en entier (entête+parsing) ou que le payload  ----
//> Résultat de l'opération / enregistrement réussi ?
bool sPaquetMedia_ToFile
  (const sPaquetMedia* pMedia,  //: Paquet à enregistrer
         FILE        * pFile,   //: Fichier destination
         bool          pHeader) //: Faut-il copier l'entête+parsing ou payload ?
{
  ASSERTpc (pMedia, false, cExNullPtr)
  ASSERTpc (pFile,  false, cExNullPtr)

  bool ok = true;

  if (pHeader)
  {
    ok =       (fwrite (cBegMediaString, cBegMediaLength,       1, pFile) == 1);
    ok = ok && (fwrite (pMedia,          sizeof (sPaquetMedia), 1, pFile) == 1);
  }

  if (pMedia->payloadSize > 0)
  {
    ok = ok && (fwrite (pMedia->payload, pMedia->payloadSize, 1, pFile) == 1);
  }

  return ok;
}

// Récupère le contenu d'un paquet média depuis un fichier. Celui-ci doit ------
// avoir été enregistré avec l'entête+parsing (pHeader=true) !            ------
//> Pointeur sur le paquet média récupéré ou 0 si problème
sPaquetMedia* sPaquetMedia_FromFile
  (FILE* pFile) //: Fichier source
{
  ASSERTpc (pFile, 0, cExNullPtr)

  fpos_t pos;
  fgetpos (pFile, &pos);

  bool ok = fread (cBegMediaBuffer, 1, cBegMediaLength, pFile) ==
    cBegMediaLength;

  IFNOT_OP (ok, fsetpos (pFile, &pos), 0) // Lecture ratée ?

  cBegMediaBuffer[cBegMediaLength] = 0;

  // Le flux est bien préfixé ... alors ça doit être bon !
  ok = strcmp (cBegMediaBuffer, cBegMediaString) == 0;
  IFNOT_OP (ok, fsetpos (pFile, &pos), 0) // Comparaison réussie ?

  sPaquetMedia* _media = malloc (sizeof (sPaquetMedia));
  IFNOT_OP     (_media, fsetpos (pFile, &pos), 0) // Allocation ratée ?

  ok = fread (_media, 1, sizeof (sPaquetMedia), pFile) == sizeof (sPaquetMedia);
  IFNOT_OP (ok, fsetpos (pFile, &pos); free (_media), 0) // Lecture ratée ?

  _media->payload = 0;

  if (_media->payloadSize > 0)
  {
    _media->payload = malloc (_media->payloadSize);
    IFNOT_OP (_media->payload, fsetpos(pFile,&pos); free(_media), 0) // Alloc ?

    ok = fread (_media->payload, 1, _media->payloadSize, pFile) ==
          _media->payloadSize;
    IFNOT_OP (ok, fsetpos(pFile,&pos); sPaquetMedia_Release(_media), 0) // Lec ?
  }

  return _media;
}
