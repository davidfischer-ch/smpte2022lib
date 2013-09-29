
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include <vlc_common.h>

#include "smpte2022.h"
#include "smpte2022media.h"

//------------------------------------------------------------------------------
//              PAQUET MEDIA
//------------------------------------------------------------------------------
// Constantes publiques ========================================================

const sMediaNx MEDIA_NX_NULL={0, true}; //. Valeur par défaut de médiaNx (~null)

// Fonctions publiques =========================================================

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
  if (_media == NULL) // Allocation ratée ?
    return NULL;

  _media->mediaNo     = pMediaNo;
  _media->timeStamp   = pTimeStamp;
  _media->payloadType = pPayloadType;
  _media->payloadSize = pPayloadSize;

  if (pPayloadSize == 0) return _media;

  _media->payload = malloc (pPayloadSize);
  if (_media->payload == NULL) // Allocation ratée ?
  {
    free (_media);
    return NULL;
  }

  void* ok = memcpy (_media->payload, pPayload, pPayloadSize);
  if (ok == NULL) // Copie ratée ?
  {
    sPaquetMedia_Release (_media);
    return NULL;
  }

  return _media;
}

// Libère la mémoire allouée par un paquet média -------------------------------
void sPaquetMedia_Release
  (sPaquetMedia* pMedia) //: Paquet à vider
{
  if (pMedia == NULL)
    return;

/*if (pMedia->payload != NULL)
  free (pMedia->payload); //MODIF pour ne pas détruire le payload, car VLC s'en charge
*/
  free (pMedia);
}

// Retourne un médiaNx à partir d'un médiaNo -----------------------------------
//> Le numéro de séquence médiaNx équivalent
inline sMediaNx sMediaNo_to_sMediaNx
  (sMediaNo pMediaNo) //: Le médiaNo à convertir
{
  return (sMediaNx) {pMediaNo, false};
}


//------------------------------------------------------------------------------
//              BUFFER MEDIA
//------------------------------------------------------------------------------
// Fonctions privées ===========================================================

// Fonction appelée par l'arbre rb lors de la suppression d'un noeud -----------
void ReleasePaquetMediaFunc
  (uint32_t pKey,   //: Clé (médiaNo) du noeud à supprimer
   void*    pValue) //: Valeur du noeud à supprimer
{
  if (pValue)
    sPaquetMedia_Release (pValue);
}

// Fonctions publiques =========================================================

// Créé un buffer média                                                     ----
// Remarque : ne pas oublier de faire le ménage avec sBufferMedia_Release ! ----
//> Nouveau buffer média
sBufferMedia_t *sBufferMedia_New()
{
  sBufferMedia_t *_buffer = malloc (sizeof (sBufferMedia_t));
     if (_buffer == NULL)
        return NULL;
  _buffer->rbtree  = sRbTree_New (ReleasePaquetMediaFunc);
  _buffer->reading = 0;
  _buffer->arrival = 0;
  _buffer->readingNx = MEDIA_NX_NULL;
  _buffer->arrivalNx = MEDIA_NX_NULL;

  return _buffer;
}

// Libère la mémoire allouée par un buffer média -------------------------------
void sBufferMedia_Release
  (sBufferMedia_t* pBuffer) //: Buffer à vider
{
  if (pBuffer == NULL)
     return;
  sRbTree_Release (&pBuffer->rbtree);
}


// Est "l'équivalent" de media_receive de VLC : ajoute le paquet média au    ---
// buffer, celui-ci étant trié par ordre de médiaNo.                         ---
// Remarque: Le buffer s'approprie le paquet média, cela veut dire que c'est ---
// le buffer média qui s'occupera de libérer la mémoire prise par le paquet! ---
//> Status de l'opération / ajout réussi ?
bool sBufferMedia_AddByReference
  (sBufferMedia_t* pBuffer, //: Buffer à modifier
   sPaquetMedia* pMedia,  //: Paquet à ajouter
   bool pOver) //: Faut-il écraser un noeud qui porterait déjà key=médiaNo ?
{
  if (pBuffer == NULL || pMedia == NULL)
     return false;

  // Ajoute key=méedia.médiaNo; value=média dans l'arbre rouge-noire
  pBuffer->arrival = sRbTree_AddByReference
                      (&pBuffer->rbtree, pMedia->mediaNo, pMedia, pOver);
  if (pBuffer->arrival == NULL) // Ajout raté ?
     return false;

  pBuffer->arrivalNx = sMediaNo_to_sMediaNx (pMedia->mediaNo);

  return true;
}

// Retourne un pointeur vers le paquet média portant un certain médiaNo --------
//> Pointeur sur le paquet média lié à key=pMediaNo ou 0 si aucun de trouvé
sPaquetMedia* sBufferMedia_Find
  (const sBufferMedia_t* pBuffer,  //: Buffer à traiter
         sMediaNo      pMediaNo) //: Paramètre de recherche
{
  if (pBuffer == NULL)
     return NULL;

  return sRbTree_Lookup (&pBuffer->rbtree, pMediaNo);
}

// Initalise la boucle foreach like sur le buffer média ------------------------
//> Status de l'opération / Est-ce que ForeachData est (un paquet) valide ?
bool sBufferMedia_InitForeach
  (sBufferMedia_t* pBuffer,  //: Buffer à traiter
   bool          pReverse) //: Faut-il effectuer la boucle à " l'envers " ?
{
  if (pBuffer == NULL)
     return false;

  return sRbTree_InitForeach (&pBuffer->rbtree, pReverse);
}

// Continue la boucle foreach like sur le buffer média -------------------------
//> Status de l'opération / Est-ce que ForeachData est (un paquet) valide ?
bool sBufferMedia_NextForeach
  (sBufferMedia_t* pBuffer) //: Buffer à traiter
{
  if (pBuffer == NULL)
     return false;

  return sRbTree_NextForeach (&pBuffer->rbtree);
}

// Retourne le médiaNo du paquet média (pointé) par la boucle foreach ----------
//> médiaNo du paquet média (pointé) par la boucle foreach ou 0 si inactif
sMediaNo sBufferMedia_ForeachKey
  (const sBufferMedia_t* pBuffer) //: Buffer à traiter
{
  if (pBuffer == NULL)
     return 0;

  return sRbTree_ForeachKey (&pBuffer->rbtree);
}

// Retourne un pointeur sur le paquet média (pointé) par la boucle foreach -----
//> Pointeur sur la paquet média (pointé) par la boucle foreach ou 0 si inactif
sPaquetMedia* sBufferMedia_ForeachValue
  (const sBufferMedia_t* pBuffer) //: Buffer à traiter
{
  if (pBuffer == NULL)
     return NULL;

  return sRbTree_ForeachValue (&pBuffer->rbtree);
}

// Calcule si un médiaNo se situe entre les limites d'arrivalNx & readingNx ----
// La valeur de médiaNo est modulaire donc 2 cas de figures se présentent : ----
// cas [1] 0 false [readingNo] true [arrivalNo] false MAX                   ----
// cas [2] 0 true [arrivalNo] false [readingNo] true MAX                    ----
//> Est-ce que médiaNo se situe dans la zone entre lecture et arrivée (buffer) ?
bool sBufferMedia_IsMediaNoInBuffer
  (const sBufferMedia_t* pBuffer,  //: Buffer à traiter
         sMediaNo      pMediaNo) //: MédiaNo à tester
{
  if (pBuffer == NULL)
     return false;

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

// Simule la " lecture " d'un paquet du buffer / supprime le pointeur ----
//> Status de l'opération / y a-t-il eu un "déplacement" dans le buffer ?
bool sBufferMedia_ReadMedia
  (sBufferMedia_t* pBuffer,   //: Buffer à vider
   sMediaNo    * pReadedNo) //: MédiaNo du média lu (supprimé)
{
  if (pBuffer == NULL)
     return false;

  // Initialise la position de lecture si nécessaire
  if (pBuffer->reading == 0)
  {
    pBuffer->reading = sRbTree_First (&pBuffer->rbtree);
  }
  if (pBuffer->reading == NULL)
    return false;

  // Initialise le médiaNo de lecture si nécessaire
  if (pBuffer->readingNx.null)
  {
    sPaquetMedia* _media = pBuffer->reading->value;
    if (_media == NULL)
      return false;

    pBuffer->readingNx = sMediaNo_to_sMediaNx (_media->mediaNo);
  }

  if (pBuffer->readingNx.null)
    return false;

  // Point sur l'élément suivant (pour la lecture)
  sRbNode* _oldNd = pBuffer->reading;
  sMediaNx _oldNx = pBuffer->readingNx;

  *pReadedNo = _oldNx.v;

  pBuffer->reading   = sRbNode_Next         (pBuffer->reading);
  pBuffer->readingNx = sMediaNo_to_sMediaNx (pBuffer->readingNx.v + 1);

  // Recherche le noeud qui devrait être présent !
  bool   _trouve = sRbTree_Lookup (&pBuffer->rbtree, _oldNx.v) != 0;
  if (! _trouve )// Noeud introuvable ?
    return false;

  // L'élément est présent !

  // Supprime l'élément du buffer
  bool    ok = sRbTree_Delete (&pBuffer->rbtree, _oldNx.v);
  if (! ok )
    return false;

  return true;
}
