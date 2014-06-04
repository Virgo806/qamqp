#ifndef amqp_exchange_h__
#define amqp_exchange_h__

#include "amqp_channel.h"

namespace QAMQP
{

class Client;
class Queue;
class ClientPrivate;
class ExchangePrivate;
class QAMQP_EXPORT Exchange : public Channel
{
    Q_OBJECT
    Q_PROPERTY(QString type READ type)
    Q_PROPERTY(ExchangeOptions option READ option)
    Q_ENUMS(ExchangeOption)

public:
    enum ExchangeType {
        Direct,
        FanOut,
        Topic,
        Headers
    };
    QString type() const;

    enum ExchangeOption {
        NoOptions = 0x0,
        Passive = 0x01,
        Durable = 0x02,
        AutoDelete = 0x4,
        Internal = 0x8,
        NoWait = 0x10
    };
    Q_DECLARE_FLAGS(ExchangeOptions, ExchangeOption)

    typedef QHash<Frame::Content::Property, QVariant> MessageProperties;
    ExchangeOptions option() const;

    virtual ~Exchange();
    void declare(ExchangeType type = Direct,
                 ExchangeOptions options = NoOptions,
                 const Frame::TableField &args = Frame::TableField());
    void declare(const QString &type = QLatin1String("direct"),
                 ExchangeOptions options = NoOptions,
                 const Frame::TableField &args = Frame::TableField());
    void remove(bool ifUnused = true, bool noWait = true);

    void bind(Queue *queue);
    void bind(const QString &queueName);
    void bind(const QString &queueName, const QString &key);

    void publish(const QString &key, const QString &message,
                 const MessageProperties &properties = MessageProperties());
    void publish(const QString &key, const QByteArray &message,
                 const QString &mimeType, const MessageProperties &properties = MessageProperties());
    void publish(const QString &key, const QByteArray &message,
                 const QString &mimeType, const QVariantHash &headers,
                 const Exchange::MessageProperties &properties = Exchange::MessageProperties());

Q_SIGNALS:
    void declared();
    void removed();

protected:
    virtual void channelOpened();
    virtual void channelClosed();

private:
    explicit Exchange(int channelNumber = -1, Client *parent = 0);

    Q_DISABLE_COPY(Exchange)
    Q_DECLARE_PRIVATE(Exchange)

    friend class Client;

};

} // namespace QAMQP

Q_DECLARE_OPERATORS_FOR_FLAGS(QAMQP::Exchange::ExchangeOptions)
Q_DECLARE_METATYPE(QAMQP::Exchange::ExchangeType)

#endif // amqp_exchange_h__