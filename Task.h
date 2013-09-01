/**
Copyright (c) 2013, Riccardo Ressi
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list
of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

Neither the name of Riccardo Ressi nor the names of its contributors may be
used to endorse or promote products derived from this software without specific
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <assert.h>
#include <memory>

#include "Message.h"

#ifndef TASK_H
#define TASK_H

// ------------------------------------------------------------------------

class ITask;
typedef std::shared_ptr< ITask > Task;

/**
 * @brief Abstract class to be implemented to describe a task that need to be
 * executed.
 */
class ITask
    : public IMessage
{

public:

    /**
     * @brief Destructor.
     */
    virtual ~ITask( )
    { }

    /**
     * @brief Executes the task.
     */
    virtual void execute( ) = 0;

    /**
     * @brief Cancels the task.
     */
    virtual void cancel( )
    { }

};

// -----------------------------------------------------------------------------

template< typename Predicate >
class TaskFunc
    : public ITask
{
    Predicate& m_predicate;

public:

    TaskFunc( Predicate& predicate )
        : m_predicate( predicate )
    { }

    virtual void execute( )
    {
        m_predicate( );
    }

};

// -----------------------------------------------------------------------------

#endif // THREAD_H
