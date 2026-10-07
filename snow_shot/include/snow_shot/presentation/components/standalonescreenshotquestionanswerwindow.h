#ifndef SNOW_SHOT_PRESENTATION_COMPONENTS_STANDALONESCREENSHOTQUESTIONANSWERWINDOW_H
#define SNOW_SHOT_PRESENTATION_COMPONENTS_STANDALONESCREENSHOTQUESTIONANSWERWINDOW_H

#include "snow_shot/app/edition.h"
#if SNOW_SHOT_ENABLE_API_CONFIGURATION
#include <QObject>
#include <QPointer>

class QImage;
class QScreen;
class ScreenshotQuestionAnswerSession;
class ScreenshotQuestionAnswerWidget;
namespace adqt::widgets {
class AdModal;
}

class StandaloneScreenshotQuestionAnswerWindow final : public QObject {
    Q_OBJECT
  public:
    explicit StandaloneScreenshotQuestionAnswerWindow(ScreenshotQuestionAnswerSession* session,
                                                       QObject* parent = nullptr);
    ~StandaloneScreenshotQuestionAnswerWindow() override;
    void showScreenshot(QImage image, QScreen* screen = nullptr);
    void close();

  signals:
    void modelSettingsRequested();

  private:
    ScreenshotQuestionAnswerSession* m_session = nullptr;
    adqt::widgets::AdModal* m_modal = nullptr;
    QPointer<ScreenshotQuestionAnswerWidget> m_page;
};
#endif // SNOW_SHOT_ENABLE_API_CONFIGURATION

#endif // SNOW_SHOT_PRESENTATION_COMPONENTS_STANDALONESCREENSHOTQUESTIONANSWERWINDOW_H
