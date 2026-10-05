#include "homein-downloader.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>

HomeInModelDownloader::HomeInModelDownloader(const QString& target_path, const QUrl& model_url, QWidget* parent)
    : QDialog(parent), target_path(target_path), model_url(model_url) {
    setWindowTitle("Home Indeed - Downloading AI Speech Model");
    setFixedSize(450, 160);
    setModal(true);

    QVBoxLayout* layout = new QVBoxLayout(this);

    status_label = new QLabel("Connecting to download server...", this);
    status_label->setWordWrap(true);
    layout->addWidget(status_label);

    progress_bar = new QProgressBar(this);
    progress_bar->setRange(0, 100);
    progress_bar->setValue(0);
    layout->addWidget(progress_bar);

    QHBoxLayout* btn_layout = new QHBoxLayout();
    btn_layout->addStretch();
    cancel_btn = new QPushButton("Cancel", this);
    btn_layout->addWidget(cancel_btn);
    layout->addLayout(btn_layout);

    connect(cancel_btn, &QPushButton::clicked, this, &HomeInModelDownloader::OnCancelClicked);

    // Ensure directory exists
    QFileInfo fi(target_path);
    QDir().mkpath(fi.absolutePath());

    QString part_path = target_path + ".part";
    file.setFileName(part_path);
    if (!file.open(QIODevice::WriteOnly)) {
        status_label->setText("Error: Cannot write to target directory.");
        return;
    }

    network_manager = new QNetworkAccessManager(this);
    QNetworkRequest request(model_url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    reply = network_manager->get(request);
    connect(reply, &QNetworkReply::downloadProgress, this, &HomeInModelDownloader::OnDownloadProgress);
    connect(reply, &QNetworkReply::readyRead, this, &HomeInModelDownloader::OnReadyRead);
    connect(reply, &QNetworkReply::finished, this, &HomeInModelDownloader::OnDownloadFinished);
}

HomeInModelDownloader::~HomeInModelDownloader() {
    if (reply) {
        reply->abort();
        reply->deleteLater();
    }
    if (file.isOpen()) {
        file.close();
    }
}

void HomeInModelDownloader::OnReadyRead() {
    if (reply && file.isOpen()) {
        file.write(reply->readAll());
    }
}

void HomeInModelDownloader::OnDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    if (bytesTotal > 0) {
        int percent = static_cast<int>((bytesReceived * 100) / bytesTotal);
        progress_bar->setValue(percent);

        double mb_received = bytesReceived / (1024.0 * 1024.0);
        double mb_total = bytesTotal / (1024.0 * 1024.0);
        status_label->setText(QString("Downloading Whisper model: %1 MB / %2 MB (%3%)")
                                  .arg(mb_received, 0, 'f', 1)
                                  .arg(mb_total, 0, 'f', 1)
                                  .arg(percent));
    } else {
        double mb_received = bytesReceived / (1024.0 * 1024.0);
        status_label->setText(QString("Downloading Whisper model: %1 MB downloaded...")
                                  .arg(mb_received, 0, 'f', 1));
    }
}

void HomeInModelDownloader::OnDownloadFinished() {
    if (!reply) return;

    if (file.isOpen()) {
        file.write(reply->readAll());
        file.close();
    }

    if (reply->error() == QNetworkReply::NoError) {
        QString part_path = target_path + ".part";
        QFile::remove(target_path);
        if (QFile::rename(part_path, target_path)) {
            download_success = true;
            status_label->setText("Download complete!");
            accept();
            return;
        } else {
            status_label->setText("Error: Failed to save completed model file.");
        }
    } else {
        status_label->setText(QString("Download error: %1").arg(reply->errorString()));
        file.remove();
    }
}

void HomeInModelDownloader::OnCancelClicked() {
    if (reply) {
        reply->abort();
    }
    if (file.isOpen()) {
        file.close();
        file.remove();
    }
    reject();
}
