/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description    : Double-linked chain buffer
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

// Constantes publiques ========================================================

const sLinkedElmnt INIT_LINKED_ELMNT = //. Valeur initiale d'un élément
  {0, 0, 0};

// Constantes privées ==========================================================

const sLinkedList INIT_LINKED_LIST = //. Valeur initiale d'une liste chaînée
  {0, 0, 0, 0, 0, 0, 0, 0, 0};

// Fonctions publiques =========================================================

// Initialise une nouvelle liste doublement chaînée ----------------------------
//> Nouvelle liste doublement chaînée
sLinkedList sLinkedList_New
  (sLinkedReleaseFunc pReleaseFunc, //: Fonction de suppression d'élément
   sLinkedPrintFunc   pPrintFunc)   //: Fonction d'affichage d'élément
{
  sLinkedList l = INIT_LINKED_LIST;
  l.releaseFunc = pReleaseFunc;
  l.printFunc   = pPrintFunc;
  return l;
}

// Libère la mémoire allouée par une liste doublement chaînée ------------------
void sLinkedList_Release
  (sLinkedList* pList) //: Liste à vider
{
  ASSERTpc (pList,, cExNullPtr)

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

// Affiche le contenu d'une liste doublement chaînée ---------------------------
void sLinkedList_Print
  (const sLinkedList* pList,    //: Liste à afficher
         bool         pBuffers) //: Faut-il afficher le contenu des buffers ?
{
  ASSERTpc (pList,, cExNullPtr)

  if (pBuffers)
  {
    ASSERTc (pList->printFunc,, cExNullFunc)

    sLinkedElmnt* _element = pList->first;

    while (_element != 0)
    {
      pList->printFunc (_element->value);
      PRINT1 ("\n")

      _element = _element->next;
    }
  }

  if (pList->count == 0)
  {
    PRINT1 ("<empty linked list>\n")
  }
  else
  {
    if (pBuffers)
    {
      PRINT1 ("\n")
    }

    PRINT1 ("count : %d\n", pList->count)
  }
}

// Créé et ajoute (en dernier) un nouvel élément à la liste chaînée ------------
//> Pointeur sur le nouvel élément ou 0 si problème
sLinkedElmnt* sLinkedList_AppendByReference
  (sLinkedList* pList,  //: Liste à modifier
   void       * pValue) //: Ce que nous voulons stocker dans l'élément
{
  ASSERTpc (pList, 0, cExNullPtr)

  sLinkedElmnt* _newElement = malloc (sizeof (sLinkedElmnt));
  IFNOT        (_newElement, 0) // Allocation ratée

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
  ASSERTpc (pList, 0, cExNullPtr)

  sLinkedElmnt* _newElement = malloc (sizeof (sLinkedElmnt));
  IFNOT        (_newElement, 0) // Allocation ratée

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
  ASSERTpc (pList,    false, cExNullPtr)
  ASSERTpc (pElement, false, cExNullPtr)

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
  ASSERTpc (pList, false, cExNullPtr)

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
  ASSERTpc (pList, false, cExNullPtr)

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
  ASSERTpc (pList, false, cExNullPtr)

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
  ASSERTpc (pList, 0, cExNullPtr)

  if (pList->foreachElmnt == 0) return 0;

  return pList->foreachElmnt->value;
}

// Retourne value d'un certain élément -----------------------------------------
//> Value de l'élément (ou 0 si le pointeur sur l'élément est 0)
void* sLinkedElmnt_GetValue
  (const sLinkedElmnt* pElement) //: L'élément à traiter
{
  if (pElement == 0) return 0;

  return pElement->value;
}
