
/******************************************************************************\
              REPRÉSENTE UN CHAMP DE BITS (GRAND ENTIER NON SIGNÉ)
                       + UNE LISTE DOUBLEMENT CHAÎNÉE

  Auteur    : David Fischer
  Intégration VLC : Rossier Jérémie (2011)
  Contact   : david.fischer.ch@gmail.com / david.fischer@hesge.ch
              jeremie.rossier@gmail.com

  Projet     : Implémentation SMPTE 2022 de VLC
  Date début : 02.05.2008
  Employeur  : Ecole d'Ingénieurs de Genève
               Laboratoire de Télécommunications
\******************************************************************************/

/* Copyright (c) 2009 David Fischer (david.fischer.ch@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Retrieved from:
  https://sourceforge.net/projects/smpte2022lib/
*/

//------------------------------------------------------------------------------
//              CHAMP BITS
//------------------------------------------------------------------------------
#ifndef __SCHAMPBITS__
#define __SCHAMPBITS__

// Types de données ============================================================

// CHAMP_NO_MAX est 255 pour un pNo uint8_t, etc ...

#define CHAMP_NO_MAX        (UINT8_MAX+1)
#define CHAMP_VALEUR_MAX    (UINT32_MAX)
#define CHAMP_TAILLE_UNITE  32
#define CHAMP_NOMBRE_UNITE  (CHAMP_NO_MAX / CHAMP_TAILLE_UNITE)

// Type de paquet de FEC (colonne ou ligne) ------------------------------------
typedef enum { LSB_FIRST = 0, MSB_FIRST = 1 } eDir;

// Structure représentant un champ de bits (grand entier non signé) ------------
typedef struct
{
  uint32_t buffer[CHAMP_NOMBRE_UNITE]; //. Stockage des bits
}
  sChampBits;

// Déclaration des Fonctions ===================================================

sChampBits sChampBits_New();

bool sChampBits_GetBit   (const sChampBits*, uint8_t pNo);
void sChampBits_SetBit   (      sChampBits*, uint8_t pNo, bool pValeur);
signed sChampBits_GetOne (const sChampBits*, uint8_t pNo, eDir pDirection);

void       sChampBits_Incremente (      sChampBits*);
signed     sChampBits_Compare    (const sChampBits*, const sChampBits*);
sChampBits sChampBits_Add        (const sChampBits*, const sChampBits*);
#endif


//------------------------------------------------------------------------------
//              LISTE DOUBLEMENT CHAINEE
//------------------------------------------------------------------------------
#ifndef __SLINKEDLIST__
#define __SLINKEDLIST__

// Types de données ============================================================

// Fonction (déléguée) servant à supprimer un élément de la liste chaînée ------
typedef void (*sLinkedReleaseFunc)(void* pValue);

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
}
sLinkedList;

// Déclaration des Constantes ==================================================

extern const sLinkedElmnt INIT_LINKED_ELMNT; //. Valeur initiale d'un élément

// Déclaration des Fonctions ===================================================

sLinkedList sLinkedList_New     (sLinkedReleaseFunc);
void        sLinkedList_Release (      sLinkedList*);

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
