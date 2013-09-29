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

#include "../smpte.h"

// Fonctions privées ===========================================================

// Libère un noeud du type cross -----------------------------------------------
void ReleaseCrossFunc
  (uint32_t pKey,   //: Clé (médiaNo) du noeud à supprimer
   void*    pValue) //: Valeur (cross) du noeud à supprimer
{
  if (pValue) sCrossFec_Release (pValue);
}

// Libère un noeud du type wait ------------------------------------------------
void ReleaseWaitFunc
  (uint32_t pKey,   //: Clé (fecNo) du noeud à supprimer
   void*    pValue) //: Valeur (wait) du noeud à supprimer
{
  if (pValue) sWaitFec_Release (pValue);
}

// Affiche un noeud du type cross ----------------------------------------------
void PrintCrossFunc
  (void* pValue) //: Valeur (cross) du noeud
{
  if (pValue) sCrossFec_Print (pValue);
}

// Affiche un noeud du type wait -----------------------------------------------
void PrintWaitFunc
  (void* pValue) //: Valeur (wait) du noeud
{
  if (pValue) sWaitFec_Print (pValue);
}

// Fonctions publiques =========================================================

// Créé un buffer de FEC -------------------------------------------------------
// Remarque : ne pas oublier de faire le ménage avec sBufferFec_Release ! ------
//> Nouveau buffer de FEC
sBufferFec sBufferFec_New()
{
  sBufferFec _buffer;

  _buffer.cross     = sRbTree_New (ReleaseCrossFunc, PrintCrossFunc);
  _buffer.wait[COL] = sRbTree_New (ReleaseWaitFunc,  PrintWaitFunc);
  _buffer.wait[ROW] = sRbTree_New (ReleaseWaitFunc,  PrintWaitFunc);

  return _buffer;
}

// Libère la mémoire allouée par un buffer de FEC ------------------------------
void sBufferFec_Release
  (sBufferFec* pBuffer) //: Buffer à vider
{
  ASSERTpc (pBuffer,, cExNullPtr)

  sRbTree_Release (&pBuffer->cross);
  sRbTree_Release (&pBuffer->wait[COL]);
  sRbTree_Release (&pBuffer->wait[ROW]);
}

// Insère un nouveau cross dans le buffer de FEC -------------------------------
//> Status de l'opération / ajout réussi ?
bool sBufferFec_AddCrossByReference
  (sBufferFec* pBuffer,  //: Buffer à modifier
   sMediaNo    pMediaNo, //: Lier le cross à ce médiaNo
   sCrossFec*  pCross,   //: Cross à ajouter
   bool pOver) //: Faut-il écraser un noeud qui porterait déjà key=médiaNo ?
{
  ASSERTpc (pBuffer, false, cExNullPtr)
  ASSERTpc (pCross,  false, cExNullPtr)

  // Enregistre le cross dans bufferFec.cross
  return sRbTree_AddByReference
    (&pBuffer->cross, pMediaNo, (void*)pCross, pOver) != 0;
}

// Retrouve un cross lié au médiaNo donné en paramètre -------------------------
//> Pointeur sur le cross lié à key=médiaNo ou 0 si inexistant
sCrossFec* sBufferFec_FindCross
  (const sBufferFec* pBuffer,  //: Buffer à traiter
         sMediaNo    pMediaNo) //: Paramètre de recherche
{
  ASSERTpc (pBuffer, 0, cExNullPtr)

  return sRbTree_Lookup (&pBuffer->cross, pMediaNo);
}

// Supprime un cross -----------------------------------------------------------
//> Status de l'opération
bool sBufferFec_DeleteCross
  (sBufferFec* pBuffer,  //: Buffer à modifier
   sMediaNo    pMediaNo) //: MédiaNo du cross à supprimer
{
  ASSERTpc (pBuffer, false, cExNullPtr)

  return sRbTree_Delete (&pBuffer->cross, pMediaNo);
}

// Insère un nouveau wait ------------------------------------------------------
//> Status de l'opération
bool sBufferFec_AddWaitByReference
  (sBufferFec* pBuffer, //: Buffer à modifier
   sWaitFec  * pWait,   //: Wait à ajouter
   bool pOver) //: Faut-il écraser un noeud qui porterait déjà key=pWait.fecNo ?
{
  ASSERTpc (pBuffer, false, cExNullPtr)
  ASSERTpc (pWait,   false, cExNullPtr)

  // Enregistre le wait dans bufferFec.wait[D]

  return sRbTree_AddByReference (&pBuffer->wait[pWait->D],
                                 pWait->fecNo, (void*)pWait, pOver) != 0;
}

// Retrouve un wait lié à une direction et un fecNo donné en paramètre ---------
//> Pointeur sur le wait[direction=D] lié à key=fecNo ou 0 si inexistant
sWaitFec* sBufferFec_FindWait
  (const sBufferFec* pBuffer, //: Buffer à traiter
         eFecD       pD,      //: Faut-il chercher dans wait[COL] ou wait[ROW] ?
         sFecNo      pFecNo)  //: Paramètre de recherche
{
  ASSERTpc (pBuffer, 0, cExNullPtr)

  return sRbTree_Lookup (&pBuffer->wait[pD], pFecNo);
}

// Supprimer un wait du buffer de FEC ------------------------------------------
//> Status de l'opération / suppression réussie ?
bool sBufferFec_DeleteWait
  (sBufferFec* pBuffer, //: Buffer à modifier
   eFecD       pD,      //: Faut-il opérer dans wait[COL] ou wait[ROW] ?
   sFecNo      pFecNo)  //: fecNo du wait à supprimer
{
  ASSERTpc (pBuffer, false, cExNullPtr)

  return sRbTree_Delete (&pBuffer->wait[pD], pFecNo);
}

// Fait le ménage en supprimant toutes traces concernant un paquet de FEC   ----
// particulier. Ce nettoyage est nécessaire au cas où certains paquets de   ----
// FEC seraient devenus obsolètes. Par exemple il serait inutile de         ----
// récupérer des paquets média déjà " sautés " (manquants à la lecture) par ----
// le player...                                                             ----
//> Status de l'opération / nettoyage réussi ?
bool sBufferFec_DeleteCrossAndWait
  (sBufferFec* pBuffer, //: Buffer à modifier
   eFecD       pD,      //: Faut-il opérer dans wait[COL] ou wait[ROW] ?
   sFecNo      pFecNo)  //: fecNo du wait (cross retrouvés depuis le wait)
{
  ASSERTpc (pBuffer, false, cExNullPtr)

  sWaitFec* _wait = sBufferFec_FindWait (pBuffer, pD, pFecNo);
  IFNOT    (_wait, false) // Wait introuvable ?

  uint8_t j;

  for (j = 0; j < _wait->number; j++)
  {
    sMediaNx _mediaUpd = sWaitFec_GetManque (_wait, j + 1);
    ASSERTc (!_mediaUpd.null, false, cExAlgorithm)

    sCrossFec* _crossUpd = sBufferFec_FindCross (pBuffer, _mediaUpd.v);
    if (!_crossUpd) continue;

    if (pD == COL) { _crossUpd->colNx = FEC_NX_NULL; }
    else           { _crossUpd->rowNx = FEC_NX_NULL; }

    // Entrée dans cross devenue inutile
    if (_crossUpd->colNx.null && _crossUpd->rowNx.null)
    {
      bool     ok = sBufferFec_DeleteCross (pBuffer, _mediaUpd.v);
      ASSERTc (ok, false, cExAlgorithm)
    }
  }

  return sBufferFec_DeleteWait (pBuffer, pD, pFecNo);
}

// Affiche le contenu du buffer cross ------------------------------------------
void sBufferFec_PrintCross
  (const sBufferFec* pBuffer,  //: Buffer à modifier
         bool        pBuffers) //: Faut-il afficher le contenu des buffers ?
{
  ASSERTpc (pBuffer,, cExNullPtr)

  sRbTree_Print (&pBuffer->cross, 2, pBuffers);
}

// Affiche le contenu du buffer wait -------------------------------------------
void sBufferFec_PrintWait
  (const sBufferFec* pBuffer,  //: Buffer à modifier
         eFecD       pD,       //: Faut-il afficher wait[COL] ou wait[ROW] ?
         bool        pBuffers) //: Faut-il afficher le contenu des buffers ?
{
  ASSERTpc (pBuffer,, cExNullPtr)

  sRbTree_Print (&pBuffer->wait[pD], 2, pBuffers);
}

// Initalise la boucle foreach like sur le buffer de FEC (cross) ---------------
//> Status de l'opération / Est-ce que ForeachData est (un cross) valide ?
bool sBufferFec_InitForeachCross
  (sBufferFec* pBuffer,  //: Buffer à traiter
   bool        pReverse) //: Faut-il effectuer la boucle à " l'envers " ?
{
  ASSERTpc (pBuffer, false, cExNullPtr)

  return sRbTree_InitForeach (&pBuffer->cross, pReverse);
}

// Continue la boucle foreach like sur le buffer de FEC (cross) ----------------
//> Status de l'opération / Est-ce que ForeachData est (un cross) valide ?
bool sBufferFec_NextForeachCross
  (sBufferFec* pBuffer) //: Buffer à traiter
{
  ASSERTpc (pBuffer, false, cExNullPtr)

  return sRbTree_NextForeach (&pBuffer->cross);
}

// Retourne le médiaNo lié au cross (pointé) par la boucle foreach -------------
//> médiaNo lié au cross (pointé) par la boucle foreach ou 0 si inactif
sMediaNo sBufferFec_ForeachKeyCross
  (const sBufferFec* pBuffer) //: Buffer à traiter
{
  ASSERTpc (pBuffer, 0, cExNullPtr)

  return sRbTree_ForeachKey (&pBuffer->cross);
}

// Retourne un pointeur sur le cross (pointé) par la boucle foreach ------------
//> Pointeur sur le cross (pointé) par la boucle foreach ou 0 si inactif
sCrossFec* sBufferFec_ForeachValueCross
  (const sBufferFec* pBuffer) //: Buffer à traiter
{
  ASSERTpc (pBuffer, 0, cExNullPtr)

  return sRbTree_ForeachValue (&pBuffer->cross);
}
