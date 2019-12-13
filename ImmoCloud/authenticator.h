#ifndef AUTHENTICATOR_H
#define AUTHENTICATOR_H

#include <QObject>
#include <QUrl>
#include <QtNetworkAuth>

class Authenticator : public QObject
{
    Q_OBJECT
public:
    virtual void StartAuth();
    bool isGranted();
    QString GetToken();

    friend class CloudInterface;

protected:
    Authenticator(QString clientID, QString clientSecret, QObject *parent = nullptr);

protected:
    QString m_clientID;
    QString m_clientSecret;
    QString m_scope;
    QUrl m_AuthorizeUrl;
    QUrl m_TokenUrl;

    QString m_token;


    QOAuth2AuthorizationCodeFlow* m_authFlow;
    QOAuthHttpServerReplyHandler* m_replyHandler; //destroy when not needen with deleteLater()->connect to granted/finished

    bool m_isGranted;

signals:
    void OnSuccess();


public slots:
};


class Authenticator_GoogleDrive : public Authenticator
{
    Q_OBJECT
public:
    Authenticator_GoogleDrive(QString clientID, QString clientSecret, QObject *parent = nullptr);


signals:
public slots:

};

#endif // AUTHENTICATOR_H
