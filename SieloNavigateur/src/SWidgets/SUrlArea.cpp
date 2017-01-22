#include "includes/SWidgets/SUrlArea.hpp"
#include "includes/SMainWindow.hpp"
#include "includes/SActions.hpp"
#include "includes/SWidgets/OLDSWebView.hpp"
#include "includes/SWindows/History/SHistory.hpp"
#include "includes/SApplication.hpp"

#include <QMessageBox>
#include <QPushButton>
#include <QUrl>

SUrlArea::SUrlArea(SMainWindow * parent) :
	QProgressBar(parent),
	m_parent(parent)
{
	QHBoxLayout* layout{ new QHBoxLayout(this) };
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	m_champs->setStyleSheet("background: #00FFFFFF; border: none;");
	setStyleSheet("QProgressBar{ border: none; background-color: #FFFFFF; } QProgressBar::chunk{background-color: #FFFFFF;}");

	m_champs->setClearButtonEnabled(true);
	layout->addWidget(m_champs);

	setTextVisible(false);

	connect(m_champs, &QLineEdit::returnPressed, this, &SUrlArea::loadUrl);
}

SUrlArea::~SUrlArea()
{

}

void SUrlArea::setText(const QString& texte)
{
	m_champs->setText(texte);
}

void SUrlArea::loadStarted()
{
	// Change the refresh action to the stop action
	if (m_parent->getActions()->refreshOrStop->text() != tr("Arrêter le chargement")) {
        m_parent->getActions()->refreshOrStop->setIcon(QIcon(mApp->themePath() + "/stop.png"));
		m_parent->getActions()->refreshOrStop->setText(tr("Arrêter le chargement"));
		m_parent->getActions()->refreshOrStop->setShortcut(QKeySequence(""));
		disconnect(m_parent->getActions()->refreshOrStop, &QAction::triggered, m_parent, &SMainWindow::refresh);
		connect(m_parent->getActions()->refreshOrStop, &QAction::triggered, m_parent, &SMainWindow::stop);
	}

    if (mApp->settings()->value("preferences/loadingBarStyle", "fineBar").toString() == "fineBar") {
		setStyleSheet("QProgressBar{ border: none; background-color: #FFFFFF; } QProgressBar::chunk{background-color: #FFFFFF; border-bottom: 2px solid #000000;}");
	}
    else if (mApp->settings()->value("preferences/loadingBarStyle", "fineBar").toString() == "flatBar") {
		setStyleSheet("QProgressBar{ border: none; background-color: #FFFFFF; } QProgressBar::chunk{background-color: #CECECE;}");
	}
	else {
		setStyleSheet("QProgressBar{ border: none; background-color: #FFFFFF; } QProgressBar::chunk{background-color: #FFFFFF; border-bottom: 2px solid #000000;}");
	}
//	setStyleSheet("QProgressBar::chunk{background-color: rgba(0, 0, 155, 0.2); border: none;}");

	QString url{ m_parent->currentPage()->url().toString() };
	if ((url.left(7) != "http://" && url.left(8) != "https://" && url.left(5) != "html/") && !url.isEmpty()) {
		QMessageBox warningMsgBox{ QMessageBox::Warning, tr("Site non sécurisé"), tr("Attention, le site sur lequel vous entrez n'est pas sécurisé !"), QMessageBox::Ignore | QMessageBox::Cancel, this };

		warningMsgBox.setButtonText(QMessageBox::Ignore, tr("Continuer"));
		warningMsgBox.setButtonText(QMessageBox::Cancel, tr("Retour a la sécurité"));

		if (warningMsgBox.exec() == QMessageBox::Cancel)
			m_parent->back();
	}
}

void SUrlArea::loadInProgress(int percent)
{
	setValue(percent);
}

void SUrlArea::loadFinished()
{
	SWebView* view{ static_cast<SWebView*>(sender()) };

	if (!view)
		return;
	if (this->value() == 100) {
        m_parent->getActions()->refreshOrStop->setIcon(QIcon(mApp->themePath() + "/refresh.png"));
		m_parent->getActions()->refreshOrStop->setText(tr("Rafraîchir la page"));
		m_parent->getActions()->refreshOrStop->setShortcuts(QKeySequence::Refresh);
		disconnect(m_parent->getActions()->refreshOrStop, &QAction::triggered, m_parent, &SMainWindow::stop);
		connect(m_parent->getActions()->refreshOrStop, &QAction::triggered, m_parent, &SMainWindow::refresh);
	}

	setStyleSheet("QProgressBar{ border: none; background-color: #FFFFFF; } QProgressBar::chunk{background-color: #FFFFFF;}");

	QRegExp youtubeRegex{ R"regex(^.*(youtu\.be\/|v\/|u\/\w\/|embed\/|watch\?v=|\&v=)([^#\&\?]*).*)regex" };
	if (view->url().toString().contains(youtubeRegex)) {
		if (!view->parent()->findChild<QPushButton*>()) {
			QVBoxLayout *tabLayout{ view->parent()->findChild<QVBoxLayout*>() };
			QPushButton *separateButton{ new QPushButton(tr("Détacher la vidéo"), static_cast<QWidget*>(view->parent())) };
			separateButton->setFlat(true);
			tabLayout->addWidget(separateButton);
			connect(separateButton, &QPushButton::clicked, m_parent, &SMainWindow::separateVideo);
		}
	}
	else if (view->parent()->findChild<QPushButton*>()) {
		view->parent()->findChild<QPushButton*>()->deleteLater();
	}

	if(!m_parent->privateBrowsing) {
		if (view->url().toString() == "about:blank")
            return;
        else {
            SHistoryItem item{ view->title(), view->url() };
            mApp->history()->addItem(item);
        }
	}
}

void SUrlArea::loadUrl()
{
	QString url{};


//	QRegExp urlReg{ "^(https?\:\/\/)?([a-zA-Z0-9-]+\.)?([a-zA-Z0-9-]+\.)?[a-zA-Z0-9-]+\.[a-zA-Z]{2,4}(:[0-9]+)?(\/[a-zA-Z0-9-]*\/?|\/[a-zA-Z0-9]+\.[a-zA-Z0-9]{1,4})?$" };
	QRegExp urlReg{ R"regex((https?\:\/\/)?([a-zA-Z0-9-]+.)?([a-zA-Z0-9-]+.)?[a-zA-Z0-9-]+\.[a-zA-Z]{2,4}(:[0-9]+)?(\/[a-zA-Z0-9-]*\/?|\/[a-zA-Z0-9]+\.[a-zA-Z0-9]{1,4})?)regex" };

	if (!m_champs->text().contains(urlReg)) {
		url = "http://www.google.com/search?q=" + m_champs->text();
		url.replace(" ", "+");
	}
	else {
		if (m_champs->text().left(7) != "http://" && m_champs->text().left(8) != "https://")
			url = "https://" + m_champs->text();
		else
			url = m_champs->text();
	}

/*
	if (m_champs->text().left(7) != "http://" && m_champs->text().left(8) != "https://")
		url = "https://" + m_champs->text();
	else
		url = m_champs->text();
*/

	m_parent->currentPage()->load(QUrl(url));
}
