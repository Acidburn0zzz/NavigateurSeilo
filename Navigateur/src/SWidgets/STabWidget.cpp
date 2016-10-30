#include "includes/SWidgets/STabWidget.hpp"
#include "includes/SWidgets/SWebView.hpp"
#include "includes/SWidgets/SUrlArea.hpp"
#include "includes/SMainWindow.hpp"

STabWidget::STabWidget(SMainWindow * parent) : 
	QTabWidget(parent),
	m_parent(parent)
{
	// Set attributes for the widgets
	setTabsClosable(true);
    setMovable(true);

    QPushButton *plusButton{ new QPushButton(QIcon(SMainWindow::dataPath + "Images/plusTab.png"), "", this) };
    plusButton->setMaximumWidth(24);
    plusButton->setFlat(true);
    setCornerWidget(plusButton);

	// Connections
	connect(this, &STabWidget::currentChanged, this, &STabWidget::tabChanged);
	connect(this, &STabWidget::tabCloseRequested, this, &STabWidget::tabClosed);
    connect(plusButton, &QPushButton::clicked, this, &STabWidget::createDefaultWebTab);
}

STabWidget::~STabWidget()
{
	// Empty
}

void STabWidget::createWebTab(QString title, SWebView * view)
{
	// Widgets in the tab
	QWidget* tabPage{ new QWidget(this) };
	QVBoxLayout* pageLayout{ new QVBoxLayout(tabPage) };
	view->changeParent(tabPage, this);

	pageLayout->setMargin(0);
	pageLayout->addWidget(view);
    tabPage->setFocusPolicy(Qt::StrongFocus);
    tabPage->installEventFilter(this);

	// Connect the web view to all actions
	connect(view, &SWebView::titleChanged, m_parent, &SMainWindow::changeTitle);
	connect(view, &SWebView::urlChanged, m_parent, &SMainWindow::changeUrl);
	connect(view, &SWebView::loadStarted, m_parent->getUrlArea(), &SUrlArea::loadStarted);
	connect(view, &SWebView::loadProgress, m_parent->getUrlArea(), &SUrlArea::loadInProgress);
	connect(view, &SWebView::loadFinished, m_parent->getUrlArea(), &SUrlArea::loadFinished);
	connect(view->page()->profile(), &QWebEngineProfile::downloadRequested, m_parent, &SMainWindow::addDownload);

	addTab(tabPage, title);

	tabPage->setAttribute(Qt::WA_DeleteOnClose);

	// Enable are disable cookies
	if (!SMainWindow::SSettings->value("preferences/enableCookies", true).toBool())
		view->page()->profile()->cookieStore()->deleteAllCookies();
	else if(m_parent->privateBrowsing)
		view->page()->profile()->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
}

void STabWidget::createWebTab(QString title, QUrl url)
{
	// Widgets in the tab
	QWidget* tabPage{ new QWidget(this) };
	SWebView* view{ new SWebView(tabPage, this, url) };
	QVBoxLayout* pageLayout{ new QVBoxLayout(tabPage) };

	pageLayout->setMargin(0);
	pageLayout->addWidget(view);
    tabPage->setFocusPolicy(Qt::StrongFocus);
    tabPage->installEventFilter(this);

	// Connect the web view to all action
	connect(view, &SWebView::titleChanged, m_parent, &SMainWindow::changeTitle);
	connect(view, &SWebView::urlChanged, m_parent, &SMainWindow::changeUrl);
	connect(view, &SWebView::loadStarted, m_parent->getUrlArea(), &SUrlArea::loadStarted);
	connect(view, &SWebView::loadProgress, m_parent->getUrlArea(), &SUrlArea::loadInProgress);
	connect(view, &SWebView::loadFinished, m_parent->getUrlArea(), &SUrlArea::loadFinished);
	connect(view->page()->profile(), &QWebEngineProfile::downloadRequested, m_parent, &SMainWindow::addDownload);

	addTab(tabPage, title);

	tabPage->setAttribute(Qt::WA_DeleteOnClose);

	// Enable are disable cookies
	if (!SMainWindow::SSettings->value("preferences/enableCookies", true).toBool())
		view->page()->profile()->cookieStore()->deleteAllCookies();
	else if(m_parent->privateBrowsing)
		view->page()->profile()->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
}

void STabWidget::createDefaultWebTab()
{
	createWebTab(tr("Home page"), SMainWindow::SSettings->value("preferences/homePage", "http://google.com").toUrl());
    setCurrentIndex(count() - 1);
}

bool STabWidget::eventFilter(QObject *watched, QEvent *event)
{

    if(event->type() == QEvent::Enter) {
        if(m_parent->getTabs() != this)
            m_parent->setTabs(this);
    }
}

void STabWidget::tabClosed(int index)
{
	if (index == -1)
		index = currentIndex();

    if (count() > 1) {
		widget(index)->deleteLater();
		removeTab(index);
	}
    else if(m_parent->getSplitter()->count() > 1) {
        this->setParent(nullptr);
        this->deleteLater();
        close();
    }
    else
        m_parent->close();
}

void STabWidget::tabChanged(int index)
{
    m_parent->changeTabTitle(m_parent->currentPage()->title());
    m_parent->changeTabUrl(m_parent->currentPage()->url());
}

