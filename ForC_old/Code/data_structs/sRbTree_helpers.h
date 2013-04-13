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

// =============================================================================

sRbNode* GrandParent (const sRbNode* n)
{
  ASSERTpc (n,                 0, cExNullPtr)
  ASSERT   (n->parent,         0, "Not the root node")
  ASSERT   (n->parent->parent, 0, "Not child of root")

  return n->parent->parent;
}

sRbNode* Sibling (const sRbNode* n)
{
  ASSERTpc (n,         0, cExNullPtr)
  ASSERT   (n->parent, 0, "Root node has no sibling")

  if (n == n->parent->left) return n->parent->right;

  return n->parent->left;
}

sRbNode* Uncle (const sRbNode* n)
{
  ASSERTpc (n,                 0, cExNullPtr)
  ASSERT   (n->parent,         0, "Root node has no uncle")
  ASSERT   (n->parent->parent, 0, "Children of root have no uncle")

  return Sibling (n->parent);
}

enum sRbColor NodeColor (const sRbNode* n)
{
  return n == 0 ? BLACK : n->color;
}

sRbNode* MaximumNode (sRbNode* n)
{
  ASSERTpc (n, 0, cExNullPtr)

  while  (n->right) n = n->right;
  return  n;
}

// =============================================================================

void VerifyProperty1        (const sRbNode* n);
void VerifyProperty2        (const sRbNode* n);
void VerifyProperty4        (const sRbNode* n);
void VerifyProperty5        (const sRbNode* n);
void VerifyProperty5_Helper (const sRbNode* n,
                             int pBlackCount, int* pPathBlackCount);

void VerifyProperties (const sRbTree* t)
{
#ifdef VERIFY_RBTREE
  VerifyProperty1 (t->root);
  VerifyProperty2 (t->root);
  // Property 3 is implicit
  VerifyProperty4 (t->root);
  VerifyProperty5 (t->root);
#endif
}

void VerifyProperty1 (const sRbNode* n)
{
  ASSERT (NodeColor (n) == RED ||
          NodeColor (n) == BLACK,, "Node color must be RED or BLACK")

  if (n == NULL) return;

  VerifyProperty1 (n->left);
  VerifyProperty1 (n->right);
}

void VerifyProperty2 (const sRbNode* root)
{
  ASSERT (NodeColor (root) == BLACK,, "Node color must be BLACK")
}

void VerifyProperty4 (const sRbNode* n)
{
  if (NodeColor (n) == RED)
  {
    ASSERT (NodeColor (n->left)   == BLACK,, "Node color must be BLACK")
    ASSERT (NodeColor (n->right)  == BLACK,, "Node color must be BLACK")
    ASSERT (NodeColor (n->parent) == BLACK,, "Node color must be BLACK")
  }

  if (n == NULL) return;

  VerifyProperty4 (n->left);
  VerifyProperty4 (n->right);
}

void VerifyProperty5_Helper (const sRbNode* n,
                             int pBlackCount, int* pPathBlackCount)
{
  if (NodeColor (n) == BLACK) pBlackCount++;

  if (n == NULL)
  {
    if (*pPathBlackCount == -1)
        *pPathBlackCount = pBlackCount;
    else
    {
      ASSERT (pBlackCount == *pPathBlackCount,, "Undefined message")
    }

    return;
  }

  VerifyProperty5_Helper (n->left,  pBlackCount, pPathBlackCount);
  VerifyProperty5_Helper (n->right, pBlackCount, pPathBlackCount);
}

void VerifyProperty5 (const sRbNode* root)
{
  int blackCountPath = -1;
  VerifyProperty5_Helper (root, 0, &blackCountPath);
}

// =============================================================================

void ReplaceNode (sRbTree* t, sRbNode* pOldNode, sRbNode* pNewNode);
void RotateLeft  (sRbTree* t, sRbNode* n);
void RotateRight (sRbTree* t, sRbNode* n);

sRbNode* LookupNode (const sRbTree* t, uint32_t key)
{
  sRbNode* n = t->root;

  while (n)
  {
    if      (key == n->key) return n;
    else if (key <  n->key) n = n->left;
    else
    {
      ASSERT (key > n->key, 0, "Undefined message")
      n = n->right;
    }
  }
  return n;
}

void ReplaceNode (sRbTree* t, sRbNode* pOldNode, sRbNode* pNewNode)
{
  if (pOldNode->parent == NULL)
  {
    t->root = pNewNode;
  }
  else
  {
    if (pOldNode == pOldNode->parent->left)
    {
      pOldNode->parent->left = pNewNode;
    }
    else
    {
      pOldNode->parent->right = pNewNode;
    }
  }

  if (pNewNode != NULL)
  {
    pNewNode->parent = pOldNode->parent;
  }
}

void RotateLeft (sRbTree* t, sRbNode* n)
{
  sRbNode* r = n->right;

  ReplaceNode (t, n, r);

  n->right = r->left;

  if (r->left != NULL) r->left->parent = n;

  r->left   = n;
  n->parent = r;
}

void RotateRight (sRbTree* t, sRbNode* n)
{
  sRbNode* L = n->left;

  ReplaceNode (t, n, L);

  n->left = L->right;

  if (L->right != NULL) L->right->parent = n;

  L->right  = n;
  n->parent = L;
}

// =============================================================================

void InsertCase2 (sRbTree* t, sRbNode* n);
void InsertCase3 (sRbTree* t, sRbNode* n);
void InsertCase4 (sRbTree* t, sRbNode* n);
void InsertCase5 (sRbTree* t, sRbNode* n);

sRbNode* NewNode
  (uint32_t pKey, void* pValue,
   enum sRbColor pColor, sRbNode* pLeft, sRbNode* pRight)
{
  sRbNode* _node = malloc (sizeof (struct sRbNode));
  IFNOT   (_node, 0) // Allocation ratée ?

  _node->key   = pKey;
  _node->value = pValue;
  _node->color = pColor;
  _node->left  = pLeft;
  _node->right = pRight;

  if (pLeft  != NULL) pLeft ->parent = _node;
  if (pRight != NULL) pRight->parent = _node;

  _node->parent = NULL;

  return _node;
}

void InsertCase1 (sRbTree* t, sRbNode* n)
{
  if (n->parent)
  {
    InsertCase2 (t, n);
  }
  else
  {
    n->color = BLACK;
  }
}

void InsertCase2 (sRbTree* t, sRbNode* n)
{
  if (NodeColor (n->parent) == BLACK)
  {
    return; // Tree is still valid
  }
  else
  {
    InsertCase3 (t, n);
  }
}

void InsertCase3 (sRbTree* t, sRbNode* n)
{
  if (NodeColor (Uncle(n)) == RED)
  {
    n->parent      ->color = BLACK;
    Uncle       (n)->color = BLACK;
    GrandParent (n)->color = RED;
    InsertCase1 (t, GrandParent (n));
  }
  else InsertCase4 (t, n);
}

void InsertCase4 (sRbTree* t, sRbNode* n)
{
  if (n == n->parent->right && n->parent == GrandParent (n)->left)
  {
    RotateLeft (t, n->parent);
    n = n->left;
  }
  else if (n == n->parent->left && n->parent == GrandParent (n)->right)
  {
    RotateRight (t, n->parent);
    n = n->right;
  }

  InsertCase5 (t, n);
}

void InsertCase5 (sRbTree* t, sRbNode* n)
{
  n->parent      ->color = BLACK;
  GrandParent (n)->color = RED;

  if (n == n->parent->left && n->parent == GrandParent (n)->left)
  {
    RotateRight (t, GrandParent (n));
  }
  else
  {
    ASSERT (n == n->parent->right &&
            n->parent == GrandParent (n)->right,, "Undefined message")

    RotateLeft (t, GrandParent (n));
  }
}

// =============================================================================

void DeleteCase2 (sRbTree* t, sRbNode* n);
void DeleteCase3 (sRbTree* t, sRbNode* n);
void DeleteCase4 (sRbTree* t, sRbNode* n);
void DeleteCase5 (sRbTree* t, sRbNode* n);
void DeleteCase6 (sRbTree* t, sRbNode* n);

void DeleteCase1 (sRbTree* t, sRbNode* n)
{
  if (n->parent == NULL) return;

  DeleteCase2 (t, n);
}

void DeleteCase2 (sRbTree* t, sRbNode* n)
{
  if (NodeColor (Sibling (n)) == RED)
  {
    n->parent  ->color = RED;
    Sibling (n)->color = BLACK;

    if (n == n->parent->left)
         RotateLeft  (t, n->parent);
    else RotateRight (t, n->parent);
  }

  DeleteCase3 (t, n);
}

void DeleteCase3 (sRbTree* t, sRbNode* n)
{
  if (NodeColor (n->parent)          == BLACK &&
      NodeColor (Sibling (n))        == BLACK &&
      NodeColor (Sibling (n)->left)  == BLACK &&
      NodeColor (Sibling (n)->right) == BLACK)
  {
    Sibling (n)->color = RED;
    DeleteCase1 (t, n->parent);
  }
  else DeleteCase4 (t, n);
}

void DeleteCase4 (sRbTree* t, sRbNode* n)
{
  if (NodeColor (n->parent)          == RED   &&
      NodeColor (Sibling (n))        == BLACK &&
      NodeColor (Sibling (n)->left)  == BLACK &&
      NodeColor (Sibling (n)->right) == BLACK)
  {
    Sibling (n)->color = RED;
    n->parent  ->color = BLACK;
  }
  else DeleteCase5 (t, n);
}

void DeleteCase5 (sRbTree* t, sRbNode* n)
{
  if (n == n->parent->left &&
      NodeColor (Sibling (n))        == BLACK &&
      NodeColor (Sibling (n)->left)  == RED   &&
      NodeColor (Sibling (n)->right) == BLACK)
  {
    Sibling (n)      ->color = RED;
    Sibling (n)->left->color = BLACK;
    RotateRight (t, Sibling (n));
  }
  else if (n == n->parent->right &&
           NodeColor (Sibling (n))        == BLACK &&
           NodeColor (Sibling (n)->right) == RED   &&
           NodeColor (Sibling (n)->left)  == BLACK)
  {
    Sibling (n)       ->color = RED;
    Sibling (n)->right->color = BLACK;
    RotateLeft (t, Sibling (n));
  }

  DeleteCase6 (t, n);
}

void DeleteCase6 (sRbTree* t, sRbNode* n)
{
  Sibling (n)->color = NodeColor (n->parent);
  n->parent  ->color = BLACK;

  if (n == n->parent->left)
  {
    ASSERT (NodeColor (Sibling (n)->right) == RED,, "Node must be RED")

    Sibling (n)->right->color = BLACK;
    RotateLeft (t, n->parent);
  }
  else
  {
    ASSERT (NodeColor (Sibling (n)->left) == RED,, "Node must be RED")

    Sibling (n)->left->color = BLACK;
    RotateRight (t, n->parent);
  }
}

// =============================================================================

void sRbTree_ReleaseHelper (sRbTree* t, sRbNode* n)
{
  if (n == 0) return;

  if (n->left)  sRbTree_ReleaseHelper (t, n->left);
  if (n->right) sRbTree_ReleaseHelper (t, n->right);

  if (t->releaseFunc) t->releaseFunc (n->key, n->value);

  free (n);
}

// =============================================================================

void sRbTree_PrintHelper
  (const sRbTree* t, sRbNode* n, unsigned pIndentStep, bool pBuffers)
{
  if (n == NULL)
  {
    PRINT1 ("<empty rbtree>")
    return;
  }

  if (!pBuffers)
  {
    PRINT1 ("count : %u", t->count)
    return;
  }
  else
  {
    ASSERT (t->printNdFunc,, "printNdFunc must be linked to a function")
  }

  if (n->right != NULL)
  {
    sRbTree_PrintHelper (t, n->right, pIndentStep, pBuffers);
  }

  unsigned i;
  for (i = 0; i < pIndentStep; i++)
  {
    PRINT1 (" ")
  }

  if (n->color == BLACK)
       PRINT1 ("%d ",   (int)n->key)
  else PRINT1 ("<%d> ", (int)n->key)

  t->printNdFunc (n->value);

  PRINT1 ("\n")

  if (n->left != NULL)
  {
    sRbTree_PrintHelper (t, n->left, pIndentStep, pBuffers);
  }
}
