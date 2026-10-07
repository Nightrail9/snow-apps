#include "snow_shot/presentation/screenshotquestionanswersession.h"

#include <algorithm>
#include <utility>

ScreenshotQuestionAnswerSession::ScreenshotQuestionAnswerSession(SnowShotApiClient* client,
                                                                 QObject* parent)
    : QObject(parent), m_client(client) {
    if (m_client != nullptr) {
        connect(m_client, &SnowShotApiClient::chatModelsChanged, this,
                &ScreenshotQuestionAnswerSession::refreshModels);
        connect(m_client, &SnowShotApiClient::customModelInvalidated, this,
                [this](const QString&, bool, bool) { refreshModels(); });
    }
    refreshModels();
}

ScreenshotQuestionAnswerSession::~ScreenshotQuestionAnswerSession() {
    cancel();
}

const QVector<ScreenshotQuestionAnswerSession::Message>&
ScreenshotQuestionAnswerSession::messages() const {
    return m_messages;
}

QVector<SnowShotChatModel> ScreenshotQuestionAnswerSession::availableModels() const {
    return m_models;
}

QString ScreenshotQuestionAnswerSession::selectedModel() const {
    return m_model;
}

QString ScreenshotQuestionAnswerSession::error() const {
    return m_error;
}

bool ScreenshotQuestionAnswerSession::isBusy() const {
    return m_request != 0;
}

bool ScreenshotQuestionAnswerSession::hasImage() const {
    return !m_image.isNull();
}

void ScreenshotQuestionAnswerSession::begin(QImage image) {
    cancel();
    m_image = std::move(image);
    m_messages.clear();
    m_error.clear();
    emit changed();
}

void ScreenshotQuestionAnswerSession::setSelectedModel(const QString& id) {
    const auto found = std::find_if(m_models.cbegin(), m_models.cend(),
                                    [&id](const SnowShotChatModel& model) {
                                        return model.id == id;
                                    });
    if (found == m_models.cend() || m_model == id) {
        return;
    }
    m_model = id;
    emit modelChanged();
}

void ScreenshotQuestionAnswerSession::ask(const QString& prompt) {
    const QString question = prompt.trimmed();
    if (m_client == nullptr || m_image.isNull() || m_request != 0 || m_model.isEmpty() ||
        question.isEmpty()) {
        return;
    }
    m_error.clear();
    m_messages.push_back({QStringLiteral("user"), question});
    m_messages.push_back({QStringLiteral("assistant"), {}});
    QVector<SnowShotChatMessage> history;
    history.reserve(m_messages.size() - 1);
    for (qsizetype i = 0; i + 1 < m_messages.size(); ++i) {
        const auto& message = m_messages.at(i);
        history.push_back({message.role, message.content});
    }
    emit changed();
    m_request = m_client->streamImageQuestion(
        {m_model, m_image, std::move(history)}, this,
        [this](const QString& delta) {
            if (m_messages.isEmpty() || m_messages.last().role != QStringLiteral("assistant")) {
                return;
            }
            m_messages.last().content += delta;
            emit changed();
        },
        [this](SnowShotTranslationResult result) {
            m_request = 0;
            if (!result.succeeded() && !result.cancelled) {
                m_error = result.error;
                if (!m_messages.isEmpty() && m_messages.last().role == QStringLiteral("assistant") &&
                    m_messages.last().content.isEmpty()) {
                    m_messages.removeLast();
                }
            }
            emit changed();
        });
    if (m_request == 0) {
        m_messages.removeLast();
        m_messages.removeLast();
        m_error = tr("Could not start the AI question request.");
        emit changed();
    }
}

void ScreenshotQuestionAnswerSession::cancel() {
    if (m_request != 0 && m_client != nullptr) {
        const auto request = std::exchange(m_request, 0);
        m_client->cancel(request);
        emit changed();
    }
}

void ScreenshotQuestionAnswerSession::clear() {
    cancel();
    m_image = {};
    m_messages.clear();
    m_error.clear();
    emit changed();
}

void ScreenshotQuestionAnswerSession::newConversation() {
    cancel();
    m_messages.clear();
    m_error.clear();
    emit changed();
}

void ScreenshotQuestionAnswerSession::refreshModels() {
    const QString previous = m_model;
    m_models = m_client == nullptr ? QVector<SnowShotChatModel>() : m_client->configuredVisionModels();
    const auto current = std::find_if(m_models.cbegin(), m_models.cend(), [this](const auto& model) {
        return model.id == m_model;
    });
    if (current == m_models.cend()) {
        m_model = m_models.isEmpty() ? QString() : m_models.constFirst().id;
        if (m_request != 0) {
            cancel();
            m_error = tr("The selected vision model is no longer available.");
        }
    }
    if (previous != m_model) {
        emit modelChanged();
    }
    emit changed();
}
