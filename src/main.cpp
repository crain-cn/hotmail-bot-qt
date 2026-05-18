#include "ui/MainWindow.h"

#include "core/SelectorConfig.h"

#include <QApplication>
#include <QFile>
#include <QIcon>

namespace {

QString loadStyleSheet()
{
    QFile file(QStringLiteral(":/styles/dark.qss"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("HotmailBotQt"));
    QApplication::setOrganizationName(QStringLiteral("HotmailBot"));
    QApplication::setApplicationVersion(QStringLiteral("0.4.0"));

    SelectorConfig::instance().load();

    const QString styleSheet = loadStyleSheet();
    if (!styleSheet.isEmpty()) {
        app.setStyleSheet(styleSheet);
    }

    MainWindow window;
    window.show();

    return app.exec();
}
