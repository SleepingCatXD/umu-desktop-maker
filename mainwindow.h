#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QTextStream>
#include <QRegularExpression>
#include <QInputDialog>
#include <QDesktopServices>
#include <QImageReader>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 按钮点击事件
    void onGenerateButtonClicked();
    void onBrowseProtonPathClicked();
    void onBrowsePfxPathClicked();
    void onBrowseProgramClicked();
    void onBrowseIconClicked();

    // 输入框文本改变事件
    void onNameTextChanged();
    void onProtonPathTextChanged();
    void onPfxPathTextChanged();
    void onProgramTextChanged();
    void onIconTextChanged();

private:
    Ui::MainWindow *ui;

    // 工具方法
    void initializeUI();
    void connectSignals();
    bool validateRequiredFields() const;
    bool validatePaths() const;
    QString getDesktopFileContent() const;
    QString sanitizeFileName(const QString &name) const;
    QString escapeDesktopString(const QString &input) const;

    // 文件操作
    bool writeDesktopFile(const QString &content, const QString &filePath);
    void showSuccessDialog(const QString &fileName, const QString &filePath, const QString &content);
};
#endif // MAINWINDOW_H
