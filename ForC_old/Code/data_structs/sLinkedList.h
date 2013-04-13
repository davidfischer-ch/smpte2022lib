/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description : Double-linked chain buffer
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

#ifndef __SLINKEDLIST__
#define __SLINKEDLIST__

// Types de données ============================================================

// Fonction (déléguée) servant à supprimer un élément de la liste chaînée ------
typedef void (*sLinkedReleaseFunc)(void* pValue);

// Fonction (déléguée) servant à afficher un élément de la liste chaînée -------
typedef void (*sLinkedPrintFunc)(void* pValue);

// Structure représentant un élément d'une liste doublement chaînée ------------
typedef struct sLinkedElmnt
{
  struct sLinkedElmnt* prev;  //. Pointeur sur le précédent (avant celui-ci)
  struct sLinkedElmnt* next;  //. Pointeur sur le prochain (après celui-ci)
//sLinkedList_t *mother; Pointeur sur la liste qui " nous stocke "
  void*                value; //. Donnée utile de l'élément
} sLinkedElmnt;

// Structure représentant une liste doublement chaînée -------------------------
typedef struct
{
  sLinkedElmnt* first; //. Pointeur sur le premier élément de la chaîne
  sLinkedElmnt* last;  //. Pointeur sur le dernier élément de la chaîne

  unsigned count; //. Nombre d'éléments (on pourrait dire maillons) de la chaîne

  sLinkedElmnt* foreachElmnt;   //. Pointeur sur l'élément en cours du foreach
  //void*         foreachValue;   //. Pointeur sur value de " en cours du foreach
  bool          foreachReverse; //. Foreach parcouru à l'envers ?
  unsigned      foreachCount;   //. Nombre d'éléments décomptés par le foreach
  bool          foreachDeleted; //. Y a-t-il eu suppression lors du foreach ?

  sLinkedReleaseFunc releaseFunc; //. Pointeur sur la fonction de suppression
  sLinkedPrintFunc   printFunc;   //. Pointeur sur la fonction d'affichage
}
sLinkedList;

// Déclaration des Constantes ==================================================

extern const sLinkedElmnt INIT_LINKED_ELMNT; //. Valeur initiale d'un élément

// Déclaration des Fonctions ===================================================

sLinkedList sLinkedList_New     (sLinkedReleaseFunc, sLinkedPrintFunc);
void        sLinkedList_Release (      sLinkedList*);
void        sLinkedList_Print   (const sLinkedList*, bool pBuffers);

sLinkedElmnt* sLinkedList_AppendByReference (sLinkedList*, void* pValue);

// Remarque : pInHere doit être un Elément de pLinkedList sinon bug !
sLinkedElmnt* sLinkedList_InsertByReference (sLinkedList*,
                                             sLinkedElmnt* pInHere,
                                             void*         pValue);

// Remarque : L'Elément doit être un Elément de pLinkedList sinon bug !
bool sLinkedList_Delete (sLinkedList*, sLinkedElmnt*);

bool sLinkedList_InitForeach     (sLinkedList*, bool pReverse);
bool sLinkedList_NextForeach     (sLinkedList*);
bool sLinkedList_DeleteOnForeach (sLinkedList*);

void* sLinkedList_ForeachValue (const sLinkedList*);

void* sLinkedElmnt_GetValue (const sLinkedElmnt*);
#endif
