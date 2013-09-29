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

#include "../smpte.h"

// Fonctions privées ===========================================================

// Fonction appelée par l'arbre rb lors de la suppression d'un noeud -----------
void ReleasePaquetMediaFunc
  (uint32_t pKey,   //: Clé (médiaNo) du noeud à supprimer
   void*    pValue) //: Valeur du noeud à supprimer
{
  if (pValue) sPaquetMedia_Release (pValue);
}

// Fonction appelée par l'arbre rb lors de l'affichage d'un noeud --------------
void PrintPaquetMediaFunc
  (void* pValue) //: Valeur du noeud à afficher
{
  if (pValue) sPaquetMedia_Print (pValue);
}

// Fonctions publiques =========================================================

// Créé un buffer média                                                     ----
// Remarque : ne pas oublier de faire le ménage avec sBufferMedia_Release ! ----
//> Nouveau buffer média
sBufferMedia sBufferMedia_New()
{
  sBufferMedia _buffer;

  _buffer.rbtree  = sRbTree_New (ReleasePaquetMediaFunc, PrintPaquetMediaFunc);
  _buffer.reading = 0;
  _buffer.arrival = 0;
  _buffer.readingNx = MEDIA_NX_NULL;
  _buffer.arrivalNx = MEDIA_NX_NULL;

  return _buffer;
}

// Libère la mémoire allouée par un buffer média -------------------------------
void sBufferMedia_Release
  (sBufferMedia* pBuffer) //: Buffer à vider
{
  ASSERTpc (pBuffer,, cExNullPtr)

  sRbTree_Release (&pBuffer->rbtree);
}

// Affiche le contenu d'un buffer média ----------------------------------------
void sBufferMedia_Print
  (const sBufferMedia* pBuffer,  //: Buffer à afficher
         bool          pBuffers) //: Faut-il afficher le contenu des buffers ?
{
  ASSERTpc (pBuffer,, cExNullPtr)

  sRbTree_Print (&pBuffer->rbtree, 2, pBuffers);
}

// Est "l'équivalent" de media_receive de VLC : ajoute le paquet média au    ---
// buffer, celui-ci étant trié par ordre de médiaNo.                         ---
// Remarque: Le buffer s'approprie le paquet média, cela veut dire que c'est ---
// le buffer média qui s'occupera de libérer la mémoire prise par le paquet! ---
//> Status de l'opération / ajout réussi ?
bool sBufferMedia_AddByReference
  (sBufferMedia* pBuffer, //: Buffer à modifier
   sPaquetMedia* pMedia,  //: Paquet à ajouter
   bool pOver) //: Faut-il écraser un noeud qui porterait déjà key=médiaNo ?
{
  ASSERTpc (pBuffer, false, cExNullPtr)
  ASSERTpc (pMedia,  false, cExNullPtr)

  // Ajoute key=méedia.médiaNo; value=média dans l'arbre rouge-noire
  pBuffer->arrival = sRbTree_AddByReference
                      (&pBuffer->rbtree, pMedia->mediaNo, pMedia, pOver);
  IFNOT (pBuffer->arrival, false) // Ajout raté ?

  pBuffer->arrivalNx = sMediaNo_to_sMediaNx (pMedia->mediaNo);

  return true;
}

// Retourne un pointeur vers le paquet média portant un certain médiaNo --------
//> Pointeur sur le paquet média lié à key=pMediaNo ou 0 si aucun de trouvé
sPaquetMedia* sBufferMedia_Find
  (const sBufferMedia* pBuffer,  //: Buffer à traiter
         sMediaNo      pMediaNo) //: Paramètre de recherche
{
  ASSERTpc (pBuffer, 0, cExNullPtr)

  return sRbTree_Lookup (&pBuffer->rbtree, pMediaNo);
}

// Initalise la boucle foreach like sur le buffer média ------------------------
//> Status de l'opération / Est-ce que ForeachData est (un paquet) valide ?
bool sBufferMedia_InitForeach
  (sBufferMedia* pBuffer,  //: Buffer à traiter
   bool          pReverse) //: Faut-il effectuer la boucle à " l'envers " ?
{
  ASSERTpc (pBuffer, false, cExNullPtr)

  return sRbTree_InitForeach (&pBuffer->rbtree, pReverse);
}

// Continue la boucle foreach like sur le buffer média -------------------------
//> Status de l'opération / Est-ce que ForeachData est (un paquet) valide ?
bool sBufferMedia_NextForeach
  (sBufferMedia* pBuffer) //: Buffer à traiter
{
  ASSERTpc (pBuffer, false, cExNullPtr)

  return sRbTree_NextForeach (&pBuffer->rbtree);
}

// Retourne le médiaNo du paquet média (pointé) par la boucle foreach ----------
//> médiaNo du paquet média (pointé) par la boucle foreach ou 0 si inactif
sMediaNo sBufferMedia_ForeachKey
  (const sBufferMedia* pBuffer) //: Buffer à traiter
{
  ASSERTpc (pBuffer, 0, cExNullPtr)

  return sRbTree_ForeachKey (&pBuffer->rbtree);
}

// Retourne un pointeur sur le paquet média (pointé) par la boucle foreach -----
//> Pointeur sur la paquet média (pointé) par la boucle foreach ou 0 si inactif
sPaquetMedia* sBufferMedia_ForeachValue
  (const sBufferMedia* pBuffer) //: Buffer à traiter
{
  ASSERTpc (pBuffer, 0, cExNullPtr)

  return sRbTree_ForeachValue (&pBuffer->rbtree);
}

// Calcule si un médiaNo se situe entre les limites d'arrivalNx & readingNx ----
// La valeur de médiaNo est modulaire donc 2 cas de figures se présentent : ----
// cas [1] 0 false [readingNo] true [arrivalNo] false MAX                   ----
// cas [2] 0 true [arrivalNo] false [readingNo] true MAX                    ----
//> Est-ce que médiaNo se situe dans la zone entre lecture et arrivée (buffer) ?
bool sBufferMedia_IsMediaNoInBuffer
  (const sBufferMedia* pBuffer,  //: Buffer à traiter
         sMediaNo      pMediaNo) //: MédiaNo à tester
{
  ASSERTpc (pBuffer, false, cExNullPtr)

  // 0 true... [arrivalNo] ...false MAX
  if (pBuffer->readingNx.null)
  {
    return pBuffer->arrivalNx.null ? false : pMediaNo <= pBuffer->arrivalNx.v;
  }

  // 0 false... [readingNo] ...true... [arrivalNo] ...false MAX
  if (pBuffer->readingNx.v <= pBuffer->arrivalNx.v)
  {
    return pMediaNo >= pBuffer->readingNx.v &&
           pMediaNo <= pBuffer->arrivalNx.v;
  }

  // 0 true... [arrivalNo] ...false... [readingNo] ...true MAX
  return pMediaNo <= pBuffer->arrivalNx.v ||
         pMediaNo >= pBuffer->readingNx.v;
}

// Simule la " lecture " d'un paquet du buffer (enregistre son payload dans ----
// un fichier, si pDestFile!=0, et supprime le paquet pointé par lectureNx) ----
//> Status de l'opération / y a-t-il eu un "déplacement" dans le buffer ?
bool sBufferMedia_ReadMedia
  (sBufferMedia* pBuffer,   //: Buffer à vider
   FILE        * pDestFile, //: Pour enregistrer le payload média (0= pas enreg)
   sMediaNo    * pReadedNo) //: MédiaNo du média lu (supprimé)
{
  ASSERTpc (pBuffer, false, cExNullPtr)

  // Initialise la position de lecture si nécessaire
  if (pBuffer->reading == 0)
  {
    pBuffer->reading = sRbTree_First (&pBuffer->rbtree);
  }

  ASSERTc (pBuffer->reading, false, cExRbTreeFirst)

  // Initialise le médiaNo de lecture si nécessaire
  if (pBuffer->readingNx.null)
  {
    sPaquetMedia* _media = pBuffer->reading->value;
    ASSERTc      (_media, false, cExRbTreeValue)

    pBuffer->readingNx = sMediaNo_to_sMediaNx (_media->mediaNo);
  }

  ASSERTc (!pBuffer->readingNx.null, false, cExLectureNxValue)

  // Point sur l'élément suivant (pour la lecture)
  sRbNode* _oldNd = pBuffer->reading;
  sMediaNx _oldNx = pBuffer->readingNx;

  *pReadedNo = _oldNx.v;

  pBuffer->reading   = sRbNode_Next         (pBuffer->reading);
  pBuffer->readingNx = sMediaNo_to_sMediaNx (pBuffer->readingNx.v + 1);

  // Recherche le noeud qui devrait être présent !
  bool   _trouve = sRbTree_Lookup (&pBuffer->rbtree, _oldNx.v) != 0;
  IFNOT (_trouve, false) // Noeud introuvable ?

  // L'élément est présent !

  // Enregistre le payload du paquet média dans un fichier si demandé
  if (pDestFile != 0)
  {
    bool ok = sPaquetMedia_ToFile (sRbNode_GetValue (_oldNd), pDestFile, false);
    ASSERT (ok, false, cExMediaToFile, _oldNx.v)
  }

  // Supprime l'élément du buffer
  bool    ok = sRbTree_Delete (&pBuffer->rbtree, _oldNx.v);
  ASSERT (ok, false, cExMediaDelete, _oldNx.v)

  return true;
}
