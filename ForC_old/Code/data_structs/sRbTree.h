/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description : Red-black tree
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

  Based on: http://en.literateprograms.org/
            Red-black_tree_(C)?action=history&offset=20080731190038
*/

#ifndef __RBTREE__
#define __RBTREE__

// Types de données ============================================================

// Fonction (déléguée) servant à supprimer un noeud de l'arbre rouge-noire ------
typedef void (*sRbReleaseFunc)(uint32_t key, void* value);

// Fonction (déléguée) servant à afficher un noeud de l'arbre rouge-noire ------
typedef void (*sRbPrintNdFunc)(void* value);

// Type de couleur attribuable à un noeud de l'arbre rouge-noire ---------------
enum sRbColor { RED, BLACK };

// Structure représentant un noeud d'un arbre rouge-noire ----------------------
typedef struct sRbNode
{
  uint32_t        key;    //. Clé attribuée au noeud
  void*           value;  //. Valeur stockée par le noeud
  struct sRbNode* left;   //. Enfant de gauche (plus petit par ordre croissant)
  struct sRbNode* right;  //. Enfant de droite (plus grand par ordre croissant)
  struct sRbNode* parent; //. Parent du noeud
  enum   sRbColor color;  //. Couleur (rouge ou noire) attribuée au noeud
} sRbNode;

// Structure représentant un arbre rouge-noire (auto équilibré) ----------------
typedef struct
{
  sRbNode* root;      //. Pointeur sur le noeud racine de l'arbre
  unsigned count;     //. Nombre de noeuds de l'arbre
  unsigned overCount; //. Nombre d'overwrite(s) de noeuds

  sRbReleaseFunc releaseFunc; //. Notre fonction de suppression de noeud
  sRbPrintNdFunc printNdFunc; //. Notre fonction d'affichage de noeud

  sRbNode* foreachNode;    //. Pointeur sur le noeud en cours du foreach
  bool     foreachReverse; //. Foreach parcouru à l'envers ?
  unsigned foreachCount;   //. Nombre d'éléments décomptés par le foreach
  bool     foreachDeleted; //. Y a-t-il eu suppression lors du foreach ?

} sRbTree;

// Déclaration des Fonctions ===================================================

sRbTree sRbTree_New     (sRbReleaseFunc, sRbPrintNdFunc);
void    sRbTree_Release (      sRbTree*);
void    sRbTree_Print   (const sRbTree*, unsigned pIndentStep, bool pBuffers);

sRbNode* sRbTree_First (const sRbTree*);
sRbNode* sRbTree_Last  (const sRbTree*);
sRbNode* sRbNode_Next  (const sRbNode*);
sRbNode* sRbNode_Prev  (const sRbNode*);

sRbNode* sRbTree_AddByReference
  (sRbTree*, uint32_t pKey, void* pValue, bool pReplaceNode);

void* sRbTree_Lookup (const sRbTree*, uint32_t pKey);
bool  sRbTree_Delete (      sRbTree*, uint32_t pKey);

bool sRbTree_InitForeach     (sRbTree*, bool pReverse);
bool sRbTree_NextForeach     (sRbTree*);
bool sRbTree_DeleteOnForeach (sRbTree*);

uint32_t sRbTree_ForeachKey   (const sRbTree*);
void*    sRbTree_ForeachValue (const sRbTree*);

void* sRbNode_GetValue (const sRbNode*);

#endif
