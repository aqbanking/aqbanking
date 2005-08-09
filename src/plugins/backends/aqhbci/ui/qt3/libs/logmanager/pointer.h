/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Tue Dec 13 2001
    copyright   : (C) 2001 by Martin Preuss
    email       : openhbci@aquamaniac.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef HBCIPOINTER_H
#define HBCIPOINTER_H

/** @file pointer.h
 *
 * @short Smart pointer HBCI::Pointer with helper classes. No C wrappers.*/

#include <assert.h>
#include <gwenhywfar/debug.h>

#ifdef __cplusplus
#include <stdio.h> /* DEBUG */
#include <string>

#include "error.h"

/**
 * Undefine this if you want exceptions instead of assert on error
 * Using "assert" is much better while debugging, since assert creates
 * a core dump which you may inspect with gdb.
 */
#define ASSERT_ON_ERROR


namespace HBCI {
  class PointerBase;
/* template <class T> class Pointer; */
  class PointerObject;
  template <class T> class PointerCastBase;
  template <class T, class U> class PointerCast;
}

namespace HBCI {

#ifndef DOXYGEN_HIDE

/**
 * This internal class is created by Pointer. It holds the real pointer
 * and an usage counter. You can neither create nor detroy such an object.
 * @author Martin Preuss<openhbci@aquamaniac.de>
 */
class PointerObject {
    friend class PointerBase;
private:
    void *_object;
    int _counter;
    bool _delete;
    string _descr;

    PointerObject(void *obj, string descr=""):
        _object(obj),_counter(0)
        ,_delete(true)
        ,_descr(descr){
        };
        ~PointerObject(){
        };

        void setDescription(string descr) {
		  /* if (_descr.empty()) */
            _descr=descr;
        };

        const string &description() const {
            return _descr;
        };

public:

};


#endif /* DOXYGEN_HIDE */




/**
 * @short Base class for the smart pointer template class.
 *
 * This is the base class to be inherited by a template class. This
 * cannot be used directly.
 *
 * @author Martin Preuss<openhbci@aquamaniac.de> */
class PointerBase {
#ifndef DOXYGEN_HIDE
private:
    PointerObject *_ptr;
    string _descr;

protected:
    void _attach(PointerObject &p) {
      _ptr=&p;
      if (_ptr) {
        _ptr->_counter++;
        if (_descr.empty())
          _descr=_ptr->_descr;
      }
      else {
#ifdef ASSERT_ON_ERROR
        assert(_ptr);
#endif
        throw HBCI::Error ("Pointer::_attach(&)",
                           ERROR_LEVEL_NORMAL,
                           0,
                           ERROR_ADVISE_DONTKNOW,
                           "No object for ."+_descr,
                           objectDescription());
      }
    };

    void _attach(PointerObject *p) {
        _ptr=p;
        if (_ptr) {
            _ptr->_counter++;
            if (_descr.empty())
                _descr=_ptr->_descr;
        }
        else {
#ifdef ASSERT_ON_ERROR
          assert(_ptr);
#endif
          throw HBCI::Error ("Pointer::_attach(pt*)",
                             ERROR_LEVEL_NORMAL,
                             0,
                             ERROR_ADVISE_DONTKNOW,
                             "No object for "+_descr);
        }
    };

    void _detach() {
        if (_ptr) {
            if (_ptr->_counter>0) {
                _ptr->_counter--;
                if (_ptr->_counter<1) {
		    if (_ptr->_delete)
			if (_ptr->_object)
			    _deleteObject(_ptr->_object);
		    delete _ptr;
		}
	    }
	}
	_ptr=0;
    };

    /**
     * This frees the object pointed to by this pointer, regardless of
     * the state of the autoDelete flag. You should use this instead of
     * setting the autodelete flag and deleting the object yourself, because
     * in that case the program might simply crash, if any other pointer tries
     * to use the object. By using this method here you would get an exception
     * which could tell you more about the cause of the crash.
     */
    void _release() {
      if (_ptr) {
	if (_ptr->_delete) {
	  if (_ptr->_object)
	    _deleteObject(_ptr->_object);
	}
	_ptr->_object=0;
      }
    };

    /**
     * This method actually deletes the object. Since the base class
     * does not know the type of this object, we have to make this method
     * virtual. The template class MUST override this.
     */
    virtual void _deleteObject(void *p) {
    };

    PointerBase(PointerBase &p): _ptr(0) {
      _descr=p._descr;
      if (p._ptr)
        _attach(p._ptr);
    };

    PointerBase(const PointerBase &p) : _ptr(0) {
      _descr=p._descr;
        if (p._ptr)
            _attach(p._ptr);
    };

  /**
   * This operator handles the case where you give another pointer as argument.
   * (like pointer1=pointer2).
   * @author Martin Preuss<openhbci@aquamaniac.de>
   */
    void operator=(PointerBase &p) {
        _detach();
        if (_descr.empty())
            _descr=p._descr;
        if (p._ptr)
            _attach(p._ptr);
    };

    void operator=(const PointerBase &p) {
        _detach();
        if (_descr.empty())
            _descr=p._descr;
        if (p._ptr)
            _attach(p._ptr);
    };

    /**
     * This operator handles the case where you do something like this:<BR>
     * pointer=new structXYZ;<BR>
     * @author Martin Preuss<openhbci@aquamaniac.de>
     */
    void operator=(void* obj) {
        PointerObject *p;
        if (_ptr)
            _detach();
        _ptr=0;
        if (obj==0)
            return;
        p=new PointerObject(obj,_descr);
        _attach(p);
    };

    /**
     * Constructor.
     */
    PointerBase(): _ptr(0) {};

    PointerBase(void *obj): _ptr(0) {
        PointerObject *p;
        p=new PointerObject(obj,_descr);
        _attach(p);
    };
#endif /* DOXYGEN_HIDE */

public:
    /**
     * Destructor.
     * If this one gets called, it automagically decrements the usage
     * counter of the object pointed to. If it reaches zero, then no other
     * pointer points to the object and the object faces deletion.
     * @author Martin Preuss<openhbci@aquamaniac.de>
     */
    virtual ~PointerBase() {
    };

    /**
     *  Set the description of this pointer. 
     *
     * Useful for debugging purposes.
     * @author Martin Preuss<martin@libchipcard.de>
     */
    void setDescription(string descr) {
        _descr=descr;
    };

    /**
     *  Get the description of this pointer.
     *
     * Useful for debugging purposes.
     * @author Martin Preuss<martin@libchipcard.de>
     */
    const string &description() const {
        return _descr;
    };

    /**
     *  Set the description of the object this pointer points to.
     *
     * Useful for debugging purposes.
     * @author Martin Preuss<martin@libchipcard.de>
     */
    void setObjectDescription(string descr) {
        if (!descr.empty())
            if (_ptr)
                _ptr->setDescription(descr);
    };

    /**
     *  Returns the description of the object. 
     *
     * Useful for debugging purposes.
     * @author Martin Preuss<martin@libchipcard.de>
     */
    string objectDescription() const {
        if (_ptr)
            return _ptr->description();
        else
            return "";
    };

    virtual int refCount() const {
      if (_ptr)
	return _ptr->_counter;
      else
	return -1;
    };

    /**
     *  Equality operator for the object pointed to.
     *
     * This operator checks whether another pointer and this one are
     * pointing to the same data.
     *
     * @author Martin Preuss<openhbci@aquamaniac.de> */
    bool operator==(const PointerBase &p) const {
        if (_ptr && p._ptr)
            return _ptr->_object==p._ptr->_object;
        else
            return false;
    };

    /**
     *  Checks whether both pointers share their data object.
     *
     * @author Martin Preuss<openhbci@aquamaniac.de>
     */
    bool sharingData(const PointerBase &p) const {
        return (_ptr==p._ptr);
    };

    /**
     *  Inequality operator for the object pointed to.
     *
     * This operator checks whether another pointer and this one are
     * not pointing to the same data.
     *
     * @author Martin Preuss<openhbci@aquamaniac.de> */
    bool operator!=(const PointerBase &p) const {
        if (_ptr && p._ptr)
            return _ptr->_object!=p._ptr->_object;
        else
            return true;
    };

    /**
     *  Returns a raw pointer to the stored data. 
     *
     * You should not really use this, but if you do so please NEVER
     * delete the object the pointer points to !  AND you should make
     * sure that as long as you are using the pointer returned there
     * is still a Pointer pointing to it (because if the last Pointer
     * stops pointing to an object that object gets deleted) !!
     *
     * @author Martin Preuss<openhbci@aquamaniac.de> */
    virtual void* voidptr() const {
        if (!_ptr)
            return 0;
        if (!(_ptr->_object))
            return 0;
        return _ptr->_object;
    };


    /** 
     *  Set the auto-deletion behaviour.
     * 
     * Set the auto-deletion behaviour of the PointerObject (the
     * wrapper object around the "real" object pointed to) that is
     * pointed to by this Pointer.
     *
     * By default, this is set to setAutoDeletion(true), i.e. the
     * object will automatically be deleted when its last
     * HBCI::Pointer gets deleted. On the other hand, when you call
     * this with b=false, then the object this pointer points to will
     * not be deleted by the last HBCI::Pointer.
     *
     * This might be useful if you are pointing to constant objects,
     * or if you need to continue using this object through raw
     * pointers elsewhere.
     * 
     * This flag is a property of the PointerObject, i.e. even for
     * multiple HBCI::Pointer's pointing to the same object there is
     * only *one* autoDelete flag per object. Changes to this flag
     * affect all of the HBCI::Pointer's at the same time.
     *
     * This HBCI::Pointer MUST already point to an object (a NULL
     * pointer is not allowed at this point) since the autodelete flag
     * is a property of the class PointerObject. If called on an
     * invalid HBCI::Pointer, this method will throw an HBCI::Error.
     *
     * @param b True to set automatic deletion to be enabled, 
     * false to disable it.
     * @author Martin Preuss<openhbci@aquamaniac.de> */
    void setAutoDelete(bool b) {
      if (_ptr) {
        if (_ptr->_object)
          _ptr->_delete=b;
      }
      else {
#ifdef ASSERT_ON_ERROR
        assert(_ptr);
#endif
        throw HBCI::Error ("PointerBase::setAutoDelete()",
                           ERROR_LEVEL_NORMAL,
                           0,
                           ERROR_ADVISE_DONTKNOW,
                           "No object in pointer",
                           description());
      }
    };

    /**
     *  Returns true if this Pointer is valid.
     *
     * This tells you if this pointer is pointing to accessible data.
     * @author Martin Preuss<openhbci@aquamaniac.de>
     * @return true if data is accessible, false if no data
     */
    bool isValid() const {
        if (_ptr)
            if (_ptr->_object)
                return true;
        return false;
    };


};


/**
 * @short A smart pointer template class.
 *
 * This class serves as a smart pointer class that is used in OpenHBCI
 * to avoid memory leaks. It does automatic reference counting for the
 * objects pointed to, like so: Each time a new Pointer to the same
 * object is created, the reference counter is incremented. Each time
 * a Pointer to an object is deleted, the reference counter is
 * decremented. When the reference counter reaches zero, the object is
 * deleted.
 *
 * Use it instead of normal pointers, for example:
 * instead of 
 *
 * <code>structXYZ *pointer; <BR>
 * pointer = new structXYZ; </code>
 *
 * use this one:
 *
 * <code>Pointer<structXYZ> pointer; <BR>
 * pointer = new structXYZ; </code>
 *
 * You can access the data easily by using the "*" operator, e.g:
 *
 * <code>structXYZ xyz = *pointer;</code> 
 * 
 * To access members of the object, either use the "*" operator or the
 * ref() method:
 *
 * <code>a = (*pointer).a; </code> or<br>
 * <code>b = pointer.ref().a;</code>
 *
 * @author Martin Preuss<openhbci@aquamaniac.de> */
template <class T> class Pointer: public PointerBase {
    friend class PointerCastBase<T>;
private:
protected:
    /**
     * This method actually deletes the object. Since the base class
     * does not know the type of this object, we have to make this method
     * virtual. The template class MUST override this.
     */
    virtual void _deleteObject(void *p) {
        delete (T*) p;
    };

    Pointer(const PointerBase &p): PointerBase(p) {
    };

public:
    /**
     *  Empty Constructor.
     */
    Pointer(): PointerBase(){};

    /**
     *  Constructor with object to be pointing to.
     */
    Pointer(T *obj): PointerBase(obj) {
    };

    /**  Copy constructor */
    Pointer(const Pointer<T> &p) : PointerBase(p) {
    };

    /**
     *  Destructor.
     *
     * If this one gets called, it automagically decrements the usage
     * counter of the object pointed to. If it reaches zero, then no other
     * pointer points to the object and the object will be deleted.
     * @author Martin Preuss<openhbci@aquamaniac.de>
     */
    virtual ~Pointer() {
        _detach();
    };

    /** @name Copy Operators */
    /*@{*/
    /**
     *  Copy operator with object pointed to.
     *
     * This operator handles the case where you do something like this:<BR>
     * pointer=new structXYZ;<BR>
     * @author Martin Preuss<openhbci@aquamaniac.de>
     */
    void operator=(T* obj) {
        PointerBase::operator=(obj);
    };

    /**
     *  Copy operator with another Pointer.
     *
     * This operator handles the case where you give another pointer
     * as argument.  (like pointer1=pointer2).
     *
     * @author Martin Preuss<openhbci@aquamaniac.de> */
    void operator=(Pointer<T> &p) {
        PointerBase::operator=(p);
    };

    /**
     *  Copy operator with another const Pointer.
     *
     * This operator handles the case where you give another pointer
     * as argument.  (like pointer1=pointer2).
     *
     * @author Martin Preuss<openhbci@aquamaniac.de> */
    void operator=(const Pointer<T> &p) {
        PointerBase::operator=(p);
    };
    /*@}*/

    /** @name Object Access */
    /*@{*/
    /**
     *  Returns a reference to the object pointed to.
     *
     * If the Pointer is invalid, this throws a HBCI::Error.
     */
    T& ref() const {
        T* p;

	p=ptr();
        assert(p);
	if (!p) {
#ifdef ASSERT_ON_ERROR
          assert(p);
#endif
          throw HBCI::Error ("Pointer::ref()",
                             ERROR_LEVEL_NORMAL,
                             0,
                             ERROR_ADVISE_DONTKNOW,
                             "No object in pointer",
                             description());
        }

        return *p;
    };

    /**
     *  Returns a reference to the object pointed to.
     *
     * If the Pointer is invalid, this throws a HBCI::Error.
     */
    T& operator*() const {
        return ref();
    };

    /**  Returns a raw pointer to the stored data. 
     *
     * If you can continue using only Pointer's, you should not
     * really need to use this. This method is necessary if and only
     * if you need to use a "raw C pointer" of the object pointed to.
     *
     * So if you need to use this method while there is still a
     * Pointer pointing to it, please <i>never</i> delete the object
     * returned. The last remaining Pointer's will take care of
     * deletion.
     *
     * On the other hand, if you need to use this pointer longer than
     * the last Pointer would exist, then either try to keep a Pointer
     * around long enough, or you need to consider setting
     * PointerBase::setAutoDelete appropriately. (Because if the last
     * Pointer stops pointing to an object, then that object will get
     * deleted unless PointerBase::setAutoDelete was changed.)
     *
     * @author Martin Preuss<openhbci@aquamaniac.de> */
    virtual T* ptr() const {
        return (T*)PointerBase::voidptr();
    };

    virtual int refCount() const {
      return PointerBase::refCount();
    }

    /**
     * This frees the object pointed to by this pointer, regardless of
     * the state of the autoDelete flag. You should use this instead of
     * setting the autodelete flag and deleting the object yourself, because
     * in that case the program might simply crash, if any other pointer tries
     * to use the object. By using this method here you would get an exception
     * which could tell you more about the cause of the crash.
     */
    void release() {
	PointerBase::_release();
    };

    /*@}*/

    /** @name Type cast */
    /*@{*/
    /**
     * @short Returns a type-safe casted Pointer of the given type.
     *
     * This method returns a type-safe casted Pointer of the given
     * type.  This obeys the same rules as a
     * <code>dynamic_cast<TARGET_TYPE></code>, and in fact internally
     * a <code>dynamic_cast</code> is used.
     *
     * Use it like this:
     * <pre>
     * class type_X;
     * class type_Y : public type_X;
     *
     * Pointer<type_X> pX;
     * Pointer<type_Y> pY = new type_Y;
     * pX = pY.cast<type_X>();
     * </pre>
     *
     * The casting fails if it is impossibe to safely cast the
     * "type_Y" to "type_X". In that case, an HBCI::Error is
     * thrown. Also, if you call this method on an invalid
     * Pointer, a HBCI::Error is thrown.
     * 
     * @author Martin Preuss<openhbci@aquamaniac.de> */
    template <class U> Pointer<U> cast() const {
        return PointerCast<U,T>::cast(*this);
        /* return Pointer<U>(*this); */

    };
    /*@}*/

    /** @name Equality */
    /*@{*/
    /**
     *  Equality operator for the object pointed to.
     *
     * This operator checks whether another pointer and this one are
     * pointing to the same data.
     *
     * @author Martin Preuss<openhbci@aquamaniac.de> */
    bool operator==(const Pointer<T> &p) const {
        return PointerBase::operator==(p);
    };

    /**
     *  Inequality operator for the object pointed to.
     *
     * This operator checks whether another pointer and this one are
     * not pointing to the same data.
     *
     * @author Martin Preuss<openhbci@aquamaniac.de> */
    bool operator!=(const Pointer<T> &p) const {
        return PointerBase::operator!=(p);
    };

    /**
     *  Checks whether both pointers share their data object.
     *
     * This method checks whether another pointer and this one share
     * the same internal data object and thus also the same data
     * pointed to. This is a stronger condition than
     * <code>operator==</code>. This method returns true only if one
     * Pointer has been copied to other Pointer's. But as soon as some
     * "raw C pointers" have been assigned to different Pointer, this
     * method would return false.  In that latter case, the
     * <code>operator==</code> would still return true, so that is why
     * the <code>operator==</code> is more likely to be useful.
     *
     * @author Martin Preuss<openhbci@aquamaniac.de> */
    bool sharingData(const Pointer<T> &p) const {
        return PointerBase::sharingData(p);
    };
    /*@}*/

};


#ifndef DOXYGEN_HIDE

/**
 *
 */
template <class T> class PointerCastBase {
protected:
    PointerCastBase();
    ~PointerCastBase();

    static Pointer<T> makePointer(const PointerBase &p) {
      return Pointer<T>(p);
    };
};


/**
 * This class lets you safely cast one Pointer to another one.
 * It will automatically perform type checking. If this class is unable to
 * cast then it throws an HBCI::Error.
 * You can use this if you have a Pointer for an object but
 * you need a pointer for a base object. For example:
 * <pre>
 * class BaseClass {
 *   BaseClass();
 *   ~BaseClass();
 * };
 *
 * class InheritingClass: public BaseClass {
 *   InheritingClass();
 *   ~InheritingClass();
 * };
 *
 * Pointer<InheritingClass> pInheriting;
 * Pointer<BaseClass> pBase;
 *
 * pInheriting=new InheritingClass();
 * pBase==PointerCast<BaseClass,InheritingClass>::cast(pInheriting);
 * </pre>
 * @author Martin Preuss<martin@libchipcard.de>
 */
template <class T, class U> class PointerCast
:public PointerCastBase<T>
{
public:
    /**
     * If the first template parameter is the base class to the second
     * template parameter then this method will cast a pointer to an
     * inheriting class into a pointer to the base class (downcast).
     * If you want just the opposite then you only need to exchange the order
     * of the template parameters (upcast).
     * @author Martin Preuss<martin@libchipcard.de>
     */
    static Pointer<T> cast(const Pointer<U> &u) {
        U *uo;
        T *t;
        Pointer<T> np;

        /* check if given pointer is valid */
        if (!u.isValid()) {
          DBG_WARN(0, "Casting an invalid pointer (%s)",
                   u.description().c_str());
          return 0;
        }

        /* then try to cast the pointer */
        uo=u.ptr();
        t=dynamic_cast<T*>(uo);

	/* could we cast it ? */
        assert(t!=0);
        if (t==0) {
          /* no, throw */
#ifdef ASSERT_ON_ERROR
          assert(t);
#endif
          throw HBCI::Error ("PointerCast::cast",
                             ERROR_LEVEL_NORMAL,
                             0,
                             ERROR_ADVISE_DONTKNOW,
                             "Bad cast",
                             u.description());
        }
        /* otherwise create a new pointer */
        np=makePointer(u);
        np.setDescription("Casted from "+u.description());
        return np;
    };

};

#endif /* DOXYGEN_HIDE */

} /* namespace HBCI */
#endif /* __cplusplus */
#endif /* HBCIPOINTER_H */

