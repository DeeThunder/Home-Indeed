#pragma once
#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>

class HomeInModelDownloader : public QDialog {
    Q_OBJECT
public:
    explicit HomeInModelDownloader(const QString& target_path, const QUrl& model_url, QWidget* parent = nullptr);
    ~HomeInModelDownloader();

    bool IsDownloaded() const { return download_success; }

private slots:
    void OnDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void OnDownloadFinished();
    void OnReadyRead();
    void OnCancelClicked();

private:
    QString target_path;
    QUrl model_url;
    QProgressBar* progress_bar;
    QLabel* status_label;
    QPushButton* cancel_btn;
    QNetworkAccessManager* network_manager;
    QNetworkReply* reply = nullptr;
    QFile file;
    bool download_success = false;
};
