/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Responsible authors:	Thomas Traenkler
		Contributing authors:	Juan Hernando Vieites

*/

#include "BBP/Model/Microcircuit/Containers/Neurons.h"

namespace bbp {

#ifdef BBP_SAFETY_MODE
#  define CHECK_MICROCIRCUIT_POINTER(m)              \
    if ((m).lock().get() == 0)                       \
    {                                                \
        throw_exception(Microcircuit_Access_Error(), \
            FATAL_LEVEL, __FILE__, __LINE__);        \
    }
#  define CHECK_CONTAINER_MICROCIRCUIT_POINTER(c)                         \
    if ((c)._microcircuit.lock().get() == 0 && (c)._elements.size() != 0) \
    {                                                                     \
        throw_exception(Microcircuit_Access_Error(),                      \
            FATAL_LEVEL, __FILE__, __LINE__);                             \
    }
#else
#  define CHECK_MICROCIRCUIT_POINTER(m) bbp_assert((m).lock().get() != 0)
#  define CHECK_CONTAINER_MICROCIRCUIT_POINTER(c) \
    bbp_assert((c)._microcircuit.lock().get() != 0 || (c)._elements.size() == 0)
#endif

// ----------------------------------------------------------------------------

Neurons::Neurons(const Cell_Target & cell_target, Microcircuit & microcircuit) 
    : _label(cell_target.label())
{
    Neurons & neurons = microcircuit.neurons();
    _microcircuit = neurons._microcircuit;

    for (Cell_Target::const_iterator i = cell_target.begin(); 
         i != cell_target.end();
         ++i) 
    {
        Neurons::iterator neuron = neurons.find(*i);
        /** \bug This situation may happen in user code quite easily, 
                  how do we deal with it? */
        bbp_assert(neuron != neurons.end());
        insert(neuron);
    }
}

// \todo Clean up this ugly hack

void Neurons::load_from_cell_target_and_microcircuit(const Cell_Target & cell_target, Microcircuit & microcircuit)
{
    _label =cell_target.label();
    Neurons & neurons = microcircuit.neurons();
    _microcircuit = neurons._microcircuit;

    for (Cell_Target::const_iterator i = cell_target.begin(); 
         i != cell_target.end();
         ++i) 
    {
        Neurons::iterator neuron = neurons.find(*i);
        /** \bug This situation may happen in user code quite easily, 
                  how do we deal with it? */
        bbp_assert(neuron != neurons.end());
        insert(neuron);
    }
}



// ----------------------------------------------------------------------------

Neurons::Neurons(const Target & target, Microcircuit & microcircuit) 
    : _label(target.name())
{
    Neurons &neurons = microcircuit.neurons();
    Target t = target.flatten();

    for(Target::cell_iterator i = t.cell_begin(); i != t.cell_end(); i++)
    {
        Neurons::iterator neuron = neurons.find(*i);
        if(neuron != neurons.end())
        {
            insert(neuron);
        }
        else
        {
            log_message("Attempted to add a non-existent cell to a Neurons.",
            WARNING_LEVEL,__FILE__,__LINE__);
        }
    }
}

// ----------------------------------------------------------------------------

Neurons::operator Cell_Target () const
{
    Cell_Target cell_target;
    for (Neurons::const_iterator i = begin(); i != end(); ++i)
    {
        cell_target.insert(i->gid());
    }
    return cell_target;
}

// ----------------------------------------------------------------------------

const Neurons Neurons::operator & (const Circle<Micron> & circle) const
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    Neurons result(_microcircuit.lock());

    //! \todo Fix commented out code below. (TT)
    //result._description = _description + " within a radius of " + 
    //    boost::lexical_cast<const Word>(radius) + " from the center point ("
    //    + 
    //    boost::lexical_cast<const Word>(circle.center().x()) + ", " +
    //    boost::lexical_cast<const Word>(circle.center().z()) + ")";

    for (Neurons::const_iterator i = begin(); i != end(); ++i )
    {
		const Neuron & neuron = * i;
        Micron distance =
            std::sqrt((neuron.position().x() - circle.center().x()) * 
                      (neuron.position().x() - circle.center().x()) + 
                      (neuron.position().z() - circle.center().z()) * 
                      (neuron.position().z() - circle.center().z()));
#ifndef NDEBUG
        std::cout << "Distance:" << distance << std::endl;
#endif

        if (distance <= circle.radius())
        {
#ifndef NDEBUG
            std::cout << "Neuron a" << neuron.gid() << " at position " 
                      << neuron.position() << " is within a radius of " 
                      << circle.radius() << std::endl;
#endif
            result.insert(boost::const_pointer_cast
                <Neuron, const Neuron>(i.ptr()));
        }
#ifndef NDEBUG
        else
        {
            std::cout << "Neuron a" << neuron.gid() << " at position " 
                      << neuron.position() << " is NOT within a radius of " 
                      << circle.radius() << std::endl;
        }
#endif
    }
    return result;
}

// ----------------------------------------------------------------------------

Neurons Neurons::operator & (const Circle<Micron> & circle)
{
    CHECK_CONTAINER_MICROCIRCUIT_POINTER(*this);
    Neurons result(_microcircuit.lock());

    //! \todo Fix commented out code below. (TT)
    //result._description = _description + " within a radius of " + 
    //    boost::lexical_cast<const Word>(radius) + " from the center point ("
    // + 
    //    boost::lexical_cast<const Word>(circle.center().x()) + ", " +
    //    boost::lexical_cast<const Word>(circle.center().z()) + ")";

    for (Neurons::iterator i = begin(); i != end(); ++i )
    {
		const Neuron & neuron = * i;
        Micron distance =
            std::sqrt((neuron.position().x() - circle.center().x()) * 
                      (neuron.position().x() - circle.center().x()) + 
                      (neuron.position().z() - circle.center().z()) * 
                      (neuron.position().z() - circle.center().z()));
#ifndef NDEBUG
        std::cout << "Distance:" << distance << std::endl;
#endif

        if (distance <= circle.radius())
        {
#ifndef NDEBUG
            std::cout << "Neuron a" << neuron.gid() << " at position " 
                      << neuron.position() << " is within a radius of " 
                      << circle.radius() << std::endl;
#endif
            result.insert(i.ptr());
        }
#ifndef NDEBUG
        else
        {
            std::cout << "Neuron a" << neuron.gid() << " at position " 
                      << neuron.position() << " is NOT within a radius of " 
                      << circle.radius() << std::endl;
        }
#endif
    }
    return result;
}

// ----------------------------------------------------------------------------

Neurons & Neurons::operator += (const Neuron_Ptr & neuron)
{
    insert(neuron);
    return * this;
}

// ----------------------------------------------------------------------------

const Neurons Neurons::operator + (const Const_Neuron_Ptr & neuron) const
{
    bbp_assert(_elements.find(neuron->gid()) == _elements.end() ||
           _elements.find(neuron->gid())->second == neuron);

	Neurons temp(* this);
    temp.insert(boost::const_pointer_cast<Neuron, const Neuron>(neuron));
    return * this;
}

// ----------------------------------------------------------------------------

Neurons Neurons::operator + (const Neuron_Ptr & neuron)
{
    bbp_assert(_elements.find(neuron->gid()) == _elements.end() ||
           _elements.find(neuron->gid())->second == neuron);

	Neurons temp(* this);
    temp.insert(neuron);
    return * this;
}

// ----------------------------------------------------------------------------

}
