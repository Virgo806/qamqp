#include "amqp.h"
#include "amqp_p.h"

#include <QTextStream>
#include <QCoreApplication>
#include "amqp_exchange.h"
#include "amqp_queue.h"
#include "amqp_authenticator.h"

using namespace QAMQP;

struct QAMQP::ClientExceptionCleaner
{
	/* this cleans up when the constructor throws an exception */
	static inline void cleanup(Client *that, ClientPrivate *d)
	{
#ifdef QT_NO_EXCEPTIONS
		Q_UNUSED(that);
		Q_UNUSED(d);
#else
		Q_UNUSED(that);
		Q_UNUSED(d);	
#endif
	}
};

//////////////////////////////////////////////////////////////////////////

ClientPrivate::ClientPrivate( Client * q ) : 
	 pq_ptr(q)
	 , port(AMQPPORT)
	 , host(QString::fromLatin1(AMQPHOST))
	 , virtualHost(QString::fromLatin1(AMQPVHOST))
{
	
}


ClientPrivate::~ClientPrivate()
{

}

void ClientPrivate::init(QObject * parent)
{
	pq_func()->setParent(parent);
	if(!network_){
		network_ = new QAMQP::Network(pq_func());
	}

	if(!connection_)
	{
		connection_ = new QAMQP::Connection(pq_func());
	}

	setAuth(new AMQPlainAuthenticator(QString::fromLatin1(AMQPLOGIN), QString::fromLatin1(AMQPPSWD)));

	QObject::connect(network_, SIGNAL(method(const QAMQP::Frame::Method &)),
		connection_, SLOT(_q_method(const QAMQP::Frame::Method &)));
}

void ClientPrivate::init(QObject * parent, const QUrl & con)
{	
	parseCnnString(con);
	init(parent);
	ClientPrivate::connect();
}


void ClientPrivate::setAuth( Authenticator* auth )
{
	auth_ = QSharedPointer<Authenticator>(auth);
}


void ClientPrivate::printConnect() const
{
	QTextStream stream(stdout);
	stream <<  "port  = " << port << endl;
	stream <<  "host  = " << host << endl;
	stream <<  "vhost = " << virtualHost << endl;
	
	if(auth_ && auth_->type() == "AMQPLAIN")
	{
		QSharedPointer<AMQPlainAuthenticator> a = auth_.staticCast<AMQPlainAuthenticator>();
		stream <<  "user  = " << a->login() << endl;
		stream <<  "passw = " << a->password() << endl;
	}
	
}

void ClientPrivate::connect()
{
	ClientPrivate::sockConnect();
	ClientPrivate::login();
}

void ClientPrivate::parseCnnString( const QUrl & con )
{
	P_Q(QAMQP::Client);
	if(con.scheme() == AMQPSCHEME )
	{
		q->setPassword(con.password());
		q->setUser(con.userName());
		q->setPort(con.port());
		q->setHost(con.host());				
		q->setVirtualHost(con.path());
	}
}

void ClientPrivate::sockConnect()
{
	disconnect();
	network_->connectTo(host, port);
}

void ClientPrivate::login()
{

}

Exchange * ClientPrivate::createExchange(int channelNumber, const QString &name )
{
	Exchange * exchange_ = new Exchange(channelNumber, pq_func());
	QObject::connect(network_, SIGNAL(method(const QAMQP::Frame::Method &)),
		exchange_, SLOT(_q_method(const QAMQP::Frame::Method &)));

	QObject::connect(connection_, SIGNAL(connected()), exchange_, SLOT(_q_open()));
	exchange_->setName(name);
	return exchange_;
}

Queue * ClientPrivate::createQueue(int channelNumber, const QString &name )
{
	Queue * queue_ = new Queue(channelNumber, pq_func());
	QObject::connect(network_, SIGNAL(method(const QAMQP::Frame::Method &)),
		queue_, SLOT(_q_method(const QAMQP::Frame::Method &)));

	QObject::connect(network_, SIGNAL(content(const QAMQP::Frame::Content &)),
		queue_, SLOT(_q_content(const QAMQP::Frame::Content &)));

	QObject::connect(network_, SIGNAL(body(int, const QByteArray &)),
		queue_, SLOT(_q_body(int, const QByteArray &)));

	QObject::connect(connection_, SIGNAL(connected()), queue_, SLOT(_q_open()));
	queue_->setName(name);
	return queue_;
}


void ClientPrivate::disconnect()
{
	network_->QAMQP::Network::disconnect();
}


//////////////////////////////////////////////////////////////////////////


QAMQP::Client::Client( QObject * parent /*= 0*/ )
	: pd_ptr(new ClientPrivate(this))
{
	QT_TRY {
		pd_func()->init(parent);
	} QT_CATCH(...) {
		ClientExceptionCleaner::cleanup(this, pd_func());
		QT_RETHROW;
	}
}

QAMQP::Client::Client( const QUrl & connectionString, QObject * parent /*= 0*/ )
	: pd_ptr(new ClientPrivate(this))
{
	QT_TRY {
		pd_func()->init(parent, connectionString);
	} QT_CATCH(...) {
		ClientExceptionCleaner::cleanup(this, pd_func());
		QT_RETHROW;
	}
}

QAMQP::Client::~Client()
{
	QT_TRY {
		QEvent e(QEvent::Destroy);
		QCoreApplication::sendEvent(this, &e);
	} QT_CATCH(const std::exception&) {
		// if this fails we can't do anything about it but at least we are not allowed to throw.
	}
}

quint32 QAMQP::Client::port() const
{
	return pd_func()->port;
}

void QAMQP::Client::setPort( quint32 port )
{
	pd_func()->port = port;
}

QString QAMQP::Client::host() const
{
	return pd_func()->host;
}

void QAMQP::Client::setHost( const QString & host )
{
	pd_func()->host = host;
}

QString QAMQP::Client::virtualHost() const
{
	return pd_func()->virtualHost;
}

void QAMQP::Client::setVirtualHost( const QString & virtualHost )
{
	pd_func()->virtualHost = virtualHost;
}

QString QAMQP::Client::user() const
{
	const Authenticator * auth = pd_func()->auth_.data();

	if(auth && auth->type() == "AMQPLAIN")
	{
		const AMQPlainAuthenticator * a = static_cast<const AMQPlainAuthenticator *>(auth);
		return a->login();
	}
	return QString();
}

void QAMQP::Client::setUser( const QString & user )
{
	Authenticator * auth = pd_func()->auth_.data();

	if(auth && auth->type() == "AMQPLAIN")
	{
		AMQPlainAuthenticator * a = static_cast<AMQPlainAuthenticator *>(auth);
		a->setLogin(user);
	}
}

QString QAMQP::Client::password() const
{
	const Authenticator * auth = pd_func()->auth_.data();

	if(auth && auth->type() == "AMQPLAIN")
	{
		const AMQPlainAuthenticator * a = static_cast<const AMQPlainAuthenticator *>(auth);
		return a->password();
	}
	return QString();
}

void QAMQP::Client::setPassword( const QString & password )
{
	Authenticator * auth = pd_func()->auth_.data();

	if(auth && auth->type() == "AMQPLAIN")
	{
		AMQPlainAuthenticator * a = static_cast<AMQPlainAuthenticator *>(auth);
		a->setPassword(password);
	}
}

void QAMQP::Client::printConnect() const
{
#ifdef _DEBUG
	pd_func()->printConnect();
#endif // _DEBUG
}

void QAMQP::Client::closeChannel()
{

}

Exchange * QAMQP::Client::createExchange(int channelNumber)
{
	return pd_func()->createExchange(channelNumber, QString());
}

Exchange * QAMQP::Client::createExchange( const QString &name, int channelNumber )
{
	return pd_func()->createExchange(channelNumber, name);
}

Queue * QAMQP::Client::createQueue(int channelNumber)
{
	return pd_func()->createQueue(channelNumber, QString());
}

Queue * QAMQP::Client::createQueue( const QString &name, int channelNumber )
{
	return pd_func()->createQueue(channelNumber, name);
}

void QAMQP::Client::open()
{
	return pd_func()->connect();
}

void QAMQP::Client::open( const QUrl & connectionString )
{
	pd_func()->parseCnnString(connectionString);
	open();
}

void QAMQP::Client::close()
{
	return pd_func()->disconnect();
}

void QAMQP::Client::reopen()
{
	pd_func()->disconnect();
	pd_func()->connect();	
}

void QAMQP::Client::setAuth( Authenticator * auth )
{
	pd_func()->setAuth(auth);
}

Authenticator * QAMQP::Client::auth() const
{
	return pd_func()->auth_.data();
}