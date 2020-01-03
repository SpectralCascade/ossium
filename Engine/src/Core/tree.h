/** COPYRIGHT NOTICE
 *  
 *  Copyright (c) 2018-2020 Tim Lane
 *  
 *  This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
 *  
 *  Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *  
 *  1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
 *  
 *  2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
 *  
 *  3. This notice may not be removed or altered from any source distribution.
 *  
**/
#ifndef TREE_H
#define TREE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#include "funcutils.h"
#include "logging.h"

using namespace std;

namespace Ossium
{

    template<class T>
    class Tree;

    /// A simple container of data and it's child data
    template<class T>
    struct Node
    {
        friend class Tree<T>;

        /// The actual data stored in the node
        T data;
        /// Pointer to parent node; null if this is a root node
        Node<T>* parent;
        /// Children nodes; empty if this is a leaf node
        vector<Node<T>*> children;
        /// Unique identifier
        int id;

        void SetParent(Node<T>* node)
        {
            if (parent != nullptr)
            {
                for (auto itr = parent->children.begin(); itr != parent->children.end(); itr++)
                {
                    if (*itr == this)
                    {
                        parent->children.erase(itr);
                        break;
                    }
                }
            }
            parent = node;
            if (parent != nullptr)
            {
                parent->children.push_back(this);
            }
        }

    };

    /// A simple hierarchical data structure.
    template<class T>
    class OSSIUM_EDL Tree
    {
    public:
        friend struct Node<T>;

        typedef function<bool(Node<T>*)> FindNodePredicate;

        Tree()
        {
            total = 0;
            nextId = 0;
        }

        ~Tree()
        {
            Clear();
        }

        /// Adds data to the tree. If parent is null, adds to root node children. Returns pointer to the node
        Node<T>* Insert(T data, Node<T>* parent = nullptr)
        {
            Node<T>* node = new Node<T>();
            node->data = data;
            node->parent = parent;
            Insert(node);
            return node;
        }

        void Insert(Node<T>* node)
        {
            node->id = nextId;
            if (node->parent != nullptr)
            {
                node->parent->children.push_back(node);
            }
            else
            {
                roots.push_back(node);
            }
            /// No need to add to flatTree array if it's to be recalulated; otherwise add to array
            if (!updateFlattened)
            {
                flatTree.push_back(node);
            }
            total++;
            nextId++;
        }

        /// Removes a specified data node and all nodes below it
        bool Remove(Node<T>* node)
        {
            /// Never remove the root node
            if (node == nullptr)
            {
                Logger::EngineLog().Warning("(!) Attempted to remove node that is already null.");
                return false;
            }
            vector<Node<T>*> all = GetAllBelow(node);
            node->children.clear();
            /// Delete all nodes below the source node
            for (auto i = all.begin(); i != all.end(); i++)
            {
                delete *i;
                *i = nullptr;
                updateFlattened = true;
                total--;
            }
            if (node->parent != nullptr)
            {
                /// Remove the source node from the tree and then we can safely delete it
                for (auto i = node->parent->children.begin(); i != node->parent->children.end(); i++)
                {
                    if (node->id == (*i)->id)
                    {
                        node->parent->children.erase(i);
                        delete node;
                        node = nullptr;
                        updateFlattened = true;
                        total--;
                        return true;
                    }
                }
            }
            else
            {
                /// Remove the source node from the roots
                for (auto i = roots.begin(); i != roots.end(); i++)
                {
                    if (node->id == (*i)->id)
                    {
                        roots.erase(i);
                        delete node;
                        node = nullptr;
                        updateFlattened = true;
                        total--;
                        return true;
                    }
                }
            }
            /// If we reach this point, there's a problem! Will not delete the node because e.g. the node is in another tree
            Logger::EngineLog().Warning("(!) Could not locate node[{0}] in tree for removal.", node->id);
            return false;
        }

        /// Finds and removes first found data node that meets the predicate condition.
        bool Remove(FindNodePredicate predicate)
        {
            /// Short circuit evalutation
            Node<T>* node = Find(predicate);
            return Remove(node);
        }

        /// Removes all nodes from the tree, except for the root
        void Clear()
        {
            roots.clear();
            flatTree.clear();
        }

        typedef function<void(Node<T>*)> NodeOp;

        /// Depth-first tree traversal
        void Walk(NodeOp walkFunc)
        {
            for (auto child : roots)
            {
                RecursiveWalk(walkFunc, child);
            }
        }

        /// Overload takes an additional function that is called when walking back up the tree.
        void Walk(NodeOp walkDown, NodeOp walkUp)
        {
            for (auto child : roots)
            {
                RecursiveWalk(walkDown, walkUp, child);
            }
        }

        /// Searches for a node that meets the provided predicate condition.
        Node<T>* Find(FindNodePredicate predicate, Node<T>* source = nullptr)
        {
            if (source == nullptr)
            {
                /// Search from roots
                for (auto i = roots.begin(); i != roots.end(); i++)
                {
                    DEBUG_ASSERT(*i != nullptr, "Root node is null!");
                    Node<T>* node = RecursiveFind(predicate, *i);
                    if (node != nullptr)
                    {
                        return node;
                    }
                }
            }
            else
            {
                /// Search from parent
                return RecursiveFind(predicate, source);
            }
            return nullptr;
        }

        /// Gets pointers to all nodes in the tree (below the source node) which meet the predicate condition; pretty slow!
        void FindAll(FindNodePredicate predicate, Node<T>* source, vector<Node<T>*>& output)
        {
            /// While we're iterating over everything we may as well rebuild the flatTree
            flatTree.clear();
            flatTree.reserve(total);
            RecursiveFindAll(predicate, source, output);
            updateFlattened = false;
        }

        /// Defaults to roots as the source
        vector<Node<T>*> FindAll(FindNodePredicate predicate)
        {
            vector<Node<T>*> nodes;
            /// While we're iterating over everything we may as well rebuild the flatTree
            flatTree.clear();
            flatTree.reserve(total);

            for (auto i = roots.begin(); i != roots.end(); i++)
            {
                RecursiveFindAll(predicate, *i, nodes);
            }
            updateFlattened = false;
            return nodes;
        }

        /// A version of FindAll() that matches multiple predicates; more efficient as it only walks the tree once,
        /// but can match more than a single predicate at a time.
        vector<Node<T>*> FindAll(const vector<FindNodePredicate>& predicates)
        {
            vector<Node<T>*> nodes;
            flatTree.clear();
            flatTree.reserve(total);

            for (auto i = roots.begin(); i != roots.end(); i++)
            {
                RecursiveFindAll(predicates, *i, nodes);
            }

            updateFlattened = false;
            return nodes;
        }

        /// Returns all nodes in the tree below a source node; includeSource adds the source node to the output vector
        vector<Node<T>*> GetAllBelow(Node<T>* source, bool includeSource = false)
        {
            vector<Node<T>*> all;
            if (source == nullptr)
            {
                Logger::EngineLog().Warning("(!) Attempted to retrieve all below a null node.");
                return all;
            }
            if (includeSource)
            {
                RecursiveGetAll(source, all);
            }
            else
            {
                for (auto i = source->children.begin(); i != source->children.end(); i++)
                {
                    RecursiveGetAll(*i, all);
                }
            }
            return all;
        }

        /// Get every single node in the tree.
        /// It's preferable to use GetFlatTree() instead of this method, as this recurses through the entire tree
        /// instead of returning the cached vector array.
        vector<Node<T>*> GetAll()
        {
            vector<Node<T>*> all;
            for (auto i = roots.begin(); i != roots.end(); i++)
            {
                RecursiveGetAll(*i, all);
            }
            return all;
        }

        /// Returns a reference to the cached, 'flattened' version of the tree.
        vector<Node<T>*>& GetFlatTree()
        {
            if (updateFlattened)
            {
                flatTree.clear();
                /// Preallocate memory for some performance gains. Use total + 1 as we include the root node.
                flatTree.reserve(total);
                for (auto i = roots.begin(); i != roots.end(); i++)
                {
                    RecursiveFlatten(*i);
                }
                updateFlattened = false;
            }
            return flatTree;
        }

        /// Returns the total number of nodes in the tree.
        unsigned int Size()
        {
            return total;
        }

        /// Sets the lazy-generated ID. Useful when ensuring unique ids after serialisation.
        void SetGeneration(int generation)
        {
            nextId = generation;
        }

        int GetGeneration()
        {
            return nextId;
        }

        /// Returns a vector array of the root nodes.
        vector<Node<T>*> GetRoots()
        {
            return roots;
        }

    private:
        /// Total number of nodes, NOT including the root node
        unsigned int total;

        /// Next node id
        int nextId;

        /// Root nodes of the tree
        vector<Node<T>*> roots;

        /// A 'flat' version of the tree with references to all nodes
        vector<Node<T>*> flatTree;

        /// Whether or not the tree should re-calculate the flatTree array next time GetFlattened() is called
        bool updateFlattened;

        /// Recursively iterates through the tree and builds the flatTree tree array
        void RecursiveFlatten(Node<T>* parent)
        {
            flatTree.push_back(parent);
            for (auto i = parent->children.begin(); i != parent->children.end(); i++)
            {
                RecursiveFlatten(*i);
            }
        }

        /// Walks recursively through the tree and operates on all nodes, depth first.
        void RecursiveWalk(NodeOp& nodeOp, Node<T>* node)
        {
            nodeOp(node);
            for (auto child : node->children)
            {
                RecursiveWalk(nodeOp, child);
            }
        }

        /// Walks recursively down and operates on all the nodes, depth first,
        /// but also operates on all the nodes on the walk back up the tree with the second function.
        void RecursiveWalk(NodeOp& walkDown, NodeOp& walkUp, Node<T>* node)
        {
            walkDown(node);
            for (auto child : node->children)
            {
                RecursiveWalk(walkDown, walkUp, child);
            }
            walkUp(node);
        }

        /// Recursive search method
        Node<T>* RecursiveFind(FindNodePredicate& predicate, Node<T>* parent)
        {
            if (predicate(parent))
            {
                return parent;
            }
            for (auto i = parent->children.begin(); i != parent->children.end(); i++)
            {
                Node<T>* found = RecursiveFind(predicate, *i);
                if (found != nullptr)
                {
                    return found;
                }
            }
            return nullptr;
        }

        void RecursiveFindAll(FindNodePredicate& predicate, Node<T>* parent, vector<Node<T>*>& output)
        {
            /// We can recalculate the flatTree tree while we're here
            if (updateFlattened)
            {
                flatTree.push_back(parent);
            }
            if (predicate(parent))
            {
                output.push_back(parent);
            }
            for (auto i = parent->children.begin(); i != parent->children.end(); i++)
            {
                RecursiveFindAll(predicate, *i, output);
            }
        }

        void RecursiveFindAll(const vector<FindNodePredicate>& predicates, Node<T>* parent, vector<Node<T>*>& output)
        {
            if (updateFlattened)
            {
                flatTree.push_back(parent);
            }
            for (auto predicate = predicates.begin(); predicate != predicates.end(); predicate++)
            {
                if ((*predicate)(parent))
                {
                    output.push_back(parent);
                    break;
                }
            }
            for (auto i = parent->children.begin(); i != parent->children.end(); i++)
            {
                RecursiveFindAll(predicates, *i, output);
            }
        }

        /// Recursively returns pointers to ALL nodes below some source node
        void RecursiveGetAll(Node<T>* source, vector<Node<T>*>& output)
        {
            output.push_back(source);
            for (auto i = source->children.begin(); i != source->children.end(); i++)
            {
                RecursiveGetAll(*i, output);
            }
        }

    };

}

#endif // TREE_H