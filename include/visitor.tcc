#ifndef _VISITOR_HPP
#error You should include matrix.hpp instead of this file directly
#endif

#ifndef _VISITOR_TCC
#define _VISITOR_TCC 1



template <typename T>
bool HierarchicalVisitor<T>::visitEnter(T* that)
{
    return that != NULL;
}

template <typename T>
bool HierarchicalVisitor<T>::visitLeaf(T* that)
{
    return that != NULL;
}

template <typename T>
bool HierarchicalVisitor<T>::visitLeave(T* that)
{
    return that != NULL;
}



/// "Hidden" from the HPP, internal stuff
/// Simply make a dynamic_cast from TBase to TSpecialized
/// Make sure it's castable, avoid NULL pointers
template <typename Return, class TBase, class TSpecialized>
class DynamicCastArgFunctor {
    private:
        sigc::slot<Return,TSpecialized*> callback;
    public:
        DynamicCastArgFunctor(sigc::slot<Return,TSpecialized*> callback);
        Return run(TBase* arg);
};

template <typename Return, class TBase, class TSpecialized>
DynamicCastArgFunctor<Return,TBase,TSpecialized>::DynamicCastArgFunctor(sigc::slot<Return,TSpecialized*> callback)
: callback(callback)
{
}

template <typename Return, class TBase, class TSpecialized>
Return DynamicCastArgFunctor<Return,TBase,TSpecialized>::run(TBase* arg)
{
    if (arg == NULL) {
        return callback(NULL);
    }
    TSpecialized* dyn_arg = dynamic_cast<TSpecialized*>(arg);
    if (dyn_arg == NULL) throw bad_cast();
    return callback(dyn_arg);
}



template <class TBase>
template <typename TSpecialized>
void SpecializedHierachicalVisitor<TBase>::addSpecialization(CallbackMap& exactMap, CallbackList& orderList, sigc::slot<bool,TSpecialized*> callback)
{
    // Get type_info
    // They're static storage, so their address can be used for comparison
    // (they cannot be copied anyway so it's the only mean of keeping a hand onto them)
    PTypeinfoKey type = &typeid(TSpecialized);
    // Wrap the slot with a dynamic_cast on the argument
    typedef DynamicCastArgFunctor<bool,TBase,TSpecialized> DynCastFun;
    DynCastFun* d = new DynCastFun(callback);
    // Insert the new resolution type and callback
    CallbackPair pair = make_pair(type, sigc::mem_fun(d, &DynCastFun::run));
    exactMap.insert(pair);
    orderList.push_back(pair);
}

/// Resolve the type callback
/// Calls the fallback's callback if a fallback is specified
/// This method can act for any visit*() method.
template <class TBase>
bool SpecializedHierachicalVisitor<TBase>::visit(CallbackMap& exactMap, CallbackList& orderList, bool defaultReturn, BaseCallback fallbackCallback, TBase* that)
{
    if (that != NULL) {
        // Try exact type match
        CallbackMapIterator it = exactMap.find(&typeid(*that));
        if (it != exactMap.end()) {
            // Found, use that callback
            return it->second(that);
        }
        // Try other type callback in order
        for (CallbackListIterator it = orderList.begin() ; it < orderList.end() ; ++it) {
            // DynamicCastArgFunctor throws a bad_cast if anything goes bad
            try {
                // If dynamic_cast succeeds, then the callback is called, and we can return
                return it->second(that);
            } catch (bad_cast& ex) {
                // Continue searching for an accepting type
                continue;
            }
        }
    }
    // Facing either a NULL value or an value impossible to cast to any known type
    if (fallback != NULL) {
        // Use the fallback HierarchicalVisitor, if available
        return fallbackCallback(that);
    } else {
        // Otherwise, use the default return value
        return defaultReturn;
    }
}

template <class TBase>
SpecializedHierachicalVisitor<TBase>::SpecializedHierachicalVisitor(bool defaultReturnEnter = true, bool defaultReturnLeaf = true, bool defaultReturnLeave = true)
: defaultReturnEnter(defaultReturnEnter)
, defaultReturnLeaf(defaultReturnLeaf)
, defaultReturnLeave(defaultReturnLeave)
, fallback(NULL)
{
}

template <class TBase>
void SpecializedHierachicalVisitor<TBase>::setFallbackVisitor(HierarchicalVisitor<TBase>* visitor)
{
    fallback = visitor;
}

template <class TBase>
template <typename TSpecialized>
void SpecializedHierachicalVisitor<TBase>::addSpecializationEnter(sigc::slot<bool,TSpecialized*> callback)
{
    addSpecialization(specializationsEnterExact, specializationsEnterOrder, callback);
}

template <class TBase>
template <typename TSpecialized>
void SpecializedHierachicalVisitor<TBase>::addSpecializationLeaf(sigc::slot<bool,TSpecialized*> callback)
{
    addSpecialization(specializationsLeafExact, specializationsLeafOrder, callback);
}

template <class TBase>
template <typename TSpecialized>
void SpecializedHierachicalVisitor<TBase>::addSpecializationLeave(sigc::slot<bool,TSpecialized*> callback)
{
    addSpecialization(specializationsLeaveExact, specializationsLeaveOrder, callback);
}

template <class TBase>
bool SpecializedHierachicalVisitor<TBase>::visitEnter(TBase* that)
{
    return visit(specializationsEnterExact, specializationsEnterOrder, defaultReturnEnter, sigc::mem_fun(*fallback, &HierarchicalVisitor<TBase>::visitEnter), that);
}

template <class TBase>
bool SpecializedHierachicalVisitor<TBase>::visitLeaf(TBase* that)
{
    return visit(specializationsLeafExact, specializationsLeafOrder, defaultReturnLeaf, sigc::mem_fun(*fallback, &HierarchicalVisitor<TBase>::visitLeaf), that);
}

template <class TBase>
bool SpecializedHierachicalVisitor<TBase>::visitLeave(TBase* that)
{
    return visit(specializationsLeaveExact, specializationsLeaveOrder, defaultReturnLeave, sigc::mem_fun(*fallback, &HierarchicalVisitor<TBase>::visitLeave), that);
}



#endif /*_VISITOR_TCC*/