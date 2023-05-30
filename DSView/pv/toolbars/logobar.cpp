/*
 * This file is part of the DSView project.
 * DSView is based on PulseView.
 *
 * Copyright (C) 2013 DreamSourceLab <support@dreamsourcelab.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <boost/bind.hpp>
 

#include <QMetaObject>
#include <QFileDialog> 
#include <QDesktopServices>
#include <QUrl>
#include <QApplication>
#include <assert.h>
#include <QComboBox>
#include <QFormLayout>
#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFile> 
#include <QLabel>

#include "logobar.h"
#include "../dialogs/about.h"
#include "../dialogs/dsmessagebox.h"
#include "../config/appconfig.h"
#include "../dialogs/dsdialog.h"
#include "../appcontrol.h"
#include "../log.h"
#include "../ui/langresource.h"
#include "../ui/msgbox.h"


namespace pv {
namespace toolbars {

LogoBar::LogoBar(SigSession *session, QWidget *parent) :
    QToolBar("File Bar", parent),
    _enable(true),
    _connected(false),
    _session(session),
    _logo_button(this)
{
    _mainForm  = NULL;

    setMovable(false);
    setContentsMargins(0,0,0,0);

    _action_en = new QAction(this);
    _action_en->setObjectName(QString::fromUtf8("actionEn"));
   
    _action_cn = new QAction(this);
    _action_cn->setObjectName(QString::fromUtf8("actionCn"));
    
    _language = new QMenu(this);
    _language->setObjectName(QString::fromUtf8("menuLanguage"));
    _language->addAction(_action_cn);
    _language->addAction(_action_en);

    _action_en->setIcon(QIcon(":/icons/English.svg"));
    _action_cn->setIcon(QIcon(":/icons/Chinese.svg"));

    _about = new QAction(this);
    _about->setObjectName(QString::fromUtf8("actionAbout"));
    _logo_button.addAction(_about);
     
    _manual = new QAction(this);
    _manual->setObjectName(QString::fromUtf8("actionManual"));
    _logo_button.addAction(_manual);
   
    _issue = new QAction(this);
    _issue->setObjectName(QString::fromUtf8("actionIssue"));
    _logo_button.addAction(_issue);   

    _update = new QAction(this);
    _log = new QAction(this);

    _menu = new QMenu(this);
    _menu->addMenu(_language);
    _menu->addAction(_about);
    _menu->addAction(_manual);
    _menu->addAction(_issue);
    _menu->addAction(_update);
    _menu->addAction(_log);
    _logo_button.setMenu(_menu);

    _logo_button.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    _logo_button.setPopupMode(QToolButton::InstantPopup);

    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    addWidget(spacer);
    addWidget(&_logo_button);
    QWidget *margin = new QWidget(this);
    margin->setMinimumWidth(20);
    addWidget(margin);

    retranslateUi();

    connect(_action_en, SIGNAL(triggered()), this, SLOT(on_actionEn_triggered()));
    connect(_action_cn, SIGNAL(triggered()), this, SLOT(on_actionCn_triggered()));
    connect(_about, SIGNAL(triggered()), this, SLOT(on_actionAbout_triggered()));
    connect(_manual, SIGNAL(triggered()), this, SIGNAL(sig_open_doc()));
    connect(_issue, SIGNAL(triggered()), this, SLOT(on_actionIssue_triggered()));
    connect(_update, SIGNAL(triggered()), this, SLOT(on_action_update()));
    connect(_log, SIGNAL(triggered()), this, SLOT(on_action_setting_log()));
}

void LogoBar::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    else if (event->type() == QEvent::StyleChange)
        reStyle();
    QToolBar::changeEvent(event);
}

void LogoBar::retranslateUi()
{

    _logo_button.setText(L_S(STR_PAGE_TOOLBAR, S_ID(IDS_TOOLBAR_HELP), "Help"));
     _language->setTitle(L_S(STR_PAGE_TOOLBAR, S_ID(IDS_TOOLBAR_HELP_LANG), "&Language"));
    _action_en->setText(L_S(STR_PAGE_TOOLBAR, S_ID(IDS_TOOLBAR_HELP_LANG_EN), "English"));
    _action_cn->setText(L_S(STR_PAGE_TOOLBAR, S_ID(IDS_TOOLBAR_HELP_LANG_CN), "中文"));   
    _about->setText(L_S(STR_PAGE_TOOLBAR, S_ID(IDS_TOOLBAR_HELP_ABOUT), "&About..."));
    _manual->setText(L_S(STR_PAGE_TOOLBAR, S_ID(IDS_TOOLBAR_HELP_MANUAL), "&Manual..."));
    _issue->setText(L_S(STR_PAGE_TOOLBAR, S_ID(IDS_TOOLBAR_HELP_BUG), "&Bug Report"));
    _update->setText(L_S(STR_PAGE_TOOLBAR, S_ID(IDS_TOOLBAR_HELP_UPDATE), "&Update"));
    _log->setText(L_S(STR_PAGE_TOOLBAR, S_ID(IDS_TOOLBAR_HELP_LOG), "L&og Options"));

    AppConfig &app = AppConfig::Instance(); 
    if (app._frameOptions.language == LAN_CN)
        _language->setIcon(QIcon(":/icons/Chinese.svg"));
    else
        _language->setIcon(QIcon(":/icons/English.svg"));
}

void LogoBar::reStyle()
{
    QString iconPath = GetIconPath();

    _about->setIcon(QIcon(iconPath+"/about.svg"));
    _manual->setIcon(QIcon(iconPath+"/manual.svg"));
    _issue->setIcon(QIcon(iconPath+"/bug.svg"));
    _update->setIcon(QIcon(iconPath+"/update.svg"));
    _log->setIcon(QIcon(iconPath+"/log.svg"));

    if (_connected)
        _logo_button.setIcon(QIcon(iconPath+"/logo_color.svg"));
    else
        _logo_button.setIcon(QIcon(iconPath+"/logo_noColor.svg"));
}

void LogoBar::dsl_connected(bool conn)
{
    _connected = conn;
    QString iconPath =  GetIconPath();
    if (_connected)
        _logo_button.setIcon(QIcon(iconPath+"/logo_color.svg"));
    else
        _logo_button.setIcon(QIcon(iconPath+"/logo_noColor.svg"));
}

void LogoBar::on_actionEn_triggered()
{
    _language->setIcon(QIcon::fromTheme("file",
        QIcon(":/icons/English.svg")));

    assert(_mainForm);
    _mainForm->switchLanguage(LAN_EN);
}

void LogoBar::on_actionCn_triggered()
{
    _language->setIcon(QIcon::fromTheme("file",
        QIcon(":/icons/Chinese.svg")));
    assert(_mainForm);
    _mainForm->switchLanguage(LAN_CN);  
}

void LogoBar::on_actionAbout_triggered()
{
    dialogs::About dlg(this);
    dlg.exec();
}

void LogoBar::on_actionManual_triggered()
{ 
    QDir dir(GetAppDataDir());
    QDesktopServices::openUrl( QUrl("file:///"+dir.absolutePath() + "/ug.pdf"));
}

void LogoBar::on_actionIssue_triggered()
{
    QDesktopServices::openUrl(QUrl(QLatin1String("https://github.com/DreamSourceLab/DSView/issues")));
}

 void LogoBar::on_action_update()
 {
     if (AppConfig::Instance()._frameOptions.language == LAN_CN){
         QDesktopServices::openUrl(QUrl(QLatin1String("https://dreamsourcelab.cn/download/")));
     }
     else{
         QDesktopServices::openUrl(QUrl(QLatin1String("https://www.dreamsourcelab.com/download/")));
     }
 }

void LogoBar::enable_toggle(bool enable)
{
    _logo_button.setDisabled(!enable);
}

void LogoBar::on_action_setting_log()
{   
    AppConfig &app = AppConfig::Instance(); 
    auto *topWind = AppControl::Instance()->GetTopWindow();
    dialogs::DSDialog dlg(topWind, false, true);
    dlg.setTitle(L_S(STR_PAGE_DLG, S_ID(IDS_DLG_LOG_OPTIONS), "Log Options"));
    dlg.setMinimumSize(460, 300);
    QWidget *panel = new QWidget(&dlg);
    dlg.layout()->addWidget(panel);
    panel->setMinimumSize(250, 110);
    QFormLayout *lay = new QFormLayout();
    panel->setLayout(lay);
    lay->setVerticalSpacing(15);
 
    QComboBox *cbBox = new QComboBox();
    cbBox->setMinimumWidth(40);
    lay->addRow(L_S(STR_PAGE_DLG, S_ID(IDS_DLG_LOG_LEVEL), "Log Level"), cbBox);

    for (int i=0; i<=5; i++){
        cbBox->addItem(QString::number(i));
    }
    cbBox->setCurrentIndex(app._appOptions.logLevel);

    QCheckBox *ckSave = new QCheckBox();
    ckSave->setChecked(app._appOptions.ableSaveLog);
    lay->addRow(L_S(STR_PAGE_DLG, S_ID(IDS_DLG_SAVE_FILE), "Save To File"), ckSave);

    QCheckBox *ckRebuild = new QCheckBox();
    ckRebuild->setChecked(app._appOptions.appendLogMode);
    lay->addRow(L_S(STR_PAGE_DLG, S_ID(IDS_DLG_APPEND_MODE), "Append mode"), ckRebuild);

    QPushButton *btOpen = new QPushButton();
    btOpen->setText(L_S(STR_PAGE_DLG, S_ID(IDS_DLG_OPEN), "Open"));
    connect(btOpen, SIGNAL(released()), this, SLOT(on_open_log_file()));

    QPushButton *btClear = new QPushButton();
    btClear->setText(L_S(STR_PAGE_DLG, S_ID(IDS_DLG_CLEARE), "Clear"));
    connect(btClear, SIGNAL(released()), this, SLOT(on_clear_log_file()));

    QWidget *btWid = new QWidget();
    QHBoxLayout *btLay = new QHBoxLayout();
    btWid->setLayout(btLay);
    btLay->setSpacing(10);
    btLay->addWidget(btOpen);
    btLay->addWidget(btClear);

    lay->addRow("", btWid);

    QFile qf(get_dsv_log_path());
    if (qf.exists() == false){
        btOpen->setEnabled(false);
        btClear->setEnabled(false);
    }

    dlg.exec();

    if (dlg.IsClickYes()){
        bool ableSave = ckSave->isChecked();
        int level = cbBox->currentIndex();
        bool appendLogMode = ckRebuild->isChecked();

        if (ableSave != app._appOptions.ableSaveLog 
            || level != app._appOptions.logLevel
            || appendLogMode != app._appOptions.appendLogMode){
            app._appOptions.ableSaveLog = ableSave;
            app._appOptions.logLevel = level;
            app._appOptions.appendLogMode = appendLogMode;            
            app.SaveApp();

            dsv_log_level(level);
            
            if (ableSave)
                dsv_log_enalbe_logfile(false);
            else
                dsv_remove_log_file();
        }
    }  
}

void LogoBar::on_open_log_file()
{
    QFile qf(get_dsv_log_path());
    if (qf.exists()){
        QDesktopServices::openUrl( QUrl("file:///" + get_dsv_log_path()));
    }
    else{
        QString strMsg(L_S(STR_PAGE_MSG, S_ID(IDS_MSG_FILE_NOT_EXIST), "Not exist!"));
        MsgBox::Show(strMsg);
    }        
}

void LogoBar::on_clear_log_file()
{
    QFile qf(get_dsv_log_path());
    if (qf.exists()){
        QString strMsg(L_S(STR_PAGE_MSG, S_ID(IDS_MSG_TO_CLEAR_LOG), "Confirm!"));
        if (MsgBox::Confirm(strMsg)){
            dsv_clear_log_file();
        }
    }
    else{
        QString strMsg(L_S(STR_PAGE_MSG, S_ID(IDS_MSG_FILE_NOT_EXIST), "Not exist!"));
        MsgBox::Show(strMsg);
    }  
}

} // namespace toolbars
} // namespace pv
