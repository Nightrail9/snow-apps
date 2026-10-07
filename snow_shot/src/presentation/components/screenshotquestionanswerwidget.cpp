#include "snow_shot/presentation/components/screenshotquestionanswerwidget.h"

#include "snow_shot/presentation/screenshotquestionanswersession.h"

#include <QComboBox>
#include <QEvent>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QScrollBar>
#include <QVBoxLayout>

ScreenshotQuestionAnswerWidget::ScreenshotQuestionAnswerWidget(
    ScreenshotQuestionAnswerSession* session, QWidget* parent)
    : QWidget(parent), m_session(session) {
    setObjectName(QStringLiteral("screenshotQuestionAnswerWidget"));
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(10);

    auto* header = new QHBoxLayout;
    m_title = new QLabel(this);
    QFont titleFont = m_title->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 2);
    m_title->setFont(titleFont);
    header->addWidget(m_title);
    header->addStretch(1);
    m_modelLabel = new QLabel(this);
    header->addWidget(m_modelLabel);
    m_models = new QComboBox(this);
    m_models->setObjectName(QStringLiteral("screenshotQuestionAnswerModel"));
    m_models->setMinimumWidth(190);
    header->addWidget(m_models);
    layout->addLayout(header);

    m_privacy = new QLabel(this);
    m_privacy->setWordWrap(true);
    layout->addWidget(m_privacy);

    m_transcript = new QPlainTextEdit(this);
    m_transcript->setObjectName(QStringLiteral("screenshotQuestionAnswerTranscript"));
    m_transcript->setReadOnly(true);
    m_transcript->setPlaceholderText(tr("Your conversation will appear here."));
    layout->addWidget(m_transcript, 1);

    m_status = new QLabel(this);
    m_status->setWordWrap(true);
    layout->addWidget(m_status);

    m_prompt = new QPlainTextEdit(this);
    m_prompt->setObjectName(QStringLiteral("screenshotQuestionAnswerPrompt"));
    m_prompt->setFixedHeight(82);
    layout->addWidget(m_prompt);

    auto* actions = new QHBoxLayout;
    m_settings = new QPushButton(this);
    m_clear = new QPushButton(this);
    m_cancel = new QPushButton(this);
    m_send = new QPushButton(this);
    m_send->setDefault(true);
    actions->addWidget(m_settings);
    actions->addStretch(1);
    actions->addWidget(m_clear);
    actions->addWidget(m_cancel);
    actions->addWidget(m_send);
    layout->addLayout(actions);

    connect(m_settings, &QPushButton::clicked, this,
            &ScreenshotQuestionAnswerWidget::modelSettingsRequested);
    connect(m_clear, &QPushButton::clicked, this, [this] {
        if (m_session != nullptr)
            m_session->newConversation();
        m_prompt->clear();
    });
    connect(m_cancel, &QPushButton::clicked, this, [this] {
        if (m_session != nullptr)
            m_session->cancel();
    });
    connect(m_send, &QPushButton::clicked, this, &ScreenshotQuestionAnswerWidget::sendPrompt);
    connect(m_prompt, &QPlainTextEdit::textChanged, this, [this] {
        if (m_send != nullptr)
            m_send->setEnabled(m_session != nullptr && !m_session->isBusy() &&
                               !m_prompt->toPlainText().trimmed().isEmpty());
    });
    connect(m_models, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (m_session != nullptr && index >= 0)
            m_session->setSelectedModel(m_models->itemData(index).toString());
    });
    if (m_session != nullptr) {
        connect(m_session, &ScreenshotQuestionAnswerSession::changed, this,
                &ScreenshotQuestionAnswerWidget::refresh);
        connect(m_session, &ScreenshotQuestionAnswerSession::modelChanged, this,
                &ScreenshotQuestionAnswerWidget::refreshModels);
    }
    retranslateUi();
    refreshModels();
    refresh();
}

void ScreenshotQuestionAnswerWidget::changeEvent(QEvent* event) {
    QWidget::changeEvent(event);
    if (event != nullptr && event->type() == QEvent::LanguageChange)
        retranslateUi();
}

void ScreenshotQuestionAnswerWidget::retranslateUi() {
    m_title->setText(tr("Ask about this screenshot"));
    m_modelLabel->setText(tr("Vision model"));
    m_privacy->setText(
        tr("The screenshot and your questions are sent to the selected model provider."));
    m_transcript->setPlaceholderText(tr("Your conversation will appear here."));
    m_prompt->setPlaceholderText(tr("Ask a question about the screenshot…"));
    m_settings->setText(tr("Configure vision models"));
    m_clear->setText(tr("New conversation"));
    m_cancel->setText(tr("Stop"));
    m_send->setText(tr("Ask"));
    refresh();
}

void ScreenshotQuestionAnswerWidget::refresh() {
    if (m_session == nullptr)
        return;
    QString text;
    for (const auto& message : m_session->messages()) {
        text += message.role == QStringLiteral("user") ? tr("You") + QStringLiteral(":\n")
                                                        : tr("AI") + QStringLiteral(":\n");
        text += message.content;
        text += QStringLiteral("\n\n");
    }
    const int previousMaximum = m_transcript->verticalScrollBar()->maximum();
    const bool wasAtEnd = m_transcript->verticalScrollBar()->value() >= previousMaximum - 2;
    m_transcript->setPlainText(text);
    if (wasAtEnd)
        m_transcript->verticalScrollBar()->setValue(m_transcript->verticalScrollBar()->maximum());
    m_status->setText(m_session->error());
    m_cancel->setEnabled(m_session->isBusy());
    m_clear->setEnabled(m_session->hasImage() || !m_session->messages().isEmpty());
    m_prompt->setEnabled(m_session->hasImage() && !m_session->availableModels().isEmpty());
    m_send->setEnabled(m_session->hasImage() && !m_session->isBusy() &&
                       !m_session->selectedModel().isEmpty() &&
                       !m_prompt->toPlainText().trimmed().isEmpty());
    m_settings->setVisible(m_session->availableModels().isEmpty());
    if (!m_session->hasImage() && m_session->error().isEmpty())
        m_status->setText(tr("Take a screenshot and choose Ask AI to start a conversation."));
    else if (m_session->availableModels().isEmpty() && m_session->error().isEmpty())
        m_status->setText(tr("Add an OpenAI-compatible model with Vision Support enabled."));
}

void ScreenshotQuestionAnswerWidget::refreshModels() {
    if (m_session == nullptr)
        return;
    const QString selected = m_session->selectedModel();
    const QSignalBlocker blocker(m_models);
    m_models->clear();
    const auto models = m_session->availableModels();
    for (const auto& model : models)
        m_models->addItem(model.name, model.id);
    const int index = m_models->findData(selected);
    if (index >= 0)
        m_models->setCurrentIndex(index);
    m_models->setVisible(!models.isEmpty());
    refresh();
}

void ScreenshotQuestionAnswerWidget::sendPrompt() {
    if (m_session == nullptr)
        return;
    const QString prompt = m_prompt->toPlainText().trimmed();
    if (prompt.isEmpty())
        return;
    m_prompt->clear();
    m_session->ask(prompt);
}
