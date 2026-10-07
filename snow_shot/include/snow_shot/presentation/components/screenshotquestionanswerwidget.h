#ifndef SNOW_SHOT_PRESENTATION_COMPONENTS_SCREENSHOTQUESTIONANSWERWIDGET_H
#define SNOW_SHOT_PRESENTATION_COMPONENTS_SCREENSHOTQUESTIONANSWERWIDGET_H

#include "snow_shot/app/edition.h"
#if SNOW_SHOT_ENABLE_API_CONFIGURATION
#include <QPointer>
#include <QWidget>

class QComboBox;
class QEvent;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class ScreenshotQuestionAnswerSession;

class ScreenshotQuestionAnswerWidget final : public QWidget {
    Q_OBJECT
  public:
    explicit ScreenshotQuestionAnswerWidget(ScreenshotQuestionAnswerSession* session,
                                            QWidget* parent = nullptr);

  signals:
    void modelSettingsRequested();

  private:
    void changeEvent(QEvent* event) override;
    void retranslateUi();
    void refresh();
    void refreshModels();
    void sendPrompt();
    QPointer<ScreenshotQuestionAnswerSession> m_session;
    QLabel* m_title = nullptr;
    QLabel* m_modelLabel = nullptr;
    QLabel* m_privacy = nullptr;
    QComboBox* m_models = nullptr;
    QLabel* m_status = nullptr;
    QPlainTextEdit* m_transcript = nullptr;
    QPlainTextEdit* m_prompt = nullptr;
    QPushButton* m_send = nullptr;
    QPushButton* m_cancel = nullptr;
    QPushButton* m_clear = nullptr;
    QPushButton* m_settings = nullptr;
};
#endif // SNOW_SHOT_ENABLE_API_CONFIGURATION

#endif // SNOW_SHOT_PRESENTATION_COMPONENTS_SCREENSHOTQUESTIONANSWERWIDGET_H
