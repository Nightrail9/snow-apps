#ifndef SNOW_SHOT_PRESENTATION_SCREENSHOTQUESTIONANSWERSESSION_H
#define SNOW_SHOT_PRESENTATION_SCREENSHOTQUESTIONANSWERSESSION_H

#include "snow_shot/app/edition.h"
#if SNOW_SHOT_ENABLE_API_CONFIGURATION
#include "snow_shot/network/snowshotapiclient.h"

#include <QImage>
#include <QObject>
#include <QString>
#include <QVector>

class ScreenshotQuestionAnswerSession final : public QObject {
    Q_OBJECT
  public:
    struct Message {
        QString role;
        QString content;
    };

    explicit ScreenshotQuestionAnswerSession(SnowShotApiClient* client,
                                             QObject* parent = nullptr);
    ~ScreenshotQuestionAnswerSession() override;

    [[nodiscard]] const QVector<Message>& messages() const;
    [[nodiscard]] QVector<SnowShotChatModel> availableModels() const;
    [[nodiscard]] QString selectedModel() const;
    [[nodiscard]] QString error() const;
    [[nodiscard]] bool isBusy() const;
    [[nodiscard]] bool hasImage() const;

    void begin(QImage image);
    void setSelectedModel(const QString& id);
    void ask(const QString& prompt);
    void cancel();
    void newConversation();
    void clear();

  signals:
    void changed();
    void modelChanged();

  private:
    void refreshModels();
    QPointer<SnowShotApiClient> m_client;
    QImage m_image;
    QVector<Message> m_messages;
    QVector<SnowShotChatModel> m_models;
    QString m_model;
    QString m_error;
    SnowShotApiClient::RequestToken m_request = 0;
};
#endif // SNOW_SHOT_ENABLE_API_CONFIGURATION

#endif // SNOW_SHOT_PRESENTATION_SCREENSHOTQUESTIONANSWERSESSION_H
