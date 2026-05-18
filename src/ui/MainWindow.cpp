#include "ui/MainWindow.h"

#include "core/ConfigStore.h"
#include "core/ConfigSyncService.h"
#include "core/SelectorConfig.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {

constexpr int kWindowWidth = 800;
constexpr int kWindowHeight = 720;
constexpr int kProgressBarCount = 10;
constexpr int kStartBarHeight = 104;

QLineEdit *makeLabeledField(const QString &placeholder, QWidget *parent)
{
    auto *edit = new QLineEdit(parent);
    edit->setPlaceholderText(placeholder);
    edit->setAlignment(Qt::AlignCenter);
    edit->setObjectName(QStringLiteral("configField"));
    return edit;
}

QPushButton *makeTitleButton(const QString &text, const QString &objectName, QWidget *parent)
{
    auto *button = new QPushButton(text, parent);
    button->setObjectName(objectName);
    button->setFixedSize(22, 22);
    return button;
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setFixedSize(kWindowWidth, kWindowHeight);
    setObjectName(QStringLiteral("mainWindow"));

    setupUi();
    wireTaskManager();
    loadConfig();
    refreshAccountTable();

    connect(&m_resultStore, &ResultStore::resultsChanged, this, &MainWindow::refreshAccountTable);
}

MainWindow::~MainWindow()
{
    saveConfig();
}

UiConfigBindings MainWindow::configBindings() const
{
    UiConfigBindings ui;
    ui.hotmail = m_hotmailCheck;
    ui.email = m_emailCheck;
    ui.api = m_apiCheck;
    ui.gmail = m_gmailCheck;
    ui.delay = m_delayEdit;
    ui.bots = m_botsEdit;
    ui.apiKey = m_apiKeyEdit;
    ui.tempMail = m_tempMailEdit;
    ui.batch = m_batchCombo;
    ui.browser = m_browserCombo;
    ui.captcha = m_captchaCombo;
    ui.webDriverUrl = m_webDriverUrlEdit;
    ui.captchaApiKey = m_captchaApiKeyEdit;
    ui.headless = m_headlessCheck;
    ui.numeric = m_numericSpin;
    ui.retry = m_retrySpin;
    ui.proxies = m_proxyEdit;
    return ui;
}

void MainWindow::loadConfig()
{
    ConfigStore::loadIntoUi(configBindings());
    onLogMessage(QStringLiteral("Config loaded."));
}

void MainWindow::saveConfig()
{
    ConfigStore::saveFromUi(configBindings());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveConfig();
    QMainWindow::closeEvent(event);
}

void MainWindow::setupUi()
{
    auto *central = new QWidget(this);
    central->setObjectName(QStringLiteral("centralPanel"));
    setCentralWidget(central);

    auto *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto *titleBar = new QWidget(central);
    titleBar->setObjectName(QStringLiteral("titleBar"));
    titleBar->setFixedHeight(30);
    setupTitleBar(titleBar);
    rootLayout->addWidget(titleBar);

    auto *body = new QWidget(central);
    auto *bodyLayout = new QHBoxLayout(body);
    bodyLayout->setContentsMargins(16, 8, 16, 8);
    bodyLayout->setSpacing(12);

    auto *leftColumn = new QVBoxLayout();
    leftColumn->setSpacing(8);

    auto *separator = new QLabel(QStringLiteral("Bot Features"), body);
    separator->setObjectName(QStringLiteral("sectionTitle"));
    leftColumn->addWidget(separator);

    m_listButton = new QPushButton(QStringLiteral("List"), body);
    m_listButton->setObjectName(QStringLiteral("secondaryButton"));
    m_listButton->setFixedSize(105, 33);
    leftColumn->addWidget(m_listButton);

    m_hotmailCheck = new QCheckBox(QStringLiteral("Hotmail"), body);
    m_emailCheck = new QCheckBox(QStringLiteral("Email"), body);
    m_apiCheck = new QCheckBox(QStringLiteral("Api"), body);
    m_gmailCheck = new QCheckBox(QStringLiteral("Gmail"), body);
    leftColumn->addWidget(m_hotmailCheck);
    leftColumn->addWidget(m_emailCheck);
    leftColumn->addWidget(m_apiCheck);
    leftColumn->addWidget(m_gmailCheck);

    m_featureList = new QListWidget(body);
    m_featureList->addItems({QStringLiteral("Github"),
                             QStringLiteral("Enesuygurs"),
                             QStringLiteral("Igyx UI")});
    m_featureList->setObjectName(QStringLiteral("featureList"));
    m_featureList->setFixedSize(147, 120);
    leftColumn->addWidget(m_featureList);
    leftColumn->addStretch();

    auto *middleScrollContent = new QWidget(body);
    auto *middleColumn = new QVBoxLayout(middleScrollContent);
    middleColumn->setContentsMargins(0, 0, 4, 0);
    middleColumn->setSpacing(8);

    m_batchCombo = new QComboBox(middleScrollContent);
    m_batchCombo->addItems({QStringLiteral("1-5"),
                            QStringLiteral("10-50"),
                            QStringLiteral("100-500"),
                            QStringLiteral("500-1000")});
    m_batchCombo->setFixedWidth(105);
    middleColumn->addWidget(m_batchCombo);

    m_browserCombo = new QComboBox(middleScrollContent);
    m_browserCombo->addItems({QStringLiteral("Mock"), QStringLiteral("WebDriver")});
    m_browserCombo->setFixedWidth(105);
    middleColumn->addWidget(m_browserCombo);

    m_webDriverUrlEdit = makeLabeledField(QStringLiteral("Driver URL"), middleScrollContent);
    m_webDriverUrlEdit->setText(QStringLiteral("http://127.0.0.1:9515"));
    middleColumn->addWidget(m_webDriverUrlEdit);

    m_headlessCheck = new QCheckBox(QStringLiteral("Headless"), middleScrollContent);
    m_headlessCheck->setChecked(true);
    middleColumn->addWidget(m_headlessCheck);

    m_captchaCombo = new QComboBox(middleScrollContent);
    m_captchaCombo->addItems(
        {QStringLiteral("Captcha: Skip"), QStringLiteral("Captcha: Manual"), QStringLiteral("Captcha: API")});
    m_captchaCombo->setFixedWidth(150);
    middleColumn->addWidget(m_captchaCombo);

    m_captchaApiKeyEdit = makeLabeledField(QStringLiteral("Captcha API"), middleScrollContent);
    middleColumn->addWidget(m_captchaApiKeyEdit);

    m_retrySpin = new QSpinBox(middleScrollContent);
    m_retrySpin->setRange(1, 5);
    m_retrySpin->setValue(2);
    m_retrySpin->setPrefix(QStringLiteral("Retry "));
    m_retrySpin->setFixedWidth(90);
    middleColumn->addWidget(m_retrySpin);

    m_delayEdit = makeLabeledField(QStringLiteral("Delay"), middleScrollContent);
    m_botsEdit = makeLabeledField(QStringLiteral("Bots"), middleScrollContent);
    m_apiKeyEdit = makeLabeledField(QStringLiteral("Api Key"), middleScrollContent);
    m_tempMailEdit = makeLabeledField(QStringLiteral("TempMail URL"), middleScrollContent);
    m_tempMailEdit->setPlaceholderText(QStringLiteral("https://api.example/inbox?email={email}"));
    middleColumn->addWidget(m_delayEdit);
    middleColumn->addWidget(m_botsEdit);
    middleColumn->addWidget(m_apiKeyEdit);
    middleColumn->addWidget(m_tempMailEdit);

    m_numericSpin = new QSpinBox(middleScrollContent);
    m_numericSpin->setRange(0, 9999999);
    m_numericSpin->setFixedSize(75, 30);
    middleColumn->addWidget(m_numericSpin);
    middleColumn->addStretch();

    auto *middleScroll = new QScrollArea(body);
    middleScroll->setWidget(middleScrollContent);
    middleScroll->setWidgetResizable(true);
    middleScroll->setFrameShape(QFrame::NoFrame);
    middleScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *centerColumn = new QVBoxLayout();
    m_proxyGroup = new QGroupBox(QStringLiteral("Proxy"), body);
    m_proxyGroup->setObjectName(QStringLiteral("proxyGroup"));
    m_proxyGroup->setFixedSize(200, 151);
    auto *proxyLayout = new QVBoxLayout(m_proxyGroup);
    m_proxyEdit = new QPlainTextEdit(m_proxyGroup);
    m_proxyEdit->setPlaceholderText(QStringLiteral("host:port\nuser:pass@host:port"));
    proxyLayout->addWidget(m_proxyEdit);
    centerColumn->addWidget(m_proxyGroup);
    centerColumn->addStretch();

    auto *rightColumn = new QVBoxLayout();
    rightColumn->setSpacing(6);
    m_progressBars.reserve(kProgressBarCount);
    for (int i = 0; i < kProgressBarCount; ++i) {
        auto *bar = new QProgressBar(body);
        bar->setRange(0, 100);
        bar->setValue(0);
        bar->setTextVisible(false);
        bar->setFixedHeight(22);
        bar->setObjectName(QStringLiteral("taskProgress"));
        m_progressBars.append(bar);
        rightColumn->addWidget(bar);
    }

    bodyLayout->addLayout(leftColumn, 1);
    bodyLayout->addWidget(middleScroll, 2);
    bodyLayout->addLayout(centerColumn, 0);
    bodyLayout->addLayout(rightColumn, 1);

    auto *startBar = new QWidget(central);
    startBar->setObjectName(QStringLiteral("startBar"));
    startBar->setFixedHeight(kStartBarHeight);
    auto *startLayout = new QHBoxLayout(startBar);
    startLayout->setContentsMargins(16, 8, 16, 8);
    m_startButton = new QPushButton(QStringLiteral("Start"), startBar);
    m_startButton->setObjectName(QStringLiteral("startButton"));
    m_startButton->setFixedSize(220, 88);
    startLayout->addStretch();
    startLayout->addWidget(m_startButton);
    startLayout->addStretch();

    m_accountTable = new QTableWidget(central);
    m_accountTable->setObjectName(QStringLiteral("accountTable"));
    m_accountTable->setColumnCount(5);
    m_accountTable->setHorizontalHeaderLabels({QStringLiteral("Status"),
                                               QStringLiteral("Email"),
                                               QStringLiteral("Password"),
                                               QStringLiteral("Stage"),
                                               QStringLiteral("ms")});
    m_accountTable->horizontalHeader()->setStretchLastSection(true);
    m_accountTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_accountTable->verticalHeader()->setVisible(false);
    m_accountTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_accountTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_accountTable->setFixedHeight(110);

    m_logView = new QPlainTextEdit(central);
    m_logView->setReadOnly(true);
    m_logView->setObjectName(QStringLiteral("logView"));
    m_logView->setFixedHeight(64);
    m_logView->setPlaceholderText(QStringLiteral("Logs..."));

    rootLayout->addWidget(body, 1);
    rootLayout->addWidget(startBar);
    rootLayout->addWidget(m_accountTable);
    rootLayout->addWidget(m_logView);

    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(m_listButton, &QPushButton::clicked, this, &MainWindow::onListClicked);
    connect(m_featureList, &QListWidget::itemClicked, this, &MainWindow::onFeatureItemClicked);
}

void MainWindow::setupTitleBar(QWidget *titleBar)
{
    auto *layout = new QHBoxLayout(titleBar);
    layout->setContentsMargins(12, 4, 8, 4);
    layout->setSpacing(6);

    auto *title = new QLabel(QStringLiteral("Hotmail Bot"), titleBar);
    title->setObjectName(QStringLiteral("windowTitle"));

    layout->addWidget(title);
    layout->addStretch();

    m_minButton = makeTitleButton(QStringLiteral("—"), QStringLiteral("minButton"), titleBar);
    m_maxButton = makeTitleButton(QStringLiteral("□"), QStringLiteral("maxButton"), titleBar);
    m_closeButton = makeTitleButton(QStringLiteral("×"), QStringLiteral("closeButton"), titleBar);

    layout->addWidget(m_minButton);
    layout->addWidget(m_maxButton);
    layout->addWidget(m_closeButton);

    connect(m_minButton, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(m_maxButton, &QPushButton::clicked, this, [this]() {
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
    });
    connect(m_closeButton, &QPushButton::clicked, this, &QWidget::close);
}

void MainWindow::wireTaskManager()
{
    connect(&m_taskManager, &TaskManager::taskStarted, this, &MainWindow::onTaskStarted);
    connect(&m_taskManager, &TaskManager::taskProgress, this, &MainWindow::onTaskProgress);
    connect(&m_taskManager, &TaskManager::taskFinished, this, &MainWindow::onTaskFinished);
    connect(&m_taskManager, &TaskManager::registrationCompleted, this,
            &MainWindow::onRegistrationCompleted);
    connect(&m_taskManager, &TaskManager::allTasksFinished, this, &MainWindow::onAllTasksFinished);
    connect(&m_taskManager, &TaskManager::logMessage, this, &MainWindow::onLogMessage);
}

void MainWindow::refreshAccountTable()
{
    const QVector<RegistrationResult> rows = m_resultStore.recentResults();
    m_accountTable->setRowCount(rows.size());

    for (int i = 0; i < rows.size(); ++i) {
        const RegistrationResult &row = rows.at(i);
        m_accountTable->setItem(i, 0, new QTableWidgetItem(row.success ? QStringLiteral("OK")
                                                                      : QStringLiteral("FAIL")));
        m_accountTable->setItem(i, 1, new QTableWidgetItem(row.email));
        m_accountTable->setItem(i, 2, new QTableWidgetItem(row.password));
        m_accountTable->setItem(i, 3, new QTableWidgetItem(registrationStageName(row.lastStage)));
        m_accountTable->setItem(i, 4, new QTableWidgetItem(QString::number(row.elapsedMs)));
    }
}

void MainWindow::onStartClicked()
{
    if (m_taskManager.isRunning()) {
        m_taskManager.stop();
        m_startButton->setText(QStringLiteral("Start"));
        onLogMessage(QStringLiteral("Stop requested."));
        return;
    }

    saveConfig();

    for (QProgressBar *bar : m_progressBars) {
        bar->setValue(0);
    }

    m_startButton->setText(QStringLiteral("Stop"));
    m_taskManager.start(ConfigStore::readSettings(configBindings()));
}

void MainWindow::onListClicked()
{
    m_resultStore.reload();
    refreshAccountTable();
    onLogMessage(QStringLiteral("Accounts file: %1").arg(m_resultStore.filePath()));
}

void MainWindow::onFeatureItemClicked()
{
    const auto items = m_featureList->selectedItems();
    if (items.isEmpty()) {
        return;
    }

    const QString feature = items.first()->text();
    QString message;
    if (feature == QStringLiteral("Github")) {
        const TaskManager::Settings settings = ConfigStore::readSettings(configBindings());
        ConfigSyncService::syncGithubSelectors(settings.selectorsSyncUrl, &message);
    } else if (feature == QStringLiteral("Igyx UI")) {
        ConfigSyncService::resetSelectors(&message);
    } else {
        message = QStringLiteral("Selectors file: %1").arg(SelectorConfig::instance().configPath());
    }
    onLogMessage(message);
}

void MainWindow::onTaskStarted(int index)
{
    if (index >= 0 && index < m_progressBars.size()) {
        m_progressBars.at(index)->setValue(0);
    }
}

void MainWindow::onTaskProgress(int index, int percent)
{
    if (index >= 0 && index < m_progressBars.size()) {
        m_progressBars.at(index)->setValue(percent);
    }
}

void MainWindow::onTaskFinished(int index, bool success, const QString &message)
{
    if (index >= 0 && index < m_progressBars.size()) {
        m_progressBars.at(index)->setValue(success ? 100 : 0);
    }
    onLogMessage(QStringLiteral("[#%1] %2").arg(index + 1).arg(message));
}

void MainWindow::onRegistrationCompleted(const RegistrationResult &result)
{
    m_resultStore.append(result);
}

void MainWindow::onAllTasksFinished()
{
    m_startButton->setText(QStringLiteral("Start"));
    onLogMessage(QStringLiteral("All tasks finished."));
}

void MainWindow::onLogMessage(const QString &message)
{
    m_logView->appendPlainText(message);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->position().y() <= 30) {
        m_dragging = true;
        m_dragOffset = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
        return;
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragOffset);
        event->accept();
        return;
    }
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
    QMainWindow::mouseReleaseEvent(event);
}
