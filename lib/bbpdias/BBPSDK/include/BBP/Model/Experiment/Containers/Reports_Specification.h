/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors:    Juan Hernando Vieites
                                Thomas Traenkler

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_REPORTS_SPECIFICATION_H
#define BBP_REPORTS_SPECIFICATION_H

#include <map>

#include "BBP/Model/Experiment/Report_Specification.h"

namespace bbp {

// ----------------------------------------------------------------------------

//! A set of report specifications for reports of an experiment.
/*! 
    The class is a container for simulation report specifications.
    \sa Report_Specification Report Compartment_Report Experiment
    \ingroup Experiment_Containers
*/
class Reports_Specification
{
public:
    /*!
        \name Access (read-only)
        For access and navigation inside the container.
    */
    //@{
    //! Forward iterator for container to visit all elements (read-only).
    class const_iterator;
    //! Get first element of the container for const linear access.
    inline const_iterator begin() const;
    //! Get const iterator that indicates the end of the container.
    inline const_iterator end() const;
    /*! 
      Get report via global identifier.
      Iterator to end returned if report not found
    */
    inline const_iterator find(const Label & name) const;
    //! Get number of reports in the container.
    inline size_t size() const;
    /*!
        \name Access (full)
        For access and navigation inside the container.
    */
    //@{

    //! Forward iterator for container to visit all elements (full access).
    class iterator;
    //! Get first element of the container for const linear access.
    inline iterator begin();
    //! Get const iterator that indicates the end of the container.
    inline iterator end();
    /*!
      Get report via global identifier.
      Iterator to end returned if report not found
    */
    inline iterator find(const Label & name);
    //@}

    /*!
        \name Management
        Insert to and remove elements from the container.
    */
    //@{
    inline void insert(const Report_Specification & report);

    //! Erase reports with specified global identifier.
    inline void erase(const Label & name);

    //! Remove all reports.
    inline void clear();
    //@}

    //! Print the reports inside this container
    inline void print() const;

protected:
    typedef std::map<Label, Report_Specification> Reports_Specification_Map;
    Reports_Specification_Map        _elements;
};

//! Outputs basic textual report information to the standard output stream.
inline std::ostream & operator << (std::ostream & lhs, 
                                   const Reports_Specification & rhs);
}
#include "BBP/Model/Experiment/Report_Specification.h"

namespace bbp
{

#ifndef SWIG // ---------------------------------------------------------------

class Reports_Specification::iterator
    : public std::iterator<std::forward_iterator_tag, 
        Report_Specification>
{
    friend class Reports_Specification;
    friend class Reports_Specification::const_iterator;

public:
    iterator()
    {}

private:
    explicit iterator(Reports_Specification::
        Reports_Specification_Map::iterator iterator)
        : _current(iterator)
    {}

public:
    //! Compare two iterators for equality.
    template <class Other_Iterator>
    bool operator == (const Other_Iterator & other) const
    {
        return (_current == other._current);
    }

    //! Compare two iterators for inequality.
    template <class Other_Iterator>
    bool operator != (const Other_Iterator & other) const
    {
        return (_current != other._current);
    }

    //! Advance to next element in the container.
    iterator & operator ++()
    {
        ++_current;
        return * this;
    }

    //! Advance to next element in the container, but return previous.
    iterator operator ++(int)
    {
        iterator temp(*this);
        ++_current;
        return temp;
    }

    //! Get (dereference) element at current iterator position.
    Report_Specification & operator * () const
    {
        return _current->second;
    }

    //! Get pointer to element at current iterator position.
    Report_Specification * operator -> () const
    {
        return & (this->operator *());
    }

private:
    Reports_Specification::Reports_Specification_Map::iterator _current;

};

// ----------------------------------------------------------------------------

class Reports_Specification::const_iterator
    : public std::iterator<std::forward_iterator_tag, 
        const Report_Specification>
{
    friend class Reports_Specification;
    friend class Reports_Specification::iterator;

public:
    const_iterator()
    {}

    const_iterator(Reports_Specification::iterator other) :
        _current(other._current)
    {}

private:
    explicit const_iterator(Reports_Specification::
        Reports_Specification_Map::const_iterator iterator)
        : _current(iterator)
    {}

public:
    //! Compare two iterators for equality.
    template <class Other_Iterator>
    bool operator == (const Other_Iterator & other) const
    {
        return (_current == other._current);
    }

    //! Compare two iterators for inequality.
    template <class Other_Iterator>
    bool operator != (const Other_Iterator & other) const
    {
        return (_current != other._current);
    }

    //! Advance to next element in the container.
    const_iterator & operator ++()
    {
        ++_current;
        return * this;
    }

    //! Advance to next element in the container, but return previous.
    const_iterator operator ++(int)
    {
        const_iterator temp(*this);
        ++_current;
        return temp;
    }

    //! Get (dereference) element at current iterator position.
    const Report_Specification & operator * () const
    {
        return _current->second;
    }

    //! Get pointer to element at current iterator position.
    const Report_Specification * operator -> () const
    {
        return & (this->operator *());
    }

private:
    Reports_Specification::Reports_Specification_Map::const_iterator _current;

};

// ----------------------------------------------------------------------------

Reports_Specification::iterator Reports_Specification::
    find(const Label & name)
{
    return iterator(_elements.find(name));
}

// ----------------------------------------------------------------------------

Reports_Specification::iterator Reports_Specification::begin(void)
{
    return iterator (_elements.begin());
}

// ----------------------------------------------------------------------------

Reports_Specification::iterator Reports_Specification::end(void)
{
    return iterator (_elements.end());
}

// ----------------------------------------------------------------------------

Reports_Specification::const_iterator Reports_Specification::
    find(const Label & name) const
{
    return const_iterator(_elements.find(name));
}

// ----------------------------------------------------------------------------

Reports_Specification::const_iterator Reports_Specification::begin(void) const
{
    return const_iterator (_elements.begin());
}

// ----------------------------------------------------------------------------

Reports_Specification::const_iterator Reports_Specification::end(void) const
{
    return const_iterator (_elements.end());
}

// ----------------------------------------------------------------------------

void Reports_Specification::erase(const Label & name)
{
    _elements.erase(name);
}

// ----------------------------------------------------------------------------

void Reports_Specification::clear()
{
    Reports_Specification_Map _;
    _elements.swap(_);
}

// ----------------------------------------------------------------------------

size_t Reports_Specification::size() const
{
    return _elements.size();
}

// ----------------------------------------------------------------------------

void Reports_Specification::print() const
{
    std::cout << *this;
}

// ----------------------------------------------------------------------------

void Reports_Specification::insert(const Report_Specification & report)
{
    _elements.insert(std::make_pair(report.label(), report));
}

// ----------------------------------------------------------------------------

std::ostream & operator << 
    (std::ostream & lhs, const Reports_Specification & rhs)
{
    for (Reports_Specification::const_iterator i = rhs.begin();
         i != rhs.end();
         ++i)
    {
        lhs << *i << std::endl;
    }
    return lhs;
}

#endif // SWIG ----------------------------------------------------------------

}
#endif
