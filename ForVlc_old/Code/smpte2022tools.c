
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
#include "smpte2022tools.h"

//------------------------------------------------------------------------------
//              CHAMP BITS
//------------------------------------------------------------------------------
// Fonctions publiques =========================================================

// Création d'un nouveau champ de bits -----------------------------------------
//> Nouveau champ de bits (initialisé à 0)
sChampBits sChampBits_New()
{
  sChampBits _champ;

  memset (&_champ, 0, sizeof (sChampBits));
  return   _champ;
}

// Lecture de l'état d'un des bits du champ de bits ----------------------------
//> Etat du bit
bool sChampBits_GetBit
  (const sChampBits* pChamp, //: Champ à traiter
   uint8_t           pNo)    //: Numéro (position) du bit à lire
{
  if (pChamp == NULL)
    return false;

  uint8_t no  = pNo / CHAMP_TAILLE_UNITE;
  uint8_t bit = pNo - no * CHAMP_TAILLE_UNITE;

  return (pChamp->buffer[no] & (1 << bit)) > 0;
}

// Changer l'état d'un des bits du champ de bits -------------------------------
void sChampBits_SetBit
  (sChampBits* pChamp,  //: Champ à traiter
   uint8_t     pNo,     //: Numéro (position) du bit à changer
   bool        pValeur) //: Valeur à affecter
{
  if (pChamp == NULL)
    return;

  uint8_t  no  = pNo      / CHAMP_TAILLE_UNITE;
  uint8_t  bit = pNo - no * CHAMP_TAILLE_UNITE;
  uint32_t val = pChamp->buffer[no];

  pChamp->buffer[no] = pValeur ? val |  (1 << bit) : // Or bit à 1
                                 val & ~(1 << bit);  // And not bit à 1
}

// Retourne le numéro du Xème bit à 1 (en effectuant la recherche du MSB au ----
// LSB et du LSB au MSB si pDirection=LSB_FIRST)                            ----
//> Position du bit à 1 ou -1 si non trouvé
signed sChampBits_GetOne
  (const sChampBits* pChamp,     //: Champ à traiter
   uint8_t           pNo,        //: Numéro (Xème) du bit à trouver
   eDir              pDirection) //: LSB_FIRST = LSB->MSB, MSB_FIRST = MSB->LSB
{
  if (pChamp == NULL)
    return 0;

  signed no, nb, n = 0;

  if (pDirection == LSB_FIRST)
  {
    for (no = 0; no < (signed)CHAMP_NOMBRE_UNITE; no++)
    {
      if (pChamp->buffer[no] == 0) continue;

      for (nb = 0; nb < (signed)CHAMP_TAILLE_UNITE; nb++)
      {
        if (pChamp->buffer[no] & (1 << nb))
        {
          if (++n == pNo) return no*CHAMP_TAILLE_UNITE+nb;
        }
      }
    }
  }
  else if (pDirection == MSB_FIRST)
  {
    for (no = (signed)(CHAMP_NOMBRE_UNITE-1); no >= 0; no--)
    {
      if (pChamp->buffer[no] == 0) continue;

      for (nb = (signed)(CHAMP_TAILLE_UNITE-1); nb >= 0; nb--)
      {
        if (pChamp->buffer[no] & (1 << nb))
        {
          if (++n == pNo) return no*CHAMP_TAILLE_UNITE+nb;
        }
      }
    }
  }

  return -1;
}

// Incrémente la valeur entière non signée représentée par le champ de bits ----
void sChampBits_Incremente
  (sChampBits* pChamp) //: Champ à traiter
{
  if (pChamp == NULL)
    return;

  unsigned no;

  for (no = 0; no < CHAMP_NOMBRE_UNITE; no++)
  {
    pChamp->buffer[no]++;

    // Aucune retenue
    if (pChamp->buffer[no] > 0) break;
  }
}

// Compare les valeurs entières non signées représentées par les champs --------
//> Résultat: <0 si champ1 < champ2 / 0 si égalité / >0 si champ1 > champ2
signed sChampBits_Compare
  (const sChampBits* pChamp1, //: Champ à comparer
   const sChampBits* pChamp2) //: Champ à comparer
{
  if (pChamp1 == NULL || pChamp2 == NULL)
    return 0;

  signed no;

  for (no = CHAMP_NOMBRE_UNITE-1; no >= 0; no--)
  {
    if (pChamp1->buffer[no] == pChamp2->buffer[no]) continue;
    if (pChamp1->buffer[no] >  pChamp2->buffer[no]) return 1;

    return -1;
  }

  return 0;
}

// Additionne les valeurs entières non signées représentées par les champs -----
//> Champ résultat de l'addition
sChampBits sChampBits_Add
  (const sChampBits* pChamp1, //: Champ à additionner
   const sChampBits* pChamp2) //: Champ à additionner
{
  if (pChamp1 == NULL || pChamp2 == NULL)
    return sChampBits_New();

  sChampBits _resultat;

  uint32_t _retenue = 0;

  unsigned no;

  for (no = 0; no < CHAMP_NOMBRE_UNITE; no++)
  {
    uint32_t v1 = pChamp1->buffer[no];
    uint32_t v2 = pChamp2->buffer[no];
    uint32_t v3 = v1 + v2;

    _resultat.buffer[no] = v3 + _retenue;

    if (_retenue)
    {
      _retenue =
      (v1 > CHAMP_VALEUR_MAX - v2)       ? 1 /*ret car v1+v2     > max */ :
     ((v3 > CHAMP_VALEUR_MAX - _retenue) ? 1 /*ret car v1+v2+ret > max */ : 0);
    }
    else
    {
      _retenue = (v1 > CHAMP_VALEUR_MAX - v2) ? 1 : 0;
    }
  }

  return _resultat;
}


//------------------------------------------------------------------------------
//              LISTE DOUBLEMENT CHAINEE
//------------------------------------------------------------------------------
// Constantes publiques ========================================================

const sLinkedElmnt INIT_LINKED_ELMNT = //. Valeur initiale d'un élément
  {0, 0, 0};

// Constantes privées ==========================================================

const sLinkedList INIT_LINKED_LIST = //. Valeur initiale d'une liste chaînée
  {0, 0, 0, 0, 0, 0, 0, 0};

// Fonctions publiques =========================================================

// Initialise une nouvelle liste doublement chaînée ----------------------------
//> Nouvelle liste doublement chaînée
sLinkedList sLinkedList_New
  (sLinkedReleaseFunc pReleaseFunc) //: Fonction de suppression d'élément
{
  sLinkedList l = INIT_LINKED_LIST;
  l.releaseFunc = pReleaseFunc;
  return l;
}

// Libère la mémoire allouée par une liste doublement chaînée ------------------
void sLinkedList_Release
  (sLinkedList* pList) //: Liste à vider
{
  if (pList == NULL)
    return;

  sLinkedElmnt* _element = pList->first;

  while (_element)
  {
    sLinkedElmnt* _next = _element->next;

    if (pList->releaseFunc != 0)
    {
      pList->releaseFunc (_element->value);
    }

    // Freeze the memory taken by (each) the element
    free (_element);

    _element = _next;
  }

  pList->first = 0;
  pList->last  = 0;
  pList->count = 0;
  pList->foreachElmnt = 0;
  pList->foreachCount = 0;
}

// Créé et ajoute (en dernier) un nouvel élément à la liste chaînée ------------
//> Pointeur sur le nouvel élément ou 0 si problème
sLinkedElmnt* sLinkedList_AppendByReference
  (sLinkedList* pList,  //: Liste à modifier
   void       * pValue) //: Ce que nous voulons stocker dans l'élément
{
  if (pList == NULL)
    return NULL;

  sLinkedElmnt* _newElement = malloc (sizeof (sLinkedElmnt));
  if (_newElement == NULL) // Allocation ratée
    return NULL;

  _newElement->value = pValue;
  _newElement->prev  = pList->last;
  _newElement->next  = 0;

  if (pList->first == 0) pList->first      = _newElement;
  if (pList->last)       pList->last->next = _newElement;

  pList->last = _newElement;
  pList->count++;

  return _newElement;
}

// Créé et insère un nouvel élément à la liste doublement chaînée --------------
// Remarque : pInHere doit être un élément de pList sinon bug !   --------------
//> Pointeur sur le nouvel élément ou 0 si problème
sLinkedElmnt* sLinkedList_InsertByReference
  (sLinkedList * pList,   //: Liste à modifier
   sLinkedElmnt* pInHere, //: Position (prendre place avant celui-ci svp)
   void        * pValue)  //: Ce que nous voulons stocker dans l'élément
{
  if (pList == NULL)
    return NULL;

  sLinkedElmnt* _newElement = malloc (sizeof (sLinkedElmnt));
  if (_newElement == NULL) // Allocation ratée
    return NULL;

  _newElement->value = pValue;
  _newElement->prev  = pInHere->prev;
  _newElement->next  = pInHere;

  if (pInHere->prev)
  {
    pInHere->prev->next = _newElement; // [prev]<->[new]<->[inHere]
  }
  else
  {
    pList->first = _newElement; // [new is first]<->[inHere]
  }

  pInHere->prev = _newElement;

  if (pList->first == 0) pList->first = _newElement;
  if (pList->last  == 0) pList->last  = _newElement;

  pList->count++;

  return _newElement;
}

// Supprime un élément de la liste doublement chaînée            ---------------
// Remarque : pElement doit être un élément de pList sinon bug ! ---------------
//> Status de l'opération / suppression réussie ?
bool sLinkedList_Delete
  (sLinkedList * pList,    //: Liste à modifier
   sLinkedElmnt* pElement) //: L'Elément à supprimer
{
  if (pList == NULL || pElement == NULL)
    return false;

  // Update links (first & last) of the linked list
  if (pElement->prev == 0) pList->first = pElement->next;
  if (pElement->next == 0) pList->last  = pElement->prev;

  if (pElement->prev) pElement->prev->next = pElement->next;
  if (pElement->next) pElement->next->prev = pElement->prev;

  if (pList->releaseFunc != 0)
  {
    pList->releaseFunc (pElement->value);
  }

  // Freeze the memory taken by the element
  free (pElement);
  pElement = 0;

  pList->count--;

  return true;
}

// Initalise la boucle foreach like sur la liste doublement chaînée ------------
//> Status de l'opération / est-ce que foreachElmnt est un élément valide ?
bool sLinkedList_InitForeach
  (sLinkedList* pList,    //: Liste à traiter
   bool         pReverse) //: Faut-il effectuer la boucle à " l'envers " ?
{
  if (pList == NULL)
    return false;

  pList->foreachReverse = pReverse;
  pList->foreachElmnt   = pReverse ? pList->last : pList->first;

  pList->foreachCount   = 0;
  pList->foreachDeleted = false;

  return pList->foreachElmnt != 0;
}

// Continue la boucle foreach like sur la liste doublement chaînée -------------
//> Status de l'opération / est-ce que foreachElmnt est un élément valide ?
bool sLinkedList_NextForeach
  (sLinkedList* pList) //: Liste à traiter
{
  if (pList == NULL)
    return false;

  if (pList->foreachCount++ > 0)
  {
    // Un élément vient d'être supprimé pendant la boucle : déjà avancé ...
    if (pList->foreachDeleted)
    {
      pList->foreachDeleted = false;
    }
    else
    {
      if (pList->foreachElmnt != 0)
      {
        pList->foreachElmnt = pList->foreachReverse ?
                              pList->foreachElmnt->prev :
                              pList->foreachElmnt->next;
      }
    }
  }

  return pList->foreachElmnt != 0;
}

// Détruit l'élément en cours de la boucle foreach like sur la liste chaînée ---
//> Status de l'opération / suppression réussie ?
bool sLinkedList_DeleteOnForeach
  (sLinkedList* pList) //: Liste à traiter
{
  if (pList == NULL)
    return false;

  pList->foreachDeleted = true;

  sLinkedElmnt* _delete = pList->foreachElmnt;

  if (pList->foreachElmnt != 0)
  {
    pList->foreachElmnt = pList->foreachReverse ?
                          pList->foreachElmnt->prev :
                          pList->foreachElmnt->next;
  }

  if (_delete != 0)
  {
    return sLinkedList_Delete (pList, _delete);
  }

  return true;
}

// Retourne value de l'élément pointé par la boucle foreach like ---------------
//> Value de l'élément pointé par la boucle foreach like ou 0 si inactif
void* sLinkedList_ForeachValue
  (const sLinkedList* pList) //: Liste à traiter
{
  if (pList == NULL)
    return NULL;

  if (pList->foreachElmnt == 0) return 0;

  return pList->foreachElmnt->value;
}

// Retourne value d'un certain élément -----------------------------------------
//> Value de l'élément (ou 0 si le pointeur sur l'élément est 0)
void* sLinkedElmnt_GetValue
  (const sLinkedElmnt* pElement) //: L'élément à traiter
{
  if (pElement == 0)
    return 0;

  return pElement->value;
}
