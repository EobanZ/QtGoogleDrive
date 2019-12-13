#include "authenticator.h"
#include <QDesktopServices>
#include <windows.h>
#include <string>
#include <qdebug.h>
Authenticator::Authenticator(QString clientID, QString clientSecret, QObject *parent) : QObject(parent), m_clientID(clientID), m_clientSecret(clientSecret), m_scope(""), m_AuthorizeUrl(""), m_TokenUrl("")
{
    m_authFlow = new QOAuth2AuthorizationCodeFlow(this);
    m_replyHandler = new QOAuthHttpServerReplyHandler(8080, this);

    connect(m_authFlow, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, &QDesktopServices::openUrl);
    connect(m_authFlow, &QOAuth2AuthorizationCodeFlow::statusChanged, [=](QAbstractOAuth::Status newStatus)
    {
        m_isGranted = newStatus == QAbstractOAuth2::Status::Granted ? true : false;
        m_token = newStatus == QAbstractOAuth2::Status::Granted ? m_authFlow->token() : "";
    });

    connect(m_authFlow, &QOAuth2AuthorizationCodeFlow::granted,[=]()
    {
        //MessageBoxA(nullptr, "granted event", "granted slot", MB_OK);
        emit OnSuccess();
        m_replyHandler->deleteLater();
    });

}

void Authenticator::StartAuth()
{
    //Check if all fields are filled and initialized
    if(m_scope == "" || m_AuthorizeUrl.url() == "" || m_TokenUrl.url() == "" || m_clientID == "" || m_clientSecret == "" || !m_authFlow)
        return;

    m_authFlow->setScope(m_scope);
    m_authFlow->setAuthorizationUrl(m_AuthorizeUrl);
    m_authFlow->setAccessTokenUrl(m_TokenUrl);
    m_authFlow->setClientIdentifier(m_clientID);
    m_authFlow->setClientIdentifierSharedKey(m_clientSecret);
    m_authFlow->setReplyHandler(m_replyHandler);
    m_authFlow->grant();
}

bool Authenticator::isGranted()
{
    return m_isGranted;
}

QString Authenticator::GetToken()
{
    return m_token;
}


Authenticator_GoogleDrive::Authenticator_GoogleDrive(QString clientID, QString clientSecret, QObject *parent): Authenticator(clientID, clientSecret, parent)
{
    m_AuthorizeUrl = QUrl("https://accounts.google.com/o/oauth2/auth");
    m_TokenUrl = QUrl("https://oauth2.googleapis.com/token");
    m_scope = "https://www.googleapis.com/auth/drive";
}
