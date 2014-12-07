/*
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

#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include "Message.h"

#include <cstddef>
#include <limits>
#include <memory>

#include <assert.h>

// ----------------------------------------------------------------------------

/**
 * @brief General purpose message queue for inter-thread communication.
 *
 * This class uses run-time polymorphism to allow message-driven communication
 * and synchronization between two or more threads.
 *
 * The class is defined as a pure abstract class with a factory method
 * (see @ref IMessageQueue::create) to build platform-specific implementations
 * of the queue while maintaining a platform-agnostic interface.
 *
 * @note
 * - Only one method of this class can (optionally) block the calling thread:
 *   @ref IMessageQueue::pop.
 * - This class is 100% thread safe.
 */
class IMessageQueue
{

public:

    /**
     * @brief Factory method to create a message queue implemented for the
     * current platform.
     *
     * @param max_capacity Maximum number of messages that can be queued at
     *        the same time. By default this limit is relaxed as much as
     *        possible.
     *
     * @return The newly created message queue.
     */
    static IMessageQueue *create(std::size_t max_capacity
                                     = std::numeric_limits<std::size_t>::max());

    /**
    * @brief Destructor.
     */
    virtual ~IMessageQueue()
    {
    }

    /**
     * @brief Pushes one message into the queue.
     *
     * @param message The message to be inserted.
     *
     * @return
     * - On success, the number of messages contained by the queue after the
     *   insertion, that is at least @a one.
     * - On failure, @a zero. This may happen if the maximum allowed capacity
     *   for the queue have been reached.
     *
     * @pre
     * - The parameter message is not null.
     * - The queue have not been cancelled.
     */
    virtual std::size_t push(Message message) = 0;

    /**
     * @brief Pops one message from the queue.
     *
     * @param[out] message Smart pointer that will be reset with the popped
     *             message in case of success.
     *
     * @param blocking If set to @a true the method blocks the current thread
     *        indefinitely until a new message is pushed into the queue by
     *        another thread or until the queue is not cancelled.
     *
     * @return
     * - On success, the number of messages contained by the queue before the
     *   extraction, that is at least @a one.
     * - On failure, @a zero (parameter message is not touched in that case).
     *
     * @pre
     * - The queue have not been cancelled.
     */
    virtual std::size_t pop(Message &message, bool blocking) = 0;

    /**
     * @brief Cancel the queue functionality indefinitely releasing any blocked
     * thread.
     *
     * The cancelled status is not reversible and is meant mainly as an action
     * to be performed before the queue destruction.
     *
     * @note Doesn't wait for the peer threads to be released, just broadcast
     * a signal to them.
     */
    virtual void cancel() = 0;

    /**
     * @brief Returns @a true if the queue have been cancelled.
     */
    virtual bool is_cancelled() const = 0;

    /**
     * @brief Returns the number of messages contained inside the queue.
     */
    virtual std::size_t size() const = 0;

    /**
     * @brief Convenient template method to pop messages.
     *
     * Since by design the user should derive messages from the class @ref
     * IMessage this template take care to dynamic cast shared pointers to
     * the user defined derived class.
     *
     * This method is meant to be used when the user uses the queue with one
     * single derived class and hence don't need to manually dynamic cast every
     * popped message from the generic @a IMessage.
     *
     * @copydetails pop(Message& message, bool blocking)
     */
    template<typename Derived>
    std::size_t
    popT(std::shared_ptr<Derived> &message, bool blocking)
    {
        Message abstract_message;
        std::size_t ret = pop(abstract_message, blocking);
        if (ret > 0)
        {
            assert(abstract_message.get() != nullptr);
            message = std::dynamic_pointer_cast<Derived>(abstract_message);
            assert(message.get() == abstract_message.get());
        }

        return ret;
    }

};


// ----------------------------------------------------------------------------

/**
 * @brief General purpose message queue for inter-thread communication.
 *
 * This template class uses compile-time polymorphism to allow message-driven
 * communication and synchronization between two or more threads.
 *
 * The implementation of this template is based on @ref IMessageQueue class.
 *
 * @note
 * - Only one method of this class can (optionally) block the calling thread:
 *   @ref IMessageQueue::pop.
 * - This class is 100% thread safe.
 */
template<typename M>
class MessageQueueT
{

public:

    /**
     * @brief Constructor.
     *
     * @param max_capacity Maximum number of messages that can be queued at
     *        the same time. By default this limit is relaxed as much as
     *        possible.
     */
    explicit inline MessageQueueT(std::size_t max_capacity
                                     = std::numeric_limits<std::size_t>::max());

    /**
    * @brief Pops one message from the queue.
     *
     * @param[out] message A reference to a message object meant to be set
     *             with the extracted message only in case of success.
     *
     * @param block If set to @a true the method blocks the current thread
     *        indefinitely until a new message is pushed into the queue
     *        by another thread or until the queue is not cancelled.
     *
     * @return
     * - On failure, @a zero (parameter message is not touched in that case).
     * - On success, the number of messages contained by the queue before the
     *   extraction that is at least @a one.
     *
     * @pre
     * - The queue have not been cancelled.
     */
    inline std::size_t pop(M &dst_message, bool block);

    /**
     * @brief Pushes one message into the queue.
     *
     * @param message The message to be inserted.
     *
     * @return
     * - On failure, @a zero. This may happen if the maximum allowed capacity
     *   for the queue have been reached.
     * - On success, the number of messages contained by the queue after the
     *   insertion that is at least @a one.
     *
     * @pre
     * - The queue have not been cancelled.
     */
    inline std::size_t push(const M &message);

    /**
     * @copydoc IMessageQueue::cancel()
     */
    inline void cancel();

    /**
     * @copydoc IMessageQueue::is_cancelled()
     */
    inline bool is_cancelled() const;

    /**
     * @copydoc IMessageQueue::size()
     */
    inline std::size_t size() const;

private:

    std::shared_ptr<IMessageQueue> m_impl;

    template<typename P>
    class MessageImpl
            : public IMessage
    {

    public:

        P m_payload;

        MessageImpl(const P &payload)
                : m_payload(payload)
        {
        }

        virtual ~MessageImpl()
        {
        }
    };

};

// ----------------------------------------------------------------------------

template<typename M>
MessageQueueT<M>::MessageQueueT(std::size_t max_capacity)
        : m_impl(IMessageQueue::create(max_capacity))
{
}

// ----------------------------------------------------------------------------

template<typename M>
std::size_t
MessageQueueT<M>::pop(M &dst_message, bool blocking)
{
    Message abstract_message;
    std::size_t ret = m_impl->pop(abstract_message, blocking);

    if (ret > 0)
    {
        assert(nullptr != abstract_message.get());

        typedef MessageImpl<M> Implementation;
        auto message =
                std::dynamic_pointer_cast<Implementation>(abstract_message);
        assert(message.get() == abstract_message.get());

        dst_message = message->m_payload;
    }

    return ret;
}

// ----------------------------------------------------------------------------

template<typename M>
std::size_t
MessageQueueT<M>::push(const M &message)
{
    Message new_message(new MessageImpl<M>(message));

    return m_impl->push(new_message);
}

// ----------------------------------------------------------------------------

template<typename M>
void
MessageQueueT<M>::cancel()
{
    m_impl->cancel();
}

// ----------------------------------------------------------------------------

template<typename M>
bool
MessageQueueT<M>::is_cancelled() const
{
    return m_impl->is_cancelled();
}

// ----------------------------------------------------------------------------

template<typename M>
std::size_t
MessageQueueT<M>::size() const
{
    return m_impl->size();
}

#endif // MESSAGEQUEUE_H
