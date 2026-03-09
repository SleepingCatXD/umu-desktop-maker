#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initializeUI();
    connectSignals();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initializeUI()
{
    ui->nameEdit->setPlaceholderText(QStringLiteral("例如: 我的游戏"));
    ui->protonPathEdit->setPlaceholderText(QStringLiteral("例如: /home/user/.steam/steam/steamapps/common/Proton 9.0"));
    ui->pfxPathEdit->setPlaceholderText(QStringLiteral("例如: /home/user/.steam/steam/steamapps/compatdata/123456"));
    ui->programEdit->setPlaceholderText(QStringLiteral("例如: /home/user/Games/game/game.exe"));
    ui->iconEdit->setPlaceholderText(QStringLiteral("可选：图标文件路径或图标名称（留空则不设置图标）"));

    ui->nameEdit->setToolTip(QStringLiteral("程序在应用菜单中显示的名称"));
    ui->protonPathEdit->setToolTip(QStringLiteral("Wine/Proton前缀路径, 截止到pfx目录前"));
    ui->pfxPathEdit->setToolTip(QStringLiteral("Proton可执行文件所在的目录路径, 不包括可执行文件"));
    ui->programEdit->setToolTip(QStringLiteral("要运行的游戏或程序的可执行文件路径"));
    ui->iconEdit->setToolTip(QStringLiteral("图标文件路径或系统图标名称"));
}

void MainWindow::connectSignals()
{
    // 按钮点击
    connect(ui->generateButton, &QPushButton::clicked,
            this, &MainWindow::onGenerateButtonClicked);
    connect(ui->browseProtonButton, &QPushButton::clicked,
            this, &MainWindow::onBrowseProtonPathClicked);
    connect(ui->browsePfxButton, &QPushButton::clicked,
            this, &MainWindow::onBrowsePfxPathClicked);
    connect(ui->browseProgramButton, &QPushButton::clicked,
            this, &MainWindow::onBrowseProgramClicked);
    connect(ui->browseIconButton, &QPushButton::clicked,
            this, &MainWindow::onBrowseIconClicked);

    // 文本改变
    connect(ui->nameEdit, &QLineEdit::textChanged,
            this, &MainWindow::onNameTextChanged);
    connect(ui->protonPathEdit, &QLineEdit::textChanged,
            this, &MainWindow::onProtonPathTextChanged);
    connect(ui->pfxPathEdit, &QLineEdit::textChanged,
            this, &MainWindow::onPfxPathTextChanged);
    connect(ui->programEdit, &QLineEdit::textChanged,
            this, &MainWindow::onProgramTextChanged);
    connect(ui->iconEdit, &QLineEdit::textChanged,
            this, &MainWindow::onIconTextChanged);
}

bool MainWindow::validateRequiredFields() const
{
    return !ui->nameEdit->text().trimmed().isEmpty() &&
           !ui->protonPathEdit->text().trimmed().isEmpty() &&
           !ui->pfxPathEdit->text().trimmed().isEmpty() &&
           !ui->programEdit->text().trimmed().isEmpty();
}

bool MainWindow::validatePaths() const
{
    const QString protonPath = ui->protonPathEdit->text().trimmed();
    const QString pfxPath = ui->pfxPathEdit->text().trimmed();
    const QString programPath = ui->programEdit->text().trimmed();
    const QString iconPath = ui->iconEdit->text().trimmed();

    if (!QFileInfo::exists(protonPath)) {
        QMessageBox::warning(const_cast<MainWindow*>(this), QStringLiteral("路径不存在"),
                             QStringLiteral("Proton路径不存在：\n%1").arg(protonPath));
        return false;
    }

    if (!QFileInfo::exists(pfxPath)) {
        QMessageBox::warning(const_cast<MainWindow*>(this), QStringLiteral("路径不存在"),
                             QStringLiteral("前缀路径不存在：\n%1").arg(pfxPath));
        return false;
    }

    if (!QFileInfo::exists(programPath)) {
        QMessageBox::warning(const_cast<MainWindow*>(this), QStringLiteral("文件不存在"),
                             QStringLiteral("程序文件不存在：\n%1").arg(programPath));
        return false;
    }

    // 检查图标路径：如果提供了且不是有效文件，则可能是系统图标名称，仅当看起来不像合法文件名时警告
    if (!iconPath.isEmpty() && !QFileInfo::exists(iconPath)) {
        static const QRegularExpression iconNameRegex(QStringLiteral("^[a-zA-Z0-9_-]+(\\.[a-zA-Z0-9_-]+)*$"));
        if (!iconPath.contains(iconNameRegex)) {
            QMessageBox::warning(const_cast<MainWindow*>(this), QStringLiteral("图标路径警告"),
                                 QStringLiteral("图标文件不存在：\n%1\n可能是系统图标名称，继续生成。").arg(iconPath));
        }
    }

    return true;
}

QString MainWindow::escapeDesktopString(const QString &input) const
{
    if (input.isEmpty())
        return input;

    QString escaped = input;
    // 使用 QString::replace 链式调用，先替换 \ 避免干扰后续替换
    escaped.replace(QLatin1Char('\\'), QStringLiteral("\\\\"))
        .replace(QLatin1Char('"'), QStringLiteral("\\\""))
        .replace(QLatin1Char('`'), QStringLiteral("\\`"))
        .replace(QLatin1Char('$'), QStringLiteral("\\$"));
    return escaped;
}

QString MainWindow::sanitizeFileName(const QString &name) const
{
    if (name.isEmpty())
        return QStringLiteral("game");

    QString sanitized;
    sanitized.reserve(name.size()); // 预分配空间
    for (const QChar &ch : name) {
        if (ch.isLetterOrNumber() || ch == QLatin1Char('_') ||
            ch == QLatin1Char('-') || ch == QLatin1Char('.')) {
            sanitized.append(ch);
        } else {
            sanitized.append(QLatin1Char('_'));
        }
    }

    // 移除开头的点
    while (sanitized.startsWith(QLatin1Char('.')))
        sanitized.remove(0, 1);

    return sanitized.isEmpty() ? QStringLiteral("game") : sanitized;
}

QString MainWindow::getDesktopFileContent() const
{
    const QString appName = ui->nameEdit->text().trimmed();
    const QString protonPath = ui->protonPathEdit->text().trimmed();
    const QString pfxPath = ui->pfxPathEdit->text().trimmed();
    const QString programPath = ui->programEdit->text().trimmed();
    const QString iconPath = ui->iconEdit->text().trimmed();

    QStringList content;
    content.reserve(8); // 预分配

    content << QStringLiteral("[Desktop Entry]")
            << QStringLiteral("Categories=Game;")
            << QStringLiteral("Comment=")
            << QStringLiteral("Exec=env PROTONPATH=\"%1\" WINEPREFIX=\"%2\" /usr/bin/umu-run \"%3\"")
                   .arg(escapeDesktopString(protonPath),
                        escapeDesktopString(pfxPath),
                        escapeDesktopString(programPath));

    if (!iconPath.isEmpty())
        content << QStringLiteral("Icon=%1").arg(escapeDesktopString(iconPath));

    content << QStringLiteral("Name=%1").arg(escapeDesktopString(appName))
            << QStringLiteral("StartupNotify=true")
            << QStringLiteral("Terminal=false")
            << QStringLiteral("Type=Application");

    return content.join(QLatin1Char('\n'));
}

bool MainWindow::writeDesktopFile(const QString &content, const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << content;
    file.close();

    // 设置可执行权限
    // 非必要
    // file.setPermissions(file.permissions() | QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther);
    return true;
}

void MainWindow::showSuccessDialog(const QString &fileName, const QString &filePath, const QString &content)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(QStringLiteral("生成成功"));
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(QStringLiteral("已成功创建桌面文件：\n%1").arg(fileName));

    QString summary = QStringLiteral("程序名称: %1\n保存位置: %2")
                          .arg(ui->nameEdit->text().trimmed(),
                               QDir::toNativeSeparators(filePath));
    msgBox.setInformativeText(summary);

    QPushButton *openLocationButton = msgBox.addButton(QStringLiteral("打开文件位置"), QMessageBox::ActionRole);
    QPushButton *viewContentButton = msgBox.addButton(QStringLiteral("查看内容"), QMessageBox::ActionRole);
    msgBox.addButton(QStringLiteral("关闭"), QMessageBox::RejectRole);

    msgBox.exec();

    if (msgBox.clickedButton() == openLocationButton) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(filePath).absolutePath()));
    } else if (msgBox.clickedButton() == viewContentButton) {
        QMessageBox::information(this, QStringLiteral("桌面文件内容"),
                                 QStringLiteral("<pre>%1</pre>").arg(content.toHtmlEscaped()));
    }
}

// ====================== 按钮点击事件 ======================

void MainWindow::onGenerateButtonClicked()
{
    if (!validateRequiredFields()) {
        QMessageBox::warning(this, QStringLiteral("输入不完整"), QStringLiteral("请填写所有必填字段！"));
        return;
    }

    if (!validatePaths())
        return;

    QString applicationsDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    if (applicationsDir.isEmpty())
        applicationsDir = QDir::homePath() + QStringLiteral("/.local/share/applications");

    QDir dir(applicationsDir);
    if (!dir.exists())
        dir.mkpath(QStringLiteral("."));

    QString baseName = sanitizeFileName(ui->nameEdit->text().trimmed());
    QString fileName = baseName + QStringLiteral(".desktop");
    QString filePath = dir.filePath(fileName);

    if (QFile::exists(filePath)) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, QStringLiteral("文件已存在"),
                                                                  QStringLiteral("文件 <b>%1</b> 已存在，是否覆盖？").arg(fileName),
                                                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (reply == QMessageBox::No) {
            bool ok;
            QString newName = QInputDialog::getText(this, QStringLiteral("输入新名称"),
                                                    QStringLiteral("请输入新的桌面文件名称："), QLineEdit::Normal,
                                                    baseName, &ok);
            if (ok && !newName.isEmpty()) {
                fileName = sanitizeFileName(newName) + QStringLiteral(".desktop");
                filePath = dir.filePath(fileName);
            } else {
                return;
            }
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }

    QString content = getDesktopFileContent();

    if (writeDesktopFile(content, filePath)) {
        showSuccessDialog(fileName, filePath, content);
    } else {
        QMessageBox::critical(this, QStringLiteral("写入失败"),
                              QStringLiteral("无法写入文件：\n%1\n\n错误：%2")
                                  .arg(QDir::toNativeSeparators(filePath),
                                       QFile(filePath).errorString()));
    }
}

void MainWindow::onBrowseProtonPathClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("选择Proton可执行文件路径"),
                                                    QDir::homePath(), QFileDialog::ShowDirsOnly);
    if (!dir.isEmpty())
        ui->protonPathEdit->setText(QDir::toNativeSeparators(dir));
}

void MainWindow::onBrowsePfxPathClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("选择Proton/Wine前缀路径"),
                                                    QDir::homePath(), QFileDialog::ShowDirsOnly);
    if (!dir.isEmpty())
        ui->pfxPathEdit->setText(QDir::toNativeSeparators(dir));
}

void MainWindow::onBrowseProgramClicked()
{
    QString filter = QStringLiteral("可执行文件 (*.exe *.bat *.cmd);;所有文件 (*.*)");
    QString file = QFileDialog::getOpenFileName(this, QStringLiteral("选择游戏/程序文件"),
                                                QDir::homePath(), filter);
    if (!file.isEmpty()) {
        ui->programEdit->setText(QDir::toNativeSeparators(file));

        if (ui->nameEdit->text().trimmed().isEmpty()) {
            QFileInfo fileInfo(file);
            QString baseName = fileInfo.completeBaseName();
            static const QRegularExpression versionRegex(QStringLiteral("[-_vV]\\d+(\\.\\d+)*$"));
            static const QRegularExpression archRegex(QStringLiteral("_(x86|x64|32|64)$"));
            baseName.replace(versionRegex, QString()).replace(archRegex, QString());
            ui->nameEdit->setText(baseName);
        }
    }
}

void MainWindow::onBrowseIconClicked()
{
    const auto formats = QImageReader::supportedImageFormats();
    QStringList filters;
    filters.reserve(formats.size());
    for (const QByteArray &format : formats) {
        filters.append(QStringLiteral("*.%1").arg(QString::fromLatin1(format)));
    }
    QString filter = QStringLiteral("图片文件 (%1);;所有文件 (*.*)").arg(filters.join(QLatin1Char(' ')));
    QString file = QFileDialog::getOpenFileName(this, QStringLiteral("选择图标文件"),
                                                QDir::homePath(), filter);
    if (!file.isEmpty())
        ui->iconEdit->setText(QDir::toNativeSeparators(file));
}

// ====================== 输入框文本改变事件 ======================

void MainWindow::onNameTextChanged()
{
    bool hasText = !ui->nameEdit->text().trimmed().isEmpty();
    ui->nameEdit->setStyleSheet(hasText ? QString() : QStringLiteral("border: 1px solid #f44336;"));
    ui->generateButton->setEnabled(validateRequiredFields());
}

void MainWindow::onProtonPathTextChanged()
{
    bool hasText = !ui->protonPathEdit->text().trimmed().isEmpty();
    ui->protonPathEdit->setStyleSheet(hasText ? QString() : QStringLiteral("border: 1px solid #f44336;"));
    ui->generateButton->setEnabled(validateRequiredFields());
}

void MainWindow::onPfxPathTextChanged()
{
    bool hasText = !ui->pfxPathEdit->text().trimmed().isEmpty();
    ui->pfxPathEdit->setStyleSheet(hasText ? QString() : QStringLiteral("border: 1px solid #f44336;"));
    ui->generateButton->setEnabled(validateRequiredFields());
}

void MainWindow::onProgramTextChanged()
{
    bool hasText = !ui->programEdit->text().trimmed().isEmpty();
    ui->programEdit->setStyleSheet(hasText ? QString() : QStringLiteral("border: 1px solid #f44336;"));
    ui->generateButton->setEnabled(validateRequiredFields());
}

void MainWindow::onIconTextChanged()
{
    // 图标路径可选，不需要验证
    Q_UNUSED(sender());
}
