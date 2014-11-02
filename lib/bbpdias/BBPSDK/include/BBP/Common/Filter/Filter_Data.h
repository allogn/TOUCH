/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute
        Blue Brain Project & Thomas Traenkler
        (c) 2006-2007. All rights reserved.

        Responsible author:     Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_FILTER_DATA_H
#define BBP_FILTER_DATA_H

#include "Control/Filter_Data_Interface.h"

namespace bbp {

// ----------------------------------------------------------------------------


template <typename Data_Type, 
          typename Data_Context  = void>
          //bool     grouped       = false>
class Filter_Data;


// ------------ NOTE THE SPECIALIZATION FOR VOID BELOW! -----------------

/*!
    stream data object wrapper class for use in the pipeline
    \todo check implicit operations with filters and grouped data objects for
    operator precedence
    \todo grouped data currently assumes shared context. check this!
    \ingroup Pipeline
*/
template <typename Data_Type, typename Data_Context>
class Filter_Data //<Data_Type, Data_Context/*, false*/>
    : public Filter_Data_Interface
{
public:
    typedef typename boost::shared_ptr<Data_Type> Data_Type_Pointer;

    //! raw stream data object
    Data_Type_Pointer   object;
    //! context of the data object provides metainformation for the stream
    boost::shared_ptr<Data_Context>     context;

public:
    Filter_Data();
    Filter_Data (const Label & name);
    explicit Filter_Data (const Data_Type & t);
    Filter_Data (const Data_Type & t, const Label & name);
    explicit Filter_Data (const Data_Type_Pointer & t);
    Filter_Data (const Data_Type_Pointer & t, const Label & name);
    ~Filter_Data(void);

public:
    // Copies the given object into a new one
    void reset(const Data_Type & rhs);
    void reset(const Data_Type_Pointer & rhs);

    // implicit conversions
    inline operator Data_Type (void) const;

    //Filter_Data<Data_Type, Data_Context, true> operator + (Filter_Data & rhs);

    //! dereferences the pointer to the contained object
    Data_Type & operator * () const;
    //! dereferences the pointer to the contained object
    Data_Type * operator -> () const;

    inline bool operator == (Data_Type & rhs);
    inline bool operator >= (const Data_Type & rhs) const;
    inline bool operator > (const Data_Type & rhs) const;
    inline bool operator <= (const Data_Type & rhs) const;
    inline bool operator < (const Data_Type & rhs) const;

    void print() const;
    void print(std::ostream & rhs) const;

};

// ----------------------------------------------------------------------------

//template <typename Data_Type, typename Data_Context>
//class Filter_Data <Data_Type, Data_Context, true>
//    : public Filter_Data_Interface
//{
//public:
//    typedef typename boost::shared_ptr<Data_Type> Data_Type_Pointer;
//
//    //! raw stream data object
//    std::vector<Data_Type_Pointer>  objects;
//    //! context of the data object provides metainformation for the stream
//    boost::shared_ptr<Data_Context>     context;
//
//    Filter_Data();
//    Filter_Data (const Label name);
//    ~Filter_Data(void);
//    //explicit Filter_Data (const Data_Type t);
//    //Filter_Data (const Data_Type t, Label name);
//    //explicit Filter_Data (Data_Type_Pointer t);
//    //Filter_Data (Data_Type_Pointer t, Label name);
//
//
//    //// Copies the given object into a new one
//    //void reset(const Data_Type &rhs);
//    //void reset(Data_Type_Pointer rhs);
//
//    //// implicit conversions
//    //  inline operator Data_Type (void) const;
//
//    ////! dereferences the pointer to the contained object
//    //Data_Type & operator * () const;
//    ////! dereferences the pointer to the contained object
//    //Data_Type * operator -> () const;
//
//    //inline bool operator == (Data_Type & rhs);
//    //inline bool operator >= (const Data_Type & rhs) const;
//    //inline bool operator > (const Data_Type & rhs) const;
//    //inline bool operator <= (const Data_Type & rhs) const;
//    //inline bool operator < (const Data_Type & rhs) const;
//
//    //void print() const;
//    //void print(std::ostream & rhs) const;
//
//};

// ----------------------------------------------------------------------------

/*!
    stream data object wrapper class for use in the pipeline
    \todo check implicit operations with filters and grouped data objects for
    operator precedence
    \todo grouped data currently assumes shared context. check this!
    \ingroup Pipeline
*/
template <typename Data_Type>
class Filter_Data <Data_Type, void>//, false>
    : public Filter_Data_Interface
{
public:
    typedef typename boost::shared_ptr<Data_Type> Data_Type_Pointer;
    //! raw stream data object
    Data_Type_Pointer   object;

    Filter_Data()
        : object (new Data_Type)
    {
        connected = true;
    }

    Filter_Data (const Label & name) 
        : Filter_Data_Interface(name),
          object (new Data_Type)
    {
        connected = true;
    }
    
    explicit Filter_Data (const Data_Type & t)
        : object (new Data_Type(t))
    {
        connected = true;
    }

    Filter_Data (const Data_Type & t, const Label & name)
        : Filter_Data_Interface(name),
          object (new Data_Type(t))
    {
        connected = true;
    }

    explicit Filter_Data (const Data_Type_Pointer & t)
        : object(t)
    {
        connected = true;
    }

    Filter_Data (const Data_Type_Pointer & t, const Label & name)
        : Filter_Data_Interface(name),
          object(t)
    {
        connected = true;
    }

    ~Filter_Data(void) {}

    //! dereferences the pointer to the contained object
    Data_Type & operator * () const
    {
        return * object;
    }

    //! dereferences the pointer to the contained object
    Data_Type * operator -> () const
    {
        return object.get();
    }


    // Implicit conversions
    inline operator Data_Type (void) const;

    inline bool operator == (Data_Type & rhs)
    {
        /*! \bug This code doesn't compile with Compartment_Report_Frame
                 as a filter object */
//        if ((* object) == rhs)
//            return true;
//        else
//            return false;
        return object.get() == &rhs;
    }

    void reset(const Data_Type_Pointer & rhs)
    {
        object = rhs;
    }

    // Copies the given object into a new one
    void reset(const Data_Type & rhs)
    {
        object.reset(new Data_Type(rhs));
    }

    //Filter_Data<Data_Type, void, true> operator + (Filter_Data & rhs);

    inline bool operator >= (const Data_Type & rhs) const
    {
        if (* object >= rhs)
            return true;
        else
            return false;
    }
    inline bool operator > (const Data_Type & rhs) const
    {
        if (* object > rhs)
            return true;
        else
            return false;
    }
    inline bool operator <= (const Data_Type & rhs) const
    {
        if (* object <= rhs)
            return true;
        else
            return false;
    }
    inline bool operator < (const Data_Type & rhs) const
    {
        if (* object < rhs)
            return true;
        else
            return false;
    }

//    void print() const
//    {
//        std::cout << * object;
//    }
//
//    void print(std::ostream & rhs) const
//    {
//        rhs << * object;
//    }

};

// ----------------------------------------------------------------------------

//template <typename Data_Type>
//class Filter_Data <Data_Type, void, true>
//    : public Filter_Data_Interface
//{
//public:
//    typedef typename boost::shared_ptr<Data_Type> Data_Type_Pointer;
//
//    //! raw stream data object
//    std::vector<Data_Type_Pointer>  objects;
//
//    Filter_Data();
//    Filter_Data (const Label name);
//    ~Filter_Data(void);
//    //explicit Filter_Data (const Data_Type t);
//    //Filter_Data (const Data_Type t, Label name);
//    //explicit Filter_Data (Data_Type_Pointer t);
//    //Filter_Data (Data_Type_Pointer t, Label name);
//
//
//    //// Copies the given object into a new one
//    //void reset(const Data_Type &rhs);
//    //void reset(Data_Type_Pointer rhs);
//
//    //// implicit conversions
//    //  inline operator Data_Type (void) const;
//
//    ////! dereferences the pointer to the contained object
//    //Data_Type & operator * () const;
//    ////! dereferences the pointer to the contained object
//    //Data_Type * operator -> () const;
//
//    //inline bool operator == (Data_Type & rhs);
//    //inline bool operator >= (const Data_Type & rhs) const;
//    //inline bool operator > (const Data_Type & rhs) const;
//    //inline bool operator <= (const Data_Type & rhs) const;
//    //inline bool operator < (const Data_Type & rhs) const;
//
//    //void print() const;
//    //void print(std::ostream & rhs) const;
//
//
//};

// ----------------------------------------------------------------------------

template <>
class Filter_Data <void, void/*, false*/>
    : public Filter_Data_Interface
{
public:
    typedef boost::shared_ptr<bool> Data_Type_Pointer;

    //! raw stream data object
    Data_Type_Pointer   object;

    Filter_Data()
        : object (new bool)
    {
        * object = false;
        connected = true;
    }

    Filter_Data (const Label & name) 
        : Filter_Data_Interface(name),
          object (new bool)
    {
        * object = false;
        connected = true;
    }
    
    ~Filter_Data(void)
    {
    }

    //! dereferences the pointer to the contained object
    bool & operator * () const
    {
        return * object;
    }

};


// ----------------------------------------------------------------------------

template <typename Data_Type, typename Data_Context>
Filter_Data<Data_Type, Data_Context/*, false*/>::Filter_Data(void)
    : object(0)
{
}

// ----------------------------------------------------------------------------

template <typename Data_Type, typename Data_Context>
Filter_Data<Data_Type, Data_Context/*, false*/>::Filter_Data 
(const Label & name) 
    : Filter_Data_Interface(name),
      object(0) 
{
}

// ----------------------------------------------------------------------------

template <typename Data_Type, typename Data_Context>
Filter_Data<Data_Type, Data_Context/*, false*/>::Filter_Data 
(const Data_Type & t)
    : object (new Data_Type(t))
{
}

// ----------------------------------------------------------------------------

template <typename Data_Type, typename Data_Context>
Filter_Data<Data_Type, Data_Context/*, false*/>::Filter_Data
(const Data_Type & t, const Label & name)
    : Filter_Data_Interface(name),
      object (new Data_Type(t))
{
}

// ----------------------------------------------------------------------------

template <typename Data_Type, typename Data_Context>
Filter_Data<Data_Type, Data_Context/*, false*/>::Filter_Data
(const Data_Type_Pointer & t)
    : object(t)
{
}

// ----------------------------------------------------------------------------

template <typename Data_Type, typename Data_Context>
Filter_Data<Data_Type, Data_Context/*, false*/>::Filter_Data
(const Data_Type_Pointer & t, const Label & name)
    : Filter_Data_Interface(name),
      object(t)
{
}

// ----------------------------------------------------------------------------

template <typename Data_Type, typename Data_Context>
Filter_Data<Data_Type, Data_Context/*, false*/>::~Filter_Data(void)
{}

// ----------------------------------------------------------------------------

//template <typename Data_Type, typename Data_Context>
//Filter_Data<Data_Type, Data_Context, true> 
//Filter_Data<Data_Type, Data_Context, false>::operator + 
//(Filter_Data<Data_Type, Data_Context, false> & rhs)
//{
//    Filter_Data<Data_Type, Data_Context, true> group;
//    group.objects.push_back(object);
//    group.objects.push_back(rhs.object);
//    return group;
//}

// ----------------------------------------------------------------------------

//template <typename Data_Type>
//Filter_Data<Data_Type, void, true> 
//Filter_Data<Data_Type, void, false>::operator + 
//(Filter_Data<Data_Type, void, false> & rhs)
//{
//    Filter_Data<Data_Type, void, true> group;
//    group.objects.push_back(object);
//    group.objects.push_back(rhs.object);
//    return group;
//}

// ----------------------------------------------------------------------------

template <typename Data_Type, typename Data_Context>
Data_Type & Filter_Data<Data_Type, Data_Context/*, false*/>::operator * () const
{
    return * object;
}

// ----------------------------------------------------------------------------

template <typename Data_Type, typename Data_Context>
Data_Type * Filter_Data<Data_Type, Data_Context/*, false*/>::operator -> () const
{
    return object;
}

// ----------------------------------------------------------------------------

template <typename Data_Type, typename Data_Context>
bool Filter_Data<Data_Type, Data_Context/*, false*/>::operator == (Data_Type & rhs)
{
    if (* object == rhs)
        return true;
    else
        return false;
}

// ----------------------------------------------------------------------------

template <typename Data_Type, typename Data_Context>
void Filter_Data<Data_Type, Data_Context/*, false*/>::reset
(const Data_Type_Pointer & rhs)
{
    object = rhs;
}

// ----------------------------------------------------------------------------

template <typename Data_Type, typename Data_Context>
void Filter_Data<Data_Type, Data_Context/*, false*/>::reset
(const Data_Type &rhs)
{
    object.reset(new Data_Type(rhs));
}

// ----------------------------------------------------------------------------

template <typename Data_Type, typename Data_Context>
bool Filter_Data<Data_Type, Data_Context/*, false*/>::
operator >= (const Data_Type & rhs) const
{
    if (* object >= rhs)
        return true;
    else
        return false;
}

// ----------------------------------------------------------------------------

template <typename Data_Type, typename Data_Context>
bool Filter_Data<Data_Type, Data_Context/*, false*/>::
operator > (const Data_Type & rhs) const
{
    if (* object > rhs)
        return true;
    else
        return false;
}

// ----------------------------------------------------------------------------

template <typename Data_Type, typename Data_Context>
bool Filter_Data<Data_Type, Data_Context/*, false*/>::
operator <= (const Data_Type & rhs) const
{
    if (* object <= rhs)
        return true;
    else
        return false;
}

// ----------------------------------------------------------------------------

template <typename Data_Type, typename Data_Context>
bool Filter_Data<Data_Type, Data_Context/*, false*/>::
operator < (const Data_Type & rhs) const
{
    if (* object < rhs)
        return true;
    else
        return false;
}

// ----------------------------------------------------------------------------

template <typename Data_Type, typename Data_Context>
void Filter_Data<Data_Type, Data_Context/*, false*/>::print() const
{
    std::cout << * object;
}

// ----------------------------------------------------------------------------

template <typename Data_Type, typename Data_Context>
void Filter_Data<Data_Type, Data_Context/*, false*/>::
print(std::ostream & rhs) const
{
    rhs << * object;
}

// ----------------------------------------------------------------------------

template <typename Data_Type, typename Data_Context>
Filter_Data<Data_Type, Data_Context/*, false*/>::operator Data_Type (void) const
{
    return * object;
}

// ----------------------------------------------------------------------------

template <typename Data_Type, typename Data_Context>
std::ostream & operator << 
(std::ostream & lhs, Filter_Data<Data_Type, Data_Context/*, false*/> & rhs)
{
    lhs << * rhs;
    return lhs;
}

// ----------------------------------------------------------------------------

//template <typename Data_Type, typename Data_Context>
//Filter_Data<Data_Type, Data_Context, true> group(
//    Filter_Data<Data_Type, Data_Context, false> & first, 
//    Filter_Data<Data_Type, Data_Context, false> & second)
//{
//    Filter_Data<Data_Type, Data_Context, true> group;
//    group.objects.push_back(first);
//    group.objects.push_back(second);
//    return group;
//}
//
//// ----------------------------------------------------------------------------
//
//template <typename Data_Type>
//Filter_Data<Data_Type, void, true> group(
//    Filter_Data<Data_Type, void, false> & first, 
//    Filter_Data<Data_Type, void, false> & second)
//{
//    Filter_Data<Data_Type, void, true> group;
//    group.objects.push_back(first);
//    group.objects.push_back(second);
//    return group;
//}

// ----------------------------------------------------------------------------

template <typename Data_Type, typename Data_Context>
std::istream & operator >> 
(std::istream & lhs, 
 const Filter_Data<Data_Type, Data_Context/*, false*/> & rhs)
{
    lhs >> * rhs; 
    return lhs;
}

// ----------------------------------------------------------------------------

template <typename Data_Type>
Filter_Data<Data_Type, void/*, false*/>::operator Data_Type (void) const
{
    return * object;
}

// ----------------------------------------------------------------------------

template <typename Data_Type>
std::ostream & operator << 
(std::ostream & lhs, const Filter_Data<Data_Type, void/*, false*/> & rhs)
{
    lhs << * rhs;
    return lhs;
}

// ----------------------------------------------------------------------------

template <typename Data_Type>
std::istream & operator >> 
(std::istream & lhs, const Filter_Data<Data_Type, void/*, false*/> & rhs)
{
    lhs >> * rhs; 
    return lhs;
}

// ----------------------------------------------------------------------------

}
#endif
