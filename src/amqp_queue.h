#ifndef amqp_queue_h__
#define amqp_queue_h__

#include "amqp_channel.h"
#include "amqp_message.h"
#include "amqp_global.h"

namespace QAMQP
{

class Client;
class ClientPrivate;
class Exchange;
class QueuePrivate;
class QAMQP_EXPORT Queue : public Channel
{
    Q_OBJECT
    Q_ENUMS(QueueOptions)
    Q_PROPERTY(QueueOptions option READ option)
    Q_PROPERTY(QString consumerTag READ consumerTag WRITE setConsumerTag)
    Q_PROPERTY(bool noAck READ noAck WRITE setNoAck)

public:
    enum QueueOption {
        NoOptions = 0x0,
        Passive = 0x01,
        Durable = 0x02,
        Exclusive = 0x4,
        AutoDelete = 0x8,
        NoWait = 0x10
    };
    Q_DECLARE_FLAGS(QueueOptions, QueueOption)

    enum ConsumeOption {
        coNoLocal = 0x1,
        coNoAck = 0x02,
        coExclusive = 0x04,
        coNoWait = 0x8
    };
    Q_DECLARE_FLAGS(ConsumeOptions, ConsumeOption)

    ~Queue();
    QueueOptions option() const;

    void declare(const QString &name = QString(),
                 QueueOptions options = QueueOptions(Durable | AutoDelete));
    void remove(bool ifUnused = true, bool ifEmpty = true, bool noWait = true);
    void purge();

    void bind(const QString &exchangeName, const QString &key);
    void bind(Exchange *exchange, const QString &key);

    void unbind(const QString &exchangeName, const QString &key);
    void unbind(Exchange *exchange, const QString &key);

    MessagePtr getMessage();

    void get();
    void ack(const MessagePtr &message);
    bool hasMessage() const;
    void consume(ConsumeOptions options = ConsumeOptions(NoOptions));
    void setConsumerTag(const QString &consumerTag);
    QString consumerTag() const;

    void setNoAck(bool noAck);
    bool noAck() const;

Q_SIGNALS:
    void declared();
    void bound();
    void unbound();
    void removed();
    void messageReceived(Queue *pQueue);
    void empty();

protected:
    // reimp Channel
    virtual void channelOpened();
    virtual void channelClosed();

private:
    Queue(int channelNumber = -1, Client *parent = 0);

    Q_DISABLE_COPY(Queue)
    Q_DECLARE_PRIVATE(Queue)

    friend class Client;
};

} // namespace QAMQP

#endif