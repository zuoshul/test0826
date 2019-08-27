#include "gisaddmapnodedialog.h"
#include "ui_gisaddmapnodedialog.h"
#include "apptextvalidator.h"
#include "sppclientmapmanager.h"

//GIS地图功能比较复杂，在对本模块框架代码完全熟悉之前，不要擅自修改，否则会留下潜在风险 zuoshul20190715

GisAddMapNodeDialog::GisAddMapNodeDialog(QWidget *parent) :
    SNDialog(parent),
    ui(new Ui::GisAddMapNodeDialog),
    m_ptreeView(NULL)
{
    ui->setupUi(this);
    hide();
    initForm();
}

GisAddMapNodeDialog::~GisAddMapNodeDialog()
{
    delete ui;
}

void GisAddMapNodeDialog::initForm()
{
    AppStyleHelper::updateWidgetStyle(ui->pushButtonCancel, AppStyleHelper::Style_PushButtonDefault);
    AppStyleHelper::updateWidgetStyle(ui->pushButtonConfirm, AppStyleHelper::Style_PushButtonDefault);
    retranslateUi();
}

void GisAddMapNodeDialog::loadCombox()
{
    std::vector<InviewLocalMap> *maps = g_SPPClientMapManager->listMaps();
    if(maps == NULL)
    {
        return;
    }
    ui->comboBoxMapTree->clear();
    ui->comboBoxMapTree->setMaxVisibleItems(25);
    QVector<Node> vecNodes;
    g_SppClient->organizeMapNode(vecNodes, *maps);
    if(!vecNodes.isEmpty())
    {
        if(m_ptreeView)
        {
            m_ptreeView->deleteLater();
            m_ptreeView = 0;
        }
        m_ptreeView = new SNTreeView();
        ui->comboBoxMapTree->setModel(m_ptreeView->model());
        ui->comboBoxMapTree->setView(m_ptreeView);
        m_ptreeView->clearItemData();
        m_ptreeView->appendItemData(vecNodes);
        m_ptreeView->setRootAsCurrentItem();
        m_ptreeView->expandAll();
        //m_ptreeView->expandThree();
    }
}

void GisAddMapNodeDialog::retranslateUi()
{
    ui->pushButtonCancel->setText(QObject::tr("TK_Cancel"));
    ui->pushButtonConfirm->setText(QObject::tr("TK_Confirm"));
    ui->labelTitle->setText(QObject::tr("TK_AddGisMapNode"));
    ui->retranslateUi(this);
}

bool GisAddMapNodeDialog::verifyContent()
{
    return verifyLineEditText(ui->lineEditName);
}

bool GisAddMapNodeDialog::doVerifyLineEditText(QLineEdit* lineEdit, QString& toolTipStr)
{
    bool checkRs = true;
    if(lineEdit->text().isEmpty())
    {
        if(lineEdit == ui->lineEditName)
        {
            checkRs = false;
            toolTipStr.append(QObject::tr("TK_MapNameEmpty"));
        }
    }
    else
    {
        if(lineEdit == ui->lineEditName)
        {
            if(ui->lineEditName->text().indexOf(" ") != -1)
            {
                checkRs = false;
                toolTipStr.append(QObject::tr("TK_IllegalCharacters"));
            }
            QString res;
            if(APPTextValidator::isStringWithSpecialChar(ui->lineEditName->text(), res))
            {
                checkRs = false;
                toolTipStr.append(res);
            }
            if(!APPTextValidator::isStringLengthValid(ui->lineEditName->text(), res))
            {
                checkRs = false;
                toolTipStr.append(res);
            }
        }
    }
    return checkRs;
}

void GisAddMapNodeDialog::openDialog()
{
    ui->lineEditName->clear();
    ui->lineEditName->setPlaceholderText("");
    loadCombox();
    showSNDialog();
}

void GisAddMapNodeDialog::on_pushButtonConfirm_clicked()
{
    if(verifyContent())
    {
        int parentId = 1;
        Node curData;
        if(NULL != m_ptreeView)
        {
            if(m_ptreeView->getCurrentItemData(curData))
            {
                InviewLocalMap map;
                g_SPPClientMapManager->getMapById(curData.id,map);
                if(map.get_local_map_type() == MSG_MAP_TYPE_NODE)
                {
                    parentId = curData.id;
                    emit(signalMapNodeSetFinished(ui->lineEditName->text(),parentId));
                    close();
                }
                else
                {
                    SNMessageBoxFlowDownWidget::showMessage(tr("TK_CannotAddAreaInLocalMap"),this,SNMessageBoxFlowDownWidget::MessageWarring);
                }
            }
        }
    }
}

void GisAddMapNodeDialog::on_pushButtonCancel_clicked()
{
    this->close();
}

void GisAddMapNodeDialog::on_pushButtonClose_clicked()
{
    this->close();
}
