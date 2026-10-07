#include "snow_shot/presentation/components/standalonescreenshotquestionanswerwindow.h"

#include "snow_shot/presentation/components/screenshotquestionanswerwidget.h"
#include "snow_shot/presentation/languagemanager.h"
#include "snow_shot/presentation/screenshotquestionanswersession.h"
#include "snow_shot/presentation/windowcloseshortcut.h"
#include "widgets/modal.h"

#include <QCursor>
#include <QGuiApplication>
#include <QScreen>
#include <QSize>

StandaloneScreenshotQuestionAnswerWindow::StandaloneScreenshotQuestionAnswerWindow(
    ScreenshotQuestionAnswerSession* session, QObject* parent)
    : QObject(parent), m_session(session), m_modal(new adqt::widgets::AdModal(this)) {
    using adqt::widgets::AdModal;
    m_modal->setObjectName(QStringLiteral("standalone-screenshot-question-answer-modal"));
    m_modal->setMode(AdModal::Mode::Window);
    m_modal->setWindowModeDetached(true);
    m_modal->setWindowModality(Qt::NonModal);
    m_modal->setWindowTitle(tr("Screenshot Q&A"));
    m_modal->setFooterVisible(false);
    m_modal->setCentered(true);
    m_modal->setWindowPreferredSize(QSize(760, 580));
    m_modal->setWindowMinimumSize(QSize(540, 420));
    m_modal->setWindowResizable(true);
    m_modal->setWindowTaskbarVisible(true);
    m_modal->setWindowMinimizeButtonVisible(true);
    m_modal->setWindowAlwaysOnTopButtonVisible(true);
    AdModal::ComponentTokens tokens;
    tokens.contentPaddingHorizontal = 0;
    tokens.contentPaddingVertical = 0;
    tokens.headerMarginBottom = 0;
    m_modal->setComponentTokens(tokens);
    connect(m_modal, &AdModal::closed, this, [this] {
        if (m_page != nullptr) {
            QWidget* content = m_modal->takeContentWidget();
            m_page.clear();
            if (content != nullptr)
                content->deleteLater();
        }
    });
    connect(&snow_shot::presentation::LanguageManager::instance(),
            &snow_shot::presentation::LanguageManager::languageChanged, this, [this] {
                m_modal->setWindowTitle(tr("Screenshot Q&A"));
            });
}

StandaloneScreenshotQuestionAnswerWindow::~StandaloneScreenshotQuestionAnswerWindow() {
    close();
}

void StandaloneScreenshotQuestionAnswerWindow::showScreenshot(QImage image, QScreen* screen) {
    if (m_session == nullptr || image.isNull())
        return;
    m_session->begin(std::move(image));
    if (m_page == nullptr) {
        if (screen == nullptr)
            screen = QGuiApplication::screenAt(QCursor::pos());
        if (screen == nullptr)
            screen = QGuiApplication::primaryScreen();
        if (screen != nullptr)
            m_modal->setWindowScreen(screen);
        m_page = new ScreenshotQuestionAnswerWidget(m_session);
        m_page->setObjectName(QStringLiteral("standalone-screenshot-question-answer-page"));
        m_modal->setContentWidget(m_page);
        connect(m_page, &ScreenshotQuestionAnswerWidget::modelSettingsRequested, this,
                &StandaloneScreenshotQuestionAnswerWindow::modelSettingsRequested);
    }
    m_modal->present();
    installWindowCloseShortcut(m_page->window(), [this] { close(); });
}

void StandaloneScreenshotQuestionAnswerWindow::close() {
    if (m_modal != nullptr)
        m_modal->close();
}
