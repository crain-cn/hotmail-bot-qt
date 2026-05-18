#pragma once

#include "core/TaskManager.h"

#include <QComboBox>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QWidget>

class QCheckBox;
class QLineEdit;

struct UiConfigBindings {
    QCheckBox *hotmail = nullptr;
    QCheckBox *email = nullptr;
    QCheckBox *api = nullptr;
    QCheckBox *gmail = nullptr;
    QLineEdit *delay = nullptr;
    QLineEdit *bots = nullptr;
    QLineEdit *apiKey = nullptr;
    QLineEdit *tempMail = nullptr;
    QComboBox *batch = nullptr;
    QComboBox *browser = nullptr;
    QComboBox *captcha = nullptr;
    QSpinBox *numeric = nullptr;
    QSpinBox *retry = nullptr;
    QLineEdit *webDriverUrl = nullptr;
    QLineEdit *captchaApiKey = nullptr;
    QCheckBox *headless = nullptr;
    QPlainTextEdit *proxies = nullptr;
};

class ConfigStore
{
public:
    static void loadIntoUi(const UiConfigBindings &ui, TaskManager::Settings *settingsOut = nullptr);
    static void saveFromUi(const UiConfigBindings &ui);
    static TaskManager::Settings readSettings(const UiConfigBindings &ui);
};
