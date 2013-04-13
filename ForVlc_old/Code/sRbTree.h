
/******************************************************************************\
                          REPRÉSENTE UN ARBRE ROUGE-NOIRE

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

Based on: http://en.literateprograms.org/
          Red-black_tree_(C)?action=history&offset=20080731190038
*/

#ifndef __RBTREE__
#define __RBTREE__

// Types de données ============================================================

// Fonction (déléguée) servant à supprimer un noeud de l'arbre rouge-noire ------
typedef void (*sRbReleaseFunc)(uint32_t key, void* value);

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

  sRbNode* foreachNode;    //. Pointeur sur le noeud en cours du foreach
  bool     foreachReverse; //. Foreach parcouru à l'envers ?
  unsigned foreachCount;   //. Nombre d'éléments décomptés par le foreach
  bool     foreachDeleted; //. Y a-t-il eu suppression lors du foreach ?

} sRbTree;

// Déclaration des Fonctions ===================================================

sRbTree sRbTree_New     (sRbReleaseFunc);
void    sRbTree_Release (      sRbTree*);

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
