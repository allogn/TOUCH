/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project 
        (c) 2006-2007. All rights reserved.

        Responsible authors:    Thomas Traenkler
                                Juan Hernando

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_BUFFER_H
#define BBP_BUFFER_H

#include <vector>

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

#include "BBP/Common/Types.h"
#include "BBP/Common/Exception/Exception.h"

//#define DEBUG_BUFFER

namespace bbp {

// ----------------------------------------------------------------------------

typedef unsigned long Buffer_Size;

// ----------------------------------------------------------------------------

//! Buffer template class storing objects of an arbitrary type.
/*!
    The buffer class is a C++ class template buffering a fixed size of objects
    of the same type in a linear array. The interface provides a FIFO (first
    in first out) access to the objects, similar to a queue. 
    
    When the buffer is full, write requests will be blocked until some space
    is available. New objects are copied to the end of the buffer with 
    write, the first object in the queue will be returned by read.
*/
template <typename T>
class Buffer
{
public:
    class Buffer_Interruption : public std::exception {};

    //! Signaling an interruption of the buffer operations.
    class Interruption
    {
        friend class Buffer;

//#ifdef BBP_THREADS_SUPPORTED
    public:
        //! Interrupts any write and read operation on the associated buffer
        /*! Operations may be interrupted if they are waiting for the buffer
            that created the signal to get filled (reads) or not empty
            (writes).
            This operation may block the called for a finite little amount
            of time. */
        void signal()
        {
            boost::mutex::scoped_lock lock(*mutex);
            signalled = true;
            condition->notify_all();
        }

    protected:
        boost::mutex *mutex;
        boost::condition *condition;
        bool signalled;

        Interruption(boost::mutex *mutex, boost::condition *condition) 
        : mutex(mutex),
          condition(condition),
          signalled(false)
        {}
//#endif
    };

    //! Creates buffer with n=10 buffered elements.
    Buffer();
    //! Creates buffer with specified number of buffered elements.
    Buffer(Buffer_Size size);
    ~Buffer(void);

    //! assign value to buffer (same as write to buffer)
    inline Buffer & operator = (const T & rhs);

    //! implicit conversion of current read position in buffer to an object
    inline operator T ();

    //! Resizes the buffer and clears its contents.
    void resize(Buffer_Size new_size);

    //! Checks if the buffer is full.
    inline bool is_full() const;
    //! Checks if the buffer is empty.
    inline bool is_empty() const;
    inline size_t queue_size() const;
    /*! Returns the size of the buffer (i.e. number of elements that 
        can be buffered). */
    inline size_t size() const;

    /*! Returns the object currently in the head of the buffer.
        Blocks if buffer empty. */
    inline T & read(Interruption *interruption = 0)
    throw (Buffer_Interruption);
    //! advance the position to be read from the buffer
    inline void advance_read_position();

    //! Atomic write operation
    /*! Blocks until the buffer is not full. */
    inline void write(const T & new_object, Interruption *interruption = 0)

    throw (Buffer_Interruption);
    /*! Returns the element at the end of the buffer (i.e. the next write 
        position). This function allows to externally fill the buffer e.g.
        when reading from disk without a need for an intermediate copy. 
        This function must not be used with multiple writers unless they
        are externally synchronized, otherwise there are write after write
        hazards. */
    inline T & write_position(Interruption *interruption = 0)

    throw (Buffer_Interruption);

    //! Advances the write position making a new object available in the buffer.
    /*! The correct sequence of calls is:
        \code
        T &o = write_position()
        ... write something into o ...
        adavance_write_position
        \endcode
        No other thread should try to write the same position or call
        write() until this function is not called. Otherwise write after
        writes hazards will occur. */
    inline void advance_write_position()

    throw (Buffer_Interruption);

    //! clears the buffer contents (does not deallocate memory)
    /*! This call will get blocked while there are pending writes
        (a pending write is an incomplete sequence of calls to
        write_position and advance_write_position call) */
    void clear();

    //! deallocates the buffer memory 
    void free();

//#ifdef BBP_THREADS_SUPPORTED
    Interruption * create_write_interruption()
    {
        return new Interruption(&_command_lock, &_write_block);
    }

    Interruption * create_read_interruption()
    {
        return new Interruption(&_command_lock, &_read_block);
    }
//#endif

private:
    //! vector storing the elements of the buffer
    std::vector<T>                      _elements;
    //! position in the buffer where to continue reading
    typename std::vector<T>::iterator   _read_position;
    //! position in the buffer where to continue writing
    typename std::vector<T>::iterator   _write_position;
    //! flag indicating if buffer is full
    bool                                _full;
    //! flag indicating if buffer is empty
    bool                                _empty;
//#ifdef BBP_THREADS_SUPPORTED
    boost::mutex                        _command_lock;
    boost::condition                    _read_block,
                                        _write_block;
//#endif
    boost::uint32_t                     _writes_pending;
};


//! output element at current read position in buffer
/*! \bug The specification and implementation of this function needs 
    reviewing. This function is not thread safe. */
template <typename T>
std::ostream & operator << (std::ostream & lhs, Buffer<T> & rhs);

// ----------------------------------------------------------------------------

template <typename T>
std::ostream & operator << (std::ostream & lhs, Buffer<T> & rhs)
{
    lhs << "Buffer: ";
    if (rhs.is_empty())
        lhs << "empty";
    else
        lhs << "head: " << rhs.read();
    return lhs;
}

// ----------------------------------------------------------------------------

template <typename T>
Buffer<T>::Buffer() :
    _writes_pending(0)
{
    resize(10);
}

// ----------------------------------------------------------------------------

template <typename T>
Buffer<T>::Buffer(Buffer_Size size) :
    _writes_pending(0)
{
    resize(size);
}

// ----------------------------------------------------------------------------

template <typename T>
Buffer<T>::~Buffer(void)
{
}

// ----------------------------------------------------------------------------

template <typename T>
inline Buffer<T> & Buffer<T>::operator = (const T & rhs)
{
    write(rhs);
    return * this;
}

// ----------------------------------------------------------------------------

template <typename T>
inline Buffer<T>::operator T ()
{
    return read();
}

// ----------------------------------------------------------------------------

template <typename T>
inline size_t Buffer<T>::size() const
{
    return _elements.size();
}

// ----------------------------------------------------------------------------

template <typename T>
inline size_t Buffer<T>::queue_size() const
{
    if (_read_position == _write_position)
    {
        return _full ? size() : 0;
    }
    else if (_read_position < _write_position)
    {
        return _write_position - _read_position;
    }
    else
    {
        return size() - (_read_position - _write_position);
    }
}

// ----------------------------------------------------------------------------

template <typename T>
void Buffer<T>::resize(Buffer_Size new_size)
{
//#ifdef BBP_THREADS_SUPPORTED
    boost::mutex::scoped_lock lock(_command_lock);

    while (_writes_pending > 0)
    {
        _write_block.wait(lock);
    }
//#endif

    if (new_size == 0)
    {
        _elements.resize(0);
        _read_position = _elements.end();
        _write_position = _elements.end();
#if !defined NDEBUG && defined DEBUG_BUFFER
        std::clog << "Buffer size is " << size() << std::endl;
#endif
        _full = true;
        _empty = true;
    }
    else if (new_size == 1)
    {
        _elements.resize(new_size, * new T());
        _read_position = _elements.begin();
        _write_position = _elements.begin();
#if !defined NDEBUG && defined DEBUG_BUFFER
        std::clog << "Buffer size is " << size() << std::endl;
#endif
        _full = false;
        _empty = true;


        throw std::runtime_error(
            "bbp::data::Buffer<T> requires sizeto be 2 or more.");
    }
    else if (new_size >= 2)
    {
        _elements.resize(new_size, * new T());
        _read_position = _elements.begin();
        _write_position = _elements.begin();
#if !defined NDEBUG && defined DEBUG_BUFFER
        std::clog << "Buffer size is " << size() << std::endl;
#endif
        _full = false;
        _empty = true;
    }
}

// ----------------------------------------------------------------------------

template <typename T>
inline bool Buffer<T>::is_full() const
{
    if (_full == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// ----------------------------------------------------------------------------

template <typename T>
inline bool Buffer<T>::is_empty() const
{
    if (_empty == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// ----------------------------------------------------------------------------

template <typename T>
inline T & Buffer<T>::write_position(Interruption * interruption)

   throw (Buffer_Interruption)

{
//#ifdef BBP_THREADS_SUPPORTED
    // lock buffer access
    boost::mutex::scoped_lock lock(_command_lock);
//#endif

    if (interruption)
    {
//#ifdef BBP_THREADS_SUPPORTED
        while (_full == true && !interruption->signalled)
        {
#if !defined NDEBUG && defined DEBUG_BUFFER
            std::cout << "Buffer is full, write_position waits" 
                    << std::endl;
#endif
            _write_block.wait(lock);
        }
        if (interruption->signalled)
        {
#if !defined NDEBUG && defined DEBUG_BUFFER
            std::cout << "Buffer write_position interrupted" << std::endl;
#endif
            interruption->signalled = false;
            throw Buffer_Interruption();
        }
//#endif
    } 
    else 
    {
//#ifdef BBP_THREADS_SUPPORTED
        while (_full == true)
        {
#if !defined NDEBUG && defined DEBUG_BUFFER
    std::cout << "Buffer is full, write_position waits "
        "(not interruptable)" << std::endl;
#endif
            _write_block.wait(lock);
        }
//#endif
    }

    _writes_pending++;

    return * _write_position;
}

// ----------------------------------------------------------------------------

template <typename T>
inline void Buffer<T>::advance_write_position()

   throw (Buffer_Interruption)

{
//#ifdef BBP_THREADS_SUPPORTED
    // lock buffer access
    boost::mutex::scoped_lock lock(_command_lock);
//#endif

    bbp_assert(_full != true &&
           _writes_pending > 0);

    ++_write_position;
    if (_write_position == _elements.end())
    {
        _write_position = _elements.begin();
    }

    if (_read_position == _write_position)
    {
        _full = true;
    }
    _empty = false;
//#ifdef BBP_THREADS_SUPPORTED
    _read_block.notify_all();
//#endif

    --_writes_pending;

//#ifdef BBP_THREADS_SUPPORTED
    if (_writes_pending == 0)
    {
        _write_block.notify_all();
    }
//#endif
}

// ----------------------------------------------------------------------------

template <typename T>
inline void Buffer<T>::write(const T & new_object, Interruption * interruption)

   throw (Buffer_Interruption)

{
//#ifdef BBP_THREADS_SUPPORTED
    // lock buffer access
    boost::mutex::scoped_lock lock(_command_lock);
//#endif

    if (interruption)
    {
//#ifdef BBP_THREADS_SUPPORTED
        while (_full == true && !interruption->signalled)
        {
#if !defined NDEBUG && defined DEBUG_BUFFER
    std::cout << "Buffer is full, write waits" << std::endl;
#endif
            _write_block.wait(lock);
        }

        if (interruption->signalled)
        {
#if !defined NDEBUG && defined DEBUG_BUFFER
    std::cout << "Buffer write interrupted" << std::endl;
#endif
            interruption->signalled = false;
            _writes_pending = false;
            throw Buffer_Interruption();
        }
//#endif
    } 
    else 
    {
//#ifdef BBP_THREADS_SUPPORTED
        while (_full == true)
        {
#if !defined NDEBUG && defined DEBUG_BUFFER
    std::cout << "Buffer is full, write waits (not interruptable)" 
              << std::endl;
#endif
            _write_block.wait(lock);
        }
//#endif
    }

    // write into buffer with copy by value;
    * _write_position = new_object;

    ++_write_position;
    if (_write_position == _elements.end())
    {
        _write_position = _elements.begin();
    }

    if (_read_position == _write_position)
    {
        _full = true;
    }
    _empty = false;
//#ifdef BBP_THREADS_SUPPORTED
    _read_block.notify_all();
//#endif
}

// ----------------------------------------------------------------------------

template <typename T>
inline void Buffer<T>::advance_read_position()
{
    //std::cout << "Advance read position" << std::endl;
//#ifdef BBP_THREADS_SUPPORTED
    boost::mutex::scoped_lock lock(_command_lock);
//#endif

    if (_empty)
        // Buffer was cleared between a previous call to read and this call
        return;

    ++_read_position;

    if (_read_position == _elements.end())
    {
        _read_position = _elements.begin();
    }
    
    if (_read_position == _write_position)
    {
        _empty = true;
    }

    if (_full == true)
    {
//#ifdef BBP_THREADS_SUPPORTED
       _write_block.notify_all();
//#endif
       _full = false;
    }
}

// ----------------------------------------------------------------------------

template <typename T>
inline T & Buffer<T>::read(Interruption * interruption)

   throw (Buffer_Interruption)

{

//#ifdef BBP_THREADS_SUPPORTED
    //std::cout << "Reading from buffer" << std::endl;
    boost::mutex::scoped_lock lock(_command_lock);
//#endif

    if (interruption)
    {
//#ifdef BBP_THREADS_SUPPORTED
        while (_empty == true && !interruption->signalled)
        {
#if !defined NDEBUG && defined DEBUG_BUFFER
            std::cerr << "Buffer empty, read waits" << std::endl;
#endif
            _read_block.wait(lock);
        }

        if (interruption->signalled)
        {
#if !defined NDEBUG && defined DEBUG_BUFFER
            std::cerr << "Buffer read interrupted" << std::endl;
#endif
            interruption->signalled = false;
            throw Buffer_Interruption();
        }
//#endif
    } 
    else 
    {
#if !defined NDEBUG && defined DEBUG_BUFFER
    std::cout << "Buffer empty, read waits (not interruptable)" 
              << std::endl;
#endif
//#ifdef BBP_THREADS_SUPPORTED
        while (_empty == true)
        {
            _read_block.wait(lock);
        }
//#endif
    }

    return * _read_position;
}

// ----------------------------------------------------------------------------

template <typename T>
void Buffer<T>::clear()
{
//#ifdef BBP_THREADS_SUPPORTED
    boost::mutex::scoped_lock lock(_command_lock);
//#endif

    while (_writes_pending > 0)
    {
//#ifdef BBP_THREADS_SUPPORTED
        _write_block.wait(lock);
//#endif
    }

    if (size() > 0)
    {
        _read_position = _elements.begin();
        _write_position = _elements.begin();

        _full = false;
//#ifdef BBP_THREADS_SUPPORTED
        _write_block.notify_all();
//#endif
        _empty = true;
    }
#if !defined NDEBUG && defined DEBUG_BUFFER
    std::clog << "Buffer cleared" << std::endl;
#endif
}

// ----------------------------------------------------------------------------

template <typename T>
void Buffer<T>::free()
{
    resize(0);
}

// ----------------------------------------------------------------------------

}
#endif
