/*---------------------------------------------------------*\
| MasterRGBUpdate.cpp                                       |
|                                                           |
|   Checks the public GitHub releases for a newer version   |
|                                                           |
|   This file is part of the MasterRGB project              |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUrl>

#include "MasterRGBUpdate.h"

namespace
{
    QList<int> VersionParts(const QString& value)
    {
        QString cleaned = value.trimmed();
        cleaned.remove(QRegularExpression("^[vV]"));

        QList<int> parts;
        const QStringList pieces = cleaned.split('.');

        for(const QString& piece : pieces)
        {
            const QRegularExpressionMatch match = QRegularExpression("^(\\d+)").match(piece);

            if(match.hasMatch())
            {
                parts.append(match.captured(1).toInt());
            }
            else
            {
                break;
            }
        }

        return parts;
    }
}

int MasterRGBUpdate::Compare(const QString& a, const QString& b)
{
    const QList<int> left  = VersionParts(a);
    const QList<int> right = VersionParts(b);
    const int count = qMax(left.count(), right.count());

    for(int i = 0; i < count; i++)
    {
        const int l = i < left.count()  ? left[i]  : 0;
        const int r = i < right.count() ? right[i] : 0;

        if(l != r)
        {
            return l < r ? -1 : 1;
        }
    }

    return 0;
}

void MasterRGBUpdate::Check(QObject* context,
                            const QString& currentVersion,
                            std::function<void(bool, const QString&, const QString&, const QString&)> callback)
{
    QNetworkAccessManager* network = new QNetworkAccessManager(context);

    QNetworkRequest request(QUrl("https://api.github.com/repos/m1000v2/MasterRGB/releases/latest"));
    request.setHeader(QNetworkRequest::UserAgentHeader, QString("MasterRGB/%1").arg(currentVersion));
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply* reply = network->get(request);

    QObject::connect(reply, &QNetworkReply::finished, context, [reply, network, callback]()
    {
        const QByteArray payload = reply->readAll();
        const QString error = reply->error() == QNetworkReply::NoError ? QString() : reply->errorString();

        reply->deleteLater();
        network->deleteLater();

        if(!error.isEmpty())
        {
            callback(false, QString(), QString(), error);
            return;
        }

        const QJsonDocument document = QJsonDocument::fromJson(payload);

        if(!document.isObject())
        {
            callback(false, QString(), QString(), "Resposta invalida do GitHub");
            return;
        }

        const QJsonObject release = document.object();
        QString tag = release.value("tag_name").toString().trimmed();
        const QString url = release.value("html_url").toString().trimmed();

        tag.remove(QRegularExpression("^[vV]"));

        if(tag.isEmpty())
        {
            callback(false, QString(), QString(), "Nenhuma versao publicada");
            return;
        }

        callback(true, tag, url, QString());
    });
}
