/**
 * @file visitor.hpp
 *
 * @brief Generic visitor design pattern implementation.
 *
 * @section LICENSE
 *
 * Copyright (c) 2011 Olivier Favre
 *
 * This file is part of Breach.
 *
 * Licensed under the Simplified BSD License,
 * for details please see LICENSE file or the website
 * http://www.opensource.org/licenses/BSD-2-Clause
 */

#ifndef _VISITOR_HPP
#define _VISITOR_HPP 1



#include <map>
#include <vector>
#include <typeinfo>
#include <sigc++/sigc++.h>



/**
 * @brief A hierarchy aware visitor.
 *
 * A leaf \link Visitable \endlink should only call \link ::visitLeaf() \endlink,
 * and return it's return value.
 * A composite \link Visitable \endlink should first call \link ::visitEnter() \endlink,
 * if the call returns \c false it should return \c false, otherwise it should continue the
 * inspection by calling each of its components \link Visitable::accept() \endlink function,
 * breaking at the first \c false returned value. It should then call \link ::visitLeave() \endlink
 * and returns its return value.
 */
template <typename T>
class HierarchicalVisitor {
    public:
        /** @brief Call when entering a composed visited object.
         *
         * Should only be called within the \link Visitable::accept() \endlink
         * function of a visited object.
         * @param that Should be a pointer to the calling object.
         * @return \c true if the inspection of the calling object is desired.
         */
        virtual bool visitEnter(T* that);
        /** @brief Call when entering a leaf visited object.
         * @param that Should be a pointer to the calling object.
         * @return Value that the calling \link Visitable::accept() \endlink function
         *         should return.
         */
        virtual bool visitLeaf(T* that);
        /** @brief Call when leaving a composed visited object.
         * @param that Should be a pointer to the calling object.
         * @return Value that the calling \link Visitable::accept() \endlink function
         *         should return.
         */
        virtual bool visitLeave(T* that);
};



template <typename T>
class Visitable {
    public:
        /** @brief Accept the visitor.
         *
         * Let the visitor inspect the implementor internals.
         * For implementation, see the documentation of \link HierarchicalVisitor \endlink
         * @see HierarchicalVisitor
         */
        virtual bool accept(HierarchicalVisitor<T>& visitor) = 0;
};



/**
 * @brief A HierarchicalVisitor that can call different method depending on the
 *        actual derived class of its argument.
 *
 * Resolving what callback (\link sigc::slot \endlink) to use is done as:
 *
 * \li Try to find an exact match between the most derived type and
 *     all the registered callback types
 * \li Try all the registered callback types in order of addition,
 *     when the first \c dynamic_cast succeeds (does not yield a bad_cast or a \c NULL pointer)
 *     it is executed and all others registrered callbacks are ignored
 *
 * This method mimics overloading, as there is no way to make use of
 * real overloading because the actual visitor type may be unknown at compilation time,
 * or (more likely) is not unique.
 *
 * In order to discover object type at runtime, we need a common base class
 * of the \link Visitable \endlink objects. No base types are allowed by
 * \c dynamic_cast.
 * The template parameter \link #TBase \endlink is privided for further restriction
 * on acceptable visited objects.
 */
template <class TBase>
class SpecializedHierachicalVisitor : public HierarchicalVisitor<TBase> {
    private:
        // Simplicating typedefs
        //! @brief Type of callback accepting the base class as argument.
        typedef sigc::slot<bool,TBase*> BaseCallback;
        //! @brief Type of a type_info when as used as map key.
        typedef const std::type_info *PTypeinfoKey;
        //! @brief Type of a pair of a type_info key and a base callback value.
        typedef std::pair<PTypeinfoKey,BaseCallback> CallbackPair;
        //! @brief Type of an exact type match lookup map.
        typedef std::map<PTypeinfoKey,BaseCallback> CallbackMap;
        //! @brief Type of an ordered type match resolving vector.
        typedef std::vector<CallbackPair> CallbackList;
        //! @brief Type of an iterator on an exact type match lookup map.
        typedef typename CallbackMap::iterator CallbackMapIterator;
        //! @brief Type of an iterator on an ordered type match resolving vector.
        typedef typename CallbackList::iterator CallbackListIterator;

        // Default behaviour
        //! @brief Default return value for \link HierarchicalVisitor::visitEnter() \endlink
        ///        if no appropriate callback is resolved and no fallback visitor is given.
        bool defaultReturnEnter;
        //! @brief Default return value for \link HierarchicalVisitor::visitLeaf() \endlink
        ///        if no appropriate callback is resolved and no fallback visitor is given.
        bool defaultReturnLeaf;
        //! @brief Default return value for \link HierarchicalVisitor::visitLeave() \endlink
        ///        if no appropriate callback is resolved and no fallback visitor is given.
        bool defaultReturnLeave;
        //! @brief Fallback visitor to be used when no appropriate callback is resolved.
        HierarchicalVisitor<TBase>* fallback;

        // visitEnter()
        //! @brief Lookup map for resolving exact type match for \link HierarchicalVisitor::visitEnter() \endlink.
        CallbackMap specializationsEnterExact;
        //! @brief Vector for trying types resolution in order for \link HierarchicalVisitor::visitEnter() \endlink.
        CallbackList specializationsEnterOrder;
        // visitLeaf()
        //! @brief Lookup map for resolving exact type match for \link HierarchicalVisitor::visitLeaf() \endlink.
        CallbackMap specializationsLeafExact;
        //! @brief Vector for trying types resolution in order for \link HierarchicalVisitor::visitLeaf() \endlink.
        CallbackList specializationsLeafOrder;
        // visitLeave()
        //! @brief Lookup map for resolving exact type match for \link HierarchicalVisitor::visitLeave() \endlink.
        CallbackMap specializationsLeaveExact;
        //! @brief Vector for trying types resolution in order for \link HierarchicalVisitor::visitLeave() \endlink.
        CallbackList specializationsLeaveOrder;

        /** @brief Fill the exact type match map and order type vector with the given slot,
         *         for a given \code visit*() \endcode method.
         * @param TSpecialized Specialization type, derived from \link #TBase \endlink.
         * @param exactMap     Lookup map for resolving exact type match for the appropriate \code visit*() \endcode method.
         * @param orderList    Vector for trying types resolution in order for the appropriate \code visit*() \endcode method.
         * @param callback     Specialized callback to register.
         */
        template <typename TSpecialized>
        void addSpecialization(CallbackMap& exactMap, CallbackList& orderList, sigc::slot<bool,TSpecialized*> callback);
        /** @brief Resolve the appropriate callback according to the actual, most derived type of the object to visit,
         *         for any \code visit*() \endcode method.
         *
         * Exact type match will be tried, otherwise, any registered type will be tried, in order,
         * for the first \c dynamic_cast accepting cast.
         *
         * If no resolution can be performed successfully, or if \a visited is \c NULL,
         * calls the fallback's callback if a fallback is specified,
         * otherwise uses configured default answer.
         *
         * @param exactMap         Lookup map for resolving exact type match for the appropriate \code visit*() \endcode method.
         * @param orderList        Vector for trying types resolution in order for the appropriate \code visit*() \endcode method.
         * @param defaultReturn    Default return value for the appropriate \code visit*() \endcode method.
         * @param fallbackCallback Default fallback callback for the appropriate \code visit*() \endcode method.
         * @param visited          Visited object the type resolution is to be run for.
         */
        virtual bool visit(CallbackMap& exactMap, CallbackList& orderList, bool defaultReturn, BaseCallback fallbackCallback, TBase* visited);

    public:
        /** @brief Constructs a specialized \link HierarchicalVisitor \endlink with given default return values.
         * @param defaultReturnEnter Default return value for \link HierarchicalVisitor::visitEnter() \endlink.
         * @param defaultReturnLeaf  Default return value for \link HierarchicalVisitor::visitLeaf() \endlink.
         * @param defaultReturnLeave Default return value for \link HierarchicalVisitor::visitLeave() \endlink.
         */
        SpecializedHierachicalVisitor(bool defaultReturnEnter = true, bool defaultReturnLeaf = true, bool defaultReturnLeave = true);
        /** @brief Specifies the fallback \link HierarchicalVisitor \endlink to be used when
         *         type resolution is unsuccessful.
         * @param visitor Fallback \link HierarchicalVisitor \endlink to use.
         */
        void setFallbackVisitor(HierarchicalVisitor<TBase>* visitor);
        /** @brief Registers a new specialized callback for the \link HierarchicalVisitor::visitEnter() \endlink.
         * @param TSpecialized Specialization type, derived from \link #TBase \endlink.
         * @param callback     Specialized callback to register.
         */
        template <typename TSpecialized>
        void addSpecializationEnter(sigc::slot<bool,TSpecialized*> callback);
        /** @brief Registers a new specialized callback for the \link HierarchicalVisitor::visitLeaf() \endlink.
         * @param TSpecialized Specialization type, derived from \link #TBase \endlink.
         * @param callback     Specialized callback to register.
         */
        template <typename TSpecialized>
        void addSpecializationLeaf(sigc::slot<bool,TSpecialized*> callback);
        /** @brief Registers a new specialized callback for the \link HierarchicalVisitor::visitLeave() \endlink.
         * @param TSpecialized Specialization type, derived from \link #TBase \endlink.
         * @param callback     Specialized callback to register.
         */
        template <typename TSpecialized>
        void addSpecializationLeave(sigc::slot<bool,TSpecialized*> callback);

        /** @brief Use dynamic type resolution on the visited object to call the right registered callback.
         * @see addSpecializationEnter()
         * @see HierarchicalVisitor::visitEnter()
         */
        virtual bool visitEnter(TBase* visited);
        /** @brief Use dynamic type resolution on the visited object to call the right registered callback.
         * @see addSpecializationLeaf()
         * @see HierarchicalVisitor::visitLeaf()
         */
        virtual bool visitLeaf(TBase* visited);
        /** @brief Use dynamic type resolution on the visited object to call the right registered callback.
         * @see addSpecializationLeave()
         * @see HierarchicalVisitor::visitLeave()
         */
        virtual bool visitLeave(TBase* visited);
};



#include "visitor.tcc"

#endif /*_VISITOR_HPP*/
