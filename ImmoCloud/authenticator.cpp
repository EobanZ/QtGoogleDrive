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
        emit OnSuccess();
        SaveToJson();
        m_replyHandler->deleteLater();
    });

}

void Authenticator::SaveToJson()
{

    QJsonObject root;
    root.insert("token", m_authFlow->token());
    root.insert("expires", m_authFlow->expirationAt().toString()); //TODO: get the refresh token in the first response and save it. https://stackoverflow.com/questions/10827920/not-receiving-google-oauth-refresh-token
    QJsonDocument doc(root);

    if(QFileInfo(m_pathToConfig+"auth.json").exists())
        QFile(m_pathToConfig+"auth.json").remove();


    QFile saveFile(m_pathToConfig+"auth.json");
    saveFile.open(QIODevice::ReadWrite);
    saveFile.write(doc.toJson());
    saveFile.close();

}

void Authenticator::GetNewToken()
{
    if(m_scope.isEmpty() || m_AuthorizeUrl.url().isEmpty() || m_TokenUrl.url().isEmpty() || m_clientID.isEmpty() || m_clientSecret.isEmpty() || !m_authFlow)
        return;


    m_authFlow->grant();
}

void Authenticator::StartAuth()
{



    QFile configFile(m_pathToConfig+"auth.json");
    if(configFile.exists() && configFile.size() > 0)
    {
        configFile.open(QIODevice::ReadOnly);
        QJsonDocument doc = QJsonDocument::fromJson(configFile.readAll());
        QJsonObject root = doc.object();
        QJsonValue tokenVal = root.value("token");
        QJsonValue expiresVal = root.value("expires");

        QString token = tokenVal.toString();
        QString expires = expiresVal.toString();
        QDateTime expiresDate = QDateTime::fromString(expires);
        configFile.close();

        m_token = token;
        if(expiresDate.toLocalTime() < QDateTime::currentDateTime().toLocalTime())
        {
            //TODO: use refreshToken instead of creating new one
            GetNewToken();
            return;

        }

        //TODO: check if token is really valid
        m_isGranted = true;
        emit OnSuccess();
    }
    else
        GetNewToken();


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


    m_pathToConfig = QDir::currentPath()+"/config/google_drive/";

    m_authFlow->setScope(m_scope);
    m_authFlow->setAuthorizationUrl(m_AuthorizeUrl);
    m_authFlow->setAccessTokenUrl(m_TokenUrl);
    m_authFlow->setClientIdentifier(m_clientID);
    m_authFlow->setClientIdentifierSharedKey(m_clientSecret);
    m_authFlow->setReplyHandler(m_replyHandler);

}
